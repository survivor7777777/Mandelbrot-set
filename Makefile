.PHONY: all clean 

CXXFLAGS := -std=c++11 -O3 -I/usr/local/include/opencv4
LDFLAGS := -O3 -lm -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -L/usr/local/lib/opencv4

OS := $(shell uname -s)
ifeq ($(OS), Darwin)
    CXXFLAGS += -Xpreprocessor -fopenmp
    LDFLAGS += -Xpreprocessor -fopenmp -lomp
else
    CXXFLAGS += -fopenmp
    LDFLAGS += -fopenmp
endif

all: mandelbrot

mandelbrot: mandelbrot.o
	$(CXX) -o mandelbrot mandelbrot.o $(LDFLAGS)

mandelbrot.o: mandelbrot.cc
	$(CXX) -c mandelbrot.cc -o mandelbrot.o $(CXXFLAGS)

clean:
	rm -f mandelbrot mandelbrot.o
