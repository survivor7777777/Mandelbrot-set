.PHONY: all clean images video

all: mandelbrot

mandelbrot: mandelbrot.o
	$(CXX) -o mandelbrot mandelbrot.o \
	    -O3 -lm -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
	    -fopenmp \
	    -L/usr/local/lib/opencv4

mandelbrot.o: mandelbrot.cc
	$(CXX) -c mandelbrot.cc -o mandelbrot.o \
	    -fopenmp \
	    -std=c++11 \
	    -O3 -I/usr/local/include/opencv4

clean:
	rm -f mandelbrot mandelbrot.o
