#include "stdafx.h"
#include "bmp.h"

static constexpr unsigned int FILE_HEADER_SIZE = 14;
static constexpr unsigned int INFO_HEADER_SIZE = 40;

static constexpr unsigned short int BMP_MAGIC = 0x4d42;

int loadBmpFile(const char *path, BitmapFile **bitmapFile)
{
	FILE *fp;
	*bitmapFile = nullptr;

	if (fopen_s(&fp, path, "rb"))
	{
		return -1;
	}

	BitmapFile *filePtr = (BitmapFile *)malloc(sizeof(BitmapFile));

	if (!fread_s(&filePtr->fileHeader, FILE_HEADER_SIZE, FILE_HEADER_SIZE, 1, fp))
	{
		fclose(fp);
		return -1;
	}

	if (filePtr->fileHeader.magic != BMP_MAGIC)
	{
		fclose(fp);
		return -1;
	}

	if (!fread_s(&filePtr->infoHeader, INFO_HEADER_SIZE, INFO_HEADER_SIZE, 1, fp))
	{
		fclose(fp);
		return -1;
	}

	filePtr->imageData = (char *)malloc(filePtr->infoHeader.imageSize);

	if (!fread_s(filePtr->imageData, filePtr->infoHeader.imageSize, filePtr->infoHeader.imageSize, 1, fp))
	{
		fclose(fp);
		return -1;
	}

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

	fwrite(&bitmapFile->fileHeader, FILE_HEADER_SIZE, 1, fp);

	fwrite(&bitmapFile->infoHeader, INFO_HEADER_SIZE, 1, fp);

	fwrite(bitmapFile->imageData, bitmapFile->infoHeader.imageSize, 1, fp);

	fclose(fp);

	return 0;
}
