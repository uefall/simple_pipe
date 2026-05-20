from __future__ import annotations

from dataclasses import dataclass
from typing import Generic, TypeVar

T = TypeVar("T")


@dataclass(frozen=True)
class Result(Generic[T]):
    ok: bool
    value: T | None = None
    error: str | None = None

    @staticmethod
    def success(value: T | None = None) -> Result[T]:
        return Result(ok=True, value=value)

    @staticmethod
    def failure(message: str) -> Result[T]:
        return Result(ok=False, error=message)
