#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

// Pattern type enum
typedef enum {
    ORIGINAL,
    POLAR,
    TRIGONOMETRIC,
    FRACTAL,
    WAVE_INTERFERENCE,
    SYMMETRY
} PatternType;

// Global variables
int Width, Height, tilesize, mode, seizure_mode, video_mode;
unsigned long randseed;
PatternType pattern_type;

// OpenGL rectangle drawing functions
void initGL(int width, int height, int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width, height);
    glutCreateWindow("Artmaker");
    
    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void clearScreen() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void drawRect(int x, int y, int width, int height, 
              unsigned char r, unsigned char g, unsigned char b, unsigned char a, 
              int filled) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;
    float af = a / 255.0f;
    
    glColor4f(rf, gf, bf, af);
    
    if (filled) {
        glBegin(GL_QUADS);
        glVertex2i(x, y);
        glVertex2i(x + width, y);
        glVertex2i(x + width, y + height);
        glVertex2i(x, y + height);
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        glVertex2i(x, y);
        glVertex2i(x + width, y);
        glVertex2i(x + width, y + height);
        glVertex2i(x, y + height);
        glEnd();
    }
}

// Function to generate and render art
void generateArt(unsigned long seed, PatternType pattern_type) {
    srand(seed);
    
    // Center coordinates for polar patterns
    int centerX = Width / 2;
    int centerY = Height / 2;
    
    // Time-based variables for video mode
    static float time_offset = 0.0f;
    if (video_mode) {
        time_offset += 0.05f;  // Controls animation speed
    }
    
    clearScreen();
    
    for(int i = 0; i < Width; i += tilesize) {
        for(int j = 0; j < Height; j += tilesize) {
            // Different seed calculation based on pattern type
            unsigned long pattern_seed;
            
            switch(pattern_type) {
                case ORIGINAL:
                    if (video_mode) {
                        pattern_seed = seed | (i*j & (int)(i*cos(time_offset)*j));
                    } else {
                        pattern_seed = seed | (i*j & i*j);
                    }
                    break;
                case POLAR: {
                    float dx = i - centerX;
                    float dy = j - centerY;
                    float distance = sqrtf(dx*dx + dy*dy);
                    float angle = atan2f(dy, dx);
                    if (video_mode) {
                        distance *= sin(time_offset) * 2.0f;
                        angle += time_offset;
                    }
                    pattern_seed = seed | ((int)(distance*10) & (int)(angle*1000));
                    break;
                }
                case TRIGONOMETRIC:
                    if (video_mode) {
                        pattern_seed = seed | ((int)(sinf(i*0.05 + time_offset)*100) * 
                                            (int)(cosf(j*0.05 - time_offset)*100));
                    } else {
                        pattern_seed = seed | ((int)(sinf(i*0.05)*100) * (int)(cosf(j*0.05)*100));
                    }
                    break;
                case FRACTAL:
                    if (video_mode) {
                        float scale = 8.0f + sinf(time_offset) * 4.0f;
                        pattern_seed = seed | ((i*j & i*j) ^ ((int)(i/scale)*(int)(j/scale) & 
                                            (int)(i/scale)*(int)(j/scale)));
                    } else {
                        pattern_seed = seed | ((i*j & i*j) ^ ((i/8)*(j/8) & (i/8)*(j/8)));
                    }
                    break;
                case WAVE_INTERFERENCE: {
                    float wave1, wave2, wave3;
                    if (video_mode) {
                        wave1 = sinf(i*0.05 + j*0.05 + time_offset) * 100;
                        wave2 = sinf(i*0.08 - j*0.03 - time_offset*1.5) * 100;
                        wave3 = sinf(sqrtf(powf(i-Width/2, 2) + powf(j-Height/2, 2)) * 0.1 + time_offset*0.5) * 100;
                    } else {
                        wave1 = sinf(i*0.05 + j*0.05) * 100;
                        wave2 = sinf(i*0.08 - j*0.03) * 100;
                        wave3 = sinf(sqrtf(powf(i-Width/2, 2) + powf(j-Height/2, 2)) * 0.1) * 100;
                    }
                    pattern_seed = seed | ((int)(wave1 + wave2 + wave3));
                    break;
                }
                case SYMMETRY: {
                    int x = i % (Width/2);
                    int y = j % (Height/2);
                    if (i >= Width/2) x = Width/2 - x;
                    if (j >= Height/2) y = Height/2 - y;
                    if (video_mode) {
                        x = (int)(x * (1.0f + 0.5f * sinf(time_offset)));
                        y = (int)(y * (1.0f + 0.5f * cosf(time_offset)));
                    }
                    pattern_seed = seed | (x*y & x*y);
                    break;
                }
            }
            
            srand(pattern_seed);
            
            // Generate color with time-based evolution in video mode
            unsigned char r, g, b;
            if (video_mode) {
                r = (unsigned char)((rand() % 256) * (0.7f + 0.3f * sinf(time_offset)));
                g = (unsigned char)((rand() % 256) * (0.7f + 0.3f * sinf(time_offset + 2.094f)));
                b = (unsigned char)((rand() % 256) * (0.7f + 0.3f * sinf(time_offset + 4.189f)));
            } else {
                r = rand() % 256;
                g = rand() % 256;
                b = rand() % 256;
            }
            unsigned char a = 255; // Full opacity
            
            // Draw rectangle
            drawRect(i, j, tilesize, tilesize, r, g, b, a, mode == 1);
        }
    }
    
    glutSwapBuffers();
}

// GLUT callback functions
void display(void) {
    generateArt(randseed, pattern_type);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) { // ESC key
        exit(0);
    } else if (key == ' ') {
        randseed = (unsigned long)time(NULL);
        generateArt(randseed, pattern_type);
    } else if (key >= '0' && key <= '5') {
        // Change pattern type based on numeric key
        pattern_type = key - '0';
        generateArt(randseed, pattern_type);
    } else {
        // Generate new art with current pattern type but different seed
        generateArt(randseed | (key * 10), pattern_type);
    }
}

void idle(void) {
    if (seizure_mode || video_mode) {
        generateArt(randseed, pattern_type);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 7) {
        printf("USAGE: randomart <width> <height> <pixelsize> <mode> <pattern_type> <video_mode> [seizure_mode]\n");
        printf("Mode is either 0 or 1 (0 = outline, 1 = filled)\n");
        printf("Pattern type is 0-5:\n");
        printf("  0 = ORIGINAL\n");
        printf("  1 = POLAR\n");
        printf("  2 = TRIGONOMETRIC\n");
        printf("  3 = FRACTAL\n");
        printf("  4 = WAVE_INTERFERENCE\n");
        printf("  5 = SYMMETRY\n");
        printf("Video mode is 0 or 1 (0 = static, 1 = video animation)\n");
        printf("Optional seizure_mode is 0 or 1 (default: 0)\n");
        exit(1);
    }
    
    Width = atoi(argv[1]);
    Height = atoi(argv[2]);
    tilesize = atoi(argv[3]);
    mode = atoi(argv[4]);
    pattern_type = atoi(argv[5]);
    video_mode = atoi(argv[6]);
    seizure_mode = (argc > 7) ? atoi(argv[7]) : 0;
    
    // Validate pattern_type
    if (pattern_type < 0 || pattern_type > 5) {
        printf("Invalid pattern type. Must be between 0 and 5.\n");
        exit(1);
    }
    
    printf("Width: %d, Height: %d, Tile size: %d, Pattern type: %d\n", Width, Height, tilesize, pattern_type);

    randseed = (unsigned long)time(NULL);
    
    // Initialize OpenGL
    initGL(Width, Height, argc, argv);
    
    // Register callbacks
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);
    
    // Start the main loop
    glutMainLoop();
    
    return 0;
}