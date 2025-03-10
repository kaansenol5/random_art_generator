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
    COLOR_MODE_1,    // Original direct RGB mapping
    COLOR_MODE_2,    // Enhanced color relationships
    COLOR_MODE_MONO, // Monochrome mode
    COLOR_MODE_RAINBOW // Rainbow wave mode
} ColorMode;

// Randomness mode enum
typedef enum {
    CLASSIC_RANDOM,    // Original seeding method
    ENHANCED_RANDOM,   // Enhanced random method
    LORENZ_RANDOM     // Chaotic Lorenz attractor-based randomness
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
RandomnessMode random_mode = CLASSIC_RANDOM;  // Change default to CLASSIC_RANDOM

// FPS counter variables
int frameCount = 0;
int fps = 0;
int lastTime = 0;

// OpenGL buffer objects
GLuint VBO;
Vertex* vertices = NULL;
int max_vertices;

// Add new global variables for Lorenz system
typedef struct {
    float x, y, z;
} LorenzState;

LorenzState lorenz_state = {1.0f, 1.0f, 1.0f};

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
    
    if (random_mode == LORENZ_RANDOM) {
        // Lorenz attractor parameters - vary by pattern type
        float sigma = 10.0f;
        float rho = 28.0f;
        float beta = 8.0f / 3.0f;
        float dt = 0.001f;
        
        // Customize parameters based on pattern type
        switch(pattern_type) {
            case VORTEX:
                sigma = 16.0f;
                rho = 45.0f;
                dt = 0.002f;
                break;
            case PSYCHEDELIC:
                sigma = 14.0f;
                beta = 4.0f;
                dt = 0.003f;
                break;
            case WAVE_INTERFERENCE:
                rho = 35.0f;
                beta = 3.0f;
                break;
            case KALEIDOSCOPE:
                sigma = 12.0f;
                rho = 32.0f;
                dt = 0.0015f;
                break;
            case CELLULAR:
                sigma = 8.0f;
                rho = 20.0f;
                beta = 2.0f;
                break;
            default:
                break;
        }
        
        // Use position to influence initial conditions differently for each pattern
        float pos_influence;
        switch(pattern_type) {
            case POLAR:
                pos_influence = sqrtf(powf((float)i/Width - 0.5f, 2) + powf((float)j/Height - 0.5f, 2)) * 0.2f;
                break;
            case SYMMETRY:
                pos_influence = fabsf((float)i/Width - (float)j/Height) * 0.3f;
                break;
            case FRACTAL:
                pos_influence = ((float)i/Width * (float)j/Height) * 0.4f;
                break;
            default:
                pos_influence = ((float)i/Width + (float)j/Height) * 0.1f;
        }
        
        // Update Lorenz state with pattern-specific variations
        float dx = sigma * (lorenz_state.y - lorenz_state.x);
        float dy = lorenz_state.x * (rho - lorenz_state.z) - lorenz_state.y;
        float dz = lorenz_state.x * lorenz_state.y - beta * lorenz_state.z;
        
        // Pattern-specific state updates
        switch(pattern_type) {
            case WAVE_INTERFERENCE:
                lorenz_state.x += (dx * dt + pos_influence * sinf(time_offset * 2.0f));
                lorenz_state.y += (dy * dt + pos_influence * cosf(time_offset * 1.5f));
                lorenz_state.z += (dz * dt + pos_influence * sinf(time_offset * 0.5f));
                break;
            case VORTEX:
                lorenz_state.x += (dx * dt * (1.0f + pos_influence));
                lorenz_state.y += (dy * dt * (1.0f - pos_influence));
                lorenz_state.z += (dz * dt);
                break;
            default:
                lorenz_state.x += (dx * dt + pos_influence * sinf(time_offset));
                lorenz_state.y += (dy * dt + pos_influence * cosf(time_offset));
                lorenz_state.z += (dz * dt + pos_influence);
        }
        
        // Normalize values with pattern-specific scaling
        float scale = (pattern_type == PSYCHEDELIC) ? 30.0f : 
                     (pattern_type == VORTEX) ? 70.0f : 
                     (pattern_type == WAVE_INTERFERENCE) ? 40.0f : 50.0f;
        
        float norm_x = fabsf(fmodf(lorenz_state.x, scale)) / scale;
        float norm_y = fabsf(fmodf(lorenz_state.y, scale)) / scale;
        float norm_z = fabsf(fmodf(lorenz_state.z, scale)) / scale;
        
        // Create pattern seed with pattern-specific bit operations
        unsigned long lorenz_seed;
        switch(pattern_type) {
            case PSYCHEDELIC:
                lorenz_seed = ((unsigned long)(norm_x * 1000) << 15) ^
                             ((unsigned long)(norm_y * 1000) << 8) ^
                             ((unsigned long)(norm_z * 1000));
                break;
            case VORTEX:
                lorenz_seed = ((unsigned long)(norm_x * 1000) << 20) |
                             ((unsigned long)(norm_y * 1000) << 10) |
                             ((unsigned long)(norm_z * 1000));
                break;
            default:
                lorenz_seed = ((unsigned long)(norm_x * 1000) << 20) ^
                             ((unsigned long)(norm_y * 1000) << 10) ^
                             ((unsigned long)(norm_z * 1000));
        }
        
        return lorenz_seed ^ base_seed;
    } else if (random_mode == ENHANCED_RANDOM) {
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

// Add this helper function near the top of the file
void hsv_to_rgb(float h, float s, float v, float* r, float* g, float* b) {
    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h * 6, 2) - 1));
    float m = v - c;
    
    if(h < 1.0f/6.0f) {
        *r = c; *g = x; *b = 0;
    } else if(h < 2.0f/6.0f) {
        *r = x; *g = c; *b = 0;
    } else if(h < 3.0f/6.0f) {
        *r = 0; *g = c; *b = x;
    } else if(h < 4.0f/6.0f) {
        *r = 0; *g = x; *b = c;
    } else if(h < 5.0f/6.0f) {
        *r = x; *g = 0; *b = c;
    } else {
        *r = c; *g = 0; *b = x;
    }
    
    *r += m; *g += m; *b += m;
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
                // Original color mode code remains unchanged
                float random_shift = (float)rand() / RAND_MAX * 0.2f - 0.1f;
                r = ((pattern_seed % 256) / 255.0f + random_shift);
                g = (((pattern_seed >> 8) % 256) / 255.0f + random_shift);
                b = (((pattern_seed >> 16) % 256) / 255.0f + random_shift);
                
                // Add time-based color pulsing with random phase
                float phase_shift = (float)rand() / RAND_MAX * M_PI;
                r *= (0.7f + 0.3f * sinf(time_offset + phase_shift));
                g *= (0.7f + 0.3f * sinf(time_offset + 2.094f + phase_shift));
                b *= (0.7f + 0.3f * sinf(time_offset + 4.189f + phase_shift));
                
                // Clamp colors
                r = fmaxf(0.0f, fminf(1.0f, r));
                g = fmaxf(0.0f, fminf(1.0f, g));
                b = fmaxf(0.0f, fminf(1.0f, b));
            } else if (color_mode == COLOR_MODE_2) {
                // Enhanced color mode code remains unchanged
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
                
                // Pattern-specific color adjustments remain unchanged
            } else if (color_mode == COLOR_MODE_MONO) {
                // Generate a single grayscale value
                float intensity = (float)(pattern_seed % 1000) / 1000.0f;
                
                // Add some temporal variation
                intensity = intensity * 0.8f + 0.2f * sinf(time_offset + random_factor);
                
                // Add contrast
                float contrast = 0.4f;
                intensity = 0.5f + (intensity - 0.5f) * (1.0f + contrast);
                
                // Ensure the value is in valid range
                intensity = fmaxf(0.0f, fminf(1.0f, intensity));
                
                // Set all color channels to the same value for monochrome
                r = g = b = intensity;
            } else { // COLOR_MODE_RAINBOW
                // Base hue from position and time
                float base_hue = ((float)i/Width + (float)j/Height)/2.0f;
                
                // Add pattern-specific modifications
                switch(pattern_type) {
                    case WAVE_INTERFERENCE:
                        base_hue += sinf(time_offset * 2.0f + (float)i/Width * 10.0f) * 0.2f;
                        break;
                    case VORTEX: {
                        float dx = i - Width/2;
                        float dy = j - Height/2;
                        float angle = atan2f(dy, dx);
                        base_hue += angle / (2.0f * M_PI) + time_offset * 0.1f;
                        break;
                    }
                    case PSYCHEDELIC:
                        base_hue *= 1.0f + sinf(time_offset * 3.0f) * 0.3f;
                        break;
                    default:
                        base_hue += time_offset * 0.1f;
                }
                
                // Add some variation based on the pattern seed
                float seed_influence = (float)(pattern_seed % 1000) / 1000.0f * 0.2f;
                base_hue += seed_influence;
                
                // Wrap hue to [0,1]
                base_hue = fmodf(base_hue, 1.0f);
                if(base_hue < 0) base_hue += 1.0f;
                
                // Saturation varies with pattern
                float saturation = 0.8f + sinf(time_offset + (float)i/Width * 5.0f) * 0.2f;
                
                // Value/brightness varies with pattern and position
                float value = 0.8f + 
                    sinf((float)j/Height * 4.0f + time_offset) * 0.1f + 
                    seed_influence * 0.2f;
                
                // Convert HSV to RGB
                hsv_to_rgb(base_hue, saturation, value, &r, &g, &b);
                
                // Ensure values are in valid range
                r = fmaxf(0.0f, fminf(1.0f, r));
                g = fmaxf(0.0f, fminf(1.0f, g));
                b = fmaxf(0.0f, fminf(1.0f, b));
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
        // Reset Lorenz state when generating new seed
        lorenz_state.x = 1.0f;
        lorenz_state.y = 1.0f;
        lorenz_state.z = 1.0f;
    } else if (key >= '0' && key <= '9') {
        // Change pattern type based on numeric key
        int new_type = key - '0';
        if (new_type < PSYCHEDELIC + 1) { // Make sure it's within enum range
            pattern_type = new_type;
            printf("Switched to pattern: %d\n", pattern_type);
        }
    } else if (key == 'c' || key == 'C') {
        // Cycle through color modes
        color_mode = (ColorMode)((color_mode + 1) % 4);  // Cycles through 4 modes
        printf("Switched to %s mode\n", 
            color_mode == COLOR_MODE_1 ? "original RGB" : 
            color_mode == COLOR_MODE_2 ? "enhanced color" : 
            color_mode == COLOR_MODE_MONO ? "monochrome" : 
            "rainbow wave");
    } else if (key == 'r' || key == 'R') {
        // Cycle through random modes
        random_mode = (RandomnessMode)((random_mode + 1) % 3);
        // Reset Lorenz state when changing modes
        lorenz_state.x = 1.0f;
        lorenz_state.y = 1.0f;
        lorenz_state.z = 1.0f;
        printf("Switched to %s mode\n", 
            random_mode == CLASSIC_RANDOM ? "classic random" : 
            random_mode == ENHANCED_RANDOM ? "enhanced random" : 
            "Lorenz chaos");
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