#ifndef BMP_H
#define BMP_H

#include <cstdlib>
#include <cstdio>
#include <intrin.h>

constexpr unsigned int FILE_HEADER_SIZE = 14;
constexpr unsigned int INFO_HEADER_SIZE = 40;

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

/**
 * Loads the BMP file from the specified path.
 * 
 * @param path the path to read from
 * @param bitmapFile the BitmapFile to read the data into
 * @return 0 upon success, -1 otherwise
 */
int loadBmpFile(const char *path, BitmapFile **bitmapFile)
{
	FILE *fp;
	*bitmapFile = nullptr;

	if (fopen_s(&fp, path, "rb")) 
	{
		return -1;
	}

	BitmapFile *filePtr = (BitmapFile *)malloc(sizeof(BitmapFile));

	fread_s(&filePtr->fileHeader, FILE_HEADER_SIZE, FILE_HEADER_SIZE, 1, fp);

	fread_s(&filePtr->infoHeader, INFO_HEADER_SIZE, INFO_HEADER_SIZE, 1, fp);

	filePtr->imageData = (char *)malloc(filePtr->infoHeader.imageSize);

	fread_s(filePtr->imageData, filePtr->infoHeader.imageSize, filePtr->infoHeader.imageSize, 1, fp);

	fclose(fp);

	*bitmapFile = filePtr;

	return 0;
}

/**
 * Saves the specified bitmap data at the passed path.
 *
 * @param path the path to save the bitmap at
 * @param bitmapFile the bitmap file data to save
 * @return 0 upon success, -1 otherwise
 */
int writeBmpFile(const char *path, const BitmapFile *bitmapFile)
{
	FILE *fp;

	if (fopen_s(&fp, path, "wb"))
	{
		return -1;
	}

	fwrite(&bitmapFile->fileHeader, FILE_HEADER_SIZE, 1, fp);

	fwrite(&bitmapFile->infoHeader, INFO_HEADER_SIZE, 1, fp);

	fwrite(bitmapFile->imageData, bitmapFile->infoHeader.imageSize, 1, fp);

	fclose(fp);

	return 0;
}

#endif // BMP_H