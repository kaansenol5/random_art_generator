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
~~~
./randomart width height pixelsize -p pattern
~~~

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
2. It has a using 1.3GB's of ram problem
3. It is kinda slow
