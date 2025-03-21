#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>  // Add pthread library for thread support
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <sys/time.h>
#define MAX_THREADS 16
// Video output related structures
typedef struct {
    AVFormatContext *format_context;
    AVCodecContext *codec_context;
    AVStream *video_stream;
    AVFrame *frame;
    uint8_t *frame_buffer;
    struct SwsContext *sws_context;
    int frame_count;
    int total_frames;
    double start_time;
} VideoContext;

// Output mode enum
typedef enum {
    REALTIME_MODE,
    VIDEO_MODE
} OutputMode;

// Output configuration
typedef struct {
    OutputMode mode;
    int duration_seconds;
    int framerate;
    char *output_filename;
} OutputConfig;

// Pattern type enum
typedef enum {
    ORIGINAL,
    POLAR,
    TRIGONOMETRIC,
    FRACTAL,
    WAVE_INTERFERENCE,
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
    COLOR_MODE_MONO  // Monochrome mode
} ColorMode;

// Randomness mode enum
typedef enum {
    CLASSIC_RANDOM,    // Original seeding method
    ENHANCED_RANDOM    // Enhanced random method
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
OutputConfig output_config = {REALTIME_MODE, 0, 0, NULL};  // Default to realtime mode

// FPS counter variables
int frameCount = 0;
int fps = 0;
int lastTime = 0;

// Texture related variables
GLuint texture_id;
uint8_t* texture_data = NULL;  // RGB texture data
uint8_t* thread_texture_buffers[MAX_THREADS];  // Per-thread texture buffers

// OpenGL buffer objects
GLuint VBO, IBO;
Vertex* vertices = NULL;
GLuint* indices = NULL;
int max_vertices;
int max_indices;

// Thread-related variables
#define MAX_THREADS 16
int num_threads = 8;  // Default to 8 threads
pthread_mutex_t vertex_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t texture_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread work structure
typedef struct {
    int start_row;
    int end_row;
    unsigned long seed;
    PatternType pattern_type;
    float time_offset;
    uint8_t* texture_buffer;  // Local texture buffer for this thread
} ThreadWork;

// Function to parse random mode from string
RandomnessMode parse_random_mode(const char* mode_str) {
    if (strcmp(mode_str, "classic") == 0) return CLASSIC_RANDOM;
    if (strcmp(mode_str, "enhanced") == 0) return ENHANCED_RANDOM;
    
    printf("Invalid random mode '%s'. Using default (classic).\n", mode_str);
    return CLASSIC_RANDOM;
}

// Function to parse color mode from string
ColorMode parse_color_mode(const char* mode_str) {
    if (strcmp(mode_str, "rgb") == 0) return COLOR_MODE_1;
    if (strcmp(mode_str, "enhanced") == 0) return COLOR_MODE_2;
    if (strcmp(mode_str, "mono") == 0) return COLOR_MODE_MONO;
    
    printf("Invalid color mode '%s'. Using default (rgb).\n", mode_str);
    return COLOR_MODE_1;
}

// Function to parse pattern type from string
PatternType parse_pattern_type(const char* pattern_str) {
    if (strcmp(pattern_str, "original") == 0) return ORIGINAL;
    if (strcmp(pattern_str, "polar") == 0) return POLAR;
    if (strcmp(pattern_str, "trig") == 0) return TRIGONOMETRIC;
    if (strcmp(pattern_str, "fractal") == 0) return FRACTAL;
    if (strcmp(pattern_str, "wave") == 0) return WAVE_INTERFERENCE;
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
    printf("  -t, --threads <num>    Set number of threads (1-%d, default: 4)\n", MAX_THREADS);
    printf("  -out-mode <sec> <fps>  Generate video output instead of real-time display\n");
    printf("  -o, --output <file>    Specify output video filename (default: auto-generated)\n");
    printf("  -r, --random <mode>    Set random mode (classic, enhanced)\n");
    printf("  -c, --color <mode>     Set color mode (rgb, enhanced, mono)\n");
    printf("\nControls (Real-time mode only):\n");
    printf("  ESC                    Exit program\n");
    printf("  Space                  Generate new random seed\n");
    printf("  0-9                    Change pattern type\n");
    printf("  C                      Cycle through color modes\n");
    printf("  R                      Cycle through randomness modes\n");
    printf("  +/-                    Increase/decrease number of threads\n");
    printf("\nExample: %s 800 600 10 -p psychedelic --fill-rects -t 8\n", program_name);
    printf("         %s 800 600 10 -p wave -out-mode 5 30 -o output.mp4 -r lorenz -c rainbow\n", program_name);
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
    
    // Enable texturing
    glEnable(GL_TEXTURE_2D);
    
    // Create and initialize texture
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Allocate texture data buffer
    texture_data = (uint8_t*)malloc(width * height * 3);  // RGB format
    memset(texture_data, 0, width * height * 3);
    
    // Allocate per-thread texture buffers - allocate full height for each thread to be safe
    for (int i = 0; i < MAX_THREADS; i++) {
        thread_texture_buffers[i] = (uint8_t*)malloc(width * height * 3);
        memset(thread_texture_buffers[i], 0, width * height * 3);
    }
    
    // Initialize texture with black
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
}

void clearScreen() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void addQuad(Vertex* vertices, int* vertex_count, 
             float x, float y, float width, float height,
             float r, float g, float b, float a) {
    // Add four vertices for the quad
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

    vertices[*vertex_count].x = x;
    vertices[*vertex_count].y = y + height;
    vertices[*vertex_count].r = r;
    vertices[*vertex_count].g = g;
    vertices[*vertex_count].b = b;
    vertices[*vertex_count].a = a;
    (*vertex_count)++;
}

// Function to calculate pattern seed with time offset
unsigned long calculate_pattern_seed(int i, int j, PatternType pattern_type, float time_offset, int Width, int Height, unsigned long base_seed) {
    int centerX = Width / 2;
    int centerY = Height / 2;
    
    if (random_mode == ENHANCED_RANDOM) {
        // Add some global randomness factors
        float random_factor = (float)rand() / RAND_MAX;
        float noise = (random_factor * 2.0f - 1.0f) * 0.2f; // Random noise between -0.2 and 0.2
        
        switch(pattern_type) {
            case ORIGINAL:
                return base_seed | ((i*j & (int)(i*cos(time_offset + noise)*j)) ^ ((int)(random_factor * 1000)));
                
            case POLAR: {
                float dx = i - centerX;
                float dy = j - centerY;
                float distance = sqrtf(dx*dx + dy*dy) / (sqrtf(Width*Width + Height*Height) * 0.5f);
                float angle = atan2f(dy, dx);
                float pattern = (sinf(distance * 10.0f + time_offset) * 0.5f + 0.5f) * 
                              (sinf(angle * 4.0f + time_offset + noise) * 0.5f + 0.5f);
                return base_seed | ((int)(pattern * 1000) ^ ((int)(random_factor * 1000)));
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
                float distance = sqrtf(dx*dx + dy*dy) / (sqrtf(Width*Width + Height*Height) * 0.5f);
                float angle = atan2f(dy, dx);
                float pattern = (sinf(distance * 10.0f + time_offset) * 0.5f + 0.5f) * 
                              (sinf(angle * 4.0f + time_offset) * 0.5f + 0.5f);
                return base_seed | ((int)(pattern * 1000));
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

// Thread function for parallel processing of art generation
void* generate_art_thread(void* arg) {
    ThreadWork* work = (ThreadWork*)arg;
    
    // Clear the thread's buffer first
    memset(work->texture_buffer, 0, Width * Height * 3);
    
    for(int j = work->start_row; j < work->end_row; j++) {
        for(int i = 0; i < Width; i++) {
            // Generate pattern seed for this pixel
            unsigned long pattern_seed = calculate_pattern_seed(i, j, work->pattern_type, 
                                                             work->time_offset, Width, Height, work->seed);
            srand(pattern_seed);
            
            float r, g, b;
            float random_factor = (float)rand() / RAND_MAX;
            
            if (color_mode == COLOR_MODE_1) {
                // Original color mode
                float random_shift = random_factor * 0.2f - 0.1f;
                r = ((pattern_seed % 256) / 255.0f + random_shift);
                g = (((pattern_seed >> 8) % 256) / 255.0f + random_shift);
                b = (((pattern_seed >> 16) % 256) / 255.0f + random_shift);
                
                // Add time-based color pulsing with random phase
                float phase_shift = random_factor * M_PI;
                r *= (0.7f + 0.3f * sinf(work->time_offset + phase_shift));
                g *= (0.7f + 0.3f * sinf(work->time_offset + 2.094f + phase_shift));
                b *= (0.7f + 0.3f * sinf(work->time_offset + 4.189f + phase_shift));
            } else if (color_mode == COLOR_MODE_2) {
                // Enhanced color mode
                float base = (float)(pattern_seed % 1000) / 1000.0f;
                r = base;
                g = fmodf(base + 0.33f + 0.1f * sinf(work->time_offset + random_factor), 1.0f);
                b = fmodf(base + 0.66f + 0.1f * cosf(work->time_offset + random_factor), 1.0f);
                
                float contrast = 0.3f;
                r = 0.5f + (r - 0.5f) * (1.0f + contrast);
                g = 0.5f + (g - 0.5f) * (1.0f + contrast);
                b = 0.5f + (b - 0.5f) * (1.0f + contrast);
            } else { // COLOR_MODE_MONO
                float intensity = (float)(pattern_seed % 1000) / 1000.0f;
                intensity = intensity * 0.8f + 0.2f * sinf(work->time_offset + random_factor);
                float contrast = 0.4f;
                intensity = 0.5f + (intensity - 0.5f) * (1.0f + contrast);
                r = g = b = intensity;
            }
            
            // Clamp colors
            r = fmaxf(0.0f, fminf(1.0f, r));
            g = fmaxf(0.0f, fminf(1.0f, g));
            b = fmaxf(0.0f, fminf(1.0f, b));
            
            // Write directly to the correct position in the buffer
            int pixel_idx = (j * Width + i) * 3;
            work->texture_buffer[pixel_idx] = (uint8_t)(r * 255.0f);
            work->texture_buffer[pixel_idx + 1] = (uint8_t)(g * 255.0f);
            work->texture_buffer[pixel_idx + 2] = (uint8_t)(b * 255.0f);
        }
    }
    
    return NULL;
}

// Modified generateArt function to use multiple threads
void generateArt(unsigned long seed, PatternType pattern_type, float time_offset) {
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
    clearScreen();
    
    // Clear main texture buffer
    memset(texture_data, 0, Width * Height * 3);
    
    // Create threads and distribute work
    pthread_t threads[MAX_THREADS];
    ThreadWork thread_work[MAX_THREADS];
    
    // Calculate rows per thread, ensuring no gaps
    int base_rows_per_thread = Height / num_threads;
    int extra_rows = Height % num_threads;
    
    int current_row = 0;
    for (int t = 0; t < num_threads; t++) {
        // Calculate this thread's row count (distribute extra rows evenly)
        int this_thread_rows = base_rows_per_thread + (t < extra_rows ? 1 : 0);
        
        // Set up thread work
        thread_work[t].start_row = current_row;
        thread_work[t].end_row = current_row + this_thread_rows;
        thread_work[t].seed = seed;
        thread_work[t].pattern_type = pattern_type;
        thread_work[t].time_offset = time_offset;
        thread_work[t].texture_buffer = thread_texture_buffers[t];
        
        // Create thread
        pthread_create(&threads[t], NULL, generate_art_thread, &thread_work[t]);
        
        current_row += this_thread_rows;
    }
    
    // Wait for all threads to finish and combine results
    for (int t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
        
        // Copy only the rows this thread was responsible for
        for (int j = thread_work[t].start_row; j < thread_work[t].end_row; j++) {
            memcpy(&texture_data[j * Width * 3],
                   &thread_texture_buffers[t][j * Width * 3],
                   Width * 3);
        }
    }
    
    // Update texture
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
    
    glutSwapBuffers();
}

// GLUT callback functions
void display(void) {
    static float time_offset = 0.0f;
    time_offset += 0.05f;
    
    // Generate art into texture
    generateArt(randseed, pattern_type, time_offset);
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw textured quad
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(Width, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(Width, Height);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, Height);
    glEnd();
    
    glutSwapBuffers();
}

void cleanup() {
    if (texture_data) {
        free(texture_data);
        texture_data = NULL;
    }
    
    // Free thread texture buffers
    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread_texture_buffers[i]) {
            free(thread_texture_buffers[i]);
            thread_texture_buffers[i] = NULL;
        }
    }
    
    glDeleteTextures(1, &texture_id);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) { // ESC key
        cleanup();
        exit(0);
    } else if (key == ' ') {
        randseed = (unsigned long)time(NULL);
    } else if (key >= '0' && key <= '9') {
        // Map number keys to patterns
        switch(key) {
            case '0': pattern_type = ORIGINAL; break;
            case '1': pattern_type = POLAR; break;
            case '2': pattern_type = TRIGONOMETRIC; break;
            case '3': pattern_type = FRACTAL; break;
            case '4': pattern_type = WAVE_INTERFERENCE; break;
            case '5': pattern_type = WAVE2; break;
            case '6': pattern_type = VORTEX; break;
            case '7': pattern_type = KALEIDOSCOPE; break;
            case '8': pattern_type = PSYCHEDELIC; break;
            case '9': pattern_type = CELLULAR; break;
            default: return;
        }
        printf("Switched to pattern: %s\n", 
            pattern_type == ORIGINAL ? "Original" :
            pattern_type == POLAR ? "Polar" :
            pattern_type == TRIGONOMETRIC ? "Trigonometric" :
            pattern_type == FRACTAL ? "Fractal" :
            pattern_type == WAVE_INTERFERENCE ? "Wave Interference" :
            pattern_type == WAVE2 ? "Wave 2" :
            pattern_type == VORTEX ? "Vortex" :
            pattern_type == KALEIDOSCOPE ? "Kaleidoscope" :
            pattern_type == PSYCHEDELIC ? "Psychedelic" :
            pattern_type == CELLULAR ? "Cellular" : "Unknown");
    } else if (key == 'c' || key == 'C') {
        // Cycle through color modes
        color_mode = (ColorMode)((color_mode + 1) % 3);  // Cycles through 3 modes
        printf("Switched to %s mode\n", 
            color_mode == COLOR_MODE_1 ? "original RGB" : 
            color_mode == COLOR_MODE_2 ? "enhanced color" : 
            "monochrome");
    } else if (key == 'r' || key == 'R') {
        // Cycle through random modes
        random_mode = (RandomnessMode)((random_mode + 1) % 2);
        printf("Switched to %s mode\n", 
            random_mode == CLASSIC_RANDOM ? "classic random" : 
            "enhanced random");
    } else if (key == '+' || key == '=') {
        // Increase number of threads
        if (num_threads < MAX_THREADS) {
            num_threads++;
            printf("Increased to %d threads\n", num_threads);
        } else {
            printf("Already at maximum thread count: %d\n", MAX_THREADS);
        }
    } else if (key == '-' || key == '_') {
        // Decrease number of threads
        if (num_threads > 1) {
            num_threads--;
            printf("Decreased to %d threads\n", num_threads);
        } else {
            printf("Already at minimum thread count: 1\n");
        }
    } else if (key == 'h' || key == 'H') {
        // Print pattern mapping help
        printf("\nPattern Mapping:\n");
        printf("0: Original\n");
        printf("1: Polar\n");
        printf("2: Trigonometric\n");
        printf("3: Fractal\n");
        printf("4: Wave Interference\n");
        printf("5: Wave 2\n");
        printf("6: Vortex\n");
        printf("7: Kaleidoscope\n");
        printf("8: Psychedelic\n");
        printf("9: Cellular\n");
    } else {
        // Generate new art with current pattern type but different seed
        randseed = randseed | (key * 10);
    }
}

void idle(void) {
    display();
}

// Function to get current time in seconds
double get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Function to print progress
void print_progress(int current_frame, int total_frames, double start_time) {
    double current_time = get_current_time();
    double elapsed = current_time - start_time;
    double progress = (double)current_frame / total_frames;
    double estimated_total = elapsed / progress;
    double remaining = estimated_total - elapsed;
    
    printf("\rProgress: %d/%d frames (%.1f%%) - Elapsed: %.1fs - Remaining: %.1fs", 
           current_frame, total_frames, progress * 100, 
           elapsed, remaining);
    fflush(stdout);
}

// Initialize video encoding context
VideoContext* init_video_encoder(const char* filename, int width, int height, int framerate) {
    VideoContext* ctx = (VideoContext*)calloc(1, sizeof(VideoContext));
    
    // Allocate format context
    avformat_alloc_output_context2(&ctx->format_context, NULL, NULL, filename);
    if (!ctx->format_context) {
        fprintf(stderr, "Could not create output context\n");
        return NULL;
    }
    
    // Find the h264 encoder
    const AVCodec *codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        return NULL;
    }
    
    // Create new video stream
    ctx->video_stream = avformat_new_stream(ctx->format_context, codec);
    if (!ctx->video_stream) {
        fprintf(stderr, "Could not allocate stream\n");
        return NULL;
    }
    
    // Allocate codec context
    ctx->codec_context = avcodec_alloc_context3(codec);
    if (!ctx->codec_context) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return NULL;
    }
    
    // Set codec parameters
    ctx->codec_context->bit_rate = 2000000;  // Reduced to 2Mbps for reasonable file size
    ctx->codec_context->width = width;
    ctx->codec_context->height = height;
    ctx->codec_context->time_base = (AVRational){1, framerate};
    ctx->codec_context->framerate = (AVRational){framerate, 1};
    ctx->codec_context->gop_size = 30;  // Increased GOP size for better compression
    ctx->codec_context->max_b_frames = 2;  // Increased B-frames
    ctx->codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
    
    // Set x264 specific encoding parameters for high quality
    AVDictionary *param = NULL;
    av_dict_set(&param, "preset", "faster", 0);  // Faster encoding, artificial content doesn't benefit much from slow
    av_dict_set(&param, "tune", "animation", 0);  // Optimize for animation content
    av_dict_set(&param, "crf", "28", 0);  // Changed to 28 for good quality with much smaller file size
    
    // Set stream parameters
    avcodec_parameters_from_context(ctx->video_stream->codecpar, ctx->codec_context);
    
    // Open codec with quality parameters
    if (avcodec_open2(ctx->codec_context, codec, &param) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return NULL;
    }
    av_dict_free(&param);
    
    // Open output file
    if (avio_open(&ctx->format_context->pb, filename, AVIO_FLAG_WRITE) < 0) {
        fprintf(stderr, "Could not open output file '%s'\n", filename);
        return NULL;
    }
    
    // Write format header
    if (avformat_write_header(ctx->format_context, NULL) < 0) {
        fprintf(stderr, "Error occurred when writing header\n");
        return NULL;
    }
    
    // Allocate frame
    ctx->frame = av_frame_alloc();
    ctx->frame->format = ctx->codec_context->pix_fmt;
    ctx->frame->width = width;
    ctx->frame->height = height;
    
    if (av_frame_get_buffer(ctx->frame, 0) < 0) {
        fprintf(stderr, "Could not allocate frame data\n");
        return NULL;
    }
    
    // Initialize frame buffer for RGB data
    ctx->frame_buffer = (uint8_t*)malloc(width * height * 3);
    
    // Initialize scaling context
    ctx->sws_context = sws_getContext(width, height, AV_PIX_FMT_RGB24,
                                    width, height, AV_PIX_FMT_YUV420P,
                                    SWS_BILINEAR,  // Simple bilinear filtering is sufficient for 1:1 color conversion
                                    NULL, NULL, NULL);
    
    if (!ctx->sws_context) {
        fprintf(stderr, "Could not initialize scaling context\n");
        return NULL;
    }
    
    return ctx;
}

// Encode a frame
int encode_frame(VideoContext* ctx, const uint8_t* rgb_data) {
    // Convert RGB to YUV
    const uint8_t* rgb_data_ptr[1] = { rgb_data };
    int rgb_linesize[1] = { ctx->codec_context->width * 3 };
    sws_scale(ctx->sws_context, rgb_data_ptr, rgb_linesize, 0, ctx->codec_context->height,
              ctx->frame->data, ctx->frame->linesize);
    
    ctx->frame->pts = ctx->frame_count;
    
    // Send frame to encoder
    int ret = avcodec_send_frame(ctx->codec_context, ctx->frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending frame for encoding\n");
        return -1;
    }
    
    while (ret >= 0) {
        AVPacket *pkt = av_packet_alloc();
        ret = avcodec_receive_packet(ctx->codec_context, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_packet_free(&pkt);
            break;
        } else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            av_packet_free(&pkt);
            return -1;
        }
        
        pkt->stream_index = ctx->video_stream->index;
        av_packet_rescale_ts(pkt, ctx->codec_context->time_base, ctx->video_stream->time_base);
        ret = av_interleaved_write_frame(ctx->format_context, pkt);
        av_packet_free(&pkt);
        if (ret < 0) {
            fprintf(stderr, "Error writing packet\n");
            return -1;
        }
    }
    
    return 0;
}

// Finalize video encoding
void finalize_video_encoder(VideoContext* ctx) {
    // Flush encoder
    encode_frame(ctx, NULL);
    
    // Write trailer
    av_write_trailer(ctx->format_context);
    
    // Close file
    avio_closep(&ctx->format_context->pb);
    
    // Free resources
    avcodec_free_context(&ctx->codec_context);
    av_frame_free(&ctx->frame);
    avformat_free_context(ctx->format_context);
    sws_freeContext(ctx->sws_context);
    free(ctx->frame_buffer);
    free(ctx);
}

// Function to read framebuffer
void read_pixels_to_buffer(uint8_t* buffer, int width, int height) {
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
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
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--threads") == 0) {
            if (i + 1 < argc) {
                int thread_count = atoi(argv[i + 1]);
                if (thread_count >= 1 && thread_count <= MAX_THREADS) {
                    num_threads = thread_count;
                    printf("Using %d threads\n", num_threads);
                } else {
                    printf("Thread count must be between 1 and %d. Using default (%d).\n", 
                           MAX_THREADS, num_threads);
                }
                i++;
            } else {
                printf("Missing thread count after -t option.\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-out-mode") == 0) {
            if (i + 2 < argc) {
                output_config.mode = VIDEO_MODE;
                output_config.duration_seconds = atoi(argv[i + 1]);
                output_config.framerate = atoi(argv[i + 2]);
                i += 2;
            } else {
                printf("Missing duration and/or framerate after -out-mode option.\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_config.output_filename = argv[i + 1];
                i++;
            } else {
                printf("Missing filename after -o option.\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--random") == 0) {
            if (i + 1 < argc) {
                random_mode = parse_random_mode(argv[i + 1]);
                i++;
            } else {
                printf("Missing random mode after -r option.\n");
                exit(1);
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--color") == 0) {
            if (i + 1 < argc) {
                color_mode = parse_color_mode(argv[i + 1]);
                i++;
            } else {
                printf("Missing color mode after -c option.\n");
                exit(1);
            }
        } else {
            printf("Unknown option '%s'\n", argv[i]);
            exit(1);
        }
    }
    
    printf("Width: %d, Height: %d, Tile size: %d, Pattern type: %d, Threads: %d\n", 
           Width, Height, tilesize, pattern_type, num_threads);

    randseed = (unsigned long)time(NULL);
    
    // Initialize OpenGL
    initGL(Width, Height, argc, argv);
    
    if (output_config.mode == VIDEO_MODE) {
        // Generate output filename if not specified
        char filename_buffer[256];
        if (!output_config.output_filename) {
            const char* pattern_name;
            switch(pattern_type) {
                case ORIGINAL: pattern_name = "original"; break;
                case POLAR: pattern_name = "polar"; break;
                case TRIGONOMETRIC: pattern_name = "trig"; break;
                case FRACTAL: pattern_name = "fractal"; break;
                case WAVE_INTERFERENCE: pattern_name = "wave"; break;
                case WAVE2: pattern_name = "wave2"; break;
                case VORTEX: pattern_name = "vortex"; break;
                case KALEIDOSCOPE: pattern_name = "kaleidoscope"; break;
                case PSYCHEDELIC: pattern_name = "psychedelic"; break;
                case CELLULAR: pattern_name = "cellular"; break;
                default: pattern_name = "unknown"; break;
            }
            
            const char* random_name;
            switch(random_mode) {
                case CLASSIC_RANDOM: random_name = "classic"; break;
                case ENHANCED_RANDOM: random_name = "enhanced"; break;
                default: random_name = "unknown"; break;
            }
            
            const char* color_name;
            switch(color_mode) {
                case COLOR_MODE_1: color_name = "rgb"; break;
                case COLOR_MODE_2: color_name = "enhanced"; break;
                case COLOR_MODE_MONO: color_name = "mono"; break;
                default: color_name = "unknown"; break;
            }
            
            snprintf(filename_buffer, sizeof(filename_buffer), 
                    "art_%dx%d_%s_%s_%s_%ds.mp4", 
                    Width, Height, pattern_name, random_name, color_name,
                    output_config.duration_seconds);
            output_config.output_filename = filename_buffer;
        }
        
        printf("Generating video: %s\n", output_config.output_filename);
        printf("Duration: %d seconds at %d fps\n", 
               output_config.duration_seconds, output_config.framerate);
        printf("Pattern: %s, Random: %s, Color: %s\n",
               pattern_type == ORIGINAL ? "original" :
               pattern_type == POLAR ? "polar" :
               pattern_type == TRIGONOMETRIC ? "trig" :
               pattern_type == FRACTAL ? "fractal" :
               pattern_type == WAVE_INTERFERENCE ? "wave" :
               pattern_type == WAVE2 ? "wave2" :
               pattern_type == VORTEX ? "vortex" :
               pattern_type == KALEIDOSCOPE ? "kaleidoscope" :
               pattern_type == PSYCHEDELIC ? "psychedelic" :
               pattern_type == CELLULAR ? "cellular" : "unknown",
               random_mode == CLASSIC_RANDOM ? "classic" :
               random_mode == ENHANCED_RANDOM ? "enhanced" : "unknown",
               color_mode == COLOR_MODE_1 ? "rgb" :
               color_mode == COLOR_MODE_2 ? "enhanced" :
               color_mode == COLOR_MODE_MONO ? "mono" : "unknown");
        
        // Initialize video encoder
        VideoContext* video_ctx = init_video_encoder(
            output_config.output_filename, 
            Width, Height, 
            output_config.framerate
        );
        
        if (!video_ctx) {
            fprintf(stderr, "Failed to initialize video encoder\n");
            exit(1);
        }
        
        // Calculate total frames
        int total_frames = output_config.duration_seconds * output_config.framerate;
        video_ctx->total_frames = total_frames;
        video_ctx->start_time = get_current_time();
        
        // Generate and encode frames
        float time_offset = 0.0f;
        float time_step = 0.05f;  // Match the real-time animation speed
        
        for (int frame = 0; frame < total_frames; frame++) {
            // Generate frame with current time offset
            clearScreen();
            generateArt(randseed, pattern_type, time_offset);
            
            // Draw textured quad
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex2f(Width, 0.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex2f(Width, Height);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, Height);
            glEnd();
            
            // Make sure rendering is complete
            glFinish();
            
            // Read pixels and encode frame
            read_pixels_to_buffer(video_ctx->frame_buffer, Width, Height);
            if (encode_frame(video_ctx, video_ctx->frame_buffer) < 0) {
                fprintf(stderr, "Error encoding frame %d\n", frame);
                break;
            }
            
            // Update progress and time offset
            print_progress(frame + 1, total_frames, video_ctx->start_time);
            video_ctx->frame_count++;
            time_offset += time_step;
        }
        
        printf("\nFinishing video encoding...\n");
        finalize_video_encoder(video_ctx);
        printf("Video generation complete: %s\n", output_config.output_filename);
        
        cleanup();
        exit(0);
    } else {
        // Register callbacks for real-time mode
        glutDisplayFunc(display);
        glutKeyboardFunc(keyboard);
        glutIdleFunc(idle);
        
        // Start the main loop
        glutMainLoop();
    }
    
    cleanup();
    return 0;
}