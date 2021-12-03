/*
* Luna: Copied from https://www.freepascal-meets-sdl.net/the-goldsrc-bsp-loader/
* Oct 25 2021

Important Sources
BSP and WAD File Formats
I cannot state how important these documents were in understanding the structure of the BSP and WAD file formats. Without them, this project wouldn’t have been possible.

http://hlbsp.sourceforge.net/index.php?content=bspdef
http://hlbsp.sourceforge.net/index.php?content=waddef
http://jheriko-rtw.blogspot.de/2010/11/dissecting-quake-2-bsp-format.html
http://www.flipcode.com/archives/Quake_2_BSP_File_Format.shtml
Matrix and Vector Functions
I used the SupraEngine.Math Unit by Benjamin ‘BeRo’ Rosseaux (benjamin@rosseaux.com) for this.

http://free-pascal-general.1045716.n5.nabble.com/GLM-library-alternative-td5728879.html
License: zlib
Implementation Hints
SDL2 surface to OGL texture
http://www.sdltutorials.com/sdl-tip-sdl-surface-to-opengl-texture
Repeating OGL textures from texture atlas
https://www.opengl.org/discussion_boards/showthread.php/162702-Texture-atlas-and-repeating-textures
https://forum.unity.com/threads/tiling-textures-within-an-atlas-possible.44482/

*/

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// C
#include <cassert>
#include <cstdint>

// C++
#include <algorithm>
#include <list>
#include <string>

#define LUMP_ENTITIES      0
#define LUMP_PLANES        1
#define LUMP_TEXTURES      2
#define LUMP_VERTICES      3
#define LUMP_VISIBILITY    4
#define LUMP_NODES         5
#define LUMP_TEXINFO       6
#define LUMP_FACES         7
#define LUMP_LIGHTING      8
#define LUMP_CLIPNODES     9
#define LUMP_LEAVES       10
#define LUMP_MARKSURFACES 11
#define LUMP_EDGES        12
#define LUMP_SURFEDGES    13
#define LUMP_MODELS       14
#define HEADER_LUMPS      15

using namespace std;

import UtlString;

struct BSPLUMP
{
	int32_t nOffset; // File offset to data
	int32_t nLength; // Length of data
};

struct BSPHEADER
{
	int32_t nVersion;           // Must be 30 for a valid HL BSP file
	BSPLUMP lump[HEADER_LUMPS]; // Stores the directory of lumps
};

list<string> BSP_CompileResourceList(const char* pszBspPath) noexcept
{
	list<string> Res;

	FILE* f = fopen(pszBspPath, "rb");	// #FOPEN
	assert(f != nullptr);

	BSPHEADER BspHeader;
	fread(&BspHeader, sizeof(BspHeader), 1, f);

	char* pszEntityInfoString = new char[BspHeader.lump[LUMP_ENTITIES].nLength + 1];	// #MEM_ALLOC
	fseek(f, BspHeader.lump[LUMP_ENTITIES].nOffset, SEEK_SET);
	fread(pszEntityInfoString, BspHeader.lump[LUMP_ENTITIES].nLength, 1, f);
	pszEntityInfoString[BspHeader.lump[LUMP_ENTITIES].nLength] = '\0';

	char* p = pszEntityInfoString, * pend = &pszEntityInfoString[BspHeader.lump[LUMP_ENTITIES].nLength];
	while (p != pend)
	{
		static bool bExpectingWad = false;
		static bool bExpectingSky = false;

		if (*p == '"')
		{
			if (!_strnicmp(p, "\"wad\"", 5))
			{
				bExpectingWad = true;
				p += 5;
				continue;
			}
			else if (!_strnicmp(p, "\"skyname\"", 9))
			{
				bExpectingSky = true;
				p += 9;
				continue;
			}

			char* p2 = ++p, * pExt = nullptr;
			while (*p2 != '"' && p2 != pend)
				p2++;

			assert(p2 != pend);

			*p2 = '\0';
			pExt = p2 - 4;

			if (bExpectingWad)
			{
				char* p3 = p;
				while (p3 != p2)	// p2 is the pEnd since it's '\0' now.
				{
					if (*p3 == ';')
					{
						*p3 = '\0';
						Res.emplace_back(p);

						p = ++p3;
						continue;
					}

					p3++;
				}

				bExpectingWad = false;
				continue;	// Handled "wad" region.
			}
			else if (bExpectingSky)
			{
				std::string szBase("gfx/env/"s + p);
				Res.emplace_back(szBase + "BK.tga");
				Res.emplace_back(szBase + "DN.tga");
				Res.emplace_back(szBase + "FT.tga");
				Res.emplace_back(szBase + "LF.tga");
				Res.emplace_back(szBase + "RT.tga");
				Res.emplace_back(szBase + "UP.tga");

				bExpectingSky = false;	// perform increament normally.
			}

			else if (!_stricmp(pExt, ".mdl") || !_stricmp(pExt, ".spr") || !_stricmp(pExt, ".wav"))
				Res.emplace_back(p);

			p = ++p2;
			continue;
		}

		p++;
	}

	Res.sort();
	Res.erase(std::unique(Res.begin(), Res.end()), Res.end());

	for (auto& s : Res)
	{
		if (stristr(s.c_str(), ".wav"))
		{
			s = "sound/" + s;
		}
		else if (stristr(s.c_str(), ".wad"))
		{
			if (auto i = s.find_last_of("/\\"); i != s.npos)
				s.erase(0, i + 1);	// Including '/' symbol
		}
	}

	fclose(f);	// #FCLOSE
	delete[] pszEntityInfoString;	// #MEM_FREED
	return Res;
}