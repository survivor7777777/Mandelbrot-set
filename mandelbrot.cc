// -*- c++ -*-
// mandelbrot.cc
// Licensed under the Creative Comons Zero v1.0 Universal

#include <omp.h>
#include <complex>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;

#define MAX_N 30000
#define MAX_M log(MAX_N)

//
// Mandelbrot function
//

double mandelbrot(const complex<long double>& c) {
    complex<long double> z = 0.0;
    for (int n = 1; n < MAX_N; n++) {
	z = z * z + c;
	if (abs(z) > 4.0)
	    return log(n);
    }
    return MAX_M;
}

//
// Color table
//

vector<cv::Vec3b> color_vector;

void interpolate_colors(const cv::Vec3b& c_s, const cv::Vec3b& c_e, const int steps) {
    const cv::Vec3d delta((c_e[0]-c_s[0])/(double)steps, (c_e[1]-c_s[1])/(double)steps, (c_e[2]-c_s[2])/(double)steps);
    for (int i = 0; i < steps; i++) {
	const cv::Vec3b c((int)(c_s[0] + i * delta[0] + 0.5), (int)(c_s[1] + i * delta[1] + 0.5), (int)(c_s[2] + i * delta[2] + 0.5));
	color_vector.push_back(c);
    }
}

void init_colors() {
    const cv::Vec3b c1(100, 7, 0);
    const cv::Vec3b c2(255, 255, 255);
    const cv::Vec3b c3(0, 170, 255);
    const cv::Vec3b c4(2, 0, 0);

    // phase-1
    interpolate_colors(c1, c2, 200);
    
    // phase-2
    interpolate_colors(c2, c3, 100);
   
    // phase-3
    interpolate_colors(c3, c4, 100);
    color_vector.push_back(c4);
}

inline cv::Vec3b color(const double m) {
    const int index = (int)((color_vector.size() - 1) * (m / MAX_M));
    return color_vector[index];
}

//
// Draw a picture
//

void draw(const complex<long double>& p, const long double range,
    const int width, const int height, const string& name) {
    const long double scale = range * 2 / width;
    cv::Mat image(height, width, CV_8UC3);
    for (int i = 0; i < height; i++) {
	long double y = p.imag() + (i - height / 2) * scale;
	cv::Vec3b *line = image.ptr<cv::Vec3b>(i);
	#pragma omp parallel for
	for (int j = 0; j < width; j++) {
	    long double x = p.real() + (j - width / 2) * scale;
	    const complex<long double> z(x, y);
	    const double m = mandelbrot(z);
	    line[j] = color(m);
	}
    }
    cv::imwrite(name, image);
}

string filename(int n) {
    stringstream name;
    name << "mandelbrot-" << setw(4) << setfill('0') << n << ".png" << ends;
    return (string)name.str();
}

void usage(const char* cmd) {
    cerr << "Usage: " << cmd
	<< " center-real center-imag"
	<< " range-start range-end"
	<< " frames"
	<< " image-width image-height"
	<< endl;
}

//
// main function
//

int main(int argc, char** argv) {
    if (argc != 8) {
	usage(argv[0]);
	cerr << "Missing parameters" << endl;
	return 1;
    }
    const long double real = stold(argv[1]);
    const long double imag = stold(argv[2]);
    const long double range_s = stold(argv[3]);
    const long double range_e = stold(argv[4]);
    const int frames = stoi(argv[5]);
    const int width = stoi(argv[6]);
    const int height = stoi(argv[7]);

    cout << setprecision(52);
    cerr << setprecision(52);

    if (range_s == range_e || range_s <= 0.0 || range_e <= 0.0 ||
	frames <= 1 || width <= 0 || height <= 0) {
	usage(argv[0]);
	cerr << "Invalid parameters specified" << endl
	    << "center-real = " << real << endl
	    << "center-imag = " << imag << endl
	    << "range-start = " << range_s << endl
	    << "range-end = " << range_e << endl
	    << "frames = " << frames << endl
	    << "image-width = " << width << endl
	    << "image-height = " << height << endl;
	return 2;
    }

    init_colors();

    const complex<long double> z(real, imag);
    const long double log_rate = (log(range_e) - log(range_s)) / (frames - 1);
    for (int i = 0; i < frames; i++) {
	const string name = filename(i);
	const long double r = range_s * exp(i * log_rate);
	cout << name << " " << z << " " << r << endl;
	draw(z, r, width, height, name);
    }
}

