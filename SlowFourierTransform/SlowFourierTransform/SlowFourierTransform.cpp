#include "stdafx.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "stb_image_write.h"
#include "image_funcs.h"

void printUsage();
bool createTextureFromImageData(unsigned char *data, const int width, const int height, GLuint *tex);
GLFWwindow *initializeWindow(const int width, const int height, const char *title);
bool loadImage(const char *path, unsigned char **data, int *width, int *height);
bool writeImageWithPostfix(const char *pathPrefix, const char *pathPostfix, unsigned char *data, int width, int height);
bool writeComplexImageWithPostfix(const char *pathPrefix, const char *pathPostfix, std::vector<std::complex<double>> &data, int width, int height);

constexpr float WINDOW_SCALING = 1.0f;

const char *FOURIER_POSTFIX = "-1-fourier.bmp";
const char *FOURIER_FILTERED_POSTFIX = "-2-filtered-fourier.bmp";
const char *RESULT_POSTFIX = "-3-result.bmp";

enum CommandLineParams
{
	INPUT_PATH = 1,
	OUTPUT_PREFIX
};

int main(const int argc, const char **argv)
{
	if (argc < 3)
	{
		printUsage();

		return -1;
	}

	const char *inputPath = argv[INPUT_PATH];
	const char *outputPrefix = argv[OUTPUT_PREFIX];

	int imageWidth, imageHeight;
	unsigned char *imageData;

	if (!loadImage(inputPath, &imageData, &imageWidth, &imageHeight))
	{
		printf("Could not load image \"%s\"\n", inputPath);

		return -1;
	}


	if (!imgf::isGrayscale(imageData))
	{
		printf("Performing grayscale conversion.\n");
		imgf::convertToGrayscale(imageData, imageWidth, imageHeight);
	}

	std::vector<std::complex<double>> complexImage, fourierSpaceImage;
	std::vector<double> magnitudeImage;
	std::vector<unsigned char> a;

	imgf::toComplexImage(imageData, imageWidth, imageHeight, complexImage);
	printf("Performing Fourier transform.\n");
	imgf::slowFourierTransform(complexImage, imageWidth, imageHeight, -1, fourierSpaceImage);
	imgf::flipQuadrants(fourierSpaceImage, imageWidth, imageHeight);
	writeComplexImageWithPostfix(outputPrefix, FOURIER_POSTFIX, fourierSpaceImage, imageWidth, imageHeight);

	imgf::butterworthLowPassFilter(fourierSpaceImage, imageWidth, imageHeight, 15, 2);
	printf("Applying Butterworth low-pass filter.\n");
	writeComplexImageWithPostfix(outputPrefix, FOURIER_FILTERED_POSTFIX, fourierSpaceImage, imageWidth, imageHeight);

	imgf::flipQuadrants(fourierSpaceImage, imageWidth, imageHeight);
	complexImage.clear();
	printf("Performing inverse Fourier transform.\n");
	imgf::slowFourierTransform(fourierSpaceImage, imageWidth, imageHeight, 1, complexImage);
	writeComplexImageWithPostfix(outputPrefix, RESULT_POSTFIX, complexImage, imageWidth, imageHeight);

	imgf::extractMagnitude(complexImage, imageWidth, imageHeight, magnitudeImage);
	imgf::toBrightnessImage(magnitudeImage, a);

	GLFWwindow *window = initializeWindow(imageWidth * WINDOW_SCALING, imageHeight * WINDOW_SCALING, inputPath);

	GLuint tex;
	createTextureFromImageData(a.data(), imageWidth, imageHeight, &tex);

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
			0, 0, imageWidth * WINDOW_SCALING, imageHeight * WINDOW_SCALING,
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
	printf("Hey, ya! Having problems running the best-of-breed Fourier Transform program? Check this out, chap:\n");
	printf("\SlowFourierTransform <input.file> <output prefix>\n");
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

bool writeComplexImageWithPostfix(const char *pathPrefix, const char *pathPostfix, std::vector<std::complex<double>> &data, int width, int height)
{
	std::vector<double> magnitudeImage;
	std::vector<unsigned char> brightnessImage;

	imgf::extractMagnitude(data, width, height, magnitudeImage);
	imgf::toBrightnessImage(magnitudeImage, brightnessImage);

	return writeImageWithPostfix(pathPrefix, pathPostfix, brightnessImage.data(), width, height);
}

bool writeImageWithPostfix(const char *pathPrefix, const char *pathPostfix, unsigned char *data, int width, int height)
{
	char buffer[512];

	memset(buffer, 0, 512);

	strcpy(buffer, pathPrefix);
	strcpy(buffer + strlen(pathPrefix), pathPostfix);

	printf("  Writing to output file: %s\n", buffer);

	return stbi_write_bmp(buffer, width, height, 3, data);
}
