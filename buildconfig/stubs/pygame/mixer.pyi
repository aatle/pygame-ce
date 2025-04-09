from typing import Any, Optional, Union, overload

from pygame.event import Event
from pygame.typing import FileLike
from typing_extensions import (
    Buffer,  # collections.abc 3.12
    deprecated,  # added in 3.13
)

from . import mixer_music

# export mixer_music as mixer.music
music = mixer_music

def init(
    frequency: int = 44100,
    size: int = -16,
    channels: int = 2,
    buffer: int = 512,
    devicename: Optional[str] = None,
    allowedchanges: int = 5,
) -> None: ...
def pre_init(
    frequency: int = 44100,
    size: int = -16,
    channels: int = 2,
    buffer: int = 512,
    devicename: Optional[str] = None,
    allowedchanges: int = 5,
) -> None: ...
def quit() -> None: ...
def get_init() -> tuple[int, int, int]: ...
def get_driver() -> str: ...
def stop() -> None: ...
def pause() -> None: ...
def unpause() -> None: ...
def fadeout(time: int, /) -> None: ...
def set_num_channels(count: int, /) -> None: ...
def get_num_channels() -> int: ...
def set_reserved(count: int, /) -> int: ...
def find_channel(force: bool = False) -> Channel: ...
def set_soundfont(paths: Optional[str] = None, /) -> None: ...
def get_soundfont() -> Optional[str]: ...
def get_busy() -> bool: ...
def get_sdl_mixer_version(linked: bool = True) -> tuple[int, int, int]: ...

class Sound:
    @overload
    def __init__(self, file: FileLike) -> None: ...
    @overload
    def __init__(self, buffer: Buffer) -> None: ...
    def play(
        self,
        loops: int = 0,
        maxtime: int = 0,
        fade_ms: int = 0,
    ) -> Channel: ...
    # possibly going to be deprecated/removed soon, in which case these
    # typestubs must be removed too
    @property
    def __array_interface__(self) -> dict[str, Any]: ...
    @property
    def __array_struct__(self) -> Any: ...
    def __buffer__(self, flags: int, /) -> memoryview[int]: ...
    def __release_buffer__(self, view: memoryview[int], /) -> None: ...
    def stop(self) -> None: ...
    def fadeout(self, time: int, /) -> None: ...
    def set_volume(self, value: float, /) -> None: ...
    def get_volume(self) -> float: ...
    def get_num_channels(self) -> int: ...
    def get_length(self) -> float: ...
    def get_raw(self) -> bytes: ...

class Channel:
    def __init__(self, id: int) -> None: ...
    @property
    def id(self) -> int: ...
    def play(
        self,
        sound: Sound,
        loops: int = 0,
        maxtime: int = 0,
        fade_ms: int = 0,
    ) -> None: ...
    def stop(self) -> None: ...
    def pause(self) -> None: ...
    def unpause(self) -> None: ...
    def fadeout(self, time: int, /) -> None: ...
    def queue(self, sound: Sound, /) -> None: ...
    def set_source_location(self, angle: float, distance: float, /) -> None: ...
    @overload
    def set_volume(self, value: float, /) -> None: ...
    @overload
    def set_volume(self, left: float, right: float, /) -> None: ...
    def get_volume(self) -> float: ...
    def get_busy(self) -> bool: ...
    def get_sound(self) -> Sound: ...
    def get_queue(self) -> Sound: ...
    def set_endevent(self, type: Union[int, Event] = 0, /) -> None: ...
    def get_endevent(self) -> int: ...

@deprecated("Use `Sound` instead (SoundType is an old alias)")
class SoundType(Sound): ...

@deprecated("Use `Channel` instead (ChannelType is an old alias)")
class ChannelType(Channel): ...
