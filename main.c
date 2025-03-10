#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

// Pattern type enum
typedef enum {
    ORIGINAL,
    POLAR,
    TRIGONOMETRIC,
    FRACTAL,
    WAVE_INTERFERENCE,
    SYMMETRY,
    WAVE2,
    VORTEX,
    KALEIDOSCOPE,
    PSYCHEDELIC,
    CELLULAR
} PatternType;

// Color mode enum
typedef enum {
    COLOR_MODE_1,  // Original direct RGB mapping
    COLOR_MODE_2   // Enhanced color relationships
} ColorMode;

// Randomness mode enum
typedef enum {
    CLASSIC_RANDOM,    // Original seeding method
    ENHANCED_RANDOM    // New enhanced random method
} RandomnessMode;

// Vertex structure
typedef struct {
    float x, y;           // Position
    float r, g, b, a;     // Color
} Vertex;

// Global variables
int Width, Height, tilesize;
bool fill_rects = false;
unsigned long randseed;
PatternType pattern_type;
ColorMode color_mode = COLOR_MODE_1;  // Default to original color mode
RandomnessMode random_mode = ENHANCED_RANDOM;  // Default to enhanced random mode

// FPS counter variables
int frameCount = 0;
int fps = 0;
int lastTime = 0;

// OpenGL buffer objects
GLuint VBO;
Vertex* vertices = NULL;
int max_vertices;

// Function to parse pattern type from string
PatternType parse_pattern_type(const char* pattern_str) {
    if (strcmp(pattern_str, "original") == 0) return ORIGINAL;
    if (strcmp(pattern_str, "polar") == 0) return POLAR;
    if (strcmp(pattern_str, "trig") == 0) return TRIGONOMETRIC;
    if (strcmp(pattern_str, "fractal") == 0) return FRACTAL;
    if (strcmp(pattern_str, "wave") == 0) return WAVE_INTERFERENCE;
    if (strcmp(pattern_str, "symmetry") == 0) return SYMMETRY;
    if (strcmp(pattern_str, "wave2") == 0) return WAVE2;
    if (strcmp(pattern_str, "vortex") == 0) return VORTEX;
    if (strcmp(pattern_str, "kaleidoscope") == 0) return KALEIDOSCOPE;
    if (strcmp(pattern_str, "cellular") == 0) return CELLULAR;
    if (strcmp(pattern_str, "psychedelic") == 0) return PSYCHEDELIC;
    
    printf("Invalid pattern type '%s'. Using default (original).\n", pattern_str);
    return ORIGINAL;
}

void print_usage(const char* program_name) {
    printf("USAGE: %s <width> <height> <pixelsize> [options]\n\n", program_name);
    printf("Options:\n");
    printf("  -p, --pattern <type>   Set pattern type (original, polar, trig, fractal, wave, wave2, symmetry,\n");
    printf("                         vortex, kaleidoscope, cellular, psychedelic)\n");
    printf("  --fill-rects           Fill rectangles instead of outlines\n");
    printf("\nExample: %s 800 600 10 -p psychedelic --fill-rects\n", program_name);
}

// OpenGL initialization and rendering functions
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

    // Calculate maximum number of vertices needed
    int tiles_x = (Width + tilesize - 1) / tilesize;
    int tiles_y = (Height + tilesize - 1) / tilesize;
    max_vertices = tiles_x * tiles_y * 6;  // 6 vertices per quad (2 triangles)
    
    // Allocate vertex buffer
    vertices = (Vertex*)malloc(max_vertices * sizeof(Vertex));
    
    // Create and bind VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, max_vertices * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
}

void clearScreen() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void addQuad(Vertex* vertices, int* vertex_count, 
             float x, float y, float width, float height,
             float r, float g, float b, float a) {
    // First triangle
    vertices[*vertex_count].x = x;
    vertices[*vertex_count].y = y;
    vertices[*vertex_count].r = r;
    vertices[*vertex_count].g = g;
    vertices[*vertex_count].b = b;
    vertices[*vertex_count].a = a;
    (*vertex_count)++;

    vertices[*vertex_count].x = x + width;
    vertices[*vertex_count].y = y;
    vertices[*vertex_count].r = r;
    vertices[*vertex_count].g = g;
    vertices[*vertex_count].b = b;
    vertices[*vertex_count].a = a;
    (*vertex_count)++;

    vertices[*vertex_count].x = x + width;
    vertices[*vertex_count].y = y + height;
    vertices[*vertex_count].r = r;
    vertices[*vertex_count].g = g;
    vertices[*vertex_count].b = b;
    vertices[*vertex_count].a = a;
    (*vertex_count)++;

    // Second triangle
    vertices[*vertex_count].x = x;
    vertices[*vertex_count].y = y;
    vertices[*vertex_count].r = r;
    vertices[*vertex_count].g = g;
    vertices[*vertex_count].b = b;
    vertices[*vertex_count].a = a;
    (*vertex_count)++;

    vertices[*vertex_count].x = x + width;
    vertices[*vertex_count].y = y + height;
    vertices[*vertex_count].r = r;
    vertices[*vertex_count].g = g;
    vertices[*vertex_count].b = b;
    vertices[*vertex_count].a = a;
    (*vertex_count)++;

    vertices[*vertex_count].x = x;
    vertices[*vertex_count].y = y + height;
    vertices[*vertex_count].r = r;
    vertices[*vertex_count].g = g;
    vertices[*vertex_count].b = b;
    vertices[*vertex_count].a = a;
    (*vertex_count)++;
}

// Function to generate pattern seed based on coordinates and pattern type
unsigned long calculate_pattern_seed(int i, int j, PatternType pattern_type, float time_offset, int Width, int Height, unsigned long base_seed) {
    int centerX = Width / 2;
    int centerY = Height / 2;
    
    if (random_mode == ENHANCED_RANDOM) {
        // Add some global randomness factors
        float random_factor = (float)rand() / RAND_MAX;
        float noise = (random_factor * 2.0f - 1.0f) * 0.2f; // Random noise between -0.2 and 0.2
        
        switch(pattern_type) {
            case ORIGINAL:
                return base_seed | (i*j & (int)(i*cos(time_offset + noise)*j)) ^ ((int)(random_factor * 1000));
                
            case POLAR: {
                float dx = i - centerX;
                float dy = j - centerY;
                float distance = sqrtf(dx*dx + dy*dy);
                float angle = atan2f(dy, dx);
                distance *= (sin(time_offset) + noise) * 2.0f;
                angle += time_offset + random_factor * M_PI;
                return base_seed | ((int)(distance*10) & (int)(angle*1000)) ^ ((int)(random_factor * 1000));
            }
            
            case TRIGONOMETRIC: {
                float freq_mod = 0.05f * (1.0f + noise);
                return base_seed | ((int)(sinf(i*freq_mod + time_offset + random_factor)*100) * 
                                  (int)(cosf(j*freq_mod - time_offset + random_factor)*100));
            }
            
            case FRACTAL: {
                float scale = 8.0f + sinf(time_offset + noise) * 4.0f;
                scale *= (1.0f + random_factor * 0.3f);  // Random scale variation
                return base_seed | ((i*j & i*j) ^ ((int)(i/scale)*(int)(j/scale) & 
                                  (int)(i/scale)*(int)(j/scale))) ^ ((int)(random_factor * 1000));
            }
            
            case WAVE_INTERFERENCE: {
                float freq_var = 1.0f + noise;
                float wave1 = sinf(i*0.05f*freq_var + j*0.05f*freq_var + time_offset + random_factor) * 100;
                float wave2 = sinf(i*0.08f*freq_var - j*0.03f*freq_var - time_offset*1.5f + random_factor) * 100;
                float wave3 = sinf(sqrtf(powf(i-Width/2, 2) + powf(j-Height/2, 2)) * 0.1f*freq_var + time_offset*0.5f) * 100;
                return base_seed | ((int)(wave1 + wave2 + wave3)) ^ ((int)(random_factor * 1000));
            }
            
            case SYMMETRY: {
                int x = i % (Width/2);
                int y = j % (Height/2);
                if (i >= Width/2) x = Width/2 - x;
                if (j >= Height/2) y = Height/2 - y;
                x = (int)(x * (1.0f + 0.5f * sinf(time_offset + noise)));
                y = (int)(y * (1.0f + 0.5f * cosf(time_offset + noise)));
                return base_seed | (x*y & x*y) ^ ((int)(random_factor * 1000));
            }

            case VORTEX: {
                float dx = i - centerX;
                float dy = j - centerY;
                float angle = atan2f(dy, dx);
                float distance = sqrtf(dx*dx + dy*dy);
                float spiral = angle + distance * 0.02f + time_offset + random_factor;
                float vortex = sinf(spiral) * cosf(distance * 0.05f + time_offset + random_factor);
                return base_seed | ((int)(vortex * 1000) ^ (int)(distance)) ^ ((int)(random_factor * 1000));
            }

            case KALEIDOSCOPE: {
                float dx = i - centerX;
                float dy = j - centerY;
                float angle = fmodf(atan2f(dy, dx) + time_offset + random_factor, M_PI/4);
                float distance = sqrtf(dx*dx + dy*dy);
                float kaleid = sinf(angle * 8 + distance * 0.1f + random_factor) * cosf(distance * 0.05f - time_offset * 2 + random_factor);
                return base_seed | ((int)(kaleid * 1000) * (int)(distance * 0.1f)) ^ ((int)(random_factor * 1000));
            }

            case CELLULAR: {
                float cell_size = 50.0f * (1.0f + 0.5f * sinf(time_offset + noise));
                int cell_x = (int)(i / cell_size);
                int cell_y = (int)(j / cell_size);
                float dx = i - (cell_x + 0.5f) * cell_size;
                float dy = j - (cell_y + 0.5f) * cell_size;
                float dist = sqrtf(dx*dx + dy*dy);
                return base_seed | ((cell_x * 17 + cell_y * 31) ^ (int)(dist * sinf(time_offset * 2 + random_factor)));
            }

            case PSYCHEDELIC: {
                float freq1 = 0.03f * (1.0f + 0.5f * sinf(time_offset + noise));
                float freq2 = 0.02f * (1.0f + 0.5f * cosf(time_offset + noise));
                float wave1 = sinf(i * freq1 + time_offset + random_factor) * cosf(j * freq2);
                float wave2 = cosf(i * freq2 - time_offset - random_factor) * sinf(j * freq1);
                float wave3 = sinf(sqrtf((i-centerX)*(i-centerX) + (j-centerY)*(j-centerY)) * 0.1f + random_factor);
                return base_seed | ((int)(wave1 * 1000) ^ (int)(wave2 * 1000) ^ (int)(wave3 * 1000));
            }

            default:
                return base_seed;
        }
    } else {
        // Classic random mode
        switch(pattern_type) {
            case ORIGINAL:
                return base_seed | (i*j & (int)(i*cos(time_offset)*j));
                
            case POLAR: {
                float dx = i - centerX;
                float dy = j - centerY;
                float distance = sqrtf(dx*dx + dy*dy);
                float angle = atan2f(dy, dx);
                distance *= sin(time_offset) * 2.0f;
                angle += time_offset;
                return base_seed | ((int)(distance*10) & (int)(angle*1000));
            }
                
            case TRIGONOMETRIC:
                return base_seed | ((int)(sinf(i*0.05 + time_offset)*100) * 
                                  (int)(cosf(j*0.05 - time_offset)*100));
                
            case FRACTAL: {
                float scale = 8.0f + sinf(time_offset) * 4.0f;
                return base_seed | ((i*j & i*j) ^ ((int)(i/scale)*(int)(j/scale) & 
                                  (int)(i/scale)*(int)(j/scale)));
            }
                
            case WAVE_INTERFERENCE: {
                float wave1 = sinf(i*0.05 + j*0.05 + time_offset) * 100;
                float wave2 = sinf(i*0.08 - j*0.03 - time_offset*1.5) * 100;
                float wave3 = sinf(sqrtf(powf(i-Width/2, 2) + powf(j-Height/2, 2)) * 0.1 + time_offset*0.5) * 100;
                return base_seed | ((int)(wave1 + wave2 + wave3));
            }
                
            case SYMMETRY: {
                int x = i % (Width/2);
                int y = j % (Height/2);
                if (i >= Width/2) x = Width/2 - x;
                if (j >= Height/2) y = Height/2 - y;
                x = (int)(x * (1.0f + 0.5f * sinf(time_offset)));
                y = (int)(y * (1.0f + 0.5f * cosf(time_offset)));
                return base_seed | (x*y & x*y);
            }

            case WAVE2: {
                float wave_x = sinf(j * 0.1f + time_offset * 2.0f) * 10;
                float wave_y = cosf(i * 0.1f + time_offset * 2.0f) * 10;
                int x_new = i + (int)wave_x;
                int y_new = j + (int)wave_y;
                return base_seed | (x_new * y_new);
            }

            case VORTEX: {
                float dx = i - centerX;
                float dy = j - centerY;
                float angle = atan2f(dy, dx);
                float distance = sqrtf(dx*dx + dy*dy);
                float spiral = angle + distance * 0.02f + time_offset;
                float vortex = sinf(spiral) * cosf(distance * 0.05f + time_offset);
                return base_seed | ((int)(vortex * 1000) ^ (int)(distance));
            }

            case KALEIDOSCOPE: {
                float dx = i - centerX;
                float dy = j - centerY;
                float angle = fmodf(atan2f(dy, dx) + time_offset, M_PI/4);
                float distance = sqrtf(dx*dx + dy*dy);
                float kaleid = sinf(angle * 8 + distance * 0.1f) * cosf(distance * 0.05f - time_offset * 2);
                return base_seed | ((int)(kaleid * 1000) * (int)(distance * 0.1f));
            }

            case CELLULAR: {
                float cell_size = 50.0f * (1.0f + 0.5f * sinf(time_offset));
                int cell_x = (int)(i / cell_size);
                int cell_y = (int)(j / cell_size);
                float dx = i - (cell_x + 0.5f) * cell_size;
                float dy = j - (cell_y + 0.5f) * cell_size;
                float dist = sqrtf(dx*dx + dy*dy);
                return base_seed | ((cell_x * 17 + cell_y * 31) ^ (int)(dist * sinf(time_offset * 2)));
            }

            case PSYCHEDELIC: {
                float freq1 = 0.03f * (1.0f + 0.5f * sinf(time_offset));
                float freq2 = 0.02f * (1.0f + 0.5f * cosf(time_offset));
                float wave1 = sinf(i * freq1 + time_offset) * cosf(j * freq2);
                float wave2 = cosf(i * freq2 - time_offset) * sinf(j * freq1);
                float wave3 = sinf(sqrtf((i-centerX)*(i-centerX) + (j-centerY)*(j-centerY)) * 0.1f);
                return base_seed | ((int)(wave1 * 1000) ^ (int)(wave2 * 1000) ^ (int)(wave3 * 1000));
            }

            default:
                return base_seed;
        }
    }
    return base_seed;
}

// Function to generate and render art
void generateArt(unsigned long seed, PatternType pattern_type) {
    // FPS calculation
    frameCount++;
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    if (currentTime - lastTime > 1000) {
        fps = frameCount * 1000 / (currentTime - lastTime);
        frameCount = 0;
        lastTime = currentTime;
        printf("FPS: %d\n", fps);
    }
    
    srand(seed);
    
    static float time_offset = 0.0f;
    time_offset += 0.05f;
    
    clearScreen();
    
    int vertex_count = 0;
    
    for(int i = 0; i < Width; i += tilesize) {
        for(int j = 0; j < Height; j += tilesize) {
            // Generate random factors for each tile
            float random_factor = (float)rand() / RAND_MAX;
            float noise = (random_factor * 2.0f - 1.0f) * 0.2f;
            
            unsigned long pattern_seed = calculate_pattern_seed(i, j, pattern_type, time_offset, Width, Height, seed);
            srand(pattern_seed);
            
            float r, g, b;
            
            if (color_mode == COLOR_MODE_1) {
                // Enhanced original color generation with more randomness
                float random_shift = (float)rand() / RAND_MAX * 0.2f - 0.1f;  // Random shift between -0.1 and 0.1
                r = ((pattern_seed % 256) / 255.0f + random_shift);
                g = (((pattern_seed >> 8) % 256) / 255.0f + random_shift);
                b = (((pattern_seed >> 16) % 256) / 255.0f + random_shift);
                
                // Add time-based color pulsing with random phase
                float phase_shift = (float)rand() / RAND_MAX * M_PI;
                r *= (0.7f + 0.3f * sinf(time_offset + phase_shift));
                g *= (0.7f + 0.3f * sinf(time_offset + 2.094f + phase_shift));
                b *= (0.7f + 0.3f * sinf(time_offset + 4.189f + phase_shift));
                
                // Clamp colors to valid range
                r = fmaxf(0.0f, fminf(1.0f, r));
                g = fmaxf(0.0f, fminf(1.0f, g));
                b = fmaxf(0.0f, fminf(1.0f, b));
            } else {
                // Enhanced color generation (COLOR_MODE_2)
                float base = (float)(pattern_seed % 1000) / 1000.0f;
                r = base;
                g = fmodf(base + 0.33f + 0.1f * sinf(time_offset + random_factor), 1.0f);
                b = fmodf(base + 0.66f + 0.1f * cosf(time_offset + random_factor), 1.0f);
                
                float contrast = 0.3f;
                r = 0.5f + (r - 0.5f) * (1.0f + contrast);
                g = 0.5f + (g - 0.5f) * (1.0f + contrast);
                b = 0.5f + (b - 0.5f) * (1.0f + contrast);
                
                r = fmaxf(0.0f, fminf(1.0f, r));
                g = fmaxf(0.0f, fminf(1.0f, g));
                b = fmaxf(0.0f, fminf(1.0f, b));
                
                switch(pattern_type) {
                    case PSYCHEDELIC: {
                        float shift = time_offset * 0.1f + random_factor * 0.1f;
                        r = fmodf(r + shift, 1.0f);
                        g = fmodf(g + shift, 1.0f);
                        b = fmodf(b + shift, 1.0f);
                        break;
                    }
                    case VORTEX: {
                        float dx = i - Width/2;
                        float dy = j - Height/2;
                        float dist = sqrtf(dx*dx + dy*dy) / (Width/2);
                        float rot = dist * 0.2f + random_factor * 0.1f;
                        r = fmodf(r + rot, 1.0f);
                        g = fmodf(g + rot, 1.0f);
                        b = fmodf(b + rot, 1.0f);
                        break;
                    }
                    default:
                        break;
                }
            }
            
            // Apply wave distortion to the tile position for WAVE2 pattern
            float x_new = i;
            float y_new = j;
            
            if (pattern_type == WAVE2) {
                // Sinusoidal Tile Distortion
                float wave_x = sinf(j * 0.1f + time_offset * 2.0f + random_factor) * 10;
                float wave_y = cosf(i * 0.1f + time_offset * 2.0f + random_factor) * 10;
                x_new += wave_x;
                y_new += wave_y;
            }
            
            addQuad(vertices, &vertex_count, x_new, y_new, tilesize, tilesize, r, g, b, 1.0f);
        }
    }
    
    // Update VBO with new vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_count * sizeof(Vertex), vertices);
    
    // Set up vertex attributes
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    
    glVertexPointer(2, GL_FLOAT, sizeof(Vertex), (void*)0);
    glColorPointer(4, GL_FLOAT, sizeof(Vertex), (void*)(2 * sizeof(float)));
    
    // Draw all quads in a single call
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    
    // Cleanup
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
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
    } else if (key >= '0' && key <= '9') {
        // Change pattern type based on numeric key
        int new_type = key - '0';
        if (new_type < PSYCHEDELIC + 1) { // Make sure it's within enum range
            pattern_type = new_type;
            printf("Switched to pattern: %d\n", pattern_type);
        }
    } else if (key == 'c' || key == 'C') {
        // Toggle color mode
        color_mode = (color_mode == COLOR_MODE_1) ? COLOR_MODE_2 : COLOR_MODE_1;
        printf("Switched to color mode %d\n", color_mode + 1);
    } else if (key == 'r' || key == 'R') {
        // Toggle random mode
        random_mode = (random_mode == CLASSIC_RANDOM) ? ENHANCED_RANDOM : CLASSIC_RANDOM;
        printf("Switched to %s mode\n", random_mode == CLASSIC_RANDOM ? "classic random" : "enhanced random");
    } else {
        // Generate new art with current pattern type but different seed
        randseed = randseed | (key * 10);
    }
}

void idle(void) {
    generateArt(randseed, pattern_type);
}

int main(int argc, char *argv[]) {
    if(argc < 4) {
        print_usage(argv[0]);
        exit(1);
    }
    
    Width = atoi(argv[1]);
    Height = atoi(argv[2]);
    tilesize = atoi(argv[3]);
    
    // Parse additional options
    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--pattern") == 0) {
            if (i + 1 < argc) {
                pattern_type = parse_pattern_type(argv[i + 1]);
                i++;
            } else {
                printf("Missing pattern type after -p option.\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "--fill-rects") == 0) {
            fill_rects = true;
        } else {
            printf("Unknown option '%s'\n", argv[i]);
            exit(1);
        }
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