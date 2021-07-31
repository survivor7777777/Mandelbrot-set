// -*- c++ -*-
// mandelbrot.cc
// Licensed under the Creative Comons Zero v1.0 Universal

#include <omp.h>
#include <complex>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;

const int MAX_N = 3000;
const long double MAX_M = log((long double)MAX_N);
const long double PI = atan(1.0L)*4.0L;

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

vector<cv::Vec3b> colormap(1000);

void init_colors() {
    const vector<const cv::Vec3b> y = {
	cv::Vec3b(100,   7,   0),
	cv::Vec3b(203, 107,  32),
	cv::Vec3b(255, 255, 237),
	cv::Vec3b(  0, 170, 255),
	cv::Vec3b(  0,   2,   0)
    };
    const vector<const double> x = {
	0.0,
	0.5,
	0.667,
	0.833,
	1.0,
    };
    assert(x.size() == y.size());
    const int n = x.size();

    // get slopes
    vector<cv::Vec3d> slope(n - 1);
    for (int i = 0; i < n - 1; i++) {
	for (int d = 0; d < 3; d++) {
	    slope[i][d] = (y[i+1][d] - y[i][d]) / (x[i+1] - x[i]);
	}
    }

    // get degree-1 coefficients
    vector<cv::Vec3d> c1(n);
    c1[0] = slope[0];
    for (int i = 1; i < n - 2; i++) {
	const double dx1 = x[i]   - x[i-1];
	const double dx2 = x[i+1] - x[i];
	const double dx3 = x[i+1] - x[i-1];
	for (int d = 0; d < 3; d++) {
	    if (slope[i-1][d] * slope[i][d] <= 0)
		c1[i][d] = 0;
	    else 
		c1[i][d] = 3 * dx3 / ((dx3 + dx2) / slope[i-1][d] + ((dx3 + dx1) / slope[i][d]));
	}
    }

    // get degree-2 and digree-3 coefficients
    vector<cv::Vec3d> c2(n-1), c3(n-1);
    for (int i = 0; i < n - 1; i++) {
	for (int d = 0; d < 3; d++) {
	    const double z = c1[i][d] + c1[i+1][d] - slope[i][d] * 2;
	    c2[i][d] = (slope[i][d] - c1[i][d] - z) / (x[i+1] - x[i]);
	    c3[i][d] = z / (x[i+1] - x[i]) / (x[i+1] - x[i]);
	}
    }

    // create colormap
    for (int i = 0, j = 1; i < colormap.size(); i++) {
	const double cx = x[0] + (x[n-1] - x[0]) * i / colormap.size(); 
	while (cx > x[j])
	    j++;
	const double delta = cx - x[j-1];
	for (int d = 0; d < 3; d++) {
	    colormap[i][d] = y[j-1][d] + c1[j-1][d] * delta + c2[j-1][d] * delta * delta + c3[j-1][d] * delta * delta * delta + 0.5;
	}
    }

    // dump colormap to "colormap.png"
    {
	const int height = 48;
	cv::Mat image(height, colormap.size(), CV_8UC3);
	for (int i = 0; i < colormap.size(); i++) {
	    for (int j = 0; j < height; j++) {
		image.at<cv::Vec3b>(j, i) = colormap[i];
	    }
	}
	cv::imwrite("colormap.png", image);
    }

    // dump colormap to "colormap.csv"
    {
	ofstream dump("colormap.csv");
	for (int i = 0; i < colormap.size(); i++) {
	    dump << i;
	    for (int d = 0; d < 3; d++)
	    	dump << ", " << (int)colormap[i][d];
	    dump << endl;
	}
    }
}


inline cv::Vec3b color(const double m) {
    int index = (int)((colormap.size()-1) * (m / MAX_M));
    return colormap[index];
}

//
// Draw a picture
//

void draw(const complex<long double>& p, const long double range,
    const int width, const int height, const string& name) {
    const long double scale = range * 2 / width;
    cv::Mat image(height, width, CV_8UC3);
    for (int i = 0; i < height; i++) {
	long double y = p.imag() + (height / 2 - i) * scale;
	cv::Vec3b *ip = image.ptr<cv::Vec3b>(i);
	#pragma omp parallel for
	for (int j = 0; j < width; j++) {
	    long double x = p.real() + (j - width / 2) * scale;
	    const complex<long double> z(x, y);
	    const double m = mandelbrot(z);
	    ip[j] = color(m);
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
    if (argc != 10) {
	usage(argv[0]);
	cerr << "Missing parameters" << endl;
	return 1;
    }
    char** p = argv;
    const long double real = stold(*++p);
    const long double imag = stold(*++p);
    const long double range_s = stold(*++p);
    const long double range_e = stold(*++p);
    const long double theta_s = stold(*++p);
    const long double turns = stold(*++p);
    const int frames = stoi(*++p);
    const int width = stoi(*++p);
    const int height = stoi(*++p);

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
	    << "theta-start = " << theta_s << endl
	    << "turns = " << turns << endl
	    << "frames = " << frames << endl
	    << "image-width = " << width << endl
	    << "image-height = " << height << endl;
	return 2;
    }

    init_colors();

    const complex<long double> z1(real, imag);
    const long double abs_z1 = abs(z1);
    const long double log_rate = (log(range_e) - log(range_s)) / (frames - 1);
    for (int i = 0; i < frames; i++) {
	const string name = filename(i);
	const long double r = range_s * exp(i * log_rate);
	const long double x = (long double)(frames - i) / frames;
	const long double dt = 2.0L * PI * x * turns + theta_s;
	const long double dr = 0.5L * x * r;
	const complex<long double> dz(dr*cos(dt), dr*sin(dt));
	const complex<long double> z = z1 + dz;
	cout << name << " " << z << " " << r << endl;
	draw(z, r, width, height, name);
    }
}

