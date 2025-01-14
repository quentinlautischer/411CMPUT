// CMPUT 411 Assignment 4

// Original author: Nathaniel Rossol
// Modified by: Dale Schuurmans

#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Material.h"

typedef unsigned int UINT;
typedef unsigned char byte;
typedef unsigned short WORD;
typedef int errno_t;

using namespace std;


class Texture
{
private:

	char* textureFilename;
	bool bTextureLoaded;
	bool bTextureNeedsToBeStreamedIn;
	bool bTextureIsAlphaBlended;

	bool bClampedU;
	bool bClapmedV;
	
	GLuint texture;

	const static unsigned int TGA_RGB = 2;
	const static unsigned int TGA_A = 3;
	const static unsigned int TGA_RLE = 10;

	struct TGAImageData
	{
		int channels;
		int size_x;	
		int size_y;				
		unsigned char *data;
	};

public:

	Texture(char* fileName, bool texture_is_alpha_blended = false, 
	        bool clamp_u = false, bool clamp_v = false);

	bool loadTexture();
	void bindTexture();
	void unbindTexture();
	unsigned int getTextureID();
	bool isAlphaBlended();

private:

	void TGA_Texture(UINT textureArray[], char* strFileName, int textureID);
	bool loadTGATextureFile(char * fileName);
	TGAImageData* loadTGAImageData(FILE* tgaFile);
	bool loadUncompressedTGA(char *, FILE *);	// Load an Uncompressed file
	bool loadCompressedTGA(char *, FILE *);		// Load a Compressed file

};

#endif
