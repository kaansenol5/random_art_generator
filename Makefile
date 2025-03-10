CC = gcc
CFLAGS = -Wall -Wextra -O3 -march=native -ffast-math
FRAMEWORKS = -framework OpenGL -framework GLUT
LIBS = -lm
TARGET = randomart
SRCS = main.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(FRAMEWORKS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean all

clean:
	rm -f $(TARGET) $(OBJS) 