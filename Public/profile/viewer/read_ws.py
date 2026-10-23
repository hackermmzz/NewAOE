# 连接明文 ws。每个二进制帧交给调用方的缓冲，这里不画图。

import base64
import hashlib
import os
import socket
import struct
import threading

WS_PORT = 9765
WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


def parse_ws_target(text):
    raw = (text or "").strip()
    if raw.startswith("wss://"):
        raise ValueError("只支持明文 ws，不支持 wss")
    if raw.startswith("ws://"):
        raw = raw[len("ws://"):]
    raw = raw.split("/", 1)[0]
    if not raw:
        return "127.0.0.1", WS_PORT
    if raw.startswith("["):
        end = raw.find("]")
        if end < 0 or len(raw) <= end + 1 or raw[end + 1] != ":":
            raise ValueError("WebSocket 地址无效")
        return raw[1:end], int(raw[end + 2:])
    if ":" in raw:
        host, port_text = raw.rsplit(":", 1)
        return host or "127.0.0.1", int(port_text)
    return raw, WS_PORT


def encode_server_frame(payload, opcode=0x2):
    header = bytearray([0x80 | opcode])
    length = len(payload)
    if length < 126:
        header.append(length)
    elif length < 65536:
        header.append(126)
        header.extend(struct.pack(">H", length))
    else:
        header.append(127)
        header.extend(struct.pack(">Q", length))
    header.extend(payload)
    return bytes(header)


def encode_client_frame(payload, opcode):
    mask = os.urandom(4)
    masked = bytes(payload[index] ^ mask[index % 4] for index in range(len(payload)))
    length = len(payload)
    header = bytearray([0x80 | opcode, 0x80])
    if length < 126:
        header[1] |= length
    elif length < 65536:
        header[1] |= 126
        header.extend(struct.pack(">H", length))
    else:
        header[1] |= 127
        header.extend(struct.pack(">Q", length))
    header.extend(mask)
    header.extend(masked)
    return bytes(header)


def take_ws_messages(buf, pending=b"", pending_opcode=None):
    """从缓冲区取出完整的服务端帧。返回消息、ping、剩余字节和未结束的分片。"""
    messages = []
    pings = []
    offset = 0
    data = bytes(buf)
    while True:
        if offset + 2 > len(data):
            break
        first = data[offset]
        second = data[offset + 1]
        fin = first & 0x80
        opcode = first & 0x0F
        masked = second & 0x80
        length = second & 0x7F
        pos = offset + 2
        if length == 126:
            if pos + 2 > len(data):
                break
            length = struct.unpack_from(">H", data, pos)[0]
            pos += 2
        elif length == 127:
            if pos + 8 > len(data):
                break
            length = struct.unpack_from(">Q", data, pos)[0]
            pos += 8
        mask = b""
        if masked:
            if pos + 4 > len(data):
                break
            mask = data[pos:pos + 4]
            pos += 4
        if pos + length > len(data):
            break
        payload = data[pos:pos + length]
        if masked:
            payload = bytes(payload[index] ^ mask[index % 4] for index in range(length))
        offset = pos + length
        if opcode == 0x8:
            raise ConnectionError("对端关闭了 WebSocket")
        if opcode == 0x9:
            pings.append(payload)
            continue
        if opcode == 0xA:
            continue
        if opcode in (1, 2):
            pending = payload
            pending_opcode = opcode
        elif opcode == 0 and pending_opcode is not None:
            pending += payload
        else:
            continue
        if fin:
            if pending_opcode == 2:
                messages.append(bytes(pending))
            pending = b""
            pending_opcode = None
    return messages, pings, data[offset:], pending, pending_opcode


class WsClient:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = None
        self.thread = None
        self.error = None
        self.sink = None
        self._stop = False
        self._lock = threading.Lock()
        self._bytes = 0

    def connect(self):
        sock = socket.create_connection((self.host, self.port), 5)
        key = base64.b64encode(os.urandom(16)).decode("ascii")
        request = (
            "GET / HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: %s\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n" % (self.host, self.port, key)
        )
        sock.sendall(request.encode("ascii"))
        response = b""
        while b"\r\n\r\n" not in response:
            part = sock.recv(4096)
            if not part:
                sock.close()
                raise ConnectionError("WebSocket 握手失败")
            response += part
            if len(response) > 65536:
                sock.close()
                raise ConnectionError("WebSocket 握手响应过长")
        status = response.split(b"\r\n", 1)[0]
        if b" 101 " not in status:
            sock.close()
            raise ConnectionError(status.decode("latin1", "replace"))
        accept = base64.b64encode(hashlib.sha1((key + WS_GUID).encode("ascii")).digest())
        if accept not in response:
            sock.close()
            raise ConnectionError("WebSocket Accept 不匹配")
        sock.settimeout(0.5)
        self.sock = sock
        self.thread = threading.Thread(target=self._run, name="profile-ws", daemon=True)
        self.thread.start()

    def snapshot(self):
        with self._lock:
            return self._bytes, self.error

    def close(self):
        self._stop = True
        sock = self.sock
        self.sock = None
        if sock is not None:
            try:
                sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            try:
                sock.close()
            except OSError:
                pass
        thread = self.thread
        if thread is not None and thread is not threading.current_thread():
            thread.join(timeout=1.0)

    def _run(self):
        raw = b""
        pending = b""
        pending_opcode = None
        try:
            while not self._stop:
                try:
                    part = self.sock.recv(65536)
                except socket.timeout:
                    continue
                if not part:
                    raise ConnectionError("连接已关闭")
                raw += part
                messages, pings, raw, pending, pending_opcode = take_ws_messages(
                    raw, pending, pending_opcode)
                for ping in pings:
                    try:
                        self.sock.sendall(encode_client_frame(ping, 0xA))
                    except OSError:
                        raise ConnectionError("连接已关闭")
                if not messages:
                    continue
                sink = self.sink
                if sink is not None:
                    sink(messages)
                with self._lock:
                    self._bytes += sum(len(item) for item in messages)
        except Exception as exc:
            if not self._stop:
                with self._lock:
                    self.error = str(exc)


def read_websocket(host, port, buffer, stop, active, bind_client, unbind_client):
    session = buffer.session()
    client = WsClient(host, port)

    def sink(messages):
        if active():
            buffer.add_chunks(messages, session)

    client.sink = sink
    if not bind_client(client):
        return
    try:
        client.connect()
        while not stop.is_set() and active():
            if client.error:
                buffer.set_error(client.error)
                break
            stop.wait(0.05)
    finally:
        client.close()
        unbind_client(client)
