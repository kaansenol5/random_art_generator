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

} PatternType;

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

    
    printf("Invalid pattern type '%s'. Using default (original).\n", pattern_str);
    return ORIGINAL;
}

void print_usage(const char* program_name) {
    printf("USAGE: %s <width> <height> <pixelsize> [options]\n\n", program_name);
    printf("Options:\n");
    printf("  -p, --pattern <type>   Set pattern type (original, polar, trig, fractal, wave, symmetry)\n");
    printf("  --fill-rects           Fill rectangles instead of outlines\n");
    printf("\nExample: %s 800 600 10 -p fractal --fill-rects\n", program_name);
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


            
        default:
            return base_seed;
    }
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
            unsigned long pattern_seed = calculate_pattern_seed(i, j, pattern_type, time_offset, Width, Height, seed);
            srand(pattern_seed);
            
            float r = (rand() % 256) / 255.0f;
            float g = (rand() % 256) / 255.0f;
            float b = (rand() % 256) / 255.0f;
            r *= (0.7f + 0.3f * sinf(time_offset));
            g *= (0.7f + 0.3f * sinf(time_offset + 2.094f));
            b *= (0.7f + 0.3f * sinf(time_offset + 4.189f));
            
            addQuad(vertices, &vertex_count, i, j, tilesize, tilesize, r, g, b, 1.0f);
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
    } else if (key >= '0' && key <= '5') {
        // Change pattern type based on numeric key
        pattern_type = key - '0';
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