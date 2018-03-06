CC = g++
CFLAGS  = -g -Wall

TARGET = KinectSensing

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(wildcard OpenKinectViewer/*.cpp) $(TARGET).cpp -L. /usr/local/lib/libfreenect2.0.2.0.dylib /usr/local/lib/libglfw.3.2.dylib -framework OpenGL -framework GLUT

clean:
	$(RM) $(TARGET)
