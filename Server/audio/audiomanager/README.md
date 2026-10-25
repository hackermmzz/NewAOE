# AudioManager

`AudioManager` owns the miniaudio engine, effect cache, active effect instances,
and one streamed non-spatial sound.

## Thread model

The module never creates a `std::thread`. Construct, call, update, and destroy
one manager from the same caller-owned audio thread. The platform device
callback and miniaudio streaming worker remain internal because native and web
audio backends require asynchronous callbacks, and removing the streaming
worker makes stream destruction unsafe.

Call `update()` regularly to reclaim completed effect instances. Playback and
stream refill do not depend on the call frequency of `update()`.

## Example

```cpp
#include "Server/audio/audiomanager/audiomanager.h"

void audioThreadMain()
{
    AudioManager audio;
    if (!audio.isReady()) {
        // Log audio.lastError().
        return;
    }

    audio.setListenerPos(0.5f, 0.5f);
    audio.playeffect("assets/hit.mp3", 0.3f, 0.6f, 0.0f, 0.0f);
    audio.playerSound("assets/music.mp3");

    while (audioThreadShouldRun()) {
        // Consume this thread's audio command queue here.
        audio.update();
    }

    // The destructor stops the engine and releases every sound and cache.
}
```

`playeffect()` assumes normalized X/Z coordinates. Its linear attenuation is
fully loud inside `0.01` units and reaches minimum gain at the diagonal of a
unit square (`sqrt(2)`). `playerSound()` replaces the previous stream, disables
spatialization, and preserves its cursor across `pause()` / `resume()`.

On Web builds, audio assets still need to be packaged into the Emscripten file
system, and browser user-gesture audio-unlock rules still apply.

## Apple build note

The implementation builds directly as C++ on macOS. Apple mobile targets use
the provided `miniaudio_impl.mm`, because miniaudio's iOS backend contains
Objective-C. The current parent `Server/CMakeLists.txt` only collects `.cpp`,
`.cc`, and `.cxx` files, so an iOS target must additionally include the existing
`.mm` file. That build-system change cannot be made from this directory alone.

