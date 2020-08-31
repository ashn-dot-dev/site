.POSIX:
.SUFFIXES:
.PHONY: all clean format
.SILENT: clean

C99_DBG = -O0 -g
C99_REL = -DNDEBUG

GNU_BASE = \
	-std=c99 -pedantic-errors \
	-Wall -Wextra \
	-Werror=conversion \
	-Werror=double-promotion \
	-Werror=float-equal \
	-Werror=implicit-function-declaration \
	-Werror=incompatible-pointer-types \
	-Werror=vla
GNU_DBG = $(GNU_BASE) -O0 -g
GNU_REL = $(GNU_BASE) -Os -DNDEBUG

EMFLAGS = \
	--shell-file shell.html \
	-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]'

CC = c99
CFLAGS = $(C99_REL)
LDLIBS = -lSDL2 -lSDL2_image
TARGET = life
OBJS = life.o

all: $(TARGET) $(TARGET).html

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDLIBS)

# Be sure to prepare an Emscripten environment with:
#   $ cd emsdk/
#   $ ./emsdk install latest
#   $ ./emsdk activate latest
#   $ source ./emsdk_env.sh
# Run the generated html/js/wasm build by serving it with:
#   $ python3 -m http.server 8080
# and navigating to:
#   http://localhost:8080/
## Multi-file build
$(TARGET).html:
	emcc -o $@ $(TARGET).c $(CFLAGS) $(EMFLAGS)
## Single-file build
$(TARGET).bundle.html:
	emcc -o $@ $(TARGET).c $(CFLAGS) $(EMFLAGS) -s SINGLE_FILE=1

clean:
	rm -f \
		$(TARGET) $(OBJS) \
		$(TARGET).html $(TARGET).js $(TARGET).wasm \
		$(TARGET).bundle.html

format:
	clang-format -i aengn.h *.c

.SUFFIXES: .c .o
.c.o:
	$(CC) -o $@ $(INCS) $(CFLAGS) -c $<
