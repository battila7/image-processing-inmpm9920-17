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

void slowFourierTransform(unsigned char *data, const int width, const int height, std::vector<unsigned char> &outResult, double sign)
{
	using complex = std::complex<double>;

	std::vector<double> norms;

	double size = (double)(width * height);

	for (long v = 0; v < height; ++v)
	{
		for (long u = 0; u < width; ++u)
		{
			auto sum = complex(0.0, 0.0);

			for (long y = 0; y < height; ++y)
			{
				for (long x = 0; x < width; ++x)
				{
					double exponent = (v * y) / (double)height;
					exponent += (u * x) / (double)width;
					exponent *= sign * 2.0 * M_PI;

					// double d = ((double)data[3 * y * width + x * 3]) / 255.0;

					complex pointAt(data[3 * y * width + x * 3], 0);

					sum += pointAt * std::exp(complex{ 1, exponent });
				}
			}

			sum *= 1.0 / size;

			norms.push_back(std::abs(sum));
		}
	}

	double max = 0, min = 10000000;

	for (long i = 0; i < norms.size(); ++i)
	{
	    double res = std::log(1 + norms[i]);

		norms[i] = res;

		if (norms[i] < min)
		{
			min = norms[i];
		}

		if (norms[i] > max)
		{
			max = norms[i];
		}
	}

	for (long i = 0; i < norms.size(); ++i)
	{
		//double sc = 255.0 / std::log(1 + std::abs(max));

		//unsigned char res = (unsigned char)(sc * std::log(1 + std::abs(norms[i])));

		double sc = (norms[i] - min) / (max - min);
		unsigned char res = 255.0 * norms[i];

		outResult.push_back(res);
		outResult.push_back(res);
		outResult.push_back(res);
	}

	/*for (long i = 0; i < norms.size(); ++i)
	{
		unsigned char res = 255.0 * ((norms[i] - min) / (max - min));
		
		outResult.push_back(res);
		outResult.push_back(res);
		outResult.push_back(res);
	}*/
}

void fourierShift(unsigned char *data, const int width, const int height)
{
	// shift A and C
	long halfHeight = height / 2, halfWidth = width / 2;

	for (long y = 0; y < halfHeight; ++y)
	{
		for (long x = 0; x < halfWidth; ++x)
		{
			long posA = 3 * y * width + x * 3;
			long posC = 3 * (y + halfHeight) * width + (x + halfWidth) * 3;

			unsigned char tmp = data[posC];
			memset(data + posC, data[posA], 3);
			memset(data + posA, tmp, 3);
		}
	}

	// shift D and B
	for (long y = 0; y < halfHeight; ++y)
	{
		for (long x = halfWidth; x < width; ++x)
		{
			long posD = 3 * y * width + x * 3;
			long posB = 3 * (y + halfHeight) * width + (x - halfWidth) * 3;

			unsigned char tmp = data[posB];
			memset(data + posB, data[posD], 3);
			memset(data + posD, tmp, 3);
		}
	}
}

void toImageData(std::vector<double> &in, unsigned char *out)
{
	for (int i = 0; i < in.size(); ++i)
	{
		out[i * 3] = in[i];
		out[i * 3 + 1] = in[i];
		out[i * 3 + 2] = in[i];
	}
}

}
#endif
