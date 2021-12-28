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

struct Range {
    double min;
    double max;
    Range(double a, double b): min(a), max(b) {}
    Range(const Range& mr): min(mr.min), max(mr.max) {}
};

const int MAX_N = 3000;
const long double MAX_M = log((long double)MAX_N);

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

void init_colormap() {
    const vector<cv::Vec3b> y = {
	cv::Vec3b(100,   7,   0),
	cv::Vec3b(203, 107,  32),
	cv::Vec3b(255, 255, 237),
	cv::Vec3b(  0, 170, 255),
	cv::Vec3b(  0,  50, 255),
	cv::Vec3b(  0,   0,   0)	
    };
    const vector<double> x = {
	0.00,
	0.20,
	0.40,
	0.60,
	0.80,
	1.00,
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

Range prev_mrange(0, MAX_M);

inline cv::Vec3b color(const double m) {
    int index = (int)((colormap.size()-1) * (m - prev_mrange.min) / (MAX_M - prev_mrange.min));
    return colormap[index];
}

//
// Draw a picture
//

Range draw(const complex<long double>& p, const long double range,
    const int width, const int height, const string& name) {
    Range mrange(DBL_MAX, 0);
    const long double scale = range * 2 / width;
    cv::Mat image(height, width, CV_8UC3);
#pragma omp parallel for
    for (int i = 0; i < height; i++) {
	long double y = p.imag() + (height / 2 - i) * scale;
	cv::Vec3b *ip = image.ptr<cv::Vec3b>(i);
	for (int j = 0; j < width; j++) {
	    long double x = p.real() + (j - width / 2) * scale;
	    const complex<long double> z(x, y);
	    const double m = mandelbrot(z);
	    if (mrange.min > m) mrange.min = m;
	    if (mrange.max < m) mrange.max = m;
	    ip[j] = color(m);
	}
    }
    cv::imwrite(name, image);
    prev_mrange = mrange;
    return mrange;
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
	cerr << "invalid number of parameters" << endl;
	return 1;
    }
    char** p = argv;
    const long double real = stold(*++p);
    const long double imag = stold(*++p);
    const long double range_s = stold(*++p);
    const long double range_e = stold(*++p);
    const int frames = stoi(*++p);
    const int width = stoi(*++p);
    const int height = stoi(*++p);

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

    init_colormap();

    const complex<long double> z(real, imag);
    draw(z, range_s, width, height, filename(0));
    const long double log_rate = (log(range_e) - log(range_s)) / (frames - 1);
    for (int i = 0; i < frames; i++) {
	const string name = filename(i);
	const long double r = range_s * exp(i * log_rate);
	const long double x = (long double)(frames - i) / frames;
	Range mr = draw(z, r, width, height, name);
	cout << i << " " << setprecision(55) << r << " " << setprecision(5) << mr.min << " " << mr.max << endl;
    }
}

