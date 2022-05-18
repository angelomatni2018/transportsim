#ifndef FRAME_DATA
#define FRAME_DATA

#include "simulator/imports.h"

namespace world {

struct FrameData {
  const std::chrono::duration<int64_t, std::milli> FRAME_DURATION = std::chrono::milliseconds(50);
  std::chrono::_V2::system_clock::time_point lastFrame = std::chrono::system_clock::now();
  std::chrono::nanoseconds rollingDuration = lastFrame - lastFrame;
  int64_t frameNumber = 0;

  void NextFrame();
};

} // namespace world

#endif