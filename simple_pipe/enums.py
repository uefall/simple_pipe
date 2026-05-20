from __future__ import annotations

from enum import Enum


class NodeRole(str, Enum):
    SRC = "SRC"
    MID = "MID"
    DES = "DES"


class DropPolicy(str, Enum):
    DROP_NEWEST = "DROP_NEWEST"
    KEEP_LATEST = "KEEP_LATEST"
