CC = g++
LD = g++
CFLAGS = -g -Wall
TARGET = VirtualMonitor

ifeq ($(shell uname -s), Darwin)
	LIBFREENECT = -L. /usr/local/lib/libfreenect2.0.2.0.dylib /usr/local/lib/libglfw.3.2.dylib
	LIBGL = -framework OpenGL -framework GLUT
else
	LIBFREENECT = -libfreenect2 -libglfw
	LIBGL = -lGL -lglut
endif

LDFLAGS += $(LIBFREENECT) $(LIBGL)

BIN_DIR = ./bin
BUILD_DIR = ./build
SRC_DIR = ./src

SRC_LIST = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_LIST = $(SRC_LIST:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

mkdir_if_necessary = @mkdir -p $(@D)

$(BIN_DIR)/$(TARGET): $(OBJ_LIST)
	$(mkdir_if_necessary)
	$(LD) $(OBJ_LIST) $(LDFLAGS) -o $@

$(OBJ_LIST): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(mkdir_if_necessary)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)

