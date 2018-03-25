#ifndef IMAGE_FUNCS_H
#define IMAGE_FUNCS_H

#include <random>
#include <cstdint>
#include <algorithm>

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

struct CharacterPosition
{
	int topLeftLine, topLeftColumn;
	int bottomRightLine, bottomRightColumn;
};

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

void convertToBinary(unsigned char *data, const int width, const int height, const int threshold)
{
	if ((threshold < MIN_RGB_VALUE) || (threshold > MAX_RGB_VALUE))
	{
		return;
	}

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			const int newValue = data[indexOf(x, y, width) + R] < threshold ? 0 : 255;

			memset(data + indexOf(x, y, width), newValue, COMPONENT_COUNT);
		}
	}
}

int firstNonBlankPixelInLine(const unsigned char *data, const int width, const int height, const int line)
{
	for (int i = 0; i < width; ++i)
	{
		const int index = line * width * COMPONENT_COUNT + i * COMPONENT_COUNT;

		if (data[index] == 0)
		{
			return i;
		}
	}

	return -1;
}

int firstNonBlankPixelInColumn(const unsigned const char *data, const int width, const int height, const int firstLine, const int lastLine, const int column)
{
	for (int line = firstLine; line <= lastLine; ++line)
	{
		const int index = line * width * COMPONENT_COUNT + column * COMPONENT_COUNT;

		if (data[index] == 0)
		{
			return line;
		}
	}

	return -1;
}

int createSegmentsFromLine(unsigned char *data, const int width, const int height, const int firstLine, std::vector<CharacterPosition> &positions)
{
	int lastLine = firstLine + 1;

	do
	{
		int p = firstNonBlankPixelInLine(data, width, height, lastLine);

		if (p == -1)
		{
			--lastLine; // decrease it so, it points to the last line with non blank content

			break;
		}
		else
		{
			++lastLine;
		}
	} while (lastLine < height);

	int startingColumn = 0;

	do
	{
		int p = firstNonBlankPixelInColumn(data, width, height, firstLine, lastLine, startingColumn);

		if (p != -1)
		{
			int lastColumn = startingColumn + 1;

			do
			{
				if (firstNonBlankPixelInColumn(data, width, height, firstLine, lastLine, lastColumn) == -1)
				{
					positions.push_back({ firstLine, startingColumn, lastLine, lastColumn - 1 });

					break;
				}
				else
				{
					++lastColumn;
				}
			} while (lastColumn < width);

			startingColumn = lastColumn + 1;
		}
		else
		{
			++startingColumn;
		}
	} while (startingColumn < width);

	return lastLine;
}

std::vector<CharacterPosition> createSegmentsFromImage(unsigned char *data, const int width, const int height)
{
	std::vector<CharacterPosition> positions;
	int lineIndex = 0;

	do
	{
		int p = firstNonBlankPixelInLine(data, width, height, lineIndex);

		if (p != -1)
		{
			lineIndex = createSegmentsFromLine(data, width, height, lineIndex, positions) + 1;
		}
		else
		{
			++lineIndex;
		}
	} while (lineIndex < height);

	return positions;
}

unsigned char *characterPositionToImageData(unsigned char *originalImage, const int width,  const CharacterPosition &position)
{
	int charSize = (position.bottomRightLine - position.topLeftLine + 1) * (position.bottomRightColumn - position.topLeftColumn + 1) * COMPONENT_COUNT;

	unsigned char *data = new unsigned char[charSize];

	int dataIndex = 0;

	for (int line = position.topLeftLine; line <= position.bottomRightLine; ++line)
	{
		for (int column = position.topLeftColumn; column <= position.bottomRightColumn; ++column)
		{
			const int index = line * width * COMPONENT_COUNT + column * COMPONENT_COUNT;

			data[dataIndex * COMPONENT_COUNT] = originalImage[index];
			data[dataIndex * COMPONENT_COUNT + 1] = originalImage[index + 1];
			data[dataIndex * COMPONENT_COUNT + 2] = originalImage[index + 2];

			++dataIndex;
		}
	}

	return data;
}

int meanFilter(unsigned char **data, const int width, const int height, const int windowSize)
{
	if (!(windowSize % 2))
	{
		return -1;
	}

	const int dataSize = height * width * COMPONENT_COUNT;

	unsigned char *result = new unsigned char[dataSize];

	const int borderSize = windowSize / 2;

	memcpy_s(result, dataSize, *data, dataSize);

	for (int y = borderSize; y < (height - borderSize); ++y)
	{
		for (int x = borderSize; x < (width - borderSize); ++x)
		{
			int sum = 0;

			for (int windowY = -borderSize; windowY <= borderSize; ++windowY)
			{
				for (int windowX = -borderSize; windowX <= borderSize; ++windowX)
				{
					sum += (*data)[indexOf(x + windowX, y + windowY, width)];
				}
			}

			const int newValue = sum / (windowSize * windowSize);
			const int centerPosition = indexOf(x, y, width);

			memset(result + centerPosition, newValue, COMPONENT_COUNT);
		}
	}

	delete *data;

	*data = result;

	return 0;
}

int medianFilter(unsigned char **data, const int width, const int height, const int windowSize)
{
	if (!(windowSize % 2))
	{
		return -1;
	}

	std::vector<int> window(windowSize * windowSize);

	const int dataSize = height * width * COMPONENT_COUNT;

	unsigned char *result = new unsigned char[dataSize];

	const int borderSize = windowSize / 2;

	memcpy_s(result, dataSize, *data, dataSize);

	for (int y = borderSize; y < (height - borderSize); ++y)
	{
		for (int x = borderSize; x < (width - borderSize); ++x)
		{
			int windowIndex = 0;

			for (int windowY = -borderSize; windowY <= borderSize; ++windowY)
			{
				for (int windowX = -borderSize; windowX <= borderSize; ++windowX)
				{
					window[windowIndex++] = (*data)[indexOf(x + windowX, y + windowY, width)];
				}
			}

			const int centerIndex = (window.size() / 2) + 1;

			std::nth_element(window.begin(), window.begin() + centerIndex, window.end());

			const int newValue = window[centerIndex];
			const int centerPosition = indexOf(x, y, width);

			memset(result + centerPosition, newValue, COMPONENT_COUNT);
		}
	}

	delete *data;

	*data = result;

	return 0;
}

int additiveBinaryNoise(unsigned char *data, const int width, const int height, const int percentage)
{
	std::random_device randomDevice;
	std::mt19937 generator(randomDevice());
	std::uniform_int_distribution<> distribution(0, 99);

	int changedCount = 0;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			if (distribution(generator) < percentage)
			{
				const int newValue = (data[indexOf(x, y, width) + R] == MIN_RGB_VALUE) ? MAX_RGB_VALUE : MIN_RGB_VALUE;

				memset(data + indexOf(x, y, width), newValue, COMPONENT_COUNT);

				changedCount++;
			}
		}
	}

	printf("Actual noise is %f%%.\n", 100.f * ((float)changedCount) / ((float)(width * height)));

	return 0;
}

int makeHistogram(unsigned char *data, const int width, const int height, uint64_t **histogram)
{
	uint64_t *result = new uint64_t[BUCKET_COUNT];

	memset(result, 0, BUCKET_COUNT * sizeof(uint64_t));

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			result[data[indexOf(x, y, width)]] += 1;
		}
	}

	*histogram = result;

	return 0;
}

int histogramEqualization(unsigned char *data, const int width, const int height)
{
	uint64_t *histogram = nullptr;

	makeHistogram(data, width, height, &histogram);

	float *cumulativeHistogram = new float[BUCKET_COUNT];

	for (int i = 0; i < BUCKET_COUNT; ++i)
	{
		cumulativeHistogram[i] = (float)histogram[i] / (float)(width * height);

		if (i > 0)
		{
			cumulativeHistogram[i] += cumulativeHistogram[i - 1];
		}
	}

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			const int newValue = std::floor((float)(BUCKET_COUNT - 1) * cumulativeHistogram[data[indexOf(x, y, width)]]);

			memset(data + indexOf(x, y, width), newValue, COMPONENT_COUNT);
		}
	}

	delete cumulativeHistogram;

	delete histogram;

	return 0;

}

}
#endif
