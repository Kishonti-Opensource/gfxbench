/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_os.h"
#include "kcl_io.h"

#include "misc2.h"
#include "stdc.h"
#include "png.h"


#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define NOCOMM
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>//mkdir
#else

#define BI_RGB        0L

typedef unsigned long       DWORD;
typedef unsigned short      WORD;
typedef long 				LONG;

struct BITMAPINFOHEADER{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} __attribute__((packed));

struct BITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} __attribute__((packed));


#endif


using namespace KCL;

unsigned short convertShort(unsigned short i)
{
	endianConverterShort ec;
	endianConverterShort ecIn;

	ecIn.v = i;
	ec.b = ecIn.a;
	ec.a = ecIn.b;
	return ec.v;
}


unsigned int convertInt(unsigned int i)
{
	return((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i>>24)&0xff);
}


unsigned int fgetReverseUINT(FILE *f)
{
	endianConverterInt ec;

	if(IsBigEndian())
	{
		ec.d =  fgetc(f);
		ec.c =  fgetc(f);
		ec.b =  fgetc(f);
		ec.a =  fgetc(f);
	}
	else
	{
		ec.a =  fgetc(f);
		ec.b =  fgetc(f);
		ec.c =  fgetc(f);
		ec.d =  fgetc(f);
	}
	return ec.v;
}


void CheckDataDirectoryValidity()
{
	KCL::AssetFile file("eula.txt");
	if(file.GetLastError())
	{
		INFO("Data directory is invalid!!!");
	}
}


bool IsBigEndian()
{
	int num = 1;
	if(*(char *)&num == 1)
	{
		//printf("Little-Endian\n");
		return 0;
	}
	else
	{
		//printf("Big-Endian\n");
		return 1;
	}
}


void png_cexcept_error(png_structp png_ptr, png_const_charp msg)
{
}


void assetWrite(png_structp png_ptr, png_bytep data, png_size_t length)
{
	KCL::AssetFile *io = (KCL::AssetFile*)png_get_io_ptr( png_ptr);
	io->Write( data, 1, length);
}


bool savePng( const char *filename, int w, int h, const unsigned char* data, int flip)
{
	if (!filename)
	{
		return 0;
	}

	png_structp png_ptr = 0;
	png_infop info_ptr = 0;
	const int bpp = 24;
	const int channels = 3;
	png_uint_32 ulRowBytes = 0;
	png_byte **ppbRowPointers = 0;

	KCL::File file2(filename, KCL::Write,KCL::RDir);
	if(file2.GetLastError())
	{
		return 0;
	}


	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)png_cexcept_error, (png_error_ptr)NULL);
	if (!png_ptr)
	{
		return 0;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) 
	{
		png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
		return 0;
	}

	png_set_write_fn(png_ptr, (png_voidp)&file2, assetWrite, NULL);
	
	png_set_IHDR(png_ptr, info_ptr, w, h, bpp / 3, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);

	if(!flip)
	png_set_bgr(png_ptr);

	ulRowBytes = w * channels;

	ppbRowPointers = new png_bytep[h];
	if(!ppbRowPointers)
		assert(0);

	const uint8 *imgData;

	if(!flip)
	{
		uint8 *data2 = new uint8[w * h * 3];
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				int srcIndex = y * w * channels + x * channels;
				int targetIndex = (h - y - 1) * w * channels + x * channels;

				data2[targetIndex+2] = data[srcIndex+0];
				data2[targetIndex+1] = data[srcIndex+1];
				data2[targetIndex+0] = data[srcIndex+2];
			}
		}
		imgData = data2;
	} else 
	{
		imgData = data;
	}

	for (int i = 0; i < h; i++)
	{
		ppbRowPointers[i] = (png_bytep)imgData + i * (((ulRowBytes + 3) >> 2) << 2);
	}
	
	png_write_image (png_ptr, ppbRowPointers);
	png_write_end(png_ptr, info_ptr);
	delete[] ppbRowPointers;
	ppbRowPointers = NULL;

	
	png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
	file2.Close();
	if (!flip) delete[] imgData;
	return 1;
}

/*
void saveBmp( const char *name, int w, int h, void *data)
{
	BITMAPINFOHEADER bih;
	BITMAPFILEHEADER bfh;

	KCL::AssetFile file(name, KCL::Write, KCL::RWDir);
	if(file.GetLastError())
	{
		return;
	}

	memset(&bih, 0, sizeof bih);
	bih.biSize = sizeof bih;
	bih.biWidth = w;
	bih.biHeight = h;
	bih.biPlanes = 1;
	bih.biBitCount = 3 * 8;
	bih.biCompression = BI_RGB;

	memset(&bfh, 0, sizeof bfh);
	bfh.bfType = 'B'|('M'<<8);
	bfh.bfSize = sizeof bfh + sizeof bih + w * h *3;
	bfh.bfOffBits = sizeof bfh + sizeof bih;

	file.Write(&bfh, sizeof bfh, 1);
	file.Write(&bih, sizeof bih, 1);
	file.Write(data, h * w * 3, 1);
	file.Close();
}
*/

void printSrc (FILE *f, const char *src)
{
	int nextChr = 0, currChr = 0;
	int ln = 1;
	char* lsrc = const_cast<char*>(src);

	while (lsrc[nextChr])
	{
		while (lsrc[nextChr] && lsrc[nextChr] != '\n')
			nextChr++;
		lsrc[nextChr++] = 0;
		//fprintf (f, "%3d:\t%s\n", ln++, src+currChr);
		INFO ("%3d:\t%s\n", ln++, lsrc+currChr);
		currChr = nextChr;
	}
}


void convertRGBAtoBGR( unsigned char* data, int pixels)
{
	for (int i=0;i<pixels;i++)
	{
		unsigned char r = data[i*4];
		unsigned char g = data[i*4+1];
		unsigned char b = data[i*4+2];

		data[i*3] = b;
		data[i*3+1] = g;
		data[i*3+2] = r;
	}
}


bool GetNativeResolution(int &w, int &h)
{
#ifdef WIN32
	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
	if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings))
	{
		return false;
	}
	w = dmScreenSettings.dmPelsWidth;
	h = dmScreenSettings.dmPelsHeight;

	return true;
#else
	return false;
#endif
}




static const char BASE64URL_INDEXTABLE[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";


void encodeBytesBase64URL(unsigned char a, unsigned char b, unsigned char c, char &out_a, char &out_b, char &out_c, char &out_d)
{
	unsigned char idx_out_a = 0;
	unsigned char idx_out_b = 0;
	unsigned char idx_out_c = 0;
	unsigned char idx_out_d = 0;

	idx_out_a = a >> 2;

	unsigned char tmp;

	//For more detail: wikipedia, Base64
	//idx_out_b = ((a << 6) >> 2) | (b >> 4); //cast-olgatni minden bit muvelet utan, ezert inkabb:
	tmp = a << 6;
	tmp = tmp >> 2;
	idx_out_b = tmp;
	tmp = b >> 4;
	idx_out_b = idx_out_b | tmp;

	
	//out_c = ((b << 4) >> 2) + (c >> 6);
	tmp = b << 4;
	tmp = tmp >> 2;
	idx_out_c = tmp;
	tmp = c >> 6;
	idx_out_c = idx_out_c | tmp;


	//out_d = ((c << 2) >> 2);
	tmp = c << 2;
	tmp = tmp >> 2;
	idx_out_d = tmp;

	out_a = BASE64URL_INDEXTABLE[idx_out_a];
	out_b = BASE64URL_INDEXTABLE[idx_out_b];
	out_c = BASE64URL_INDEXTABLE[idx_out_c];
	out_d = BASE64URL_INDEXTABLE[idx_out_d];
}


void EncodeBase64URL(std::string &result, const void* data, size_t sz, bool usePercentEncodedPadding)
{
	result = "";

	if(data == 0 || sz == 0)
	{
		return;
	}

	const unsigned char* cdata = (const unsigned char*)data;

	const size_t fullGroupCount = sz / 3;
	const size_t lastGroupSize = sz % 3;
	char a = 0;
	char b = 0;
	char c = 0;
	char d = 0;

	size_t idx = 0;
	for(size_t i=0; i<fullGroupCount; ++i)
	{
		encodeBytesBase64URL(cdata[idx], cdata[idx+1], cdata[idx+2], a, b, c, d);

		result += a;
		result += b;
		result += c;
		result += d;

		idx += 3;
	}

	if(lastGroupSize == 1)
	{
		encodeBytesBase64URL(cdata[idx], 0, 0, a, b, c, d);

		result += a;
		result += b;

		if(usePercentEncodedPadding)
		{
			result += "%3D";
			result += "%3D";
		}
	}
	else if(lastGroupSize == 2)
	{
		encodeBytesBase64URL(cdata[idx], cdata[idx+1], 0, a, b, c, d);

		result += a;
		result += b;
		result += c;

		if(usePercentEncodedPadding)
		{
			result += "%3D";
		}
	}
}


void encrypt(const unsigned int length, char*& data)
{
	char encryptKey[] = {"Stockholm"};
	for(unsigned int i = 0; i < length; i++)
	{
		for(unsigned int h = 0; h < strlen(encryptKey); h++)
		{
			data[i] ^= encryptKey[h];
		}
	}
}



CombineData::CombineData()
{
	m_width = 0;
	m_height = 0;
	m_component_num = 0;
	m_data = 0;
}


CombineData::~CombineData()
{
	delete [] m_data;
	m_data = 0;
}


void CombineData::Create(const unsigned int x, const unsigned int y, const unsigned int componentSize)
{
	m_component_num = componentSize;
	m_width = x;
	m_height = y;

	m_data = new KCL::int8[x * y * componentSize];
	memset(m_data, 0, x * y * componentSize);
}


void CombineData::Put(const unsigned int x, const unsigned int y, const unsigned int n, const KCL::int8 data)
{
	uint32 i = x % m_width;
	uint32 j = y % m_height;

	m_data[ (i + j * m_width) * m_component_num + n] = data;
}


KCL::int8 CombineData::Get(const unsigned int x, const unsigned int y, const unsigned int n)
{
	uint32 i = x % m_width;
	uint32 j = y % m_height;

	return m_data[  (i + j * m_width) * m_component_num + n];
}


KCL::int8*& CombineData::GetData()
{
	return 	m_data;
}
