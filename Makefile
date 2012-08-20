LDFLAGS = -lglut -lGLU -lGL -lSDL -lm
CFLAGS = -Wall -std=c99 -march=native

all: chaosjulia

clean:
	rm -f chaosjulia
