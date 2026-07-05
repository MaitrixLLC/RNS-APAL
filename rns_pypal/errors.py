"""RNS-PYPAL error types and diagnostic helpers."""

from __future__ import annotations

import inspect
from pathlib import Path
from typing import NoReturn


class RNSPypalError(Exception):
    """Base exception for RNS-PYPAL."""


class RNSCriticalError(ValueError, RNSPypalError):
    """A critical mathematical or representation invariant was violated."""


def critical_error(reason: str, **context: object) -> NoReturn:
    """Raise a critical RNS error with caller location and diagnostic context."""

    frame = inspect.currentframe()
    caller = frame.f_back if frame is not None else None
    if caller is None:
        location = "unknown location"
        function = "unknown"
    else:
        location = f"{Path(caller.f_code.co_filename).name}:{caller.f_lineno}"
        function = caller.f_code.co_name

    lines = [
        f"CRITICAL ERROR: {reason}",
        f"location: {location}",
        f"function: {function}",
    ]
    if context:
        lines.append("context:")
        for key, value in context.items():
            lines.append(f"  {key}: {value!r}")

    raise RNSCriticalError("\n".join(lines))
