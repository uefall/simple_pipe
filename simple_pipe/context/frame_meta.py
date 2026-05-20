from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any


@dataclass
class FrameInput:
    """Host application frame payload pushed into the pipeline."""

    image: Any = None
    frame_index: int = 0
    channel_index: int = 0
    program_code: int = 0
    extensions: dict[str, Any] = field(default_factory=dict)


@dataclass
class FrameMeta:
    """
    Frame-level execution context passed between operators.

    Cross-frame state (tracker trajectories, galleries) must live inside
    stateful operators, not on FrameMeta.
    """

    images: dict[str, Any] = field(default_factory=dict)
    frame_index: int = 0
    channel_index: int = 0
    program_code: int = 0
    detections: list[dict[str, Any]] | None = None
    extensions: dict[str, Any] = field(default_factory=dict)

    @classmethod
    def from_input(cls, frame: FrameInput) -> FrameMeta:
        images: dict[str, Any] = {}
        if frame.image is not None:
            images["main"] = frame.image
        return cls(
            images=images,
            frame_index=frame.frame_index,
            channel_index=frame.channel_index,
            program_code=frame.program_code,
            extensions=dict(frame.extensions),
        )

    def copy_shallow(self) -> FrameMeta:
        return FrameMeta(
            images=dict(self.images),
            frame_index=self.frame_index,
            channel_index=self.channel_index,
            program_code=self.program_code,
            detections=list(self.detections) if self.detections is not None else None,
            extensions=dict(self.extensions),
        )
