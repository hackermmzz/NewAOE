#pragma once

namespace newaoe::render::editor {

// Runs the command-line frontend. argv must contain the executable name and
// the --offscreen switch; the switch may appear anywhere after argv[0].
// No Sokol API is touched by this function.
[[nodiscard]] int runOffscreenCli(int argc, char** argv);

}  // namespace newaoe::render::editor
