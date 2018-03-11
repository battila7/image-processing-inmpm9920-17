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
			result.push_back(std::complex<double>(data[COMPONENT_COUNT * y * width + x * COMPONENT_COUNT], 0));
		}
	}
}

void sft(std::vector<std::complex<double>> &data, const int width, const int height, double sign, std::vector<std::complex<double>> &result)
{
	double size = (double)(width * height);

	for (double v = 0; v < height; ++v)
	{
		for (double u = 0; u < width; ++u)
		{
			auto sum = std::complex<double>(0.0, 0.0);

			for (double y = 0; y < height; ++y)
			{
				for (double x = 0; x < width; ++x)
				{
					double exponent = (v * y) / (double)height;
					exponent += (u * x) / (double)width;
					exponent *= sign * 2.0 * M_PI;

					sum += data[y * width + x] * std::exp(std::complex<double>( 1, exponent ));
				}
			}

			sum *= 1.0 / std::sqrt(size);

			result.push_back(sum);
		}
	}
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

void slowFourierTransform(unsigned char *data, const int width, const int height, std::vector<unsigned char> &outResult, double sign)
{
	using complex = std::complex<double>;

	std::vector<double> norms;

	double size = (double)(width * height);

	for (double v = 0; v < height; ++v)
	{
		for (double u = 0; u < width; ++u)
		{
			auto sum = complex(0.0, 0.0);

			double re = 0;
			double im = 0;

			for (double y = 0; y < height; ++y)
			{
				for (double x = 0; x < width; ++x)
				{
					double exponent = (v * y) / (double)height;
					exponent += (u * x) / (double)width;
					exponent *= 2.0 * M_PI;

					double s = data[(long)(3.0 * y * width + x * 3.0)];

					re += (double)s * std::cos(exponent);
					im -= (double)s * std::sin(exponent);

					// double d = ((double)data[3 * y * width + x * 3]) / 255.0;

					exponent *= sign;

					complex pointAt(s, 0);

					sum += pointAt * std::exp(complex{ 1, exponent });
				}
			}

			sum *= 1.0 / std::sqrt(size);

			re *= 1.0 / std::sqrt(size);
			im *= 1.0 / std::sqrt(size);

			norms.push_back(std::abs(sum));
			//norms.push_back(std::sqrt(re * re + im * im));
		}
	}

	double max = 0, min = 10000000;

	for (long i = 0; i < norms.size(); ++i)
	{
	    double res = std::log(1 + norms[i]);

		if (norms[i] < min)
		{
			min = norms[i];
		}

		if (norms[i] > max)
		{
			max = norms[i];
		}

		//norms[i] = res;
	}

	for (long i = 0; i < norms.size(); ++i)
	{
		//double sc = 255.0 / std::log(1 + std::abs(max));

		//unsigned char res = (unsigned char)(sc * std::log(1 + std::abs(norms[i])));

		double sc = (norms[i] - min) / (max - min);
		unsigned char res = (255.0 / std::log(1 + max)) * std::log(1 + norms[i]);

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
