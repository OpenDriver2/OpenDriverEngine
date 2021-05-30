#include "image.h"

#include <stdio.h>

//-------------------------------------------------------------------------------

// 16 bit color to BGRA
// originalTransparencyKey makes it pink
TVec4D<ubyte> rgb5a1_ToBGRA8(ushort color, bool originalTransparencyKey /*= true*/)
{
	ubyte r = (color & 0x1F) * 8;
	ubyte g = ((color >> 5) & 0x1F) * 8;
	ubyte b = ((color >> 10) & 0x1F) * 8;

	ubyte a = (color >> 15) * 255;

	// restore source transparency key
	if (originalTransparencyKey && color == 0)
	{
		return TVec4D<ubyte>(255, 0, 255, 0);
	}

	return TVec4D<ubyte>(b, g, r, a);
}

// 16 bit color to RGBA
// originalTransparencyKey makes it pink
TVec4D<ubyte> rgb5a1_ToRGBA8(ushort color, bool originalTransparencyKey /*= true*/)
{
	ubyte r = (color & 0x1F) * 8;
	ubyte g = ((color >> 5) & 0x1F) * 8;
	ubyte b = ((color >> 10) & 0x1F) * 8;

	ubyte a = (color >> 15) * 255;

	// restore source transparency key
	if (originalTransparencyKey && color == 0)
	{
		return TVec4D<ubyte>(255, 0, 255, 0);
	}

	return TVec4D<ubyte>(r, g, b, a);
}

//-------------------------------------------------------------
// Saves TGA file
//-------------------------------------------------------------
void SaveTGA(const char* filename, ubyte* data, int w, int h, int c)
{
	TGAHEADER tgaHeader;

	// Initialize the Targa header
	tgaHeader.identsize = 0;
	tgaHeader.colorMapType = 0;
	tgaHeader.imageType = 2;
	tgaHeader.colorMapStart = 0;
	tgaHeader.colorMapLength = 0;
	tgaHeader.colorMapBits = 0;
	tgaHeader.xstart = 0;
	tgaHeader.ystart = 0;
	tgaHeader.width = w;
	tgaHeader.height = h;
	tgaHeader.bits = c * 8;
	tgaHeader.descriptor = 0;

	int imageSize = w * h * c;

	FILE* pFile = fopen(filename, "wb");
	if (!pFile)
		return;

	// Write the header
	fwrite(&tgaHeader, sizeof(TGAHEADER), 1, pFile);

	// Write the image data
	fwrite(data, imageSize, 1, pFile);

	fclose(pFile);
}

//-------------------------------------------------------------
// Saves TIM file - 4 bit image
//-------------------------------------------------------------
void SaveTIM_4bit(char* filename,
	ubyte* image_data, int image_size,
	int x, int y, int w, int h,
	ubyte* clut_data, int clut_h)
{
	// compose TIMs
	//
	// prep headers
	TIMHDR hdr;
	hdr.magic = 0x10;
	hdr.flags = 0x08; // for 4bpp

	TIMIMAGEHDR cluthdr;
	TIMIMAGEHDR datahdr;

	cluthdr.origin_x = 0;
	cluthdr.origin_y = 0;

	cluthdr.width = 16;					// CLUTs always 16 bit color
	cluthdr.height = clut_h;
	cluthdr.len = (cluthdr.width * cluthdr.height * sizeof(ushort)) + sizeof(TIMIMAGEHDR);

	datahdr.origin_x = x;
	datahdr.origin_y = y;

	datahdr.width = (w >> 2);
	datahdr.height = h;
	datahdr.len = image_size + sizeof(TIMIMAGEHDR);

	FILE* pFile = fopen(filename, "wb");
	if (!pFile)
		return;

	// write header
	fwrite(&hdr, 1, sizeof(hdr), pFile);

	// write clut
	fwrite(&cluthdr, 1, sizeof(cluthdr), pFile);
	fwrite(clut_data, 1, cluthdr.len - sizeof(TIMIMAGEHDR), pFile);

	// write data
	fwrite(&datahdr, 1, sizeof(datahdr), pFile);
	fwrite(image_data, 1, datahdr.len - sizeof(TIMIMAGEHDR), pFile);

	// dun
	fclose(pFile);
}

void SaveTIM_8bit(char* filename,
	ubyte* image_data, int image_size,
	int x, int y, int w, int h,
	ubyte* clut_data, int clut_h)
{
	// compose TIMs
	//
	// prep headers
	TIMHDR hdr;
	hdr.magic = 0x10;
	hdr.flags = 0x09; // for 8bpp

	TIMIMAGEHDR cluthdr;
	TIMIMAGEHDR datahdr;

	cluthdr.origin_x = 0;
	cluthdr.origin_y = 0;

	cluthdr.width = 256;					// CLUTs always 16 bit color
	cluthdr.height = clut_h;
	cluthdr.len = (cluthdr.width * cluthdr.height * sizeof(ushort)) + sizeof(TIMIMAGEHDR);

	datahdr.origin_x = x;
	datahdr.origin_y = y;

	datahdr.width = (w >> 1);
	datahdr.height = h;
	datahdr.len = image_size + sizeof(TIMIMAGEHDR);

	FILE* pFile = fopen(filename, "wb");
	if (!pFile)
		return;

	// write header
	fwrite(&hdr, 1, sizeof(hdr), pFile);

	// write clut
	fwrite(&cluthdr, 1, sizeof(cluthdr), pFile);
	fwrite(clut_data, 1, cluthdr.len - sizeof(TIMIMAGEHDR), pFile);

	// write data
	fwrite(&datahdr, 1, sizeof(datahdr), pFile);
	fwrite(image_data, 1, datahdr.len - sizeof(TIMIMAGEHDR), pFile);

	// dun
	fclose(pFile);
}

void SaveTIM_16bit(char* filename,
	ubyte* image_data, int image_size,
	int x, int y, int w, int h)
{
	// compose TIMs
	//
	// prep headers
	TIMHDR hdr;
	hdr.magic = 0x10;
	hdr.flags = 0x02; // for 16bpp

	TIMIMAGEHDR datahdr;

	datahdr.origin_x = x;
	datahdr.origin_y = y;

	datahdr.width = w;
	datahdr.height = h;
	datahdr.len = image_size + sizeof(TIMIMAGEHDR);

	FILE* pFile = fopen(filename, "wb");
	if (!pFile)
		return;

	// write header
	fwrite(&hdr, 1, sizeof(hdr), pFile);

	// write data
	fwrite(&datahdr, 1, sizeof(datahdr), pFile);
	fwrite(image_data, 1, datahdr.len - sizeof(TIMIMAGEHDR), pFile);

	// dun
	fclose(pFile);
}