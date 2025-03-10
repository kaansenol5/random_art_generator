# Random "art" generator
# YOU MIGHT HAVE A SEIZURE IF YOU ARE EPILEPTIC 
# TAKE CARE
# I AM NOT RESPONSIBLE

i have no idea how & why this works, i made this accidentally

# Compiling
 ~~~
 make
 ~~~
you need opengl and glut. 
# Usage

USAGE: ./artmaker <width> <height> <pixelsize> [options]

Options:
  -p, --pattern <type>   Set pattern type (original, polar, trig, fractal, wave, wave2, symmetry,
                         vortex, kaleidoscope, cellular, psychedelic)
  --fill-rects           Fill rectangles instead of outlines
  -t, --threads <num>    Set number of threads (1-16, default: 4)
  -out-mode <sec> <fps>  Generate video output instead of real-time display
  -o, --output <file>    Specify output video filename (default: auto-generated)

Controls (Real-time mode only):
  ESC                    Exit program
  Space                  Generate new random seed
  0-9                    Change pattern type
  C                      Cycle through color modes
  R                      Cycle through randomness modes
  +/-                    Increase/decrease number of threads


pattern can be original, polar, fractal, trig, wave, symmetry and more. 
press R to change some seeding logic, it gives a fuzzy look
press C to change colors, makes it more vibrant
each key 1-9 and 0 is mapped to a pattern
default is original
smaller pixel size = slower generation, higher detail
press space to change the thing you see
other key presses change the seed. this feature kinda sucks. 
press space to make it look good again

# Issues
1. I *still* do not know why this works
2. It has a ram problem
3. It is kinda slow, but okay. 
