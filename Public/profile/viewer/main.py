# AOE profile 查看器的界面。
# 文件、PID、WebSocket 三种读取分别在 read_file.py、read_pid.py、read_ws.py。
# 读取在单独线程里持续进行，界面线程只取快照并画图。
# 事件密的时候按像素画密度。色块上能放下时画出 label 对应的模块名。
# 实时缓冲到达上限后只丢掉多出来的最旧事件，时间跨度往前滑，不会整段跳回去。
# 运行：python Public/profile/viewer/profile_viewer.py
# 自测：python Public/profile/viewer/profile_viewer.py --self-test

import heapq
import math
import os
import struct
import sys
import threading
import time
from collections import deque

from read_file import read_file
from read_pid import shm_name, read_pid
from read_ws import WS_PORT, encode_server_frame, parse_ws_target, read_websocket, take_ws_messages

MAGIC = 0xA5
TYPE_DYNAMIC = 1
TYPE_LABEL = 2
TYPE_SCOPE = 3
HEADER_DYNAMIC = 30
HEADER_LABEL = 10
STATIC_EVENT_SIZE = 22
CHUNK_HEADER = 12
RING_HEADER = 8
WS_MAX_BYTES = 8 * 1024 * 1024


class Issue:
    def __init__(self, offset, message):
        self.offset = offset
        self.message = message


class Event:
    def __init__(self, kind, start, duration, label_id, event_id, name, truncated, thread, offset):
        self.kind = kind
        self.start = start
        self.duration = duration
        self.end = start + duration
        self.label_id = label_id
        self.event_id = event_id
        self.name = name
        self.truncated = truncated
        self.thread = thread
        self.offset = offset


class EventWindow:
    """List-like append window with amortized O(1) prefix removal.

    Live traces constantly discard their oldest event.  A normal list slice
    copies every remaining item, so keeping a 100k-event window used to copy
    almost 100k references for every newly arrived event.  This sequence keeps
    a logical head and only compacts occasionally.
    """

    COMPACT_HEAD = 32768

    def __init__(self, values=None):
        self._items = list(values) if values is not None else []
        self._head = 0

    def __len__(self):
        return len(self._items) - self._head

    def __iter__(self):
        for index in range(self._head, len(self._items)):
            yield self._items[index]

    def __getitem__(self, key):
        length = len(self)
        if isinstance(key, slice):
            return [self[index] for index in range(*key.indices(length))]
        index = int(key)
        if index < 0:
            index += length
        if index < 0 or index >= length:
            raise IndexError("event index out of range")
        return self._items[self._head + index]

    def append(self, value):
        self._items.append(value)

    def clear(self):
        self._items = []
        self._head = 0

    def sort(self, *args, **kwargs):
        self._compact(force=True)
        self._items.sort(*args, **kwargs)

    def drop_prefix(self, count):
        count = min(max(0, int(count)), len(self))
        if count <= 0:
            return
        if count == len(self):
            self.clear()
            return
        self._head += count
        self._compact(force=False)

    def _compact(self, force=False):
        if self._head <= 0:
            return
        if not force and (
            self._head < self.COMPACT_HEAD or self._head * 2 < len(self._items)
        ):
            return
        self._items = self._items[self._head:]
        self._head = 0


class Trace:
    def __init__(self):
        self.events = EventWindow()
        self.labels = {}
        self.issues = []
        self.chunk_count = 0
        self.byte_count = 0
        self.min_time = 0
        self.max_time = 0


def format_bytes(size):
    if size < 1024:
        return "%d B" % size
    if size < 1024 * 1024:
        return "%.1f KiB" % (size / 1024.0)
    return "%.2f MiB" % (size / (1024.0 * 1024.0))


def format_duration(ns):
    value = float(ns)
    magnitude = abs(value)
    if 0.0 < magnitude < 0.01:
        return "%.2e ns" % value
    if magnitude < 1e3:
        return "%.0f ns" % value if magnitude >= 10 else "%.2f ns" % value
    if magnitude < 1e6:
        return "%.2f µs" % (value / 1e3) if magnitude < 1e4 else "%.1f µs" % (value / 1e3)
    if magnitude < 1e9:
        return "%.2f ms" % (value / 1e6) if magnitude < 1e7 else "%.1f ms" % (value / 1e6)
    if magnitude >= 1e15:
        return "%.2e s" % (value / 1e9)
    return "%.2f s" % (value / 1e9)


def format_thread(thread_id):
    if thread_id is None:
        return "raw buffer"
    return "0x%x" % thread_id


_COLOR_CACHE = {}


def color_for(text):
    cached = _COLOR_CACHE.get(text)
    if cached is not None:
        return cached
    hash_value = 2166136261
    for char in text:
        hash_value ^= ord(char)
        hash_value = (hash_value * 16777619) & 0xFFFFFFFF
    hue = (hash_value % 280) + 40
    cached = _hsl(hue, 0.64, 0.57)
    if len(_COLOR_CACHE) > 4096:
        _COLOR_CACHE.clear()
    _COLOR_CACHE[text] = cached
    return cached


def _hsl(hue, saturation, lightness):
    c = (1.0 - abs(2.0 * lightness - 1.0)) * saturation
    hp = (hue % 360) / 60.0
    x = c * (1.0 - abs(hp % 2.0 - 1.0))
    if hp < 1:
        red, green, blue = c, x, 0.0
    elif hp < 2:
        red, green, blue = x, c, 0.0
    elif hp < 3:
        red, green, blue = 0.0, c, x
    elif hp < 4:
        red, green, blue = 0.0, x, c
    elif hp < 5:
        red, green, blue = x, 0.0, c
    else:
        red, green, blue = c, 0.0, x
    match = lightness - c / 2.0
    return "#%02x%02x%02x" % (
        int((red + match) * 255),
        int((green + match) * 255),
        int((blue + match) * 255),
    )


def _read_string(data, offset, length):
    return bytes(data[offset:offset + length]).decode("utf-8", "replace")


def unwrap_ring(blob):
    if len(blob) < RING_HEADER:
        raise ValueError("共享内存不足 8 字节")
    head, used = struct.unpack_from("<II", blob, 0)
    data = blob[RING_HEADER:]
    capacity = len(data)
    if capacity == 0 or head >= capacity or used > capacity:
        raise ValueError("环形头损坏 head=%s used=%s capacity=%s" % (head, used, capacity))
    if used == 0:
        return b""
    if head + used <= capacity:
        return bytes(data[head:head + used])
    first = capacity - head
    return bytes(data[head:]) + bytes(data[:used - first])


def clip_messages(buf):
    """丢掉尾部凑不齐的一条消息。实时读取时旧 used 会多出这段。"""
    offset = 0
    end = len(buf)
    while offset + CHUNK_HEADER <= end:
        payload = struct.unpack_from("<I", buf, offset + 8)[0]
        total = CHUNK_HEADER + payload
        if total < CHUNK_HEADER or offset + total > end:
            break
        offset += total
    return buf[:offset]


def parse_trace(blob, ring=False, allow_incomplete_tail=False):
    trace = Trace()
    trace.byte_count = len(blob)
    if ring:
        payload = clip_messages(unwrap_ring(blob))
    else:
        payload = blob
    if not payload:
        return trace
    if (not ring and len(payload) >= 2 and payload[0] == MAGIC
            and TYPE_DYNAMIC <= payload[1] <= TYPE_SCOPE):
        trace.chunk_count = 1
        _parse_records(trace, payload, 0, len(payload), 0, None, 0)
    else:
        _parse_chunks(trace, payload, allow_incomplete_tail)
    _finish_trace(trace)
    return trace


def _parse_chunks(trace, payload, allow_incomplete_tail):
    offset = 0
    end = len(payload)
    while offset < end:
        chunk_index = trace.chunk_count
        if end - offset < CHUNK_HEADER:
            if not allow_incomplete_tail:
                trace.issues.append(Issue(offset, "尾部不足 12 字节，无法读取 chunk 头"))
            break
        thread_id, payload_bytes = struct.unpack_from("<QI", payload, offset)
        payload_start = offset + CHUNK_HEADER
        payload_end = payload_start + payload_bytes
        if payload_end > end:
            if not allow_incomplete_tail:
                trace.issues.append(Issue(
                    offset,
                    "chunk #%d 声明 %d 字节，剩余 %d" % (chunk_index, payload_bytes, end - payload_start),
                ))
            break
        trace.chunk_count += 1
        _parse_records(trace, payload, payload_start, payload_end, chunk_index, thread_id, 0, False)
        offset = payload_end
    return offset


def _parse_records(trace, payload, begin, end, chunk_index, thread_id, base_offset, allow_incomplete_tail=False):
    offset = begin
    while offset < end:
        if end - offset < 2:
            if not allow_incomplete_tail:
                trace.issues.append(Issue(base_offset + offset, "chunk #%d 尾部不足 2 字节" % chunk_index))
            break
        magic = payload[offset]
        record_type = payload[offset + 1]
        if magic != MAGIC:
            trace.issues.append(Issue(
                base_offset + offset,
                "chunk #%d 魔数 0x%02x，期望 0x%02x" % (chunk_index, magic, MAGIC),
            ))
            break
        if record_type == TYPE_LABEL:
            if end - offset < HEADER_LABEL:
                if not allow_incomplete_tail:
                    trace.issues.append(Issue(base_offset + offset, "LabelDefinition 不足 %d 字节" % HEADER_LABEL))
                break
            label_id, flags, _reserved, module_size = struct.unpack_from("<IBBH", payload, offset + 2)
            record_size = HEADER_LABEL + module_size
            if offset + record_size > end:
                if not allow_incomplete_tail:
                    trace.issues.append(Issue(base_offset + offset, "LabelDefinition 越过 chunk 边界"))
                break
            name = _read_string(payload, offset + HEADER_LABEL, module_size)
            previous = trace.labels.get(label_id)
            if previous is not None and previous[0] != name:
                trace.issues.append(Issue(base_offset + offset, "label #%d 重复定义且名称不一致" % label_id))
            trace.labels[label_id] = (name, bool(flags & 1), base_offset + offset)
        elif record_type == TYPE_DYNAMIC:
            if end - offset < HEADER_DYNAMIC:
                if not allow_incomplete_tail:
                    trace.issues.append(Issue(base_offset + offset, "DynamicEvent 不足 %d 字节" % HEADER_DYNAMIC))
                break
            flags, _reserved, module_size = struct.unpack_from("<BBH", payload, offset + 2)
            record_size = HEADER_DYNAMIC + module_size
            if offset + record_size > end:
                if not allow_incomplete_tail:
                    trace.issues.append(Issue(base_offset + offset, "DynamicEvent 越过 chunk 边界"))
                break
            event_id, start, duration = struct.unpack_from("<QQQ", payload, offset + 6)
            name = _read_string(payload, offset + HEADER_DYNAMIC, module_size)
            trace.events.append(Event(
                "dynamic", start, duration, 0, event_id, name, bool(flags & 1),
                thread_id, base_offset + offset,
            ))
        elif record_type == TYPE_SCOPE:
            if offset + STATIC_EVENT_SIZE > end:
                if not allow_incomplete_tail:
                    trace.issues.append(Issue(base_offset + offset, "ScopeEvent 不足 %d 字节" % STATIC_EVENT_SIZE))
                break
            start, duration, label_id = struct.unpack_from("<QQI", payload, offset + 2)
            trace.events.append(Event(
                "static", start, duration, label_id, None, "", False,
                thread_id, base_offset + offset,
            ))
            record_size = STATIC_EVENT_SIZE
        else:
            trace.issues.append(Issue(base_offset + offset, "未知 record type %d" % record_type))
            break
        offset += record_size
    return offset


def _finish_trace(trace):
    missing = {}
    for event in trace.events:
        if event.kind != "static":
            continue
        label = trace.labels.get(event.label_id)
        if label is None:
            event.name = "Label #%d" % event.label_id
            missing.setdefault(event.label_id, []).append(event)
        else:
            event.name = label[0]
            event.truncated = label[1]
    for label_id, events in missing.items():
        if label_id in trace.labels:
            continue
        for event in events:
            trace.issues.append(Issue(event.offset, "ScopeEvent 引用了未定义的 label #%d" % label_id))
    trace.events.sort(key=lambda event: (event.start, -event.duration))
    if trace.events:
        trace.min_time = min(event.start for event in trace.events)
        trace.max_time = max(event.end for event in trace.events)


# 时间轴按固定时间片汇总。放大到比时间片更细时再扫原始事件。
SERIES_BUCKETS = 4096
SERIES_QUANTUM = 1000000
MAX_LIVE_EVENTS = 100000
SPARSE_RECT_LIMIT = 500
EVENT_SCAN_LIMIT = 24000
MAX_DRAW_LANES = 12


def lower_bound(events, time_ns):
    lo = 0
    hi = len(events)
    while lo < hi:
        mid = (lo + hi) // 2
        if events[mid].start < time_ns:
            lo = mid + 1
        else:
            hi = mid
    return lo


def upper_bound(events, time_ns):
    lo = 0
    hi = len(events)
    while lo < hi:
        mid = (lo + hi) // 2
        if events[mid].start <= time_ns:
            lo = mid + 1
        else:
            hi = mid
    return lo


def group_threads(events):
    by_thread = {}
    for event in events:
        found = by_thread.get(event.thread)
        if found is None:
            by_thread[event.thread] = [event]
        else:
            found.append(event)
    order = sorted(by_thread, key=lambda item: -1 if item is None else item)
    return order, by_thread


def _slowest(events, limit=100):
    if len(events) <= limit:
        return sorted(events, key=lambda event: event.duration, reverse=True)
    return heapq.nlargest(limit, events, key=lambda event: event.duration)


def extrema_queues(events):
    """Build FIFO monotonic queues for the live window's time bounds."""
    minimum = deque()
    maximum = deque()
    for event in events:
        while minimum and minimum[-1].start >= event.start:
            minimum.pop()
        minimum.append(event)
        while maximum and maximum[-1].end <= event.end:
            maximum.pop()
        maximum.append(event)
    return minimum, maximum


class ThreadSeries:
    """一条线程上按开始时间排好的事件，以及固定宽度的密度桶。"""

    def __init__(self):
        self.events = EventWindow()
        self.anchor = 0
        self.quantum = 0
        self.buckets = []
        self.longs = EventWindow()
        self.buckets_dirty = False
        self.ordered = True

    def add(self, event):
        if self.events and event.start < self.events[-1].start:
            self.ordered = False
        self.events.append(event)
        if not self.buckets:
            self.anchor = event.start
            self.quantum = SERIES_QUANTUM
            self.buckets = [None] * SERIES_BUCKETS
            self.longs = EventWindow()
            self.buckets_dirty = False
        if event.start < self.anchor:
            self._expand()
            return
        end_index = int((event.end - self.anchor) // self.quantum)
        if end_index >= len(self.buckets):
            self._expand()
            return
        self._splat(event, True)

    def _expand(self):
        if not self.events:
            return
        start = self.events[0].start
        end = self.events[0].end
        for event in self.events:
            if event.start < start:
                start = event.start
            if event.end > end:
                end = event.end
        span = end - start
        if span < 1:
            span = 1
        quantum = self.quantum if self.quantum > 0 else SERIES_QUANTUM
        while (span // quantum) + 2 > SERIES_BUCKETS:
            quantum *= 2
        self.anchor = start
        self.quantum = quantum
        self.buckets = [None] * SERIES_BUCKETS
        self.longs = EventWindow()
        self.buckets_dirty = False
        for event in self.events:
            self._splat(event, True)

    def _splat(self, event, record_long):
        quantum = self.quantum
        buckets = self.buckets
        if quantum <= 0 or not buckets:
            return
        first = int((event.start - self.anchor) // quantum)
        last = int((event.end - self.anchor) // quantum)
        if first < 0:
            first = 0
        limit = len(buckets) - 1
        if last > limit:
            last = limit
        if last - first > 512:
            if record_long:
                self.longs.append(event)
            last = first
        color = color_for(event.name or "?")
        duration = event.duration
        for index in range(first, last + 1):
            current = buckets[index]
            if current is None or duration >= current[0]:
                buckets[index] = (duration, color, event)

    def resplat(self, event):
        self._splat(event, False)

    def drop_prefix(self, count):
        """丢掉最前面的 count 条。替换列表，不改界面线程可能还拿着的旧列表。"""
        if count <= 0 or not self.events:
            return
        if count >= len(self.events):
            self.events.clear()
            self.buckets = []
            self.longs.clear()
            self.buckets_dirty = False
            self.anchor = 0
            self.quantum = 0
            return
        removed = self.events[:count]
        self.events.drop_prefix(count)
        gone = {id(event) for event in removed}
        if self.longs:
            long_count = 0
            while long_count < len(self.longs) and id(self.longs[long_count]) in gone:
                long_count += 1
            self.longs.drop_prefix(long_count)
        # Buckets are an optional coarse cache.  Rebuilding them here made a
        # one-event eviction scan/re-splat the entire 100k-event window.  Mark
        # them stale and rebuild only if a bucket consumer actually asks.
        self.buckets_dirty = True


def series_from_events(events):
    series = ThreadSeries()
    if not events:
        return series
    series.events = EventWindow(events)
    previous = events[0].start
    for event in events:
        if event.start < previous:
            series.ordered = False
            break
        previous = event.start
    series.anchor = events[0].start
    series.quantum = SERIES_QUANTUM
    series.buckets = [None] * SERIES_BUCKETS
    series._expand()
    return series


def _paint_column(colors, owners, best, columns, origin, span, scale, start, end, duration, color, event):
    view_end = origin + span
    if end < origin or start > view_end:
        return
    left = start if start > origin else origin
    right = end if end < view_end else view_end
    if right <= left:
        return
    x0 = int((left - origin) * scale)
    x1 = int((right - origin) * scale) + 1
    if x0 < 0:
        x0 = 0
    if x1 > columns:
        x1 = columns
    if x1 <= x0:
        x1 = min(columns, x0 + 1)
    for x in range(x0, x1):
        if duration >= best[x]:
            best[x] = duration
            colors[x] = color
            owners[x] = event


def columns_from_series(series, origin, span, columns):
    if series.buckets_dirty:
        series._expand()
    colors = [None] * columns
    owners = [None] * columns
    if columns <= 0 or span <= 0 or series.quantum <= 0 or not series.buckets:
        return colors, owners
    best = [-1] * columns
    scale = columns / float(span)
    quantum = series.quantum
    anchor = series.anchor
    buckets = series.buckets
    view_end = origin + span
    first = int((origin - anchor) // quantum)
    last = int((view_end - anchor) // quantum)
    if first < 0:
        first = 0
    limit = len(buckets) - 1
    if last > limit:
        last = limit
    for index in range(first, last + 1):
        item = buckets[index]
        if item is None:
            continue
        duration, color, event = item
        _paint_column(
            colors, owners, best, columns, origin, span, scale,
            anchor + index * quantum, anchor + (index + 1) * quantum,
            duration, color, event,
        )
    for event in series.longs:
        _paint_column(
            colors, owners, best, columns, origin, span, scale,
            event.start, event.end, event.duration, color_for(event.name or "?"), event,
        )
    return colors, owners


def module_name(event):
    if event is None:
        return ""
    name = event.name or ""
    if event.kind == "static" and (not name or name.startswith("Label #")):
        return ""
    return name


def fit_text(measure, text, width):
    if not text or width < 24:
        return ""
    limit = width - 6
    if measure(text) <= limit:
        return text
    lo = 0
    hi = len(text)
    best = ""
    while lo <= hi:
        mid = (lo + hi) // 2
        candidate = text[:mid] + "…"
        if measure(candidate) <= limit:
            best = candidate
            lo = mid + 1
        else:
            hi = mid - 1
    return best


def name_spans(owners, min_px):
    spans = []
    index = 0
    count = len(owners)
    while index < count:
        name = module_name(owners[index])
        if not name:
            index += 1
            continue
        end = index + 1
        while end < count and module_name(owners[end]) == name:
            end += 1
        if end - index >= min_px:
            spans.append((index, end, name))
        index = end
    return spans


def visible_lanes(series, origin, span, predicate=None, scan_limit=EVENT_SCAN_LIMIT,
                  max_lanes=MAX_DRAW_LANES):
    """Build non-overlapping visible lanes without creating one Tk item per event.

    Normal lanes are interval-disjoint, so painting them into a pixel row is
    O(events + pixels), rather than O(events * pixels).  Very deep nesting is
    folded into the final overflow lane.
    """
    events = series.events
    if not events:
        return [], 0
    view_end = origin + span
    lo = lower_bound(events, origin - span)
    hi = upper_bound(events, view_end)
    source_count = max(0, hi - lo)
    if source_count <= 0:
        return [], 0
    scan_limit = max(1, int(scan_limit))
    visible = []
    if source_count <= scan_limit:
        indices = range(lo, hi)
    else:
        # Sample short consecutive bursts instead of every Nth item. Periodic
        # profiler records often repeat in groups; fixed stepping aliases those
        # groups and can leave only "main" or "WorkerThread" visible.
        burst = min(max_lanes, scan_limit)
        groups = max(1, scan_limit // burst)
        stride = (source_count + groups - 1) // groups
        indices = (
            index
            for group_start in range(lo, hi, stride)
            for index in range(group_start, min(group_start + burst, hi))
        )
    for index in indices:
        event = events[index]
        if event.end < origin or event.start > view_end:
            continue
        if predicate is not None and not predicate(event):
            continue
        visible.append(event)
    # Long scopes may begin more than one view-span before the current origin.
    if series.longs:
        present = {id(event) for event in visible}
        for event in series.longs:
            if id(event) in present or event.end < origin or event.start > view_end:
                continue
            if predicate is not None and not predicate(event):
                continue
            visible.append(event)
    visible.sort(key=lambda event: (event.start, -event.duration))
    lane_ends = []
    lanes = []
    for event in visible:
        lane = -1
        for index, lane_end in enumerate(lane_ends):
            if event.start >= lane_end:
                lane = index
                break
        if lane < 0:
            if len(lanes) < max_lanes:
                lane = len(lanes)
                lanes.append([])
                lane_ends.append(event.end)
            else:
                lane = max_lanes - 1
                if event.end > lane_ends[lane]:
                    lane_ends[lane] = event.end
        else:
            lane_ends[lane] = event.end
        lanes[lane].append(event)
    return lanes, source_count


def columns_from_lane(events, origin, span, columns):
    """Rasterize one lane with a sweep line, bounded for overlapping overflow."""
    colors = [None] * columns
    owners = [None] * columns
    if not events or columns <= 0 or span <= 0:
        return colors, owners
    active = []
    event_index = 0
    for column in range(columns):
        pixel_start = origin + column * span / columns
        pixel_end = origin + (column + 1) * span / columns
        while event_index < len(events) and events[event_index].start <= pixel_end:
            event = events[event_index]
            heapq.heappush(active, (-event.duration, event_index, event))
            event_index += 1
        while active and active[0][2].end < pixel_start:
            heapq.heappop(active)
        if active:
            event = active[0][2]
            owners[column] = event
            colors[column] = color_for(event.name or "?")
    return colors, owners


def _apply_saved_labels(trace, saved):
    for label_id, value in saved.items():
        trace.labels.setdefault(label_id, value)
    for event in trace.events:
        if event.kind != "static":
            continue
        label = trace.labels.get(event.label_id)
        if label is None:
            if not event.name:
                event.name = "Label #%d" % event.label_id
            continue
        event.name = label[0]
        event.truncated = label[1]


def columns_from_events(events, origin, span, columns, predicate):
    colors = [None] * columns
    owners = [None] * columns
    if columns <= 0 or span <= 0 or not events:
        return colors, owners
    view_end = origin + span
    lo = lower_bound(events, origin - span)
    hi = upper_bound(events, view_end)
    count = hi - lo
    if count <= 0:
        return colors, owners
    step = 1 if count <= EVENT_SCAN_LIMIT else max(1, count // EVENT_SCAN_LIMIT)
    best = [-1] * columns
    scale = columns / float(span)
    index = lo
    while index < hi:
        event = events[index]
        index += step
        if event.end < origin or event.start > view_end:
            continue
        if predicate is not None and not predicate(event):
            continue
        _paint_column(
            colors, owners, best, columns, origin, span, scale,
            event.start, event.end, event.duration, color_for(event.name or "?"), event,
        )
    return colors, owners


def buckets_are_fine(series, span, columns):
    if series is None or series.quantum <= 0 or columns <= 0 or span <= 0:
        return False
    return series.quantum <= (span / float(columns)) * 4.0


class LiveBuffer:
    """后台线程往这里追加 chunk。界面线程只取一次快照，不再整段重解析。"""

    def __init__(self):
        self._lock = threading.Lock()
        self.trace = Trace()
        self.order = []
        self.series = {}
        self.byte_count = 0
        self.generation = 0
        self.error = None
        self.slow = []
        self._waiting = {}
        self._segments = deque()
        self._slow_at = 0.0
        self._timed = False
        self._ordered = True
        self._last_start = None
        self._format = None
        self._minimum, self._maximum = extrema_queues(())
        self._session = 1

    def session(self):
        with self._lock:
            return self._session

    def reset(self, new_session=True):
        with self._lock:
            if new_session:
                self._session += 1
            self.trace = Trace()
            self.order = []
            self.series = {}
            self.byte_count = 0
            self.error = None
            self.slow = []
            self._waiting = {}
            self._segments.clear()
            self._timed = False
            self._ordered = True
            self._last_start = None
            self._slow_at = 0.0
            self._format = None
            self._minimum, self._maximum = extrema_queues(())
            self.generation += 1

    def set_error(self, message):
        with self._lock:
            self.error = message

    def _active(self, session):
        return session is None or session == self._session

    def peek(self):
        with self._lock:
            return (
                self.generation,
                self.trace,
                tuple(self.order),
                self.series,
                self.byte_count,
                self.error,
                self.slow,
                len(self.trace.issues),
            )

    def replace_parsed(self, trace, session=None):
        with self._lock:
            if not self._active(session):
                return
            saved = dict(self.trace.labels)
        _apply_saved_labels(trace, saved)
        order, grouped = group_threads(trace.events)
        series = {thread: series_from_events(events) for thread, events in grouped.items()}
        slow = _slowest(trace.events)
        minimum, maximum = extrema_queues(trace.events)
        with self._lock:
            if not self._active(session):
                return
            self.trace = trace
            self.order = order
            self.series = series
            self.byte_count = trace.byte_count
            self.error = None
            self.slow = slow
            self._waiting = {}
            self._segments.clear()
            self._timed = bool(trace.events)
            self._ordered = True
            self._last_start = trace.events[-1].start if trace.events else None
            self._minimum = minimum
            self._maximum = maximum
            self._slow_at = time.monotonic()
            self.generation += 1

    def add_chunks(self, chunks, session=None):
        need_events = None
        with self._lock:
            if not self._active(session):
                return
            for chunk in chunks:
                before = len(self.trace.events)
                _parse_chunks(self.trace, chunk, False)
                added = self.trace.events[before:]
                for event in added:
                    self._take_event(event)
                self._flush_labels()
                self.byte_count += len(chunk)
                self._segments.append((len(chunk), len(added)))
            self._trim()
            self.generation += 1
            now = time.monotonic()
            if now - self._slow_at >= 1.0:
                need_events = list(self.trace.events)
                self._slow_at = now
        if need_events is not None:
            slow = _slowest(need_events)
            with self._lock:
                self.slow = slow

    def append_bytes(self, blob, session=None):
        """解析文件新读到的字节。返回消费掉的长度，尾部不完整的留在调用方。"""
        if not blob:
            return 0
        need_events = None
        with self._lock:
            if not self._active(session):
                return 0
            if self._format is None:
                if len(blob) < 2:
                    return 0
                if blob[0] == MAGIC and TYPE_DYNAMIC <= blob[1] <= TYPE_SCOPE:
                    self._format = "raw"
                elif len(blob) < CHUNK_HEADER:
                    return 0
                else:
                    self._format = "chunk"
            before_events = len(self.trace.events)
            before_labels = len(self.trace.labels)
            if self._format == "raw":
                consumed = _parse_records(self.trace, blob, 0, len(blob), 0, None, 0, True)
                if consumed > 0:
                    self.trace.chunk_count = max(self.trace.chunk_count, 1)
            else:
                consumed = _parse_chunks(self.trace, blob, True)
            added = self.trace.events[before_events:]
            for event in added:
                self._take_event(event)
            self._flush_labels()
            if consumed > 0:
                self.byte_count += consumed
                self._segments.append((consumed, len(added)))
                self._trim()
            if added or len(self.trace.labels) != before_labels:
                self.generation += 1
                now = time.monotonic()
                if now - self._slow_at >= 1.0 or not self.slow:
                    need_events = list(self.trace.events)
                    self._slow_at = now
        if need_events is not None:
            slow = _slowest(need_events)
            with self._lock:
                self.slow = slow
        return consumed

    def _take_event(self, event):
        if event.kind == "static":
            label = self.trace.labels.get(event.label_id)
            if label is None:
                event.name = "Label #%d" % event.label_id
                self._waiting.setdefault(event.label_id, []).append(event)
            else:
                event.name = label[0]
                event.truncated = label[1]
        series = self.series.get(event.thread)
        if series is None:
            series = ThreadSeries()
            self.series[event.thread] = series
            self.order.append(event.thread)
            self.order.sort(key=lambda item: -1 if item is None else item)
        series.add(event)
        if self._last_start is not None and event.start < self._last_start:
            self._ordered = False
        self._last_start = event.start
        while self._minimum and self._minimum[-1].start >= event.start:
            self._minimum.pop()
        self._minimum.append(event)
        while self._maximum and self._maximum[-1].end <= event.end:
            self._maximum.pop()
        self._maximum.append(event)
        if not self._timed:
            self._timed = True
            self.trace.min_time = event.start
            self.trace.max_time = event.end
            return
        if event.start < self.trace.min_time:
            self.trace.min_time = event.start
        if event.end > self.trace.max_time:
            self.trace.max_time = event.end

    def _flush_labels(self):
        if not self._waiting:
            return
        for label_id in list(self._waiting):
            label = self.trace.labels.get(label_id)
            if label is None:
                continue
            for event in self._waiting.pop(label_id):
                event.name = label[0]
                event.truncated = label[1]
                series = self.series.get(event.thread)
                if series is not None:
                    series.resplat(event)

    def _trim(self):
        # 只丢掉超出上限的部分。一次削到 80% 会让跨度从大约 6 秒跳回 5 秒。
        if len(self.trace.events) <= MAX_LIVE_EVENTS and self.byte_count <= WS_MAX_BYTES:
            return
        drop = 0
        events_left = len(self.trace.events)
        while self._segments and (
            events_left > MAX_LIVE_EVENTS or self.byte_count > WS_MAX_BYTES
        ):
            nbytes, nevents = self._segments[0]
            need = events_left - MAX_LIVE_EVENTS
            if need < 0:
                need = 0
            if self.byte_count > WS_MAX_BYTES and nbytes > 0 and nevents > 0:
                excess = self.byte_count - WS_MAX_BYTES
                by_bytes = (excess * nevents + nbytes - 1) // nbytes
                if by_bytes > need:
                    need = by_bytes
            if nevents <= 0:
                self._segments.popleft()
                self.byte_count -= nbytes
                if self.byte_count < 0:
                    self.byte_count = 0
                continue
            if need <= 0:
                break
            take = nevents if need >= nevents else need
            if take >= nevents:
                self._segments.popleft()
                self.byte_count -= nbytes
            else:
                drop_bytes = int(nbytes * take / nevents)
                if nbytes > 0 and drop_bytes <= 0:
                    drop_bytes = 1
                if drop_bytes > nbytes:
                    drop_bytes = nbytes
                self._segments[0] = (nbytes - drop_bytes, nevents - take)
                self.byte_count -= drop_bytes
            drop += take
            events_left -= take
            if self.byte_count < 0:
                self.byte_count = 0
        if self.byte_count < 0:
            self.byte_count = 0
        if drop <= 0:
            return
        if drop > len(self.trace.events):
            drop = len(self.trace.events)
        prefix = self.trace.events[:drop]
        self._forget_waiting(prefix)
        counts = {}
        for event in prefix:
            counts[event.thread] = counts.get(event.thread, 0) + 1
        gone = {id(event) for event in prefix}
        while self._minimum and id(self._minimum[0]) in gone:
            self._minimum.popleft()
        while self._maximum and id(self._maximum[0]) in gone:
            self._maximum.popleft()
        self.trace.events.drop_prefix(drop)
        for thread, count in counts.items():
            series = self.series.get(thread)
            if series is None:
                continue
            series.drop_prefix(count)
            if not series.events:
                self.series.pop(thread, None)
        if not self.trace.events:
            self.order = []
            self.series = {}
            self._waiting = {}
            self._timed = False
            self._ordered = True
            self._last_start = None
            self._minimum.clear()
            self._maximum.clear()
            self.trace.min_time = 0
            self.trace.max_time = 0
            return
        self.order = [thread for thread in self.order if thread in self.series]
        if not self._minimum or not self._maximum:
            self._minimum, self._maximum = extrema_queues(self.trace.events)
        self.trace.min_time = self._minimum[0].start
        self.trace.max_time = self._maximum[0].end
        self._timed = True

    def _forget_waiting(self, removed):
        if not self._waiting or not removed:
            return
        gone = {id(event) for event in removed}
        for label_id in list(self._waiting):
            kept = [event for event in self._waiting[label_id] if id(event) not in gone]
            if kept:
                self._waiting[label_id] = kept
            else:
                del self._waiting[label_id]


def pack_label(label_id, name, flags=0):
    raw = name.encode("utf-8")
    return struct.pack("<BBIBBH", MAGIC, TYPE_LABEL, label_id, flags, 0, len(raw)) + raw


def pack_scope(start, duration, label_id):
    return struct.pack("<BBQQI", MAGIC, TYPE_SCOPE, start, duration, label_id)


def pack_dynamic(name, event_id, start, duration, flags=0):
    raw = name.encode("utf-8")
    return struct.pack("<BBBBHQQQ", MAGIC, TYPE_DYNAMIC, flags, 0, len(raw), event_id, start, duration) + raw


def pack_chunk(thread_id, records):
    return struct.pack("<QI", thread_id, len(records)) + records


def build_ring(messages, capacity, head):
    data = bytearray(capacity)
    used = 0
    cursor = head % capacity
    for message in messages:
        if used + len(message) > capacity:
            raise ValueError("sample does not fit")
        for byte in message:
            data[cursor] = byte
            cursor = (cursor + 1) % capacity
        used += len(message)
    return struct.pack("<II", head % capacity, used) + bytes(data)


def self_test():
    window = EventWindow(range(100000))
    for value in range(100000, 210000):
        window.append(value)
        window.drop_prefix(1)
    assert len(window) == 100000
    assert window[0] == 110000
    assert window[-1] == 209999
    assert window[:3] == [110000, 110001, 110002]

    records = pack_label(7, "serializer.write") + pack_scope(1000, 250, 7)
    chunk = pack_chunk(0x11, records)
    file_trace = parse_trace(chunk, ring=False)
    assert file_trace.chunk_count == 1
    assert len(file_trace.events) == 1
    assert file_trace.events[0].name == "serializer.write"
    assert file_trace.events[0].duration == 250
    assert file_trace.events[0].thread == 0x11
    assert file_trace.events[0].label_id == 7
    assert not file_trace.issues

    dynamic = pack_chunk(0x22, pack_dynamic("receive packet", 9, 50, 80))
    dynamic_trace = parse_trace(dynamic, ring=False)
    assert dynamic_trace.events[0].name == "receive packet"
    assert dynamic_trace.events[0].event_id == 9
    assert dynamic_trace.events[0].kind == "dynamic"

    capacity = 64
    head = 50
    ring = build_ring([chunk], capacity, head)
    assert len(ring) == RING_HEADER + capacity
    ring_trace = parse_trace(ring, ring=True)
    assert ring_trace.events[0].name == "serializer.write"
    assert ring_trace.events[0].start == 1000

    older = pack_chunk(1, pack_dynamic("old", 1, 1, 1))
    newer = pack_chunk(2, pack_dynamic("new", 2, 5, 3))
    popped = build_ring([newer], capacity, (head + len(older)) % capacity)
    popped_trace = parse_trace(popped, ring=True)
    assert len(popped_trace.events) == 1
    assert popped_trace.events[0].name == "new"
    assert "old" not in [event.name for event in popped_trace.events]

    raw = pack_scope(10, 4, 3)
    raw_trace = parse_trace(raw, ring=False)
    assert raw_trace.events[0].thread is None
    assert any("未定义" in issue.message for issue in raw_trace.issues)

    torn = chunk + b"\x00\x01"
    clipped = clip_messages(torn)
    assert clipped == chunk
    assert shm_name(42) == "aoe_trace_memeory_42"

    frame = encode_server_frame(chunk)
    messages, _pings, rest, pending, pending_opcode = take_ws_messages(frame[:5])
    assert messages == []
    messages, _pings, rest, pending, pending_opcode = take_ws_messages(
        rest + frame[5:], pending, pending_opcode)
    assert messages == [chunk]
    assert rest == b""
    ws_trace = parse_trace(b"".join(messages), ring=False)
    assert ws_trace.events[0].name == "serializer.write"
    later = LiveBuffer()
    later.add_chunks([
        pack_chunk(1, pack_scope(10, 5, 3)),
        pack_chunk(1, pack_label(3, "later")),
    ])
    later_trace = later.peek()[1]
    assert later_trace.events[0].name == "later"
    assert later_trace.events[0].thread == 1

    burst = []
    for index in range(32):
        burst.append(pack_chunk(index % 2, pack_dynamic("n%d" % index, index, index * 1000, 50)))
    live = LiveBuffer()
    live.add_chunks(burst)
    live_gen, live_trace, live_order, live_series, _bytes, _error, _slow, _issues = live.peek()
    assert live_gen == 1
    assert len(live_trace.events) == 32
    assert live_trace.events[0].name == "n0"
    assert live_order == (0, 1)
    assert len(live_series[0].events) == 16
    origin = live_trace.min_time
    span = max(1, live_trace.max_time - origin)
    colors, owners = columns_from_series(live_series[0], origin, span, 64)
    assert any(item is not None for item in owners)
    assert any(item is not None for item in colors)

    named = pack_chunk(4, pack_label(9, "Game.tick") + pack_scope(10, 20, 9))
    streamed = LiveBuffer()
    assert streamed.append_bytes(named[:10]) == 0
    assert streamed.append_bytes(named[:10] + named[10:]) == len(named)
    assert streamed.peek()[1].events[0].name == "Game.tick"

    kept = LiveBuffer()
    kept.replace_parsed(parse_trace(pack_chunk(1, pack_label(4, "Game.tick") + pack_scope(1, 2, 4))))
    kept.replace_parsed(parse_trace(pack_chunk(1, pack_scope(5, 6, 4))))
    assert kept.peek()[1].events[0].name == "Game.tick"
    assert 4 in kept.peek()[1].labels

    class _Named:
        def __init__(self, name, kind="static"):
            self.name = name
            self.kind = kind

    assert name_spans([_Named("A"), _Named("A"), _Named("B"), None, _Named("A")], 2) == [(0, 2, "A")]
    assert fit_text(lambda text: len(text) * 7, "Game.tick", 40).endswith("…")
    assert fit_text(lambda text: len(text) * 7, "Game.tick", 200) == "Game.tick"

    # 越过事件上限时跨度应保持不动。旧逻辑一次削到 80%，6 秒会跳成大约 5 秒。
    step = 1000000
    batch = 2000

    def emit_scopes(start_index, count):
        parts = []
        for offset in range(count):
            parts.append(pack_scope((start_index + offset) * step, 1000, 1))
        return pack_chunk(1, b"".join(parts))

    sliding = LiveBuffer()
    cursor = 0
    while cursor < MAX_LIVE_EVENTS:
        count = batch if cursor + batch <= MAX_LIVE_EVENTS else MAX_LIVE_EVENTS - cursor
        sliding.add_chunks([emit_scopes(cursor, count)])
        cursor += count
    filled = sliding.peek()[1]
    span_full = filled.max_time - filled.min_time
    assert len(filled.events) == MAX_LIVE_EVENTS
    assert sliding._ordered
    assert filled.min_time == 0
    assert span_full == (MAX_LIVE_EVENTS - 1) * step + 1000
    sliding.add_chunks([emit_scopes(cursor, batch)])
    cursor += batch
    slid = sliding.peek()
    slid_trace = slid[1]
    span_slid = slid_trace.max_time - slid_trace.min_time
    assert len(slid_trace.events) == MAX_LIVE_EVENTS
    assert span_slid == span_full
    assert slid_trace.min_time == batch * step
    assert len(slid[3][1].events) == MAX_LIVE_EVENTS
    assert slid[3][1].events[0].start == slid_trace.min_time
    _colors, owners = columns_from_series(slid[3][1], slid_trace.min_time, max(1, span_slid), 64)
    assert any(owner is not None for owner in owners)
    sliding.add_chunks([emit_scopes(cursor, 500)])
    partial = sliding.peek()[1]
    assert len(partial.events) == MAX_LIVE_EVENTS
    assert partial.max_time - partial.min_time == span_full
    assert partial.min_time == (batch + 500) * step

    assert parse_ws_target("127.0.0.1:9765") == ("127.0.0.1", 9765)
    assert parse_ws_target("ws://127.0.0.1:9765/profile") == ("127.0.0.1", 9765)
    try:
        parse_ws_target("wss://127.0.0.1:9765")
        raise AssertionError("wss should be rejected")
    except ValueError:
        pass
    print("profile_viewer self-test ok")


class Viewer:
    def __init__(self, root):
        import tkinter as tk
        from tkinter import filedialog, ttk
        self.tk = tk
        self.filedialog = filedialog
        self.ttk = ttk
        self.root = root
        self.trace = Trace()
        self.filtered = []
        self.zoom = 1.0
        self.offset = 0
        self.source = None
        self.selected = None
        self.hits = []
        self.density_rows = []
        self.drag = None
        self.suppress_click = False
        self.after_id = None
        self.draw_job = None
        self.ws_client = None
        self.buffer = LiveBuffer()
        self.series = {}
        self.order = []
        self.seen_gen = -1
        self.slow_cache = []
        self.slow_item_events = {}
        self.byte_count = 0
        self.photo = None
        self.issue_stamp = None
        self.slow_stamp = None
        self.thread_ids = None
        self.live_stop = None
        self.live_thread = None
        self.live_kind = None
        self.follow = threading.Event()
        self.reader_epoch = 0
        self.reader_lock = threading.Lock()
        self.shown_name = None
        self.name_font = None
        self._build()

    def _build(self):
        tk = self.tk
        root = self.root
        root.title("AOE Profile Viewer")
        root.geometry("1380x880")
        root.minsize(980, 680)
        root.configure(bg="#08100f")
        import tkinter.font as tkfont
        self.name_font = tkfont.Font(root=root, family="Consolas", size=9)
        self.bg = "#08100f"
        self.panel = "#101918"
        self.line = "#2a3a36"
        self.text = "#e9f2ef"
        self.muted = "#8ca39d"
        self.accent = "#5de4c7"

        style = self.ttk.Style(root)
        try:
            style.theme_use("clam")
        except tk.TclError:
            pass
        style.configure(
            "Profile.Treeview", background="#0b1413", fieldbackground="#0b1413",
            foreground=self.text, borderwidth=0, relief="flat", rowheight=27,
            font=("Microsoft YaHei UI", 9),
        )
        style.configure(
            "Profile.Treeview.Heading", background="#162220", foreground=self.muted,
            borderwidth=0, relief="flat", padding=(9, 7),
            font=("Microsoft YaHei UI", 9, "bold"),
        )
        style.map(
            "Profile.Treeview", background=[("selected", "#21463e")],
            foreground=[("selected", "#f2fffb")],
        )
        style.configure(
            "Profile.TCombobox", fieldbackground="#0b1413", background="#162220",
            foreground=self.text, arrowcolor=self.muted, bordercolor=self.line,
            lightcolor=self.line, darkcolor=self.line, padding=(8, 4),
        )
        style.map(
            "Profile.TCombobox",
            fieldbackground=[("readonly", "#0b1413")],
            foreground=[("readonly", self.text)],
            selectbackground=[("readonly", "#0b1413")],
            selectforeground=[("readonly", self.text)],
        )

        top = tk.Frame(root, bg=self.bg)
        top.pack(fill="x", padx=16, pady=(14, 8))
        tk.Label(top, text="AOE", fg=self.accent, bg=self.bg,
                 font=("Consolas", 16, "bold")).pack(side="left")
        tk.Label(top, text="  Profile Viewer", fg=self.text, bg=self.bg,
                 font=("Microsoft YaHei UI", 16, "bold")).pack(side="left")
        tk.Label(top, text="   magic 0xA5 · 文件、共享内存，或 ws://127.0.0.1:9765",
                 fg=self.muted, bg=self.bg, font=("Microsoft YaHei UI", 10)).pack(side="left")

        source = tk.Frame(root, bg=self.panel, highlightbackground=self.line, highlightthickness=1)
        source.pack(fill="x", padx=16, pady=4)
        self.mode = tk.StringVar(value="file")
        tk.Radiobutton(source, text="文件", variable=self.mode, value="file",
                       command=self._sync_mode, bg=self.panel, fg=self.text,
                       selectcolor="#0b1413", activebackground=self.panel,
                       activeforeground=self.text).pack(side="left", padx=(10, 4), pady=8)
        tk.Radiobutton(source, text="PID", variable=self.mode, value="pid",
                       command=self._sync_mode, bg=self.panel, fg=self.text,
                       selectcolor="#0b1413", activebackground=self.panel,
                       activeforeground=self.text).pack(side="left", padx=4, pady=8)
        tk.Radiobutton(source, text="WebSocket", variable=self.mode, value="ws",
                       command=self._sync_mode, bg=self.panel, fg=self.text,
                       selectcolor="#0b1413", activebackground=self.panel,
                       activeforeground=self.text).pack(side="left", padx=4, pady=8)
        self.path_var = tk.StringVar()
        self.path_entry = tk.Entry(source, textvariable=self.path_var, bg="#0b1413", fg=self.text,
                                    insertbackground=self.text, relief="flat", width=48)
        self.path_entry.pack(side="left", padx=6, ipady=4)
        self.browse_button = tk.Button(source, text="浏览", command=self.choose_file,
                                        bg="#162220", fg=self.text, relief="flat")
        self.browse_button.pack(side="left", padx=4)
        self.load_button = tk.Button(source, text="读取", command=self.load_current,
                                      bg=self.accent, fg="#06201a", relief="flat")
        self.load_button.pack(side="left", padx=4)
        self.live_var = tk.BooleanVar(value=False)
        tk.Checkbutton(source, text="实时", variable=self.live_var, command=self._toggle_live,
                       bg=self.panel, fg=self.text, selectcolor="#0b1413",
                       activebackground=self.panel, activeforeground=self.text).pack(side="left", padx=8)
        self.source_label = tk.Label(source, text="未读取", fg=self.muted, bg=self.panel,
                                      font=("Consolas", 10))
        self.source_label.pack(side="left", padx=8)

        stats = tk.Frame(root, bg=self.bg)
        stats.pack(fill="x", padx=16, pady=6)
        self.stat_labels = {}
        for key, title in (
            ("events", "事件"),
            ("threads", "线程"),
            ("span", "跨度"),
            ("labels", "标签"),
            ("issues", "诊断"),
        ):
            box = tk.Frame(stats, bg=self.panel, highlightbackground=self.line, highlightthickness=1)
            box.pack(side="left", fill="x", expand=True, padx=3)
            tk.Label(box, text=title, fg=self.muted, bg=self.panel,
                     font=("Microsoft YaHei UI", 9)).pack(anchor="w", padx=10, pady=(6, 0))
            label = tk.Label(box, text="0", fg=self.text, bg=self.panel, font=("Consolas", 14))
            label.pack(anchor="w", padx=10, pady=(0, 6))
            self.stat_labels[key] = label

        tools = tk.Frame(root, bg=self.panel, highlightbackground=self.line, highlightthickness=1)
        tools.pack(fill="x", padx=16, pady=6)
        tk.Label(tools, text="Label 筛选", fg=self.muted, bg=self.panel).pack(side="left", padx=(12, 0))
        self.search_var = tk.StringVar()
        self.search_var.trace_add("write", lambda *_: self.apply_filter())
        tk.Entry(tools, textvariable=self.search_var, bg="#0b1413", fg=self.text,
                 insertbackground=self.text, relief="flat", width=24).pack(side="left", padx=6, pady=9, ipady=5)
        tk.Label(tools, text="线程", fg=self.muted, bg=self.panel).pack(side="left", padx=(8, 4))
        self.thread_var = tk.StringVar(value="全部线程")
        self.thread_menu = self.ttk.Combobox(
            tools, textvariable=self.thread_var, values=("全部线程",),
            state="readonly", width=20, style="Profile.TCombobox",
        )
        self.thread_menu.pack(side="left", ipady=3)
        self.thread_var.trace_add("write", lambda *_: self.apply_filter())
        tk.Label(tools, text="最短 µs", fg=self.muted, bg=self.panel).pack(side="left", padx=(10, 4))
        self.min_var = tk.StringVar(value="0")
        self.min_var.trace_add("write", lambda *_: self.apply_filter())
        tk.Entry(tools, textvariable=self.min_var, bg="#0b1413", fg=self.text,
                 insertbackground=self.text, relief="flat", width=8).pack(side="left", ipady=3)
        tk.Button(tools, text="－", command=lambda: self.set_zoom(self.zoom / 2.0),
                  bg="#162220", fg=self.text, relief="flat", width=3).pack(side="left", padx=(12, 2))
        self.zoom_label = tk.Label(tools, text="1.0×", fg=self.accent, bg=self.panel, font=("Consolas", 11, "bold"))
        self.zoom_label.pack(side="left", padx=4)
        tk.Button(tools, text="＋", command=lambda: self.set_zoom(self.zoom * 2.0),
                  bg="#162220", fg=self.text, relief="flat", width=3).pack(side="left", padx=2)
        tk.Button(tools, text="重置视图", command=self.reset_view,
                  bg="#162220", fg=self.text, relief="flat").pack(side="left", padx=8)

        body = tk.Frame(root, bg=self.bg)
        body.pack(fill="both", expand=True, padx=16, pady=6)
        left = tk.Frame(body, bg=self.panel, highlightbackground=self.line, highlightthickness=1)
        left.pack(side="left", fill="both", expand=True)
        timeline_head = tk.Frame(left, bg=self.panel)
        timeline_head.pack(fill="x", padx=12, pady=(9, 7))
        tk.Label(timeline_head, text="线程时间轴", fg=self.text, bg=self.panel,
                 font=("Microsoft YaHei UI", 12, "bold")).pack(side="left")
        self.window_label = tk.Label(timeline_head, text="", fg=self.muted, bg=self.panel,
                                     font=("Consolas", 9))
        self.window_label.pack(side="right")
        self.canvas = tk.Canvas(left, bg="#0a1211", highlightthickness=0, height=320)
        self.canvas.pack(fill="both", expand=True, padx=8, pady=(0, 4))
        self.navigator = tk.Canvas(left, bg="#0c1514", highlightthickness=0, height=22)
        self.navigator.pack(fill="x", padx=8, pady=(0, 8))
        self.canvas.bind("<Configure>", lambda _event: self.draw())
        self.canvas.bind("<Button-1>", self.on_press)
        self.canvas.bind("<B1-Motion>", self.on_drag)
        self.canvas.bind("<ButtonRelease-1>", self.on_release)
        self.canvas.bind("<MouseWheel>", self.on_wheel)
        self.canvas.bind("<Button-4>", lambda _event: self.set_zoom(self.zoom * 1.25))
        self.canvas.bind("<Button-5>", lambda _event: self.set_zoom(self.zoom / 1.25))
        self.navigator.bind("<Button-1>", self.on_navigator)
        self.navigator.bind("<B1-Motion>", self.on_navigator)

        right = tk.Frame(body, bg=self.panel, highlightbackground=self.line, highlightthickness=1, width=340)
        right.pack(side="left", fill="y", padx=(8, 0))
        right.pack_propagate(False)
        tk.Label(right, text="事件详情", fg=self.text, bg=self.panel,
                 font=("Microsoft YaHei UI", 12, "bold")).pack(anchor="w", padx=14, pady=(10, 8))
        self.detail = tk.Text(right, bg=self.panel, fg=self.text, relief="flat", height=16,
                              wrap="word", font=("Consolas", 10), padx=7, pady=5,
                              selectbackground="#21463e")
        self.detail.pack(fill="both", expand=True, padx=8, pady=(0, 8))
        self.detail.tag_configure("title", foreground=self.accent,
                                  font=("Microsoft YaHei UI", 15, "bold"), spacing3=12)
        self.detail.tag_configure("key", foreground=self.muted,
                                  font=("Microsoft YaHei UI", 9))
        self.detail.tag_configure("value", foreground=self.text,
                                  font=("Consolas", 10), justify="right")
        self.detail.configure(state="disabled")

        lower = tk.Frame(root, bg=self.bg)
        lower.pack(fill="both", padx=16, pady=(0, 8))
        slow_box = tk.Frame(lower, bg=self.panel, highlightbackground=self.line, highlightthickness=1)
        slow_box.pack(fill="both", expand=True)
        table_head = tk.Frame(slow_box, bg=self.panel)
        table_head.pack(fill="x", padx=12, pady=(8, 5))
        tk.Label(table_head, text="最慢事件", fg=self.text, bg=self.panel,
                 font=("Microsoft YaHei UI", 11, "bold")).pack(side="left")
        self.table_count = tk.Label(table_head, text="", fg=self.muted, bg=self.panel,
                                    font=("Consolas", 9))
        self.table_count.pack(side="right")
        table_body = tk.Frame(slow_box, bg="#0b1413")
        table_body.pack(fill="both", expand=True, padx=8, pady=(0, 8))
        self.slow_list = self.ttk.Treeview(
            table_body, columns=("label", "thread", "start", "duration", "type"),
            show="headings", height=7, style="Profile.Treeview", selectmode="browse",
        )
        headings = (
            ("label", "Label", 360, "w"),
            ("thread", "Thread", 190, "w"),
            ("start", "Start", 150, "e"),
            ("duration", "Duration", 150, "e"),
            ("type", "Type", 110, "w"),
        )
        for column, title, width, anchor in headings:
            self.slow_list.heading(column, text=title, anchor=anchor)
            self.slow_list.column(column, width=width, minwidth=80, anchor=anchor, stretch=column == "label")
        table_scroll = self.ttk.Scrollbar(table_body, orient="vertical", command=self.slow_list.yview)
        self.slow_list.configure(yscrollcommand=table_scroll.set)
        table_scroll.pack(side="right", fill="y")
        self.slow_list.pack(side="left", fill="both", expand=True)
        self.slow_list.bind("<<TreeviewSelect>>", self.on_slow_select)
        self.slow_events = []
        diag_box = tk.Frame(lower, bg=self.panel, highlightbackground=self.line, highlightthickness=1)
        diag_box.pack(fill="both", expand=True, pady=(8, 0))
        tk.Label(diag_box, text="解析诊断", fg=self.text, bg=self.panel,
                 font=("Microsoft YaHei UI", 10, "bold")).pack(anchor="w", padx=12, pady=(7, 3))
        self.diag = tk.Text(diag_box, bg="#0b1413", fg="#e4a4a4", relief="flat", height=3,
                             font=("Consolas", 10))
        self.diag.pack(fill="both", expand=True, padx=8, pady=(0, 8))
        self.diag.configure(state="disabled")
        self.status = tk.Label(root, text="", fg=self.muted, bg=self.bg, anchor="w")
        self.status.pack(fill="x", padx=16, pady=(0, 8))
        self._set_detail("打开 trace.profile，输入进程 PID，或连接 ws://127.0.0.1:9765。")
        root.protocol("WM_DELETE_WINDOW", self._on_close)
        self._sync_mode()

    def _sync_mode(self):
        mode = self.mode.get()
        self.browse_button.configure(state="normal" if mode == "file" else "disabled")
        text = self.path_var.get().strip()
        if mode == "pid" and text and not text.isdigit():
            self.path_var.set("")
        if mode == "ws" and (not text or "\\" in text or ("/" in text and not text.startswith("ws://"))):
            self.path_var.set("127.0.0.1:%d" % WS_PORT)
        if mode != "ws":
            self._close_ws()
        self._stop_live_worker()

    def _close_ws(self):
        client = self.ws_client
        self.ws_client = None
        if client is not None:
            client.close()

    def _bind_ws_client(self, epoch, stop, client):
        with self.reader_lock:
            if stop.is_set() or self.reader_epoch != epoch:
                return False
            self.ws_client = client
            return True

    def _unbind_ws_client(self, client):
        with self.reader_lock:
            if self.ws_client is client:
                self.ws_client = None

    def _on_close(self):
        self._stop_live_worker()
        self._close_ws()
        self.root.destroy()

    def _stop_live_worker(self):
        stop = self.live_stop
        thread = self.live_thread
        self.live_stop = None
        self.live_thread = None
        self.live_kind = None
        if stop is not None:
            stop.set()
        if thread is not None and thread is not threading.current_thread():
            thread.join(timeout=0.3)

    def choose_file(self):
        path = self.filedialog.askopenfilename(
            title="打开 profile",
            filetypes=[("profile", "*.profile"), ("全部", "*.*")],
        )
        if not path:
            return
        self.mode.set("file")
        self.path_var.set(path)
        self.load_current()

    def load_current(self):
        text = self.path_var.get().strip()
        mode = self.mode.get()
        self._stop_live_worker()
        self._close_ws()
        try:
            if mode == "ws":
                host, port = parse_ws_target(text)
                self.source = ("ws", (host, port))
                self.live_var.set(True)
                label = "ws://%s:%d" % (host, port)
            elif mode == "pid":
                pid = int(text)
                self.source = ("pid", pid)
                label = shm_name(pid)
            else:
                if not text:
                    self._set_status("先选择文件")
                    return
                self.source = ("file", text)
                label = os.path.basename(text)
        except Exception as exc:
            self._set_status(str(exc))
            return
        self.buffer.reset()
        self.selected = None
        self.offset = 0
        self.zoom = 1.0
        self.thread_ids = None
        self.issue_stamp = None
        self.slow_stamp = None
        self.shown_name = None
        self.series = {}
        self.order = []
        self.trace = Trace()
        self.filtered = []
        self._set_follow()
        self._start_reader()
        self._set_status("正在读取 %s" % label)
        self._update_source_label()
        self._arm_refresh()

    def _fill_threads(self):
        self._ensure_series()
        options = ["全部线程"]
        options.extend(format_thread(item) for item in self.order)
        self.thread_menu.configure(values=tuple(options))
        self.thread_ids = tuple(self.order)
        if self.thread_var.get() not in options:
            self.thread_var.set("全部线程")

    def _set_follow(self):
        if self.source is not None and (self.live_var.get() or self.source[0] == "ws"):
            self.follow.set()
        else:
            self.follow.clear()

    def _toggle_live(self):
        self._set_follow()
        if self.source is None:
            return
        if self.live_var.get() or self.source[0] == "ws":
            if self.live_thread is None or not self.live_thread.is_alive():
                self._start_reader()
            self._arm_refresh()

    def _arm_refresh(self):
        if self.after_id is None and self.source is not None:
            self.after_id = self.root.after(self._live_delay(), self._poll)

    def _start_reader(self):
        self._stop_live_worker()
        if self.source is None:
            return
        kind, value = self.source
        stop = threading.Event()
        with self.reader_lock:
            self.reader_epoch += 1
            epoch = self.reader_epoch
        self.live_stop = stop
        self.live_kind = self.source
        buffer = self.buffer
        follow = self.follow
        self.live_thread = threading.Thread(
            target=self._reader_main,
            args=(epoch, kind, value, buffer, stop, follow),
            name="profile-read",
            daemon=True,
        )
        self.live_thread.start()

    def _reader_main(self, epoch, kind, value, buffer, stop, follow):
        try:
            active = lambda: self.reader_epoch == epoch
            if kind == "ws":
                host, port = value
                read_websocket(
                    host, port, buffer, stop, active,
                    lambda client: self._bind_ws_client(epoch, stop, client),
                    self._unbind_ws_client,
                )
            elif kind == "pid":
                read_pid(value, buffer, stop, follow, active, parse_trace)
            else:
                read_file(value, buffer, stop, follow, active)
        except Exception as exc:
            if self.reader_epoch == epoch:
                buffer.set_error(str(exc))

    def _adopt(self):
        gen, trace, order, series, byte_count, error, slow, _issues = self.buffer.peek()
        self.seen_gen = gen
        self.trace = trace
        self.order = order
        self.series = series
        self.slow_cache = slow
        self.byte_count = byte_count
        return error

    def _poll(self):
        self.after_id = None
        if self.source is None:
            return
        kind, _value = self.source
        gen, trace, order, series, byte_count, error, slow, issue_count = self.buffer.peek()
        if error:
            self._set_status(error)
        elif (byte_count or trace.events) and self.status.cget("text").startswith("正在读取"):
            self._set_status("已读取")
        if gen != self.seen_gen:
            selected_key = self._event_key(self.selected) if kind == "pid" else None
            self.seen_gen = gen
            self.trace = trace
            self.order = order
            self.series = series
            self.slow_cache = slow
            self.byte_count = byte_count
            if kind == "pid":
                self.selected = self._find_event(selected_key)
            if self.selected is not None and self.selected.name != self.shown_name:
                self.shown_name = self.selected.name
                self._show_selected()
            if tuple(order) != self.thread_ids:
                self._fill_threads()
            if self._match_all():
                self.filtered = trace.events
                self._update_stats()
                self._fill_slow()
                self._fill_diag_if_needed(issue_count)
                self.draw()
            else:
                self.apply_filter()
        self._update_source_label()
        alive = self.live_thread is not None and self.live_thread.is_alive()
        if alive or self.live_var.get() or kind == "ws":
            self.after_id = self.root.after(self._live_delay(), self._poll)

    def _update_source_label(self):
        if self.source is None:
            return
        kind, value = self.source
        if kind == "ws":
            host, port = value
            text = "ws://%s:%d · %s" % (host, port, format_bytes(self.byte_count))
        elif kind == "pid":
            text = "%s · %s" % (shm_name(value), format_bytes(self.byte_count))
        else:
            text = "%s · %s" % (os.path.basename(value), format_bytes(self.byte_count))
        if self.source_label.cget("text") != text:
            self.source_label.configure(text=text)

    def _filters(self):
        query = self.search_var.get().strip().lower()
        thread_text = self.thread_var.get()
        try:
            min_ns = max(0.0, float(self.min_var.get() or 0)) * 1000.0
        except ValueError:
            min_ns = 0.0
        return query, thread_text, min_ns

    def _match_all(self):
        query, thread_text, min_ns = self._filters()
        return (not query) and thread_text == "全部线程" and min_ns <= 0.0

    def _keeps(self, event, query, thread_text, min_ns):
        if query and query not in event.name.lower():
            return False
        if thread_text != "全部线程" and format_thread(event.thread) != thread_text:
            return False
        if event.duration < min_ns:
            return False
        return True

    def _ensure_series(self):
        if self.series:
            return
        order, grouped = group_threads(self.trace.events)
        self.order = order
        self.series = {thread: series_from_events(events) for thread, events in grouped.items()}

    def _live_delay(self):
        return 40

    def apply_filter(self):
        query, thread_text, min_ns = self._filters()
        if not query and thread_text == "全部线程" and min_ns <= 0.0:
            self.filtered = self.trace.events
        else:
            self.filtered = [
                event for event in self.trace.events
                if self._keeps(event, query, thread_text, min_ns)
            ]
        self._update_stats()
        self._fill_slow()
        self._fill_diag()
        self.draw()

    def _update_stats(self):
        span = self.trace.max_time - self.trace.min_time if self.trace.events else 0
        threads = len(self.order) if self.order else len({event.thread for event in self.trace.events})
        self.stat_labels["events"].configure(text=str(len(self.trace.events)))
        self.stat_labels["threads"].configure(text=str(threads))
        self.stat_labels["span"].configure(text=format_duration(span))
        self.stat_labels["labels"].configure(text=str(len(self.trace.labels)))
        self.stat_labels["issues"].configure(text=str(len(self.trace.issues)))

    def _fill_slow(self):
        if self._match_all() and self.slow_cache:
            ranked = self.slow_cache
        else:
            ranked = _slowest(self.filtered)
        self.slow_events = ranked
        self.table_count.configure(
            text="%d / %d events" % (len(self.filtered), len(self.trace.events)))
        stamp = tuple((event.duration, event.start, event.thread, event.name) for event in ranked)
        if stamp == self.slow_stamp:
            return
        self.slow_stamp = stamp
        children = self.slow_list.get_children()
        if children:
            self.slow_list.delete(*children)
        self.slow_item_events = {}
        base = self.trace.min_time
        for index, event in enumerate(ranked):
            item_id = "event-%d" % index
            self.slow_item_events[item_id] = event
            self.slow_list.insert(
                "", "end", iid=item_id,
                values=(
                    event.name,
                    format_thread(event.thread),
                    format_duration(event.start - base),
                    format_duration(event.duration),
                    "Dynamic" if event.kind == "dynamic" else "Static",
                ),
            )

    def _fill_diag_if_needed(self, issue_count):
        last = self.trace.issues[-1].message if self.trace.issues else ""
        stamp = (issue_count, last)
        if stamp == self.issue_stamp:
            return
        self.issue_stamp = stamp
        self._fill_diag()

    def _fill_diag(self):
        self.diag.configure(state="normal")
        self.diag.delete("1.0", "end")
        if not self.trace.issues:
            self.diag.insert("end", "没有诊断。")
            self.diag.configure(fg=self.accent)
        else:
            self.diag.configure(fg="#e4a4a4")
            for issue in self.trace.issues[:80]:
                self.diag.insert("end", "0x%x  %s\n" % (issue.offset, issue.message))
        self.diag.configure(state="disabled")

    def _view_window(self):
        total = self.trace.max_time - self.trace.min_time if self.trace.events else 1
        total = max(total, 1)
        span = self._span_for_zoom(total)
        other_edge = total - span
        min_offset = min(0.0, other_edge)
        max_offset = max(0.0, other_edge)
        self.offset = max(min_offset, min(self.offset, max_offset))
        return self.trace.min_time + self.offset, span, total

    def _span_for_zoom(self, total, zoom=None):
        value = self.zoom if zoom is None else zoom
        try:
            span = float(total) / value
        except (OverflowError, ZeroDivisionError):
            span = sys.float_info.max / 4.0
        if not math.isfinite(span):
            span = sys.float_info.max / 4.0
        # Do not impose a UI zoom floor.  Even though source timestamps are
        # integral nanoseconds, sub-nanosecond windows are useful for keeping
        # wheel/button navigation continuous at an event edge.
        return max(math.ulp(0.0), span)

    def set_zoom(self, zoom):
        total = self.trace.max_time - self.trace.min_time if self.trace.events else 1
        total = max(total, 1)
        old_span = self._span_for_zoom(total)
        anchor = self.offset + old_span / 2.0
        try:
            next_zoom = float(zoom)
        except (TypeError, ValueError, OverflowError):
            return
        if math.isnan(next_zoom) or next_zoom <= 0.0:
            return
        if math.isinf(next_zoom):
            next_zoom = sys.float_info.max
        self.zoom = next_zoom
        new_span = self._span_for_zoom(total)
        self.offset = anchor - new_span / 2.0
        self.zoom_label.configure(text=self._zoom_text())
        self.draw()

    def reset_view(self):
        self.zoom = 1.0
        self.offset = 0
        self.zoom_label.configure(text="1.0×")
        self.draw()

    def _zoom_text(self):
        if self.zoom < 0.01:
            return "%.2e×" % self.zoom
        if self.zoom < 10:
            return "%.2f×" % self.zoom
        if self.zoom < 10000:
            return "%.0f×" % self.zoom
        return "%.2e×" % self.zoom

    def draw(self):
        canvas = self.canvas
        canvas.delete("all")
        self.hits = []
        self.density_rows = []
        width = max(canvas.winfo_width(), canvas.winfo_reqwidth(), 10)
        height = max(canvas.winfo_height(), canvas.winfo_reqheight(), 10)
        self._ensure_series()
        if not self.filtered:
            canvas.create_text(width / 2, height / 2, text="没有可显示的事件", fill=self.muted)
            self._draw_navigator(1, 0, 1)
            self.zoom_label.configure(text=self._zoom_text())
            return
        origin, span, total = self._view_window()
        query, thread_text, min_ns = self._filters()
        match_all = (not query) and thread_text == "全部线程" and min_ns <= 0.0
        predicate = None if match_all else (lambda event: self._keeps(event, query, thread_text, min_ns))
        gutter = 154
        top = 34
        lane_h = 18
        row_gap = 12
        plot_w = max(1, width - gutter - 12)
        scan_limit = max(2000, plot_w * 8)
        plans = []
        y = top
        visible = 0
        source_visible = 0
        for thread in self.order:
            if thread_text != "全部线程" and format_thread(thread) != thread_text:
                continue
            series = self.series.get(thread)
            if series is None:
                continue
            lanes, source_count = visible_lanes(
                series, origin, span, predicate, scan_limit=scan_limit)
            if not lanes:
                continue
            row_height = len(lanes) * lane_h + row_gap
            if y > height - 8:
                break
            plans.append((thread, lanes, source_count, y, row_height))
            visible += sum(len(lane) for lane in lanes)
            source_visible += source_count
            y += row_height
        dense = visible > SPARSE_RECT_LIMIT or source_visible > SPARSE_RECT_LIMIT
        for index in range(6):
            x = gutter + plot_w * index / 5.0
            canvas.create_line(x, top - 8, x, height - 8, fill="#1d2c29")
            label = format_duration(self.offset + span * index / 5.0)
            canvas.create_text(x + 4, 8, anchor="nw", text=label, fill=self.muted, font=("Consolas", 9))
        if hasattr(self, "window_label"):
            self.window_label.configure(
                text="%s → %s" % (format_duration(self.offset), format_duration(self.offset + span)))
        if dense:
            self._draw_dense_lanes(plans, origin, span, gutter, top, lane_h, plot_w, height)
        else:
            self._draw_sparse_lanes(plans, origin, span, gutter, lane_h, plot_w, height)
        self._draw_navigator(total, self.offset, span)
        self.zoom_label.configure(text=self._zoom_text())

    def _draw_thread_row(self, thread, source_count, drawn_count, y, row_height, width):
        canvas = self.canvas
        canvas.create_rectangle(0, y - 2, width, y + row_height - 4,
                                outline="", fill="#101a18")
        canvas.create_text(12, y + 1, anchor="nw", text=format_thread(thread),
                           fill="#b8c8c3", font=("Consolas", 9))
        count_text = "%d visible" % source_count
        if source_count != drawn_count:
            count_text += " · %d drawn" % drawn_count
        canvas.create_text(12, y + 18, anchor="nw", text=count_text,
                           fill="#657a74", font=("Consolas", 8))

    def _draw_sparse_lanes(self, plans, origin, span, gutter, lane_h, plot_w, height):
        canvas = self.canvas
        view_end = origin + span
        for thread, lanes, source_count, y, row_height in plans:
            drawn_count = sum(len(lane) for lane in lanes)
            self._draw_thread_row(thread, source_count, drawn_count, y, row_height, gutter + plot_w + 12)
            for lane_index, events in enumerate(lanes):
                event_y = y + lane_index * lane_h
                for event in events:
                    if event.end < origin or event.start > view_end:
                        continue
                    x0 = gutter + (max(event.start, origin) - origin) / float(span) * plot_w
                    x1 = gutter + (min(event.end, view_end) - origin) / float(span) * plot_w
                    x1 = max(x1, x0 + 1.5)
                    color = color_for(event.name or "?")
                    outline = "#ffffff" if event is self.selected else ""
                    canvas.create_rectangle(
                        x0, event_y, x1, event_y + 14,
                        fill=color, outline=outline, width=1,
                    )
                    self._paint_module_name(x0, event_y + 7, x1 - x0, module_name(event))
                    self.hits.append((x0, event_y, x1, event_y + 14, event))

    def _draw_dense_lanes(self, plans, origin, span, gutter, top, lane_h, plot_w, height):
        canvas = self.canvas
        if not plans:
            return
        visible_bottom = min(height - 6, plans[-1][3] + plans[-1][4])
        plot_h = max(1, visible_bottom - top)
        photo = self.photo
        if photo is None or photo.width() != plot_w or photo.height() != plot_h:
            photo = self.tk.PhotoImage(width=plot_w, height=plot_h)
            self.photo = photo
        photo.put("#0d1614", to=(0, 0, plot_w, plot_h))
        lane_rows = []
        selected_box = None
        view_end = origin + span
        for thread, lanes, source_count, y, row_height in plans:
            drawn_count = sum(len(lane) for lane in lanes)
            self._draw_thread_row(thread, source_count, drawn_count, y, row_height, gutter + plot_w + 12)
            for lane_index, events in enumerate(lanes):
                event_y = y + lane_index * lane_h
                if event_y + 14 > visible_bottom:
                    continue
                colors, owners = columns_from_lane(events, origin, span, plot_w)
                self._put_bar(photo, colors, event_y - top, 14)
                lane_rows.append((event_y, owners))
                if self.selected in events and self.selected.end >= origin and self.selected.start <= view_end:
                    x0 = gutter + (max(self.selected.start, origin) - origin) / float(span) * plot_w
                    x1 = gutter + (min(self.selected.end, view_end) - origin) / float(span) * plot_w
                    selected_box = (x0, event_y, max(x1, x0 + 1.5), event_y + 14)
        canvas.create_image(gutter, top, anchor="nw", image=photo)
        for event_y, owners in lane_rows:
            self.density_rows.append((gutter, event_y, plot_w, 14, owners))
            for start, end, name in name_spans(owners, 28):
                self._paint_module_name(gutter + start, event_y + 7, end - start, name)
        if selected_box is not None:
            canvas.create_rectangle(*selected_box, outline="#ffffff", width=1)

    def _paint_module_name(self, x, y, width, text):
        if self.name_font is None or not text:
            return
        shown = fit_text(self.name_font.measure, text, width)
        if not shown:
            return
        self.canvas.create_text(x + 3, y, anchor="w", text=shown, fill="#10211c", font=self.name_font)

    def _put_bar(self, photo, colors, y, height):
        tokens = ["#101816" if color is None else color for color in colors]
        line = "{%s}" % " ".join(tokens)
        photo.put(" ".join([line] * height), to=(0, y))

    def _draw_navigator(self, total, offset, span):
        nav = self.navigator
        nav.delete("all")
        width = max(nav.winfo_width(), 10)
        nav.create_rectangle(0, 4, width, 18, fill="#162220", outline="#334945")
        if total <= 0:
            return
        if span >= total:
            # The complete capture already fits in the window.  Keep the
            # navigator well-formed while allowing the timeline itself to
            # extend into empty time on either side.
            left = 0.0
            thumb = float(width)
        else:
            thumb = min(float(width), max(18.0, width * (span / float(total))))
            travel = max(0.0, width - thumb)
            available = max(1.0, total - span)
            left = travel * min(1.0, max(0.0, offset / available))
        nav.create_rectangle(left, 4, left + thumb, 18, fill=self.accent, outline="")

    def on_navigator(self, event):
        total = self.trace.max_time - self.trace.min_time if self.trace.events else 1
        total = max(total, 1)
        _origin, span, _total = self._view_window()
        width = max(self.navigator.winfo_width(), 1)
        ratio = min(1.0, max(0.0, event.x / float(width)))
        other_edge = total - span
        min_offset = min(0.0, other_edge)
        max_offset = max(0.0, other_edge)
        self.offset = min_offset + ratio * (max_offset - min_offset)
        self._request_draw()

    def on_press(self, event):
        self.drag = (event.x, self.offset)
        self.suppress_click = False

    def on_drag(self, event):
        if self.drag is None or not self.trace.events:
            return
        start_x, start_offset = self.drag
        delta = event.x - start_x
        if abs(delta) > 4:
            self.suppress_click = True
        _origin, span, _total = self._view_window()
        plot_w = max(1, self.canvas.winfo_width() - 144)
        self.offset = start_offset - delta / float(plot_w) * span
        self._request_draw()

    def _request_draw(self):
        if self.draw_job is not None:
            return
        self.draw_job = self.root.after(16, self._flush_draw)

    def _flush_draw(self):
        self.draw_job = None
        self.draw()

    def on_release(self, event):
        self.drag = None
        if self.draw_job is not None:
            self.root.after_cancel(self.draw_job)
            self.draw_job = None
        if self.suppress_click:
            self.draw()
            return
        for x0, y0, x1, y1, item in reversed(self.hits):
            if x0 <= event.x <= x1 and y0 <= event.y <= y1:
                self.selected = item
                self._show_selected()
                self.draw()
                return
        for x0, y0, width, height, owners in reversed(self.density_rows):
            if x0 <= event.x <= x0 + width and y0 <= event.y <= y0 + height and owners:
                col = int((event.x - x0) * len(owners) / float(width))
                if col < 0:
                    col = 0
                if col >= len(owners):
                    col = len(owners) - 1
                item = owners[col]
                if item is None:
                    return
                self.selected = item
                self._show_selected()
                self.draw()
                return
        self.draw()

    def on_wheel(self, event):
        if event.delta > 0:
            self.set_zoom(self.zoom * 1.25)
        else:
            self.set_zoom(self.zoom / 1.25)

    def on_slow_select(self, _event):
        selection = self.slow_list.selection()
        if not selection:
            return
        self.selected = self.slow_item_events.get(selection[0])
        if self.selected is None:
            return
        self._focus_selected()
        self._show_selected()
        self.draw()

    def _focus_selected(self):
        if self.selected is None or not self.trace.events:
            return
        total = max(1, self.trace.max_time - self.trace.min_time)
        span = self._span_for_zoom(total)
        start = self.selected.start - self.trace.min_time
        self.offset = start - span / 3.0

    def _show_selected(self):
        event = self.selected
        if event is None:
            self._set_detail("点击时间轴或最慢事件列表。")
            return
        module = module_name(event)
        if event.kind == "static" and not module:
            module = "(标签 #%s 的模块名还没到)" % event.label_id
        rows = [
            ("类型", "Dynamic Scope" if event.kind == "dynamic" else "Static Scope"),
            ("线程", format_thread(event.thread)),
            ("开始位置", format_duration(event.start - self.trace.min_time)),
            ("耗时", format_duration(event.duration)),
            ("Label ID", event.label_id if event.kind == "static" else "—"),
            ("Event ID", "—" if event.event_id is None else "0x%x" % event.event_id),
            ("文件偏移", "0x%x" % event.offset),
            ("名称截断", "是" if event.truncated else "否"),
        ]
        self.detail.configure(state="normal")
        self.detail.delete("1.0", "end")
        self.detail.insert("end", module or "(空 Label)", "title")
        for key, value in rows:
            self.detail.insert("end", "%s\n" % key, "key")
            self.detail.insert("end", "%s\n\n" % value, "value")
        self.detail.configure(state="disabled")

    def _set_detail(self, text):
        self.detail.configure(state="normal")
        self.detail.delete("1.0", "end")
        self.detail.insert("end", text)
        self.detail.configure(state="disabled")

    def _set_status(self, text):
        self.status.configure(text=text)

    def _trace_stamp(self, trace):
        if not trace.events:
            return (0, None, None, trace.chunk_count, len(trace.issues))
        first = trace.events[0]
        last = trace.events[-1]
        return (
            len(trace.events), first.start, first.name, last.start, last.name,
            trace.chunk_count, len(trace.issues),
        )

    def _event_key(self, event):
        if event is None:
            return None
        return (event.kind, event.start, event.duration, event.thread, event.name, event.offset)

    def _find_event(self, key):
        if key is None:
            return None
        for event in self.trace.events:
            if self._event_key(event) == key:
                return event
        return None


def smoke_ui():
    import tkinter as tk
    root = tk.Tk()
    root.withdraw()
    viewer = Viewer(root)
    records = pack_label(3, "tick") + pack_scope(100, 40, 3)
    viewer.trace = parse_trace(pack_chunk(5, records), ring=False)
    viewer.source = ("file", "sample")
    viewer.canvas.configure(width=800, height=300)
    viewer._fill_threads()
    viewer.apply_filter()
    root.update_idletasks()
    root.update()
    viewer.draw()
    assert viewer.stat_labels["events"].cget("text") == "1"
    assert viewer.filtered[0].name == "tick"
    assert len(viewer.hits) == 1
    # Regression: the old per-event/per-pixel path appeared to freeze around
    # ten thousand visible events.  Dense rendering must stay bounded.
    crowded = pack_chunk(5, pack_label(3, "tick") + b"".join(
        pack_scope(index * 100, 40, 3) for index in range(10000)))
    viewer.buffer.replace_parsed(parse_trace(crowded, ring=False))
    viewer.series = {}
    viewer._adopt()
    viewer.filtered = viewer.trace.events
    started = time.monotonic()
    viewer.draw()
    elapsed = time.monotonic() - started
    assert elapsed < 0.75
    assert viewer.density_rows
    assert len(viewer.hits) == 0
    assert any(owner is not None for _x, _y, _w, _h, owners in viewer.density_rows for owner in owners)
    # Zoom has no application-level min/max.  Below 1x the view may include
    # empty time outside the capture and above timestamp resolution the
    # drawable span is allowed to become sub-nanosecond.
    total = max(1, viewer.trace.max_time - viewer.trace.min_time)
    viewer.set_zoom(0.25)
    _origin, span, _total = viewer._view_window()
    assert viewer.zoom == 0.25
    assert span == total * 4.0
    assert viewer.offset < 0.0 and viewer.offset + span > total
    viewer.set_zoom(float(total) * 100.0)
    _origin, span, _total = viewer._view_window()
    assert viewer.zoom > total
    assert 0.0 < span < 1.0
    root.destroy()
    print("profile_viewer smoke-ui ok")


def main(argv):
    if "--self-test" in argv:
        self_test()
        return 0
    if "--smoke-ui" in argv:
        smoke_ui()
        return 0
    import tkinter as tk
    root = tk.Tk()
    Viewer(root)
    root.mainloop()
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
