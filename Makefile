.PHONY: all clean very-clean clean-image video

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

.SUFFIXES: .dat .mp4

.dat.mp4:
	$(MAKE) clean-image
	./run.sh $*.dat

all: mandelbrot

mandelbrot: mandelbrot.o
	$(CXX) -o mandelbrot mandelbrot.o $(LDFLAGS)

mandelbrot.o: mandelbrot.cc
	$(CXX) -c mandelbrot.cc -o mandelbrot.o $(CXXFLAGS)

clean:
	rm -f mandelbrot mandelbrot.o

very-clean:
	$(MAKE) clean
	$(MAKE) clean-image
	rm -f *.mp4

clean-image:
	ls | grep "mandelbrot-.*.png" | xargs rm

video: mandelbrot run.sh scene1.mp4 scene2.mp4 seahorse.mp4
