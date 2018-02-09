// DecodeEncodeBMP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "bmp.h"

void printUsage();

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		printUsage();

		return -1;
	}

	const char *inputPath = argv[1], *outputPath = argv[2];

	BitmapFile *bitmapFile;
	char *data;

	if (loadBmpFile(inputPath, &bitmapFile))
	{
		printf("Could not load BMP file \"%s\"!\n", inputPath);

		return -1;
	}

	if (writeBmpFile(outputPath, bitmapFile))
	{
		printf("Could not save BMP file \"%s\"!\n", outputPath);

		return -1;
	}

	GLFWwindow *window;

	if (!glfwInit())
	{
		printf("Failed to initialize GLFW!\n");

		return -1;
	}

	window = glfwCreateWindow(bitmapFile->infoHeader.width, bitmapFile->infoHeader.height, "BMP Viewer", NULL, NULL);

	if (!window)
	{
		printf("Failed to create viewer window!\n");

		glfwTerminate();

		return -1;
	}

	glfwMakeContextCurrent(window);

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawPixels(bitmapFile->infoHeader.width, bitmapFile->infoHeader.height, GL_BGR_EXT, GL_UNSIGNED_BYTE, bitmapFile->imageData);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();

	free(bitmapFile->imageData);
	
	free(bitmapFile);

	return 0;
}

void printUsage()
{
	printf("Hey, ya! Having problems running the state-of-art BMP Viewer? Check this out, chap:\n");
	printf("\nDecodeEncodeBMP <input.file> <output.file>\n");
}
