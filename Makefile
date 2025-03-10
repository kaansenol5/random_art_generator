CC = gcc
CFLAGS = -Wall -O2
FRAMEWORKS = -framework OpenGL -framework GLUT
FFMPEG_LIBS = -L/opt/homebrew/lib -lavcodec -lavformat -lavutil -lswscale
FFMPEG_CFLAGS = $(shell pkg-config --cflags libavcodec libavformat libavutil libswscale)
LIBS = $(FRAMEWORKS) $(FFMPEG_LIBS)

TARGET = artmaker

$(TARGET): main.c
	$(CC) $(CFLAGS) $(FFMPEG_CFLAGS) $< -o $@ $(LIBS)

clean:
	rm -f $(TARGET) 