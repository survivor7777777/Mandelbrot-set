.PHONY: all clean very-clean clean-image video

CXXFLAGS := -std=c++11 -O3
LDFLAGS := -O3 -lm -lopencv_core -lopencv_imgcodecs -lopencv_imgproc

OS := $(shell uname -s)
ARCH := $(shell uname -m)
ifeq ($(OS), Darwin)
    CXXFLAGS += -Xpreprocessor -fopenmp
    LDFLAGS += -Xpreprocessor -fopenmp -lomp
ifeq ($(ARCH), arm64)
    CXXFLAGS += -I/opt/homebrew/include -I/opt/homebrew/include/opencv4
    LDFLAGS += -L/opt/homebrew/lib -L/opt/homebrew/lib/opencv4
else
    CXXFLAGS += -I/usr/local/include -I/usr/local/include/opencv4
    LDFLAGS += -L/usr/local/lib -L/usr/local/lib/opencv4
endif
else
    CXXFLAGS += -fopenmp -I/usr/local/include/opencv4
    LDFLAGS += -L/usr/local/lib/opencv4
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
	ls | grep "mandelbrot-.*.png" | xargs rm -f

video: mandelbrot run.sh scene1.mp4 scene2.mp4 seahorse.mp4

scene1.mp4 scene2.mp4 seahorse.mp4 test.mp4: mandelbrot

colormap-graph.png: colormap-graph.gnuplot colormap.csv
	gnuplot colormap-graph.gnuplot
