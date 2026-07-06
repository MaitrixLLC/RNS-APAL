"""RNS-PYPAL error types and diagnostic helpers."""

from __future__ import annotations

from enum import StrEnum
import inspect
from pathlib import Path
from typing import NoReturn
import warnings


class RNSPypalError(Exception):
    """Base exception for RNS-PYPAL."""


class RNSRangeError(ValueError, RNSPypalError):
    """A checked assignment or range operation found an out-of-range value."""


class RNSCriticalError(ValueError, RNSPypalError):
    """A critical mathematical or representation invariant was violated."""


class RNSDiagnosticWarning(UserWarning):
    """Warning category for non-fatal RNS-PYPAL diagnostics."""


class RNSDiagnosticLevel(StrEnum):
    """How a diagnostic condition should be handled."""

    WARNING = "warning"
    ERROR = "error"
    CRITICAL = "critical"


def _diagnostic_message(prefix: str, reason: str, **context: object) -> str:
    """Return a diagnostic message with caller location and context."""

    frame = inspect.currentframe()
    caller = frame.f_back.f_back if frame is not None and frame.f_back is not None else None
    if caller is None:
        location = "unknown location"
        function = "unknown"
    else:
        location = f"{Path(caller.f_code.co_filename).name}:{caller.f_lineno}"
        function = caller.f_code.co_name

    lines = [
        f"{prefix}: {reason}",
        f"location: {location}",
        f"function: {function}",
    ]
    if context:
        lines.append("context:")
        for key, value in context.items():
            lines.append(f"  {key}: {value!r}")

    return "\n".join(lines)


def report_diagnostic(
    level: str | RNSDiagnosticLevel,
    reason: str,
    **context: object,
) -> None:
    """Handle a diagnostic condition according to ``level``."""

    level = RNSDiagnosticLevel(level)
    if level == RNSDiagnosticLevel.WARNING:
        warnings.warn(
            _diagnostic_message("WARNING", reason, **context),
            RNSDiagnosticWarning,
            stacklevel=2,
        )
        return
    if level == RNSDiagnosticLevel.ERROR:
        raise RNSRangeError(_diagnostic_message("ERROR", reason, **context))
    critical_error(reason, **context)


def critical_error(reason: str, **context: object) -> NoReturn:
    """Raise a critical RNS error with caller location and diagnostic context."""

    raise RNSCriticalError(_diagnostic_message("CRITICAL ERROR", reason, **context))
