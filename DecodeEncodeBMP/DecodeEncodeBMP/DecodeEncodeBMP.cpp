// DecodeEncodeBMP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <GLFW/glfw3.h>
#include "bmp.h"

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		return -1;
	}

	char *inputPath = argv[1], *outputPath = argv[2];

	BitmapFile *bitmapFile;
	char *data;

	if (loadBmpFile(inputPath, &bitmapFile))
	{
		return -1;
	}

	if (writeBmpFile(outputPath, bitmapFile))
	{
		return -1;
	}

	GLFWwindow *window;

	if (!glfwInit())
	{
		return -1;
	}

	window = glfwCreateWindow(bitmapFile->infoHeader.width, bitmapFile->infoHeader.height, "BMP Viewer", NULL, NULL);

	if (!window)
	{
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
