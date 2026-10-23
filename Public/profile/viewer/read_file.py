# 持续读取 trace 文件。界面传入缓冲和停止信号，这里不画图。

import os


def read_file(path, buffer, stop, follow, active):
    session = buffer.session()
    pending = b""
    offset = 0
    while not stop.is_set() and active():
        try:
            with open(path, "rb") as handle:
                handle.seek(0, os.SEEK_END)
                end = handle.tell()
                if end < offset:
                    buffer.reset(new_session=False)
                    offset = 0
                    pending = b""
                    handle.seek(0, os.SEEK_END)
                    end = handle.tell()
                if end == offset and not pending:
                    buffer.set_error(None)
                    if not follow.is_set():
                        follow.wait(0.1)
                    else:
                        stop.wait(0.02)
                    continue
                handle.seek(offset)
                fresh = handle.read(256 * 1024)
        except Exception as exc:
            buffer.set_error(str(exc))
            stop.wait(0.2)
            continue
        if not active():
            return
        data = pending + fresh
        consumed = buffer.append_bytes(data, session)
        if consumed > len(data):
            consumed = len(data)
        pending = data[consumed:]
        offset += len(fresh)
        if len(pending) > 2 * 1024 * 1024:
            buffer.set_error("文件尾部解析停住了")
            pending = b""
        if not fresh or len(fresh) < 256 * 1024:
            if follow.is_set():
                stop.wait(0.02)
            else:
                follow.wait(0.1)
