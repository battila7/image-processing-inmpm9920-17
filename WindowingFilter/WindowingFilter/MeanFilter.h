#ifndef MEAN_FILTER
#define MEAN_FILTER

#include <random>

enum Component
{
	R = 0,
	G = 1,
	B = 2
};

int componentAt(unsigned char *data, const int x, const int y, const int width, Component component)
{
	return data[y * width * 3 + x * 3 + component];
}


int meanFilter(unsigned char *data, const int width, const int height, const int windowSize)
{
	if (!(windowSize % 2))
	{
		return -1;
	}

	const int borderSize = windowSize / 2;

	for (int y = borderSize; y < (height - borderSize); ++y)
	{
		for (int x = borderSize; x < (width - borderSize); ++x)
		{
			int sumR = 0, sumG = 0, sumB = 0;

			for (int windowY = -borderSize; windowY <= borderSize; ++windowY)
			{
				for (int windowX = -borderSize; windowX <= borderSize; ++windowX)
				{
					if (windowY == 0 && windowX == 0)
					{
						continue;
					}

					sumR += componentAt(data, x + windowX, y + windowY, width, R);
					sumG += componentAt(data, x + windowX, y + windowY, width, G);
					sumB += componentAt(data, x + windowX, y + windowY, width, B);
				}
			}

			const int denom = (windowSize * windowSize) - 1;

			data[y * width * 3 + x * 3 + R] = sumR / denom;
			data[y * width * 3 + x * 3 + G] = sumG / denom;
			data[y * width * 3 + x * 3 + B] = sumB / denom;
		}
	}

	return 0;
}

int additiveBinaryNoise(unsigned char *data, const int width, const int height, const int percentage)
{
	std::random_device rd; 
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 99);

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (dis(gen) < percentage)
			{
				const int newValue = componentAt(data, x, y, width, R) > 127 ? 0 : 255;

				data[y * width * 3 + x * 3 + R] = newValue;
				data[y * width * 3 + x * 3 + G] = newValue;
				data[y * width * 3 + x * 3 + B] = newValue;
			}
		}
	}

	return 0;
}

int makeHistogram(unsigned char *data, const int width, const int height, Component component, int *histogram, int *buckets)
{


	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{

		}
	}
}

#endif