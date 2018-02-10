#include "stdafx.h"
#include "bmp.h"

void printUsage();

int main(int argc, char **argv)
{
	/*
	 * The input and output file path are required parameters.
	 */
	if (argc < 3)
	{
		printUsage();

		return -1;
	}

	const char *inputPath = argv[1], *outputPath = argv[2];

	BitmapFile *bitmapFile;

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

	/*
	 * The window is the same size as the image, which is kinda unsafe for larger
	 * images but works fairly well for our case.
	 */
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
		
		/*
		 * Actually, there's no need for this many redraws, but hey
		 * this is just a simple BMP Viewer, it ain't gonna crash your PC.
		 */
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
