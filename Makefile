CC = g++ -std=c++11
LD = g++ -std=c++11
CFLAGS = -g -Wall -O2
TARGET = VirtualMonitor

LIBFREENECT = -L/usr/local/lib -lfreenect2 -lglfw

ifeq ($(shell uname -s), Darwin)
	LIBGL = -framework OpenGL -framework GLUT
else
	LIBGL = -lGL -lglut
endif

LDFLAGS += $(LIBFREENECT) $(LIBGL)

BIN_DIR = ./bin
BUILD_DIR = ./build
SRC_DIR = ./src

SRC_LIST = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_LIST = $(SRC_LIST:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

mkdir_if_necessary = @mkdir -p $(@D)

all: $(BIN_DIR)/$(TARGET)

debug: CFLAGS += -DDEBUG
debug: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJ_LIST)
	$(mkdir_if_necessary)
	$(LD) $(OBJ_LIST) $(LDFLAGS) -o $@

$(OBJ_LIST): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(mkdir_if_necessary)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)

