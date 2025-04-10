import sys
from typing import Any

class BufferProxy:
    @property
    def parent(self) -> Any: ...
    @property
    def length(self) -> int: ...
    @property
    def raw(self) -> bytes: ...
    # possibly going to be deprecated/removed soon, in which case these
    # typestubs must be removed too
    @property
    def __array_interface__(self) -> dict[str, Any]: ...
    @property
    def __array_struct__(self) -> Any: ...
    if sys.version_info >= (3, 12):
        def __buffer__(self, flags: int, /) -> memoryview[int]: ...
        def __release_buffer__(self, view: memoryview[int], /) -> None: ...
    def __init__(self, parent: Any) -> None: ...  # TODO: parent: TypedDict | Protocol
    def write(
        self,
        buffer: str | bytes,  # See https://docs.python.org/3/c-api/arg.html at s#
        offset: int = 0,
    ) -> None: ...
