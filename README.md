# Random Art Generator 🎨

⚠️ **EPILEPSY WARNING**: This program generates rapidly changing patterns and colors that may trigger seizures in people with photosensitive epilepsy. Please exercise caution.

## Overview

A mesmerizing art generator that creates real-time visual patterns using OpenGL. The program generates unique, animated patterns through various algorithms and can output both real-time animations and video files.

## Features

- Multiple pattern types:
  - Original
  - Polar
  - Trigonometric
  - Fractal
  - Wave Interference
  - Wave2
  - Symmetry
  - Vortex
  - Kaleidoscope
  - Cellular
  - Psychedelic

- Different color modes:
  - Classic RGB
  - Enhanced Color
  - Monochrome
  - Rainbow Wave

- Multiple randomization methods:
  - Classic Random
  - Enhanced Random
  - Lorenz Chaos

- Multi-threaded rendering for improved performance
- Video output capability
- Real-time interactive controls

## Prerequisites

- OpenGL
- GLUT (OpenGL Utility Toolkit)
- FFmpeg libraries (for video output):
  - libavcodec
  - libavformat
  - libavutil
  - libswscale

## Installation

1. Install the required dependencies
2. Clone this repository
3. Compile using make:
```bash
make
```

## Usage

### Basic Command

```bash
./artmaker <width> <height> <pixelsize> [options]
```

### Options

- `-p, --pattern <type>` - Set pattern type (original, polar, trig, fractal, wave, wave2, symmetry, vortex, kaleidoscope, cellular, psychedelic)
- `--fill-rects` - Fill rectangles instead of outlines
- `-t, --threads <num>` - Set number of threads (1-16, default: 4)
- `-out-mode <sec> <fps>` - Generate video output instead of real-time display
- `-o, --output <file>` - Specify output video filename (default: auto-generated)

### Interactive Controls

When running in real-time mode:

- `ESC` - Exit program
- `Space` - Generate new random seed
- `0-9` - Change pattern type
- `C` - Cycle through color modes
- `R` - Cycle through randomness modes
- `+/-` - Increase/decrease number of threads

### Examples

Real-time display with psychedelic pattern:
```bash
./artmaker 800 600 10 -p psychedelic --fill-rects -t 8
```

Generate a 5-second video at 30fps:
```bash
./artmaker 800 600 10 -p wave -out-mode 5 30 -o output.mp4
```

## Performance Notes

- The program utilizes multi-threading to improve rendering performance
- Larger pixel sizes will result in better performance but lower resolution
- Video generation mode may require significant CPU resources


## License

This project is open source and available under the MIT License.


# Issues
1. I *still* do not know why this works
2. It has a ram problem
3. It is kinda slow, but okay. 
