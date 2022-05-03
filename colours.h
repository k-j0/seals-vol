#pragma once

#define ESC "\033"

// Turn the following line to `#if 0` to remove colours entirely from showing up in outputs
// Already disabled by default when building with MinGW
#ifndef __MINGW32__

// Colours that can be prepended to console outputs
#define COLOUR(x) ESC "[0;" #x ";40m" // 40: black bg
#define WHITE ESC "[0m" // reset to defaults
#define BLUE COLOUR(36)
#define YELLOW COLOUR(33)
#define RED COLOUR(31)

#else

// Disable coloured outputs
#define COLOUR(x)
#define WHITE
#define BLUE
#define YELLOW
#define RED

#endif
