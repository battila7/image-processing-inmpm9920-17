#ifndef BMP_H
#define BMP_H

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
int loadBmpFile(const char *path, BitmapFile **bitmapFile);

/**
 * Saves the specified bitmap data at the passed path.
 *
 * @param path the path to save the bitmap at
 * @param bitmapFile the bitmap file data to save
 * @return 0 upon success, -1 otherwise
 */
int writeBmpFile(const char *path, const BitmapFile *bitmapFile);

#endif // BMP_H