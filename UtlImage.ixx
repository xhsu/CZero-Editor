module;

// C++
#include <filesystem>
#include <tuple>
#include <list>

// C
#include <cassert>

// Platform
#include <Windows.h>

// OpenGL
#include <GL/gl.h>
#include <GL/glu.h>
#define GL_CLAMP_TO_EDGE 0x812F	// Fucking windows GL.

// STB library
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


export module UtlImage;

export struct Image_t
{
	GLuint	m_iTexId{ 0U };
	int		m_iWidth{ 0 };
	int		m_iHeight{ 0 };
};

export // Simple helper function to load an image into a OpenGL texture with common settings
bool UTIL_LoadTextureFromFile(const char* filename, GLuint* piTextureIndex, int* piWidth, int* piHeight)
{
	// Load from file
	unsigned char* pBuffer = stbi_load(filename, piWidth, piHeight, NULL, 4);	// #MEM_ALLOC

	if (!pBuffer)
		return false;

	// Create a OpenGL texture identifier
	glGenTextures(1, piTextureIndex);
	glBindTexture(GL_TEXTURE_2D, *piTextureIndex);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *piWidth, *piHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pBuffer);
	stbi_image_free(pBuffer);	// #MEM_FREED

	return true;
}

namespace ImageLoader
{
	namespace fs = std::filesystem;
	using QueuedImage_t = std::tuple<fs::path, Image_t*>;
	using QueueOfImage_t = std::list<QueuedImage_t>;

	inline static QueueOfImage_t Queue{};	// #POTENTIAL_BUG

	bool Load(const fs::path& hPath, Image_t* pImage)
	{
		return UTIL_LoadTextureFromFile(hPath.string().c_str(), &pImage->m_iTexId, &pImage->m_iWidth, &pImage->m_iHeight);
	}

	export bool Add(const fs::path& hPath, Image_t* pImage)
	{
		if (!fs::exists(hPath))
			return false;

		Queue.emplace_back(hPath, pImage);
		return true;
	}

	/*
	* The goal is to solve async problem when image fail loading while OpenGL is rendering.
	*/
	export void Execute(void)
	{
		for (auto iter = Queue.begin(); iter != Queue.end();)
		{
			auto b = std::apply(Load, *iter);
			assert(b);

			if (b)
			{
				iter = Queue.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}
};
