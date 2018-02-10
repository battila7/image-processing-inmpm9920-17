#include "stdafx.h"
#include "bmp.h"

static constexpr unsigned int FILE_HEADER_SIZE = 14;
static constexpr unsigned int INFO_HEADER_SIZE = 40;

static constexpr unsigned short int BMP_MAGIC = 0x4d42;

static constexpr unsigned short int SINGLE_PLANE = 1;
static constexpr unsigned short int RGB_NO_COMPRESSION = 0;
static constexpr unsigned short int TWENTY_FOUR_BITS_PER_PIXEL = 24;

/*
 * Checks if the specified header (and therefore, the whole file) contains valid
 * values. In order for a file to be considered valid, the following should be satisfied
 *   <ul>
 *     <li>The number of planes must be 1.</li>
 *	   <li>The number of bits per pixel must be 24.</li>
 *     <li>No compression should be applied to the values, and they must be in RGB color space.</li>
 *     <li>The image size must be grater than zero.</li>
 *   </ul>
 *
 * @param infoHeader the header of the file
 * @return 1 if the header is valid, 0 otherwise
 */
static int checkFileIntegrity(const BitmapInformationHeader *infoHeader)
{
	return (infoHeader->planes == SINGLE_PLANE)
		&& (infoHeader->bitsPerPixel == TWENTY_FOUR_BITS_PER_PIXEL)
		&& (infoHeader->compressionMethod == RGB_NO_COMPRESSION)
		&& (infoHeader->imageSize > 0);
}

int loadBmpFile(const char *path, BitmapFile **bitmapFile)
{
	FILE *fp;
	*bitmapFile = nullptr;

	if (fopen_s(&fp, path, "rb"))
	{
		return -1;
	}

	BitmapFile *filePtr = (BitmapFile *)malloc(sizeof(BitmapFile));

	/*
	 * Load the file header.
	 */
	if (!fread_s(&filePtr->fileHeader, FILE_HEADER_SIZE, FILE_HEADER_SIZE, 1, fp))
	{
		fclose(fp);
		return -1;
	}

	/*
	 * Check if it starts with the BMP magic value.
	 * If it doesn't, then the file is not a BMP for sure. Otherwise, chances are, we've hit a BMP.
	 */
	if (filePtr->fileHeader.magic != BMP_MAGIC)
	{
		fclose(fp);
		return -1;
	}

	/*
	 * Load the information header.
	 */
	if (!fread_s(&filePtr->infoHeader, INFO_HEADER_SIZE, INFO_HEADER_SIZE, 1, fp))
	{
		fclose(fp);
		return -1;
	}

	/*
	 * If the following function return success, that means, that we have a valid-ish
	 * information header, and it is safe to assume, that the file contains BMP data.
	 */
	if (!checkFileIntegrity(&filePtr->infoHeader))
	{
		fclose(fp);
		return -1;
	}

	filePtr->imageData = (char *)malloc(filePtr->infoHeader.imageSize);

	/*
	 * Load the image data of the specified size.
	 */
	if (!fread_s(filePtr->imageData, filePtr->infoHeader.imageSize, filePtr->infoHeader.imageSize, 1, fp))
	{
		fclose(fp);
		return -1;
	}

	/*
	 * "return" the pointer.
	 */
	*bitmapFile = filePtr;

	return 0;
}

int writeBmpFile(const char *path, const BitmapFile *bitmapFile)
{
	FILE *fp;

	if (fopen_s(&fp, path, "wb"))
	{
		return -1;
	}

	if (!fwrite(&bitmapFile->fileHeader, FILE_HEADER_SIZE, 1, fp))
	{
		fclose(fp);
		return -1;
	}

	if (!fwrite(&bitmapFile->infoHeader, INFO_HEADER_SIZE, 1, fp))
	{
		fclose(fp);
		return -1;
	}

	if (!fwrite(bitmapFile->imageData, bitmapFile->infoHeader.imageSize, 1, fp))
	{
		fclose(fp);
		return -1;
	}

	fclose(fp);

	return 0;
}
