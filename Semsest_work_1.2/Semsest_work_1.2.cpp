// Semsest_work_1.2.cpp: определяет точку входа для консольного приложения.
//


#include "stdafx.h"
#include <iostream>
#include <cmath>
#include<fstream>
using namespace std;

//Set structure packing mode to 2 Bytes
//It prevents structure for being extended by additional alignment Bytes
#pragma pack(push,2)
//C++11 fixed width integer types are used
struct BITMAPFILEHEADER {
	uint16_t bfType;
	uint32_t bfSize;
	uint32_t bfReserved1;
	uint32_t bfOffBits;
};

//C++11 fixed width integer types are used
struct BITMAPINFOHEADER {
	uint32_t biSize;
	int32_t  biWidth;
	int32_t  biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t  biXPelsPerMeter;
	int32_t  biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};

//C++11 fixed width integer types are used
struct RGBTRIPLE {
	uint8_t rgbtBlue;
	uint8_t rgbtGreen;
	uint8_t rgbtRed;
};

struct HSL {
	double H;
	double S;
	double L;
};

void convert_rgb_to_hsl(RGBTRIPLE pixel, HSL &hsl);
double max(double a, double b, double c);
double min(double a, double b, double c);

void setlight(double power, HSL &hsl);

void convert_hsl_to_rgb(RGBTRIPLE &pixel, HSL hsl);
void normalization(double &tc);
double convert_by_color(double p, double q, double tc);

int setlight_rgb_by_color(double power, int color);

int main()
{
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bfi;

	ifstream fin("input.bmp", ios::binary);
	fin.read((char*)&bfh, 14);
	fin.read((char*)&bfi, 40);

	int width = bfi.biWidth;
	int height = bfi.biHeight;

	//determine line size in bytes
	//RGBTRIPLE bytes (width*3) + additional trash of (width%4) bytes
	int line = width * 3 + width % 4;
	//image size (should be equal to bfi.biSize)
	int size = line*height;

	RGBTRIPLE **pixels = new RGBTRIPLE*[height];
	for (int i = height - 1; i >= 0; i--)
	{
		pixels[i] = (RGBTRIPLE*)(new char[size]);
		fin.read((char*)pixels[i], line);
	}

	fin.close();

	//proces
	double light = 0;
	cout << "Enter light:";
	cin >> light;
	HSL hsl;
	for (int i = 0;i < bfi.biHeight;++i) {
		for (int j = 0;j < bfi.biWidth;++j) {
			convert_rgb_to_hsl(pixels[i][j], hsl);
			if (hsl.H != NULL) {
				setlight(light, hsl);
				convert_hsl_to_rgb(pixels[i][j], hsl);
			}
			else {
				pixels[i][j].rgbtBlue = setlight_rgb_by_color(light, pixels[i][j].rgbtBlue);
				pixels[i][j].rgbtGreen = setlight_rgb_by_color(light, pixels[i][j].rgbtGreen);
				pixels[i][j].rgbtRed = setlight_rgb_by_color(light, pixels[i][j].rgbtRed);
			}
		}
	}

	ofstream fout("outputphoto.bmp", ios::binary);
	fout.write((char*)&bfh, sizeof(bfh));
	fout.write((char*)&bfi, sizeof(bfi));

	//Writing pixels to file and deallocating memory
	for (int i = height - 1; i >= 0; i--)
	{
		fout.write((char*)pixels[i], line);
		delete[]pixels[i];
	}
	delete[]pixels;

	fout.close();
	return 0;
}

void convert_rgb_to_hsl(RGBTRIPLE Pixel, HSL &hsl) {
	double r, g, b;
	// from 0-255 to 0-1
	b = Pixel.rgbtBlue / 255.0;
	g = Pixel.rgbtGreen / 255.0;
	r = Pixel.rgbtRed / 255.0;
	//set max, min
	double MAX, MIN;
	MAX = max(r, g, b);
	MIN = min(r, g, b);
	//set L
	hsl.L = (double)((MAX + MIN) / 2.0);
	//set H
	if (MAX == MIN) {
		hsl.H = NULL;
	}
	else if (MAX == r && g >= b) {
		hsl.H = 60 * (double)((g - b) / (MAX - MIN));
	}
	else if (MAX == r && g < b) {
		hsl.H = 60 * (double)((g - b) / (MAX - MIN)) + 360;
	}
	else if (MAX == g) {
		hsl.H = (double)((b - r) / (MAX - MIN)) + 120;
	}
	else if (MAX == b) {
		hsl.H = 60 * (double)((r - g) / (MAX - MIN)) + 240;
	}
	//set S
	if (hsl.L == 0 || MAX == MIN) {
		hsl.S = 0;
	}
	else if (hsl.L <= 0.5) {
		hsl.S = double((MAX - MIN) / (MAX + MIN));
	}

}
double max(double a, double b, double c) {
	if (a >= b) {
		if (a >= c) {
			return a;
		}
		else {
			return c;
		}
	}
	else {
		if (b >= c) {
			return b;
		}
		else {
			return c;
		}
	}
}
double min(double a, double b, double c) {
	if (a <= b) {
		if (a <= c) {
			return a;
		}
		else
		{
			return c;
		}
	}
	else {
		if (b <= c) {
			return b;
		}
		else {
			return c;
		}
	}
}

void setlight(double power, HSL &hsl) {
	hsl.L -= power;
	if (hsl.L > 1) {
		hsl.L = 1;
	}
	else if (hsl.L < 0) {
		hsl.L = 0;
	}
}

void convert_hsl_to_rgb(RGBTRIPLE &pixel, HSL hsl) {
	double q, p, hk, tr, tg, tb;
	if (hsl.L < 0.5) {
		q = hsl.L*(1.0 + hsl.S);
	}
	else {
		q = hsl.L + hsl.S - (hsl.L*hsl.S);
	}
	p = 2 * hsl.L - q;
	hk = hsl.H / 360.0;
	tr = hk + (1.0 / 3.0);
	tg = hk;
	tb = hk - (1.0 / 3.0);
	//normalize
	normalization(tr);
	normalization(tg);
	normalization(tb);
	// set colors
	pixel.rgbtRed = (int)(convert_by_color(p, q, tr) * 255);
	pixel.rgbtGreen = (int)(convert_by_color(p, q, tg) * 255);
	pixel.rgbtBlue = (int)(convert_by_color(p, q, tb) * 255);
}
void normalization(double &tc) {
	if (tc < 0) {
		tc += 1;
	}
	else if (tc > 1) {
		tc -= 1;
	}
}
double convert_by_color(double p, double q, double tc) {
	if (tc < (1.0 / 6.0)) {
		return p + (double)((q - p) * 6.0 * tc);
	}
	else if (tc < 0.5) {
		return q;
	}
	else if (tc < (1.0 / 3.0)) {
		return p + ((q - p)*((2.0 / 3.0) - tc) * 6.0);
	}
	else {
		return p;
	}
}

int setlight_rgb_by_color(double power, int color) {
	color -= 255 * power;
	if (color < 0) {
		color = 0;
	}
	else if (color > 255) {
		color = 255;
	}
	return color;
}
