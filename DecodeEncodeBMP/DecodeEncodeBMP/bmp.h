#ifndef BMP_H
#define BMP_H

#include <cstdlib>
#include <cstdio>
#include <intrin.h>

typedef struct BitmapFileHeader
{
	unsigned short int magic;
	unsigned int fileSize;
	unsigned short int garbage[2];
	unsigned int offset;
} BitmapFileHeader;

typedef struct BitmapInformationHeader
{
	unsigned int headerSize;
	int width, height;
	unsigned short int planes;
	unsigned short int bitsPerPixel;
	unsigned int compressionMethod;
	unsigned int imageSize;
	unsigned int horizontalResolution, verticalResolution;
	unsigned int colorCount;
	unsigned int importantColors;
} BitmapInformationHeader;

typedef struct BitmapFile
{
	BitmapFileHeader fileHeader;
	BitmapInformationHeader infoHeader;
	void *imageData;
} BitmapFile;

int load_bitmap_file(char *path, BitmapFile **bitmapFile)
{
	FILE *fp;
	*bitmapFile = nullptr;

	if (fopen_s(&fp, path, "rb")) 
	{
		return -1;
	}

	BitmapFile *filePtr = (BitmapFile *)malloc(sizeof(BitmapFile));

	fread_s(&filePtr->fileHeader, 14, 14, 1, fp);

	fread_s(&filePtr->infoHeader, 40, 40, 1, fp);

	unsigned int rowSize = ((filePtr->infoHeader.bitsPerPixel * filePtr->infoHeader.width + 31) / 32) * 4;
	unsigned int imageDataSize = rowSize * filePtr->infoHeader.height;

	filePtr->imageData = (char *)malloc(imageDataSize);

	fread_s(filePtr->imageData, imageDataSize, imageDataSize, 1, fp);

	*bitmapFile = filePtr;

	return 0;
}

#endif // BMP_H