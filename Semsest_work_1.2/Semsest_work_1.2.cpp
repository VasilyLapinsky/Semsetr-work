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

float max(float a, float b, float c);
float min(float a, float b, float c);

void fromRGBtoHSL(float rgb[], float hsl[])
{
	const float maxRGB = max(rgb[0], rgb[1], rgb[2]);
	const float minRGB = min(rgb[0], rgb[1], rgb[2]);
	const float delta2 = maxRGB + minRGB;
	hsl[2] = delta2 * 0.5f;

	const float delta = maxRGB - minRGB;
	if (delta < FLT_MIN)
		hsl[0] = hsl[1] = 0.0f;
	else
	{
		hsl[1] = delta / (hsl[2] > 0.5f ? 2.0f - delta2 : delta2);
		if (rgb[0] >= maxRGB)
		{
			hsl[0] = (rgb[1] - rgb[2]) / delta;
			if (hsl[0] < 0.0f)
				hsl[0] += 6.0f;
		}
		else if (rgb[1] >= maxRGB)
			hsl[0] = 2.0f + (rgb[2] - rgb[0]) / delta;
		else
			hsl[0] = 4.0f + (rgb[0] - rgb[1]) / delta;
	}
}

void fromHSLtoRGB(const float hsl[], float rgb[])
{
	if (hsl[1] < FLT_MIN)
		rgb[0] = rgb[1] = rgb[2] = hsl[2];
	else if (hsl[2] < FLT_MIN)
		rgb[0] = rgb[1] = rgb[2] = 0.0f;
	else
	{
		const float q = hsl[2] < 0.5f ? hsl[2] * (1.0f + hsl[1]) : hsl[2] + hsl[1] - hsl[2] * hsl[1];
		const float p = 2.0f * hsl[2] - q;
		float t[] = { hsl[0] + 2.0f, hsl[0], hsl[0] - 2.0f };

		for (int i = 0; i<3; ++i)
		{
			if (t[i] < 0.0f)
				t[i] += 6.0f;
			else if (t[i] > 6.0f)
				t[i] -= 6.0f;

			if (t[i] < 1.0f)
				rgb[i] = p + (q - p) * t[i];
			else if (t[i] < 3.0f)
				rgb[i] = q;
			else if (t[i] < 4.0f)
				rgb[i] = p + (q - p) * (4.0f - t[i]);
			else
				rgb[i] = p;
		}
	}
}

void setlight(double power, float hsl[]) {
	hsl[2] -= power;
	if (hsl[2] > 1) {
		hsl[2] = 1;
	}
	else if (hsl[2] < 0) {
		hsl[2] = 0;
	}
}

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
	float rgb[3], hsl[3];
	for (int i = 0;i < bfi.biHeight;++i) {
		for (int j = 0;j < bfi.biWidth;++j) {
			rgb[0] = pixels[i][j].rgbtRed / 255.0;
			rgb[1] = pixels[i][j].rgbtGreen / 255.0;
			rgb[2] = pixels[i][j].rgbtBlue / 255.0;
			fromRGBtoHSL(rgb, hsl);
			setlight(light, hsl);
			fromHSLtoRGB(hsl, rgb);
			pixels[i][j].rgbtRed = rgb[0] * 255;
			pixels[i][j].rgbtGreen = rgb[1] * 255;
			pixels[i][j].rgbtBlue = rgb[2] * 255;
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

float max(float a, float b, float c) {
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
float min(float a, float b, float c) {
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
