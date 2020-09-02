CENTER=-0.7746806106269039 -0.1374168856037867
RANGE=1 2e-13
FRAMES=750
IMAGE_SIZE=1920 1080

.PHONY: all clean images video

all: mandelbrot

mandelbrot: mandelbrot.o
	$(CXX) -o mandelbrot mandelbrot.o \
	    -O3 -lm -lopencv_core -lopencv_imgcodecs -lopencv_imgproc \
	    -L/usr/local/lib/opencv4

mandelbrot.o: mandelbrot.cc
	$(CXX) -c mandelbrot.cc -o mandelbrot.o \
	    -std=c++11 \
	    -O3 -I/usr/local/include/opencv4

clean:
	rm -f mandelbrot mandelbrot.o
	rm -f mandelbrot-*.png
	rm -f mandelbrot.mp4

images: mandelbrot
	./mandelbrot $(CENTER) $(RANGE) $(FRAMES) $(IMAGE_SIZE)

video: 
	ffmpeg -r 30 -i mandelbrot-%04d.png -vcodec libx264 -pix_fmt yuv420p -r 30 mandelbrot.mp4
