#include "stdafx.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"""
#include "image_funcs.h"

void printUsage();
bool createTextureFromImageData(unsigned char *data, const int width, const int height, GLuint *tex);
GLFWwindow *initializeWindow(const int width, const int height, const char *title);
bool loadImage(const char *path, unsigned char **data, int *width, int *height);
bool writeImageWithPostfix(const char *pathPrefix, const char *pathPostfix, unsigned char *data, int width, int height);
bool writeCharacters(const char *pathPrefix, unsigned char *data, int width, std::vector<imgf::CharacterPosition> &positions);
bool writeCharacterPositions(const char *positionFilePath, std::vector<imgf::CharacterPosition> &positions);

constexpr int BINARY_CONVERSION_THRESHOLD = 127;

constexpr float WINDOW_SCALING = 0.33f;

enum CommandLineParams
{
	INPUT_PATH = 1,
	OUTPUT_PREFIX,
	POSITION_FILE_PATH
};

int main(const int argc, const char **argv)
{
	if (argc < 4)
	{
		printUsage();

		return -1;
	}

	const char *inputPath = argv[INPUT_PATH];
	const char *outputPrefix = argv[OUTPUT_PREFIX];
	const char *positionFilePath = argv[POSITION_FILE_PATH];

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

	printf("Converting picture to binary, using threshold %d.\n", BINARY_CONVERSION_THRESHOLD);
	imgf::convertToBinary(imageData, imageWidth, imageHeight, BINARY_CONVERSION_THRESHOLD);

	printf("Identifying characters.\n");
	std::vector<imgf::CharacterPosition> characterPositions = imgf::createSegmentsFromImage(imageData, imageWidth, imageHeight);
	printf("\tIdentified %d characters.\n", characterPositions.size());

	std::vector<imgf::CharacterPosition> finalPositions = imgf::refineSegments(imageData, imageWidth, imageHeight, characterPositions);

	writeCharacters(outputPrefix, imageData, imageWidth, finalPositions);

	printf("Writing character positions to %s\n", positionFilePath);
	writeCharacterPositions(positionFilePath, finalPositions);

	GLFWwindow *window = initializeWindow(imageWidth * WINDOW_SCALING, imageHeight * WINDOW_SCALING, inputPath);

	GLuint tex;
	createTextureFromImageData(imageData, imageWidth, imageHeight , &tex);

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
	printf("Hey, ya! Having problems running the state-of-art Character Segmentation program? Check this out, chap:\n");
	printf("\CharacterSegmentation <input.file> <output prefix> <position.file>\n");
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

bool writeImageWithPostfix(const char *pathPrefix, const char *pathPostfix, unsigned char *data, int width, int height)
{
	char buffer[512];

	memset(buffer, 0, 512);

	strcpy(buffer, pathPrefix);
	strcpy(buffer + strlen(pathPrefix), pathPostfix);

	printf("  Writing to output file: %s\n", buffer);

	return stbi_write_bmp(buffer, width, height, 3, data);
}

bool writeCharacters(const char *pathPrefix, unsigned char *data, int width, std::vector<imgf::CharacterPosition> &positions)
{
	int charIndex = 0;

	char buffer[255];

	const int outputSize = 32 * 32 * 3;

	for (const auto &position : positions)
	{
		unsigned char *charData = imgf::characterPositionToImageData(data, width, position);

		std::sprintf(buffer, "-%d.bmp", charIndex);

		const int width = position.bottomRightColumn - position.topLeftColumn + 1;
		const int height = position.bottomRightLine - position.topLeftLine + 1;

		unsigned char *resizedData = new unsigned char[32 * 32 * 3];

		stbir_resize_uint8(charData, width, height, 0, resizedData, 32, 32, 0, 3);

		writeImageWithPostfix(pathPrefix, buffer, resizedData, 32, 32);

		delete resizedData;
		delete charData;

		++charIndex;
	}

	return true;
}

bool writeCharacterPositions(const char *positionFilePath, std::vector<imgf::CharacterPosition> &positions)
{
	std::ofstream positionStream;

	positionStream.open(positionFilePath, std::ofstream::out);

	positionStream << positions.size() << std::endl;

	for (const auto &position : positions)
	{
		positionStream << position.topLeftLine << " " << position.topLeftColumn << " ";
		positionStream << position.bottomRightLine << " " << position.bottomRightColumn << std::endl;
	}

	positionStream.close();

	return true;
}
