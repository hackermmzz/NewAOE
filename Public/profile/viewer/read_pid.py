# 按 PID 读取共享内存 aoe_trace_memeory_<pid>。界面传入解析函数和缓冲。

import os
import subprocess

SHM_PREFIX = "aoe_trace_memeory_"
REGION_BYTES = 4 * 1024 * 1024


def shm_name(pid):
    return SHM_PREFIX + str(int(pid))


def read_region_file(path):
    last_error = None
    for _ in range(4):
        try:
            with open(path, "rb") as handle:
                return handle.read()
        except OSError as exc:
            last_error = exc
    raise last_error


def wsl_distros():
    if os.name != "nt":
        return []
    try:
        raw = subprocess.check_output(["wsl.exe", "-l", "-q"], stderr=subprocess.DEVNULL)
    except (OSError, subprocess.CalledProcessError):
        return []
    if raw.startswith(b"\xff\xfe") or (len(raw) >= 4 and raw[1] == 0 and raw[3] == 0):
        text = raw.decode("utf-16le", "replace")
    else:
        text = raw.decode("utf-8", "replace")
    names = []
    for line in text.replace("\x00", "").splitlines():
        name = line.strip().lstrip("\ufeff")
        if name:
            names.append(name)
    return names


def shm_file_candidates(pid):
    name = shm_name(pid)
    if os.name != "nt":
        return ["/dev/shm/" + name]
    paths = []
    for distro in wsl_distros():
        paths.append("\\\\wsl.localhost\\%s\\dev\\shm\\%s" % (distro, name))
        paths.append("\\\\wsl$\\%s\\dev\\shm\\%s" % (distro, name))
    return paths


def read_windows_mapping(name):
    if os.name != "nt":
        return None
    import ctypes
    from ctypes import wintypes
    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    file_map_read = 0x0004
    kernel32.OpenFileMappingW.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.LPCWSTR]
    kernel32.OpenFileMappingW.restype = wintypes.HANDLE
    kernel32.MapViewOfFile.argtypes = [
        wintypes.HANDLE, wintypes.DWORD, wintypes.DWORD, wintypes.DWORD, ctypes.c_size_t,
    ]
    kernel32.MapViewOfFile.restype = ctypes.c_void_p
    kernel32.UnmapViewOfFile.argtypes = [ctypes.c_void_p]
    kernel32.UnmapViewOfFile.restype = wintypes.BOOL
    kernel32.CloseHandle.argtypes = [wintypes.HANDLE]
    kernel32.CloseHandle.restype = wintypes.BOOL
    for tag in (name, "Local\\" + name, "Global\\" + name):
        handle = kernel32.OpenFileMappingW(file_map_read, False, tag)
        if not handle:
            continue
        view = kernel32.MapViewOfFile(handle, file_map_read, 0, 0, REGION_BYTES)
        if not view:
            kernel32.CloseHandle(handle)
            continue
        try:
            return ctypes.string_at(view, REGION_BYTES)
        finally:
            kernel32.UnmapViewOfFile(view)
            kernel32.CloseHandle(handle)
    return None


def read_shared_memory(pid):
    errors = []
    for path in shm_file_candidates(pid):
        if os.path.isfile(path):
            return read_region_file(path)
        errors.append(path)
    mapped = read_windows_mapping(shm_name(pid))
    if mapped is not None:
        return mapped
    raise FileNotFoundError("找不到共享内存 %s。试过: %s" % (shm_name(pid), ", ".join(errors) or "命名映射"))


def stable_read(read_once, attempts=3):
    previous = None
    last = b""
    for _ in range(attempts):
        last = read_once()
        if last == previous:
            return last
        previous = last
    return last


def read_pid(pid, buffer, stop, follow, active, parse):
    session = buffer.session()
    stamp = None
    while not stop.is_set() and active():
        if not follow.is_set() and stamp is not None:
            follow.wait(0.1)
            continue
        try:
            blob = read_shared_memory(pid)
            marker = (len(blob), blob[:8], blob[-16:] if len(blob) >= 16 else blob)
            if marker != stamp:
                stamp = marker
                buffer.replace_parsed(parse(blob, ring=True, allow_incomplete_tail=True), session)
                continue
            buffer.set_error(None)
        except Exception as exc:
            buffer.set_error(str(exc))
        if follow.is_set():
            stop.wait(0.02)
        else:
            follow.wait(0.1)
