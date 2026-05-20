from __future__ import annotations

import queue
import threading
from abc import ABC, abstractmethod
from collections import defaultdict
from typing import Callable

from simple_pipe.context.frame_meta import FrameMeta
from simple_pipe.enums import DropPolicy, NodeRole
from simple_pipe.result import Result


class Operator(ABC):
    """
    Async actor-like operator: meta_flow enqueues, handle thread processes,
    dispatch thread pushes to downstream subscribers.
    """

    def __init__(self, node_id: str, node_type: str, build_init_params: dict | None = None) -> None:
        self.node_id = node_id
        self.node_type = node_type
        self.build_init_params = dict(build_init_params or {})
        self._downstreams: list[Operator] = []
        self._upstreams: list[Operator] = []
        self._in_queue: queue.Queue[FrameMeta | None] = queue.Queue()
        self._out_queue: queue.Queue[FrameMeta | None] = queue.Queue()
        self._max_in_queue_size = 8
        self._drop_policy = DropPolicy.DROP_NEWEST
        self._running = False
        self._handle_thread: threading.Thread | None = None
        self._dispatch_thread: threading.Thread | None = None
        self._lock = threading.Lock()
        # fan-in: wait for one meta per upstream per frame_index
        self._fanin_pending: dict[int, dict[str, FrameMeta]] = defaultdict(dict)
        self._fanin_upstream_ids: set[str] = set()

    @abstractmethod
    def node_role(self) -> NodeRole:
        raise NotImplementedError

    def configure(self, patch: dict) -> Result[None]:
        return Result.success()

    def attach_to(self, upstreams: list[Operator]) -> None:
        self._upstreams = list(upstreams)
        self._fanin_upstream_ids = {u.node_id for u in upstreams}

    def add_downstream(self, downstream: Operator) -> None:
        if downstream not in self._downstreams:
            self._downstreams.append(downstream)

    def set_input_queue_limit(self, max_size: int, policy: DropPolicy) -> None:
        self._max_in_queue_size = max(1, max_size)
        self._drop_policy = policy

    def meta_flow(self, meta: FrameMeta) -> None:
        if not self._running:
            return
        self._enqueue_with_backpressure(meta)

    def _enqueue_with_backpressure(self, meta: FrameMeta) -> None:
        with self._lock:
            if self._in_queue.qsize() >= self._max_in_queue_size:
                if self._drop_policy == DropPolicy.DROP_NEWEST:
                    return
                # KEEP_LATEST: drop oldest
                try:
                    self._in_queue.get_nowait()
                except queue.Empty:
                    pass
            try:
                self._in_queue.put_nowait(meta)
            except queue.Full:
                pass

    def start(self) -> None:
        if self._running:
            return
        self._running = True
        self._handle_thread = threading.Thread(
            target=self._handle_run, name=f"{self.node_id}-handle", daemon=True
        )
        self._dispatch_thread = threading.Thread(
            target=self._dispatch_run, name=f"{self.node_id}-dispatch", daemon=True
        )
        self._handle_thread.start()
        self._dispatch_thread.start()

    def stop(self) -> None:
        self._running = False
        try:
            self._in_queue.put_nowait(None)
        except queue.Full:
            pass
        if self._handle_thread and self._handle_thread.is_alive():
            self._handle_thread.join(timeout=2.0)
        if self._dispatch_thread and self._dispatch_thread.is_alive():
            self._dispatch_thread.join(timeout=2.0)

    def _handle_run(self) -> None:
        while self._running:
            try:
                meta = self._in_queue.get(timeout=0.1)
            except queue.Empty:
                continue
            if meta is None:
                break
            try:
                if self._fanin_upstream_ids:
                    merged = self._collect_fanin(meta)
                    if merged is None:
                        continue
                    meta = merged
                out = self.handle_frame_meta(meta)
                if out is not None:
                    self._out_queue.put(out)
            except Exception:
                continue

    def _collect_fanin(self, meta: FrameMeta) -> FrameMeta | None:
        """Merge metas from all upstreams for the same frame_index."""
        idx = meta.frame_index
        # meta.extensions should carry _upstream_id when pushed from upstream
        upstream_id = meta.extensions.get("_upstream_id", "unknown")
        bucket = self._fanin_pending[idx]
        bucket[upstream_id] = meta
        if not self._fanin_upstream_ids.issubset(bucket.keys()):
            return None
        parts = [bucket[uid] for uid in sorted(self._fanin_upstream_ids)]
        del self._fanin_pending[idx]
        return self._merge_fanin_metas(parts)

    def _merge_fanin_metas(self, parts: list[FrameMeta]) -> FrameMeta:
        base = parts[0].copy_shallow()
        merged_ext = dict(base.extensions)
        merged_images = dict(base.images)
        merged_detections: list[dict] = []
        for part in parts:
            merged_images.update(part.images)
            merged_ext.update({k: v for k, v in part.extensions.items() if not k.startswith("_")})
            if part.detections:
                merged_detections.extend(part.detections)
        base.images = merged_images
        base.extensions = merged_ext
        if merged_detections:
            base.detections = merged_detections
        return base

    def _dispatch_run(self) -> None:
        while self._running:
            try:
                meta = self._out_queue.get(timeout=0.1)
            except queue.Empty:
                continue
            if meta is None:
                break
            for downstream in self._downstreams:
                out = meta.copy_shallow()
                out.extensions["_upstream_id"] = self.node_id
                downstream.meta_flow(out)

    @abstractmethod
    def handle_frame_meta(self, meta: FrameMeta) -> FrameMeta | None:
        raise NotImplementedError

    def handle_frame_meta_batch(self, batch: list[FrameMeta]) -> None:
        for meta in batch:
            out = self.handle_frame_meta(meta)
            if out is not None:
                self._out_queue.put(out)
