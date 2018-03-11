#ifndef IMAGE_FUNCS_H
#define IMAGE_FUNCS_H

#define _USE_MATH_DEFINES
#include <random>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <complex>

enum Component
{
	R = 0,
	G = 1,
	B = 2
};

constexpr int COMPONENT_COUNT = 3;
constexpr int BUCKET_COUNT = 256;

constexpr float R_WEIGHT = 0.3f;
constexpr float G_WEIGHT = 0.59f;
constexpr float B_WEIGHT = 0.11f;

constexpr int MIN_RGB_VALUE = 0;
constexpr int MAX_RGB_VALUE = 255;

namespace imgf
{

int indexOf(const int x, const int y, const int width)
{
	return y * width * COMPONENT_COUNT + x * COMPONENT_COUNT;
}

bool isGrayscale(const unsigned char *data)
{
	return data[0] == data[1] == data[2];
}

void convertToGrayscale(unsigned char *data, const int width, const int height)
{
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			const int position = indexOf(x, y, width);
			const int grayValue = R_WEIGHT * data[position + R]
				+ G_WEIGHT * data[position + G]
				+ B_WEIGHT * data[position + B];

			memset(data + position, grayValue, COMPONENT_COUNT);
		}
	}
}

void toComplexImage(unsigned char *data, const int width, const int height, std::vector<std::complex<double>> &result)
{
	for (long y = 0; y < height; ++y)
	{
		for (long x = 0; x < width; ++x)
		{
			result.push_back(std::complex<double>(data[COMPONENT_COUNT * y * width + x * COMPONENT_COUNT]));
		}
	}
}

void butterworthLowPassFilter(std::vector<std::complex<double>> &data, const int width, const int height, double cutoff, double order)
{
	for (long y = 0; y < height; ++y)
	{
		for (long x = 0; x < width; ++x)
		{
			long shiftedY = y - (height / 2);
			long shiftedX = x - (width / 2);

			double dist = std::sqrt(shiftedY * shiftedY + shiftedX * shiftedX);

			double h = 1.0 / (1.0 + std::pow(dist / cutoff, 2 * order));

			data[width * y + x] *= h;
		}
	}
}

void slowFourierTransform(std::vector<std::complex<double>> &data, const int width, const int height, double sign, std::vector<std::complex<double>> &result)
{
	double size = (double)(width * height);

	double step = size / 10.0;

	double percent = 0;

	for (double v = 0; v < height; ++v)
	{
		for (double u = 0; u < width; ++u)
		{
			double current = v * width + u, intpart;

			if (std::modf((current + width + 1) / step, &intpart) == 0)
			{
				printf("  %.0f%%", ((current + width + 1) / step) * 10);
			}

			auto sum = std::complex<double>(0.0, 0.0);

			for (double y = 0; y < height; ++y)
			{
				for (double x = 0; x < width; ++x)
				{
					double exponent = (v * y) / (double)height;
					exponent += (u * x) / (double)width;
					exponent *= sign * 2.0 * M_PI;

					sum += data[y * width + x] * std::exp(std::complex<double>( 0, exponent ));
				}
			}

			sum *= 1.0 / std::sqrt(size);

			result.push_back(sum);
		}
	}

	printf("\n");
}

void flipQuadrants(std::vector<std::complex<double>> &data, const int width, const int height)
{
	// flip A and C
	long halfHeight = height / 2, halfWidth = width / 2;

	for (long y = 0; y < halfHeight; ++y)
	{
		for (long x = 0; x < halfWidth; ++x)
		{
			long posA = y * width + x;
			long posC = (y + halfHeight) * width + (x + halfWidth);

			std::complex<double> tmp = data[posC];
			data[posC] = data[posA];
			data[posA] = tmp;
		}
	}

	// flip D and B
	for (long y = 0; y < halfHeight; ++y)
	{
		for (long x = halfWidth; x < width; ++x)
		{
			long posD = y * width + x;
			long posB = (y + halfHeight) * width + (x - halfWidth);

			std::complex<double> tmp = data[posB];
			data[posB] = data[posD];
			data[posD] = tmp;
		}
	}
}

void extractMagnitude(std::vector<std::complex<double>> &data, const int width, const int height, std::vector<double> &result)
{
	double max = 0;

	for (long i = 0; i < data.size(); ++i)
	{
		result.push_back(std::abs(data[i]));

		if (result[i] > max)
		{
			max = result[i];
		}
	}

	for (long i = 0; i < result.size(); ++i)
	{
		unsigned char res = (255.0 / std::log(1 + max)) * std::log(1 + result[i]);

		result[i] = res;
	}
}

void toBrightnessImage(std::vector<double> &data, std::vector<unsigned char> &result)
{
	for (long i = 0; i < data.size(); ++i)
	{
		result.push_back((unsigned char)data[i]);
		result.push_back((unsigned char)data[i]);
		result.push_back((unsigned char)data[i]);
	}
}

}
#endif
