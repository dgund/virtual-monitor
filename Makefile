CC = g++ -std=c++11
LD = g++ -std=c++11

CXXFLAGS += -g -Wall -O2
LDFLAGS += -lpthread

TARGET = VirtualMonitor

LIBFREENECT = -L/usr/local/lib -lfreenect2 -lglfw

LIBWX_CXXFLAGS = `wx-config --cxxflags`
LIBWX_LDFLAGS = `wx-config --libs`

ifeq ($(shell uname -s), Darwin)
	LIBGL = -framework OpenGL -framework GLUT
	LIBMOUSE = -framework ApplicationServices
else
	LIBGL = -lGL -lglut
endif

CXXFLAGS += $(LIBWX_CXXFLAGS)
LDFLAGS += $(LIBFREENECT) $(LIBGL) $(LIBMOUSE) $(LIBWX_LDFLAGS)

BIN_DIR = ./bin
BUILD_DIR = ./build
SRC_DIR = ./src

SRC_LIST = $(wildcard $(SRC_DIR)/*.cpp)
OBJ_LIST = $(SRC_LIST:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

mkdir_if_necessary = @mkdir -p $(@D)

all: $(BIN_DIR)/$(TARGET)

debug: CXXFLAGS += -DDEBUG
debug: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJ_LIST)
	$(mkdir_if_necessary)
	$(LD) $(OBJ_LIST) $(LDFLAGS) -o $@

$(OBJ_LIST): $(BUILD_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(mkdir_if_necessary)
	$(CC) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)

