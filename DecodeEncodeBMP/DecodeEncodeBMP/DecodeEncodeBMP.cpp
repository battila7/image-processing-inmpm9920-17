// DecodeEncodeBMP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <GLFW/glfw3.h>
#include "bmp.h"

int main(void)
{
	BitmapFile *bitmapFile;
	char *data;

	if (load_bitmap_file("C:\\Users\\Attila\\bmp.bmp", &bitmapFile))
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

	return 0;
}
