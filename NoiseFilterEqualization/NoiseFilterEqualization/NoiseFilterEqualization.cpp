#include "stdafx.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image_funcs.h"

void printUsage();
bool createTextureFromImageData(unsigned char *data, const int width, const int height, GLuint *tex);
GLFWwindow *initializeWindow(const int width, const int height, const char *title);
bool loadImage(const char *path, unsigned char **data, int *width, int *height);

constexpr int BINARY_CONVERSION_THRESHOLD = 127;
constexpr int NOISE_PERCENTAGE = 5;
constexpr int MEAN_WINDOW_SIZE = 5;

int main(const int argc, const char **argv)
{
	if (argc < 2)
	{
		printUsage();

		return -1;
	}

	const char *inputPath = argv[1];

	int imageWidth, imageHeight;
	unsigned char *imageData;

	if (!loadImage(inputPath, &imageData, &imageWidth, &imageHeight))
	{
		printf("Could not load image \"%s\"\n", inputPath);

		return -1;
	}

	if (!imgf::isGrayscale(imageData))
	{
		imgf::convertToGrayscale(imageData, imageWidth, imageHeight);
	}

	imgf::histogramEqualization(imageData, imageWidth, imageHeight);

	imgf::convertToBinary(imageData, imageWidth, imageHeight, BINARY_CONVERSION_THRESHOLD);

	imgf::additiveBinaryNoise(imageData, imageWidth, imageHeight, NOISE_PERCENTAGE);

	imgf::meanFilter(&imageData, imageWidth, imageHeight, MEAN_WINDOW_SIZE);

	GLFWwindow *window = initializeWindow(imageWidth, imageHeight, inputPath);

	GLuint tex;
	createTextureFromImageData(imageData, imageWidth, imageHeight, &tex);

	GLuint textureFbo = 0;
	glGenFramebuffers(1, &textureFbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, textureFbo);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, tex, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	while (!glfwWindowShouldClose(window)) 
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, imageWidth, imageHeight);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, textureFbo);
		glBlitFramebuffer(0, 0, imageWidth, imageHeight,
						  0, 0, imageWidth, imageHeight,
						  GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		
		glfwPollEvents();

		if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose(window, 1);
		}
		
		glfwSwapBuffers(window);
	}

	glfwTerminate();

	return 0;
}

void printUsage()
{
	printf("Hey, ya! Having problems running the state-of-art Noise & Stuff program? Check this out, chap:\n");
	printf("\NoiseFilterEqualization <input.file>\n");
}

bool createTextureFromImageData(unsigned char *data, const int width, const int height, GLuint *tex)
{
	int force_channels = 3;
	int widthInBytes = width * 3;

	unsigned char *top = nullptr;
	unsigned char *bottom = nullptr;
	unsigned char temp = 0;
	int halfHeight = height / 2;

	for (int row = 0; row < halfHeight; row++)
	{
		top = data + row * widthInBytes;
		bottom = data + (height - row - 1) * widthInBytes;

		for (int col = 0; col < widthInBytes; col++)
		{
			temp = *top;
			*top = *bottom;
			*bottom = temp;

			top++;
			bottom++;
		}
	}

	glGenTextures(1, tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, *tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	GLfloat max_aniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);

	return true;
}

GLFWwindow *initializeWindow(const int width, const int height, const char *title)
{
	if (!glfwInit())
	{
		printf("Could not start GLFW3\n");

		return nullptr;
	}

	GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);

	if (!window)
	{
		printf("Could not open window with GLFW3\n");

		glfwTerminate();

		return nullptr;
	}

	glfwMakeContextCurrent(window);

	glfwWindowHint(GLFW_SAMPLES, 4);

	glewExperimental = GL_TRUE;
	glewInit();

	return window;
}

bool loadImage(const char *path, unsigned char **data, int *width, int *height)
{
	int channels;

	*data = stbi_load(path, width, height, &channels, 3);

	return *data != nullptr;
}
