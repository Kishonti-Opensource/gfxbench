/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file image.cpp
	Implementation of image handling classes.
	Contains ImageBase and descendant classes Image2D and ImageCube.
*/
#include <kcl_image.h>
#include <kcl_os.h>
#include <kcl_io.h>
#include <hdr.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>

#include <png.h>
#include <etc1.h>

#ifdef DEFINE_ASTC_UNCOMPRESS
#include "astc_codec_internals.h"
#endif

#define RGB9E5_EXPONENT_BITS          5
#define RGB9E5_MANTISSA_BITS          9
#define RGB9E5_EXP_BIAS               15
#define RGB9E5_MAX_VALID_BIASED_EXP   31

#define MAX_RGB9E5_EXP               (RGB9E5_MAX_VALID_BIASED_EXP - RGB9E5_EXP_BIAS)
#define RGB9E5_MANTISSA_VALUES       (1<<RGB9E5_MANTISSA_BITS)
#define MAX_RGB9E5_MANTISSA          (RGB9E5_MANTISSA_VALUES-1)
#define MAX_RGB9E5                   (((float)MAX_RGB9E5_MANTISSA)/RGB9E5_MANTISSA_VALUES * (1<<MAX_RGB9E5_EXP))
#define EPSILON_RGB9E5               ((1.0/RGB9E5_MANTISSA_VALUES) / (1<<RGB9E5_EXP_BIAS))

#ifdef __LITTLE_ENDIAN
#undef __LITTLE_ENDIAN
#endif
#ifdef __BIG_ENDIAN
#undef __BIG_ENDIAN
#endif
#define __LITTLE_ENDIAN  1
#define __BIG_ENDIAN     2

//#ifdef _WIN32
#define __BYTE_ORDER __LITTLE_ENDIAN
//#endif

/*
#if defined( WIN32 ) && !defined(_METRO_)
	#define NOMINMAX
	#define NOCOMM
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	#if defined(_METRO_)
		#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
	#else
		#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
	#endif

	#define BI_RGB        0L

	typedef unsigned long       DWORD;
	typedef unsigned short      WORD;
	typedef long 				LONG;

	PACK( struct BITMAPINFOHEADER{
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
	});

	PACK(
	struct BITMAPFILEHEADER {
		WORD    bfType;
		DWORD   bfSize;
		WORD    bfReserved1;
		WORD    bfReserved2;
		DWORD   bfOffBits;
	});
#endif
*/

struct TTgaHeader
{
	unsigned char   identsize;
	unsigned char   colourmaptype;
	unsigned char   imagetype;
	unsigned short  colourmapstart;
	unsigned char   colourmaplength;
	unsigned char   colourmapbits;
	unsigned short  xstart;
	unsigned short  ystart;
	unsigned short  width;
	unsigned short  height;
	unsigned char   bits;
	unsigned char   descriptor;
};

#define MAGIC_FILE_CONSTANT 0x5CA1AB13
struct astc_header
{
	KCL::uint8 magic[4];
	KCL::uint8 blockdim_x;
	KCL::uint8 blockdim_y;
	KCL::uint8 blockdim_z;
	KCL::uint8 xsize[3];			// x-size = xsize[0] + xsize[1] + xsize[2]
	KCL::uint8 ysize[3];			// x-size, y-size and z-size are given in texels;
	KCL::uint8 zsize[3];			// block count is inferred
};


namespace
{
	union endianConverterInt
	{
		unsigned int v;
		struct
		{
			unsigned char a;
			unsigned char b;
			unsigned char c;
			unsigned char d;
		};
	};

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


	unsigned int convertUInt(unsigned int i)
	{
		return((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i>>24)&0xff);
	}


//	unsigned int fgetReverseUINT(FILE *f)
//	{
//		endianConverterInt ec;
//
//		if(IsBigEndian())
//		{
//			ec.d =  fgetc(f);
//			ec.c =  fgetc(f);
//			ec.b =  fgetc(f);
//			ec.a =  fgetc(f);
//		}
//		else
//		{
//			ec.a =  fgetc(f);
//			ec.b =  fgetc(f);
//			ec.c =  fgetc(f);
//			ec.d =  fgetc(f);
//		}
//		return ec.v;
//	}

	const int TexMapBase = 0x8515;
}

namespace KCL
{
#ifndef max
	template<typename T>
	inline T max(const T &a, const T &b)
	{
		return (a > b) ? a : b;
	}
#endif

#ifndef min
	template<typename T>
	inline T min(const T &a, const T &b)
	{
		return (a < b) ? a : b;
	}
#endif
}

using namespace KCL;


void detwiddle(int x1, int y1, int size, int imgsize, KCL::uint16* dst, KCL::uint16 **source);


static const FormatDescription FormatDescriptions[] =
{
	{ Image_RGB888,                  1, 1, 24, 0, "RGB888"   },
	{ Image_BGR888,                  1, 1, 24, 0, "BGR888" },
	{ Image_RGBA8888,                1, 1, 32, 8, "RGBA8888" },
	{ Image_RGB565,                  1, 1, 16, 0, "RGB565"   },
	{ Image_RGBA5551,                1, 1, 16, 1, "RGBA5551" },
	{ Image_RGBA4444,                1, 1, 16, 4, "RGBA4444" },
	{ Image_ETC1,					 4, 4, 24, 1, "ETC1"     },
	{ Image_ETC2_RGB,                4, 4, 4, 0,  "ETC2 RGB" },
	{ Image_ETC2_RGBA8888,           4, 4, 8, 4,  "ETC2 RGBA" },
	{ Image_DXT1,                    4, 4, 4, 1,  "DXT1" },
    { Image_DXT1_RGBA,               4, 4, 4, 1,  "DXT1" },
	{ Image_DXT2,                    0, 0, 0, 0,  "NOT SUPPORTED" },
	{ Image_DXT3,                    4, 4, 8, 1,  "DXT3" },
	{ Image_DXT4,                    0, 0, 0, 0,  "NOT SUPPORTED" },
	{ Image_DXT5,                    4, 4, 8, 4,  "DXT5" },
	{ Image_PVRTC2,                  16, 8, 2, 1, "PVRTC2" },
	{ Image_PVRTC4,                  8, 8, 4, 1,  "PVRTC4" },
	{ Image_ATC_RGB,                 0, 0, 0, 0,  "NOT SUPPORTED" },
	{ Image_ATC_RGBA_EA,             0, 0, 0, 0,  "NOT SUPPORTED" },
	{ Image_ATC_RGBA_IA,             0, 0, 0, 0,  "NOT SUPPORTED" },
	{ Image_FLXTC_RGB565,            0, 0, 0, 0,  "NOT SUPPORTED" },
	{ Image_FLXTC_RGBA4444,          0, 0, 0, 0,  "NOT SUPPORTED" },
	{ Image_LUMINANCE_L8,            1, 1, 8, 0,  "I8" },
	{ Image_ALPHA_A8,                1, 1, 8, 8,  "A8" },
	{ Image_LUMINANCE_ALPHA_LA88,    1, 1, 16, 8, "IA88" },
	{ ImageTypeAny,                  0, 0, 0, 0, "NOT SUPPORTED" },
};


/// Needed fields of BITMAPFILEHEADER.
/// \see Windows BMP file format documentation.
typedef struct _BmpFileHdr
{
	KCL::uint32 header;
	KCL::uint32 size;
	KCL::uint32 offset;
} BmpFileHdr;


/// Needed fields of BITMAPIMAGEHEADER.
/// \see Windows BMP file format documentation.
typedef struct _BmpImgHdr
{
	KCL::uint32 size;
	KCL::uint32 width;
	KCL::uint32 height;
	KCL::uint16 bitCount;
	KCL::uint32 compression;
	KCL::uint32 colorUsed;
	KCL::uint32 *colors;
} BmpImgHdr;


ImageBase::ImageBase () :
	m_id (0),
	m_width (0),
	m_height (0),
	m_format (Image_RGB888),
	m_mipmaps (0),
	m_mutable (false),
	m_desc (0)
{
}


ImageBase::~ImageBase () {}


KCL::uint32 ImageBase::getBpp () const
{
	switch (m_format)
	{
	case Image_LUMINANCE_L8:
	case Image_ALPHA_A8:
	case Image_DXT3:
	case Image_DXT5:
	case Image_ETC2_RGBA8888:
		return 8;
	case Image_LUMINANCE_ALPHA_LA88:
	case Image_RGB565:
	case Image_RGBA4444:
	case Image_RGBA5551:
		return 16;
	case Image_RGB888:
		return 24;
	case Image_RGBA8888:
		return 32;
	case Image_ETC1:
	case Image_DXT1:
	case Image_PVRTC4:
	case Image_ETC2_RGB:
	case Image_ETC2_RGB_A1:
		return 4;
	case Image_PVRTC2:
		return 2;
	default:
		return 0;
	}
}


void ImageBase::getDescription ()
{
	const FormatDescription *fd = FormatDescriptions;
	while (fd->format != ImageTypeAny)
	{
		if ((unsigned)fd->format == m_format)
		{
			m_desc = fd;
			return;
		}
		++fd;
	}
}


Image2D::Image2D () :
	ImageBase (),
	m_size (0),
	m_data (NULL),
	mm_enabled( false),
	m_has_pvr_alpha( false)
{}


Image2D::~Image2D ()
{
	release_data();
}


void Image2D::reset ()
{
	release_data();
	m_width = 0;
	m_height = 0;
	m_format = KCL::ImageTypeAny;
	m_id = 0;
	m_mutable = false;
}


bool Image2D::init ()
{
	reset();
	return true;
}


bool Image2D::release_data()
{
	if (m_data)
	{
		delete [] m_data;
		m_data = NULL;
		return true;
	}
	return false;
}


void rewriteFileExtension(char** fn, const char* newext)
{
	int savepos = 0;
	if(newext==NULL)
	{
		return;
	}

	savepos = strchr(*fn, '.') - *fn;

	for(unsigned int i = 0; i < strlen(newext); i++)
	{
		(*fn)[savepos + i + 1] = newext[i];
	}

	(*fn)[savepos + strlen(newext) + 1] = '\0';
}

void DecodeRGB888toRGBA8888 (KCL::uint32 width, KCL::uint32 height, const KCL::uint8 *rgb, KCL::uint16 *rgba);
bool convert(ImageFormat srcFormat, KCL::uint8* src, ImageFormat dstFormat, KCL::uint8 *dst, int width, int height);
KCL::uint32 readColor(KCL::uint8* address, ImageFormat format);
int writeColor(KCL::uint32 color, KCL::uint8* address, ImageFormat format);

bool Image2D::load (const char *file_, bool flip)
{
	if(strlen(file_) < 1 )
	{
		return false;
	}

	m_name = file_;
	std::string filename2(file_);

	filename2.replace(filename2.rfind("."), filename2.length(), ".pvr");
	if (KCL::File::Exists(filename2))
	{
		loadPvr(filename2.c_str());
		return true;
	}

	filename2.replace(filename2.rfind("."), filename2.length(), ".astc");
	if (KCL::File::Exists(filename2))
	{
		loadASTC(filename2.c_str());
		return true;
	}

	filename2.replace(filename2.rfind("."), filename2.length(), ".png");
	if (KCL::File::Exists(filename2))
	{
		loadPng(filename2.c_str());
		return true;
	}

	return false;
}

#pragma warning(disable : 4996)
uint8 *load_astc_file(const char *filename, size_t& size, int &width, int &height, int &mipmaps)
{
	AssetFile file(filename);
	if(file.GetLastError())
	{
		return NULL;
	}
	mipmaps = 0;
	astc_header hdr;
	size_t hdr_bytes_read = file.Read(&hdr, 1, sizeof(astc_header));

	if (hdr_bytes_read != sizeof(astc_header))
	{
		return NULL;
	}

	uint32 magicval = hdr.magic[0] + 256 * (uint32) (hdr.magic[1]) + 65536 * (uint32) (hdr.magic[2]) + 16777216 * (uint32) (hdr.magic[3]);

	if (magicval != MAGIC_FILE_CONSTANT)
	{
		return NULL;
	}

	int xdim = hdr.blockdim_x;
	int ydim = hdr.blockdim_y;
	int zdim = hdr.blockdim_z;

	if (xdim < 3 || xdim > 12 || ydim < 3 || ydim > 12 || (zdim < 3 && zdim != 1) || zdim > 12)
	{
		INFO("File %s not recognized %d %d %d\n", filename, xdim, ydim, zdim);
		return NULL;
	}


	int xsize = hdr.xsize[0] + 256 * hdr.xsize[1] + 65536 * hdr.xsize[2];
	int ysize = hdr.ysize[0] + 256 * hdr.ysize[1] + 65536 * hdr.ysize[2];
	int zsize = hdr.zsize[0] + 256 * hdr.zsize[1] + 65536 * hdr.zsize[2];

	width = xsize;
	height = ysize;

	int xblocks = (xsize + xdim - 1) / xdim;
	int yblocks = (ysize + ydim - 1) / ydim;
	int zblocks = (zsize + zdim - 1) / zdim;
	unsigned int datasize = xblocks * yblocks * zblocks * 16;
	size = datasize;
	uint8 *buffer = (uint8 *) malloc(datasize);
	if (!buffer)
	{
		return NULL;
	}
	size_t bytes_to_read = xblocks * yblocks * zblocks * 16;
	size_t bytes_read = file.Read(buffer, 1, bytes_to_read);
	file.Close();

	if (bytes_read != bytes_to_read)
	{
		INFO("Failed to read file %s\n", filename);
		return NULL;
	}


	std::vector<uint8> buff;
	buff.insert(buff.end(), buffer, buffer + datasize);
	for(int i = 1;i < 10;i++)
	{
		std::string mipname;

		unsigned int pos = strchr(filename, '.') - filename;
		for(unsigned int h = 0; h < pos; h++)
		{
			mipname += filename[h];
		}

		mipname += "_mip_";
		char number[2];
		sprintf(number, "%d", i);
		mipname += number;
		mipname += ".astc";


		AssetFile file(mipname);
		if(file.GetLastError())
		{
			INFO("Missing ASTC mipmaps!!!");
			return NULL;
		}
		mipmaps++;

		size_t hdr_bytes_read = file.Read(&hdr, 1, sizeof(astc_header));
		if (hdr_bytes_read != sizeof(astc_header))
		{
			return NULL;
		}

		uint32 magicval = hdr.magic[0] + 256 * (uint32) (hdr.magic[1]) + 65536 * (uint32) (hdr.magic[2]) + 16777216 * (uint32) (hdr.magic[3]);
		if (magicval != MAGIC_FILE_CONSTANT)
		{
			return NULL;
		}

		int xdim = hdr.blockdim_x;
		int ydim = hdr.blockdim_y;
		int zdim = hdr.blockdim_z;

		if (xdim < 3 || xdim > 12 || ydim < 3 || ydim > 12 || (zdim < 3 && zdim != 1) || zdim > 12)
		{
			INFO("File %s not recognized %d %d %d\n", filename, xdim, ydim, zdim);
			return NULL;
		}


		int xsize = hdr.xsize[0] + 256 * hdr.xsize[1] + 65536 * hdr.xsize[2];
		int ysize = hdr.ysize[0] + 256 * hdr.ysize[1] + 65536 * hdr.ysize[2];
		int zsize = hdr.zsize[0] + 256 * hdr.zsize[1] + 65536 * hdr.zsize[2];


		int xblocks = (xsize + xdim - 1) / xdim;
		int yblocks = (ysize + ydim - 1) / ydim;
		int zblocks = (zsize + zdim - 1) / zdim;
		size_t bytes_to_read = xblocks * yblocks * zblocks * 16;
		file.Read(buffer, 1, bytes_to_read);
		buff.insert(buff.end(), buffer, buffer + bytes_to_read);
	}

	free(buffer);
	size = buff.size();
	buffer = new uint8[buff.size()];
	std::copy(buff.begin(), buff.end(), buffer);
	return buffer;
}


bool Image2D::loadASTC (const char *filename)
{
	size_t datasize = -1;
	int x;
	int y;
	int mipmaps;
	uint8* output_image = load_astc_file(filename, datasize, x, y, mipmaps);
	if(output_image==NULL)
	{
		return false;
	}

	m_mipmaps = mipmaps;
	m_format = Image_RGBA_ASTC_8x8;
	m_width = x;
	m_height = y;
	m_size = datasize;
	m_data = output_image;
	return true;
}


bool Image2D::loadBmp (const char *file, bool flip)
{
	FILE *fp;
	BmpFileHdr	fHdr;
	BmpImgHdr	iHdr;

	fp = fopen (file, "rb");
	if (fp == NULL) {
		return false;
	}

	KCL::uint16	isBmp = 0;
	fread (&isBmp, 2, 1, fp);
	fHdr.header = (isBmp == 0x4D42);
	fread (&fHdr.size, 4, 1, fp);
	fseek (fp, 4, SEEK_CUR);
	fread (&fHdr.offset, 4, 1, fp);

	if (!fHdr.header)
	{
		fclose (fp);
		return false;
	}

	fread (&iHdr.size, 4, 1, fp);
	fread (&iHdr.width, 4, 1, fp);
	fread (&iHdr.height, 4, 1, fp);
	fseek (fp, 2, SEEK_CUR);
	fread (&iHdr.bitCount, 2, 1, fp);
	if (!(iHdr.bitCount == 8 || iHdr.bitCount == 24 || iHdr.bitCount == 32))
	{
		fclose (fp);
		return false;
	}
	fread (&iHdr.compression, 4, 1, fp);
	if (iHdr.compression != 0)
	{
		fclose (fp);
		return false;
	}
	fseek (fp, 20, SEEK_CUR);

//TODO: check if palette size is not less then 256, otherwise fseek skips too much
	if (iHdr.bitCount == 8)
	{
		fseek (fp, 0x400, SEEK_CUR);
	}

	if (iHdr.bitCount == 8)
	{
		m_format = Image_LUMINANCE_L8;
	}
	else if (iHdr.bitCount == 24)
	{
		m_format = Image_RGB888;
	}
	else if (iHdr.bitCount == 32)
	{
		m_format = Image_RGBA8888;
	}
	else
	{
		return false;
	}

	m_width = iHdr.width;
	m_height = iHdr.height;

	m_data = new KCL::uint8[m_width * m_height * getBpp() / 8];
	assert(m_data);
	m_size = m_width*m_height*getBpp()/8;
	KCL::uint32 j;
	int bytes = getBpp ()/8;
	if (flip)
	{
		KCL::int32 i;
		for (i = m_height-1; i >= 0; i--)
		{
			for (j = 0; j < m_width*bytes; j+=bytes)
			{
				if (m_format == Image_LUMINANCE_L8 || m_format == Image_ALPHA_A8)
				{
					fread (&m_data[i*m_width+j], 1, 1, fp);
				}
				else if (m_format == Image_RGB888)
				{
					KCL::uint32 rgb = 0;

					fread (&rgb, 3, 1, fp);
					m_data[i*m_width*3+j] = (rgb>>16)&0xff;
					m_data[i*m_width*3+j+1] = (rgb>>8)&0xff;
					m_data[i*m_width*3+j+2] = rgb&0xff;
				}
				else if (m_format == Image_RGBA8888)
				{
					KCL::uint32 rgb = 0;
					fread (&rgb, 4, 1, fp);
					m_data[i*m_width*4+j] = rgb>>16;
					m_data[i*m_width*4+j+1] = rgb>>8;
					m_data[i*m_width*4+j+2] = rgb;
				}
			}
			if (iHdr.bitCount == 8 || iHdr.bitCount == 24)
			{
				int k = j;
				while (j%4)
				{
					j++;
				}
				fseek (fp, j - k, SEEK_CUR);
			}
		}
	}
	else
	{
		KCL::uint32 i;
		bytes = getBpp ()/8;
		for (i = 0; i < m_height; i++)
		{
			for (j = 0; j < m_width*bytes; j+=bytes)
			{
				if (m_format == Image_LUMINANCE_L8 || m_format == Image_ALPHA_A8)
				{
					fread (&m_data[i*m_width+j], 1, 1, fp);
				}
				else if (m_format == Image_RGB888)
				{
					KCL::uint32 rgb = 0;

					fread (&rgb, 3, 1, fp);
					m_data[i*m_width*3+j] = (rgb>>16)&0xff;
					m_data[i*m_width*3+j+1] = (rgb>>8)&0xff;
					m_data[i*m_width*3+j+2] = rgb&0xff;
				}
				else if (m_format == Image_RGBA8888)
				{
					KCL::uint32 rgb = 0;
					fread (&rgb, 4, 1, fp);
					m_data[i*m_width*4+j] = rgb>>16;
					m_data[i*m_width*4+j+1] = rgb>>8;
					m_data[i*m_width*4+j+2] = rgb;
				}
			}
			if (iHdr.bitCount == 8 || iHdr.bitCount == 24)
			{
				int k = j;
				while (j%4)
				{
					j++;
				}
				fseek (fp, j - k, SEEK_CUR);
			}
		}
	}
	fclose (fp);
	return true;
}

bool Image2D::loadPvr (const char *filename)
{
	AssetFile file(filename);

	if(file.GetLastError())
	{
		return false;
	}

	PVRHeader header;
	file.Read(&header.headSize, sizeof(unsigned int), 1);
	file.Read(&header.height, sizeof(unsigned int), 1);
	file.Read(&header.width, sizeof(unsigned int), 1);
	file.Read(&header.mipLevels, sizeof(unsigned int), 1);
	file.Read(&header.imageType, sizeof(unsigned int), 1);
	file.Read(&header.dataSize, sizeof(unsigned int), 1);
	file.Read(&header.bitsPPixel, sizeof(unsigned int), 1);
	file.Read(&header.redMask, sizeof(unsigned int), 1);
	file.Read(&header.greenMask, sizeof(unsigned int), 1);
	file.Read(&header.blueMask, sizeof(unsigned int), 1);
	file.Read(&header.alphaMask, sizeof(unsigned int), 1);
	file.Read(&header.PVRid, sizeof(unsigned int), 1);
	file.Read(&header.numOfSurfaces, sizeof(unsigned int), 1);

	//little/big-Endian test
	if(IsBigEndian())
	{
		header.headSize = convertUInt( header.headSize);
		header.height = convertUInt( header.height);
		header.width = convertUInt( header.width);
		header.mipLevels = convertUInt( header.mipLevels);
		header.imageType = convertUInt( header.imageType);
		header.dataSize = convertUInt( header.dataSize);
		header.bitsPPixel = convertUInt( header.bitsPPixel);
		header.redMask = convertUInt( header.redMask);
		header.greenMask = convertUInt( header.greenMask);
		header.blueMask = convertUInt( header.blueMask);
		header.alphaMask = convertUInt( header.alphaMask);
		header.PVRid = convertUInt( header.PVRid);
		header.numOfSurfaces = convertUInt( header.numOfSurfaces);
	}
	if(header.PVRid != 0x21525650)
	{
		return loadPvr3( filename);
	}

	m_height = header.height;
	m_width  = header.width;

	switch (header.imageType & 0xFF)
	{
		case 16:
			m_format=Image_RGBA4444;
			break;
		case 17:
			m_format=Image_RGBA5551;
			break;
		case 5:
		case 18:
			m_format=Image_RGBA8888;
			break;
		case 2:
		case 19:
			m_format=Image_RGB565;
			break;
		case 4:
		case 21:
			m_format=Image_RGB888;
			break;
		case 22:
			m_format = Image_LUMINANCE_L8;
			break;
		case 8:
		case 23:
			m_format = Image_LUMINANCE_ALPHA_LA88;
		break;
		case 24:
			m_format=Image_PVRTC2;
			break;
		case 13:
		case 25:
			m_format=Image_PVRTC4;
			break;
		case 32:
			m_format=Image_DXT1;
			break;
		case 33:
			m_format=Image_DXT2;
			break;
		case 34:
			m_format=Image_DXT3;
			break;
		case 35:
			m_format=Image_DXT4;
			break;
		case 36:
			m_format=Image_DXT5;
			break;
		case 54:
			m_format=Image_ETC1;
			break;
		default:
			m_format=ImageTypeAny;
			break;
	}

	getDescription ();
	if (header.imageType & 0x100) // if has mipmaps
	{
		m_mipmaps = header.mipLevels;
	}
	if (header.imageType & 0x8000) // if has mipmaps
	{
		m_has_pvr_alpha = true;
	}
	m_size = header.dataSize;
	m_data = new KCL::uint8[header.dataSize];
	file.Read(m_data, 1, header.dataSize);


#if 0
	if( header.imageType & 0x200)
	{
		KCL::uint16 *src = (KCL::uint16*)m_data;
		KCL::uint16 *dst = new KCL::uint16[header.dataSize];
		detwiddle( 0, 0, m_width, m_width, dst, &src);

		delete m_data;
		m_data = (KCL::uint8*)dst;
	}
#endif


	return true;
}


bool Image2D::loadPvr3 (const char *filename)
{
	AssetFile file(filename);

	if(file.GetLastError())
	{
		return false;
	}

	PVRHeaderV3 header;

	file.Read( &header.m_version, sizeof(KCL::uint32), 1);
	file.Read( &header.m_flags, sizeof(KCL::uint32), 1);
	file.Read( &header.m_pixel_format, sizeof(KCL::uint64), 1);
	file.Read( &header.m_color_space, sizeof(KCL::uint32), 1);
	file.Read( &header.m_channel_type, sizeof(KCL::uint32), 1);
	file.Read( &header.m_height, sizeof(KCL::uint32), 1);
	file.Read( &header.m_width, sizeof(KCL::uint32), 1);
	file.Read( &header.m_depth, sizeof(KCL::uint32), 1);
	file.Read( &header.m_num_surfaces, sizeof(KCL::uint32), 1);
	file.Read( &header.m_num_faces, sizeof(KCL::uint32), 1);
	file.Read( &header.m_num_mipmaps, sizeof(KCL::uint32), 1);
	file.Read( &header.m_metadata_size, sizeof(KCL::uint32), 1);

	if( header.m_version != PVRTEX3_IDENT)
	{
		return false;
	}

	m_height = header.m_height;
	m_width  = header.m_width;

	uint64 pixel_format = header.m_pixel_format;
	uint64 pixel_format_hi = pixel_format & PVRTEX_PFHIGHMASK;

	if( pixel_format_hi == 0)
	{
		switch( pixel_format)
		{
		case 6:
			{
				m_format=Image_ETC1;
				break;
			}
		case 22:
			{
				m_format=Image_ETC2_RGB;
				break;
			}
		case 23:
			{
				m_format=Image_ETC2_RGBA8888;
				break;
			}
		case 7:
			{
				m_format=Image_DXT1;
				break;
			}
		default:
			m_format=ImageTypeAny;
		}
	}
	else
	{
		m_format = Image_RGBA8888;
	}

	getDescription ();
	m_mipmaps = header.m_num_mipmaps - 1;
	m_size = file.GetLength() - PVRTEX3_HEADERSIZE - header.m_metadata_size;
	m_data = new KCL::uint8[m_size];
	file.Seek( header.m_metadata_size, SEEK_CUR);
	file.Read( m_data, 1, m_size);

	return true;
}


void assetRead(png_structp png_ptr,png_bytep m_data, png_size_t length)
{
	AssetFile *io = (AssetFile*)png_get_io_ptr( png_ptr);
	io->Read( m_data, 1, length);
}


bool Image2D::loadPng (const char *filename)
{
	KCL::AssetFile file(filename);
	if(file.GetLastError())
	{
		return false;
	}

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	unsigned int i;
	unsigned char sig[8];
	int  bit_depth, color_type;
	png_uint_32  width, height;
	size_t  rowbytes;
	png_bytepp  row_pointers = NULL;

	file.Read(sig, sizeof(char), 8);
	if (!png_check_sig(sig, 8))
	{
		return false;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
	{
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return false;
	}

	png_set_read_fn(png_ptr, &file, assetRead);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

	png_set_option(png_ptr, PNG_SKIP_sRGB_CHECK_PROFILE, PNG_OPTION_OFF);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	KCL::uint8 channels = png_get_channels( png_ptr, info_ptr);
	switch (channels) {
	case 1:
		m_format = Image_LUMINANCE_L8;
		break;
	case 2:
		m_format = Image_LUMINANCE_ALPHA_LA88;
		break;
	case 3:
		m_format = Image_RGB888;
		break;
	case 4:
		m_format = Image_RGBA8888;
		break;
	}

	m_size = rowbytes*height;
	m_data = new KCL::uint8[rowbytes*height];

	if ((row_pointers = (png_bytepp) new png_bytep[height]) == NULL) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		delete [] m_data;
		m_data = NULL;
		return false;
	}


	for (i = 0;  i < height;  ++i)
		row_pointers[i] = m_data + i*rowbytes;

	png_read_image(png_ptr, row_pointers);

	m_width = width;
	m_height = height;

	delete [] row_pointers;

	row_pointers = NULL;

	png_read_end (png_ptr, NULL);

	if (png_ptr && info_ptr) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		png_ptr = NULL;
		info_ptr = NULL;
	}

	getDescription ();
	return true;
}


void Image2D::setData (KCL::uint32 size, const KCL::uint8 *data)
{
	release_data ();
	m_data = new KCL::uint8[size];
	memcpy (m_data, data, size);
}


void Image2D::setDimensions (KCL::uint32 width, KCL::uint32 height, KCL::uint32 format)
{
	m_width = width;
	m_height = height;
	m_format = (KCL::ImageFormat)format;
	release_data();
}


void Image2D::allocBuffer(KCL::uint32 width, KCL::uint32 height, KCL::uint32 format)
{
	m_format = (KCL::ImageFormat)format;
	int bytes = getBpp ()/8;
	setDimensions(width, height, format);
	m_data = new KCL::uint8[height*width*bytes];
}


void Image2D::convertRGB ()
{
	//if (m_format == RGB_ETC)
	//	decodeETC1toRGB888 ();

	if (m_format == Image_ETC1)
	{
		decodeETC1toRGB565 ();
	}
	else if(m_format == Image_RGBA_ASTC_8x8)
	{
		decodeASTCtoRGB888();
	}
	else
	{
		abort();
	}
}


void Image2D::decodeASTCtoRGB888()
{
#ifdef DEFINE_ASTC_UNCOMPRESS
	int x, y, z;
	astc_header hdr;

	hdr.blockdim_x = 8;
	hdr.blockdim_y = 8;
	hdr.blockdim_z = 1;

	astc_decode_mode decode_mode = DECODE_HDR;
	swizzlepattern swz_decode = { 0, 1, 2, 3 };

	int bitness = 8;
	int xdim = hdr.blockdim_x;
	int ydim = hdr.blockdim_y;
	int zdim = hdr.blockdim_z;

	int xsize = m_width;
	int ysize = m_height;
	int zsize = 1;

	int xblocks = (xsize + xdim - 1) / xdim;
	int yblocks = (ysize + ydim - 1) / ydim;
	int zblocks = (zsize + zdim - 1) / zdim;


	build_quantization_mode_table();
	uint8* buffer = m_data;

	astc_codec_image *img = allocate_image(bitness, xsize, ysize, zsize, 0);
	initialize_image(img);

	imageblock pb;
	for (z = 0; z < zblocks; z++)
	{
		for (y = 0; y < yblocks; y++)
		{
			for (x = 0; x < xblocks; x++)
			{
				int offset = (((z * yblocks + y) * xblocks) + x) * 16;
				uint8_t *bp = buffer + offset;
				physical_compressed_block pcb = *(physical_compressed_block *) bp;
				symbolic_compressed_block scb;
				physical_to_symbolic(xdim, ydim, zdim, pcb, &scb);
				decompress_symbolic_block(decode_mode, xdim, ydim, zdim, x * xdim, y * ydim, z * zdim, &scb, &pb);
				write_imageblock(img, &pb, xdim, ydim, zdim, x * xdim, y * ydim, z * zdim, swz_decode);
			}
		}
	}
	free(buffer);



	int channel = determine_image_channels(img);

	m_width  = img->xsize;
	m_height = img->ysize;

	if(channel==1)
	{
		m_format = Image_ALPHA_A8;//TODO fix this?
	}
	else if(channel==3)
	{
		m_format = Image_RGB888;
	}
	else if(channel==4)
	{
		m_format = Image_RGBA8888;
	}

	KCL::uint8 *row_pointers8 = new uint8_t[m_width * m_height * channel];

	for (unsigned int y = 0; y < m_height; y++)
	{
		switch (channel)
		{
		case 1:
			for (unsigned int x = 0; x < m_width; x++)
			{
				row_pointers8[(m_width * channel * (m_height - 1 - y)) + x * channel] = img->imagedata8[0][y][4 * x];
			}
			break;
		case 2:
			for (unsigned int x = 0; x < m_width; x++)
			{
				row_pointers8[(m_width * channel * (m_height - 1 - y)) + x * channel + 1] = img->imagedata8[0][y][4 * x + 1];
				row_pointers8[(m_width * channel * (m_height - 1 - y)) + x * channel + 0] = img->imagedata8[0][y][4 * x + 0];
			}
			break;
		case 3:
			for (unsigned int x = 0; x < m_width; x++)
			{
				row_pointers8[(m_width * channel * (m_height - 1 - y)) + x * channel + 2] = img->imagedata8[0][y][4 * x + 2];
				row_pointers8[(m_width * channel * (m_height - 1 - y)) + x * channel + 1] = img->imagedata8[0][y][4 * x + 1];
				row_pointers8[(m_width * channel * (m_height - 1 - y)) + x * channel + 0] = img->imagedata8[0][y][4 * x + 0];
			}
			break;
		case 4:
			for (unsigned int x = 0; x < m_width; x++)
			{
				row_pointers8[(m_width * channel * (m_height - 1 -y)) + x * channel + 3] = img->imagedata8[0][y][4 * x + 3];
				row_pointers8[(m_width * channel * (m_height - 1 -y)) + x * channel + 2] = img->imagedata8[0][y][4 * x + 2];
				row_pointers8[(m_width * channel * (m_height - 1 -y)) + x * channel + 1] = img->imagedata8[0][y][4 * x + 1];
				row_pointers8[(m_width * channel * (m_height - 1 -y)) + x * channel + 0] = img->imagedata8[0][y][4 * x + 0];
			}
			break;
		}
	}

	destroy_image(img);
	m_mipmaps = 0;
	m_data = (KCL::uint8*)row_pointers8;
#endif
}


void Image2D::decodeETC1toRGB888 ()
{
	KCL::uint8 *data = new KCL::uint8[m_width*m_height*3];
	DecodeETC1toRGB888 (m_width, m_height, m_data, data);
	delete [] m_data;
	m_data = data;
	m_mipmaps = 0;
	m_format = Image_RGB888;
	getDescription();
}


void Image2D::decodeETC1toRGB565 ()
{
	KCL::uint16 *data = new KCL::uint16[m_width*m_height];
	DecodeETC1toRGB565 (m_width, m_height, m_data, data);
	delete [] m_data;
	m_data = (KCL::uint8*)data;
	m_mipmaps = 0;
	m_format = Image_RGB565;
	getDescription();
}

void Image2D::decodeRGB888toRGBA8888 ()
{
	if (m_format ==Image_RGB888 )
	{
		m_size = m_width*m_height*4;
		uint8 *data = new uint8[m_size];

		for(unsigned int i = 0;i < m_width * m_height;i++)
		{
			data[i*4]   = m_data[i*3];
			data[i*4+1] = m_data[i*3+1];
			data[i*4+2] = m_data[i*3+2];
			data[i*4+3] = 255;
		}
		delete [] m_data;
		m_data = data;
		m_mipmaps = 0;
		m_format = Image_RGBA8888;
	}
	if (m_format == Image_LUMINANCE_L8 )
	{
		m_size = m_width * m_height * 4;
		uint8 *data = new uint8[m_size];

		for(unsigned int i = 0;i < m_width * m_height;i++)
		{
			data[i*4]   = m_data[i];
			data[i*4+1] = m_data[i];
			data[i*4+2] = m_data[i];
			data[i*4+3] = 255;
		}
		delete [] m_data;
		m_data = data;
		m_mipmaps = 0;
		m_format = Image_RGBA8888;
	}
}


void Image2D::generateFalseColorMipmapDataWithId (KCL::uint32 imageId, int size)
{
	release_data ();
	m_width = size;
	m_height = size;
	m_format = Image_RGB888;
	int totalsize = 0;
	m_mipmaps = 0;
	for (int i = 1; i <= size; i<<=1)
	{
		totalsize += i*i;
		m_mipmaps++;
	}
	totalsize *= 3; // RGB888
	m_data = new KCL::uint8[totalsize];

	KCL::uint8 *data = 0;
	for (KCL::uint32 map = 0; map < m_mipmaps; map++)
	{
		data = getMipmapData (map);
		for (int i = 0; i < 1<<((m_mipmaps-1-map)*2); i++)
		{
			data[i*3] = (KCL::uint8) imageId;
			data[i*3+1] = (KCL::uint8) map;
			data[i*3+2] = m_mipmaps;
		}
	}
}


KCL::uint8 *Image2D::getMipmapData (KCL::uint32 level)
{
	if (!m_mipmaps) return 0;
	if (m_width != m_height) return m_data;	// TODO:

	int offset = 0;
	int bpp = getBpp ()/8;

	for (KCL::uint32 i = 0; i < level; ++i)
	{
		offset += 1<<((m_mipmaps-i-1)*2);
	}
	offset *= bpp;

	return m_data + offset;
}


void Image2D::getMipmapData (KCL::uint32 level, KCL::uint8 **data, KCL::uint32 *mipmapsize, KCL::uint32 *mipWidth, KCL::uint32 *mipHeight, KCL::uint32* mempitch) const
{
	if (!m_mipmaps)
	{
		KCL::uint32 mipx = m_width;		/// calculate mipsize
		KCL::uint32 mipy = m_height;
		KCL::uint32 minx = m_desc?m_desc->minx:1;
		KCL::uint32 miny = m_desc?m_desc->miny:1;
		mipx = mipx > minx ? mipx : minx;
		mipy = mipy > miny ? mipy : miny;

		*data = m_data;
		if(mipmapsize)
		{
			*mipmapsize = mipx*mipy*getBpp ()/8;
		}
		return;
	}

	int offset = 0;
	int bpp = getBpp ();

	KCL::uint32 mipx = m_width;		/// calculate mipsize
	KCL::uint32 mipy = m_height;
	KCL::uint32 mipx2 = m_width;		/// calculate image size
	KCL::uint32 mipy2 = m_height;
	KCL::uint32 minx = m_desc?m_desc->minx:1;
	KCL::uint32 miny = m_desc?m_desc->miny:1;

	for (KCL::uint32 i = 0; i <= level; ++i)
	{
		mipx = max(mipx, minx);
		mipy = max(mipy, miny);

		if (mipWidth) *mipWidth = mipx2>1?mipx2:1;
		if (mipHeight) *mipHeight = mipy2>1?mipy2:1;


		int size = mipx*mipy;
		size *= bpp;
		size /= 8;

		if(mipmapsize)
		{
			*mipmapsize = size;
		}

		if( mempitch)
		{
			*mempitch = size / mipy;
		}

		if (i < level)
			offset += size;

		mipx >>= 1;
		mipy >>= 1;
		mipx2 >>= 1;
		mipy2 >>= 1;
	}
	if (m_data != 0)
	{
		*data = m_data + offset;
	}
	else
	{
		*data = 0;
	}
}



ImageCube::ImageCube() :
ImageBase ()
{
	for (int i = 0; i < 6; ++i)
	{
		m_data[i] = NULL;
	}
}


ImageCube::~ImageCube()
{
	for (int i = 0; i < 6; ++i)
	{
		delete [] m_data[i];
		m_data[i] = NULL;
	}
}


void ImageCube::convertRGB ()
{
	if (m_format == Image_ETC1) decodeETC1 ();
}


void ImageCube::decodeETC1 ()
{
	for (int i = 0; i < 6; i++)
	{
		KCL::uint8 *data = new KCL::uint8[m_width*m_height*3];
		DecodeETC1toRGB888 (m_width, m_height, m_data[i], data);
		delete [] m_data[i];
		m_data[i] = data;
		m_mipmaps = 0;
		m_format = Image_RGB888;
	}
}


void ImageCube::setData (int k, size_t size, KCL::uint8 *data)
{
	delete [] m_data[k];
	m_data[k] = new KCL::uint8[size];
	memcpy (m_data[k], data, size);
}


void ImageCube::setDimensions (KCL::uint32 width, KCL::uint32 height, KCL::uint32 format)
{
	m_width = width;
	m_height = height;
	m_format = (KCL::ImageFormat)format;
}


void ImageCube::allocBuffer (KCL::uint32 width, KCL::uint32 height, KCL::uint32 format)
{
	m_format = (KCL::ImageFormat)format;
	int bytes = getBpp ()/8;
	setDimensions(width, height, format);
	for (int i = 0; i < 6; i++) {
		m_data[i] = new KCL::uint8[height*width*bytes];
	}
}

bool ImageCube::load (const char *fileBase)
{
	Image2D *face = 0;
	char file[1024] = {0};

	for (int i = 0; i < 6; i++)
	{
		face = new Image2D;
		sprintf (file, "%s_%0x.bmp", fileBase, TexMapBase+i);
		strcpy((char*)&m_filename[i][0],file);

		if (face->load (file))
		{
			m_format = face->m_format;
			setData (i, face->m_size, face->data());
			m_mipmaps = face->m_mipmaps;
			m_width = face->getWidth();
			m_height = face->getHeight();
		}
		else
		{
			INFO("ERROR: file not found=%s",file);
			return false;
		}
		delete face;
	}
	return true;
}


void detwiddle(int x1, int y1, int size, int imgsize, KCL::uint16* dst, KCL::uint16 **source)
{
	if (size == 1)
	{
		dst[y1*imgsize+x1] = **source;
		(*source)++;
	}
	else
	{
		int ns = size/2;
		detwiddle(x1, y1, ns, imgsize, dst, source);
		detwiddle(x1, y1+ns, ns, imgsize, dst, source);
		detwiddle(x1+ns, y1, ns, imgsize, dst, source);
		detwiddle(x1+ns, y1+ns, ns, imgsize, dst, source);
	}
}

KCL::uint32 getBpp(ImageFormat format)
{
	switch (format)
	{
	case Image_LUMINANCE_L8:
	case Image_ALPHA_A8:
	case Image_DXT3:
	case Image_DXT5:
	case Image_ETC2_RGBA8888:
		return 8;

	case Image_LUMINANCE_ALPHA_LA88:
	case Image_RGB565:
	case Image_RGBA4444:
	case Image_RGBA5551:
		return 16;

	case Image_RGB888:
		return 24;

	case Image_RGBA8888:
		return 32;

	case Image_ETC1:
	case Image_DXT1:
    case Image_DXT1_RGBA:
	case Image_PVRTC4:
	case Image_ETC2_RGB:
	case Image_ETC2_RGB_A1:
		return 4;

	case Image_PVRTC2:
		return 2;

	default:
		return 0;
	}
}

KCL::Image::Image(void) :
    m_size(0),
    m_type(Image_Unknown),
    m_format(ImageTypeAny),
    m_width(0),
    m_height(0),
    m_depth(0),
    m_mipcount(0),
    m_minx(1),
    m_miny(1),
    m_ptrRawData(NULL),
    m_sRGB_values(false)
{
}

KCL::Image::Image(const KCL::Image &source, bool deepCopy):
    m_name(source.m_name),
    m_size(source.m_size),
    m_linePitch(source.m_linePitch),
    m_slicePitch(source.m_slicePitch),
    m_type(source.m_type),
    m_format(source.m_format),
    m_width(source.m_width),
    m_height(source.m_height),
    m_depth(source.m_depth),
    m_mipcount(source.m_mipcount),
    m_minx(source.m_minx),
    m_miny(source.m_miny),
    m_ptrUint8(NULL),
    m_sRGB_values(false)
{
    if(deepCopy)
	{
        m_ptrUint8 = new KCL::uint8[m_size];
	    memcpy(m_ptrUint8,source.m_ptrUint8,m_size);
    }
}


void KCL::Image::Allocate2D(KCL::uint32 width, KCL::uint32 height, ImageFormat format)
{
	releaseData();

	m_width=width;
	m_height=height;
	m_depth=1;
	m_format=format;
	m_type=Image_2D;
    m_mipcount = 0;
    m_minx = 1;
    m_miny = 1;
	KCL::uint32 bpp = getBpp();
	switch (m_format)
	{
	case Image_DXT1:
	case Image_DXT1_RGBA:
		m_linePitch = 2 * m_width;
		break;
    case Image_DXT5:
        m_linePitch = 4 * m_width;
        break;

	default:
		m_linePitch = (((bpp * m_width - 1) | 0x7) + 1) >> 3;
		break;
	}

	m_size = m_slicePitch = m_linePitch * m_height;
	m_ptrUint8 = new KCL::uint8[m_size];

	setupMIPminXminY();
}

KCL::Image::Image(KCL::uint32 width, KCL::uint32 height, ImageFormat format):m_ptrUint8(NULL), m_sRGB_values(false)
{
	Allocate2D(width,height,format);
}

KCL::Image::Image(KCL::uint32 width, KCL::uint32 height, KCL::uint32 depth, ImageFormat format) :
    m_type(Image_3D),
    m_format(format),
    m_width(width),
    m_height(height),
    m_depth(depth),
    m_mipcount(0),
    m_minx(1),
    m_miny(1),
    m_sRGB_values(false)
{
	KCL::uint32 bpp = getBpp();
	switch (m_format)
	{
	case Image_DXT1:
	case Image_DXT1_RGBA:
		m_linePitch = 2 * m_width;
		break;

    case Image_DXT5:
        m_linePitch = 4 * m_width;
        break;

	default:
		m_linePitch = (((bpp * m_width - 1) | 0x7) + 1) >> 3;
		break;
	}

	m_slicePitch = m_linePitch * m_height;
	m_size = m_slicePitch * m_depth;
	m_ptrUint8 = new KCL::uint8[m_size];

    setupMIPminXminY();
}

KCL::Image::Image(KCL::uint32 width, KCL::uint32 height, KCL::uint32 depth, size_t linePitch, size_t slicePitch, ImageType type, ImageFormat format):
    m_size(depth * slicePitch),
	m_linePitch(linePitch),
	m_slicePitch(slicePitch),
	m_type(type),
    m_format(format),
    m_width(width),
    m_height(height),
    m_depth(depth),
    m_mipcount(0),
    m_minx(1),
    m_miny(1),
    m_sRGB_values(false)
{
	m_ptrUint8 = new KCL::uint8[m_size];

    setupMIPminXminY();
}

KCL::Image::~Image(void)
{
	releaseData();
}

KCL::uint32 KCL::Image::getBpp() const	{ return ::getBpp(m_format); }

void KCL::Image::getBlockDimensions(KCL::uint32 &x, KCL::uint32 &y, KCL::uint32 &z) const
{
	switch (m_format)
	{
		case Image_RGB9E5:
		case Image_RGB888:
		case Image_RGBA8888:
		case Image_RGBA_32F:
			x = y = z = 1;
			break;

		case Image_DXT1:
		case Image_DXT1_RGBA:
		case Image_DXT5:
			x = y = 4; z = 1;
			break;

		case Image_ETC1:
		case Image_ETC2_RGB:
		case Image_ETC2_RGB_A1:
		case Image_ETC2_RGBA8888:
			x = y = 4; z = 1;
			break;

		case Image_RGBA_ASTC_4x4:
			x = y = 4; z = 1;
			break;
		case Image_RGBA_ASTC_5x5:
			x = y = 5; z = 1;
			break;
		case Image_RGBA_ASTC_6x6:
			x = y = 6; z = 1;
			break;
		case Image_RGBA_ASTC_8x8:
			x = y = 8; z = 1;
			break;

		default:
			assert(0);
			break;
	}
}

KCL::uint32 KCL::Image::getBlockSize() const
{
	switch (m_format)
	{
		case Image_RGB888:
			return 24;
		case Image_RGB9E5:
		case Image_RGBA8888:
			return 32;

		case Image_RGBA_32F:
			return 128;

		case Image_DXT1:
		case Image_DXT1_RGBA:
			return 64;
		case Image_DXT5:
			return 128;

		case Image_ETC1:
			return 64;
		case Image_ETC2_RGB:
		case Image_ETC2_RGB_A1:
			return 64;
		case Image_ETC2_RGBA8888:
			return 128;

		case Image_RGBA_ASTC_4x4:
		case Image_RGBA_ASTC_5x5:
		case Image_RGBA_ASTC_6x6:
		case Image_RGBA_ASTC_8x8:
			return 128;

		default:
			assert(0);
			return 0;
	}
}


bool KCL::Image::isPresent(const char *file_)
{
	if (strlen(file_) < 1)
	{
		return false;
	}
	std::string filename2(file_);

	filename2.replace(filename2.rfind("."), filename2.length(), ".pvr");
	if (KCL::File::Exists(filename2) == false)
	{
		filename2.replace(filename2.rfind("."), filename2.length(), ".astc");
		if (KCL::File::Exists(filename2) == false)
		{
			filename2.replace(filename2.rfind("."), filename2.length(), ".tga");
			if (KCL::File::Exists(filename2) == false)
			{
				filename2.replace(filename2.rfind("."), filename2.length(), ".png");
				if (KCL::File::Exists(filename2) == false)
				{
					return false;
				}
			}
		}
	}
	return true;
}


bool KCL::Image::load(const char *file_, bool flip)
{
	releaseData();

	if(strlen(file_) < 1 )
	{
		return false;
	}

	m_name = file_;
	std::string filename2(file_);
	if (filename2.rfind(".") == std::string::npos)
	{
		return false;
	}
	while (1)
	{
		filename2.replace(filename2.rfind("."), filename2.length(), ".pvr");
		if (KCL::File::Exists(filename2))
		{
			loadPvr2(filename2.c_str()); break;
		}
		filename2.replace(filename2.rfind("."), filename2.length(), ".astc");
		if (KCL::File::Exists(filename2))
		{
			loadASTC(filename2.c_str()); break;
		}
		filename2.replace(filename2.rfind("."), filename2.length(), ".tga");
		if (KCL::File::Exists(filename2))
		{
			loadTga(filename2.c_str()); break;
		}
		filename2.replace(filename2.rfind("."), filename2.length(), ".png");
		if (KCL::File::Exists(filename2))
		{
			loadPng(filename2.c_str()); break;
		}
		filename2.replace(filename2.rfind("."), filename2.length(), ".raw");
		if (KCL::File::Exists(filename2))
		{
			loadFloat3Raw(filename2.c_str()); break;
		}
		filename2.replace(filename2.rfind("."), filename2.length(), ".hdr");
		if (KCL::File::Exists(filename2))
		{
			loadHdr(filename2.c_str()); break;
		}
		return false;
	}

	m_type = Image_2D;
	m_depth = 1;

    setupMIPminXminY();
	m_image_filename = filename2;
	return true;
}

int KCL::Image::load3D(const char *fileBase)
{
	m_type = Image_3D;
	return loadArray(fileBase, "%s_%04d.png", 1, INT_MAX);
}

bool KCL::Image::loadCube(const char * fileBase)
{
	int loadedCount = loadArray(fileBase, "%s_%0x.bmp", TexMapBase, 6);
	if (loadedCount == 6)
	{
		m_type = Image_Cube;
		return true;
	}
	else
	{
		releaseData();
		return false;
	}
}

int KCL::Image::loadCubeArray(const char *fileBase)
{
	std::vector<std::string> filenames;

	char name[1024] = {0};
	int cube_idx = 0;

	bool done = false;
	while (!done)
	{
		for (int face = 0; face < 6; face++)
		{
			sprintf(name, "%s%03d_%0x.png", fileBase, cube_idx, TexMapBase + face);

			if (!KCL::Image::isPresent(name))
			{
				done = true;
				break;
 			}

			filenames.push_back(name);
		}
		cube_idx++;
	}

	if (filenames.empty() || (filenames.size() % 6) != 0)
	{
		INFO("ERROR: Can not load %s as cube map array! It has invalid number of faces: %d", fileBase, filenames.size());
		return 0;
	}

	int loadedCount = loadFaces(filenames);
	if (loadedCount == filenames.size())
	{
		m_type = Image_CubeArray;
		// Depth is the number of cube faces, so cubemaps * 6
		return m_depth;
	}
	else
	{
		releaseData();
		return 0;
	}
}

int KCL::Image::loadArray(const char *fileBase)
{
	m_type = Image_Array;
	return loadArray(fileBase, "%s_%04d.png", 1, INT_MAX);
}

int KCL::Image::loadArray(const char *fileBase, const char *pattern, int startIdx, int count)
{
	if (strlen(fileBase) < 1 )
	{
		return false;
	}

	releaseData();
	std::vector<Image*> images;

	m_name = fileBase;
	char file[1024] = {0};

    if(count == INT_MAX)
    {
        count = 0;
        while(1)
        {
			if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

            sprintf (file, pattern, fileBase, startIdx + count);
            if (!KCL::Image::isPresent(file))
            {
                break;
            }
            ++count;
        }
    }

	for (int i = 0; i < count; i++)
	{
		if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

		Image *face = new Image;
		sprintf (file, pattern, fileBase, startIdx + i);

		if (!face->load (file))
		{
			INFO("ERROR: file not found=%s",file);
			delete face;
			break;
		}

		// Store the size and format of the first image. The others will have to have the same.
		if (i == 0)
		{
			m_format = face->m_format;
			m_width = face->m_width;
			m_height = face->m_height;
			m_linePitch = face->m_linePitch;
			m_slicePitch = face->m_size;
		}
		else if (m_format != face->m_format ||
			m_width != face->m_width ||
			m_height != face->m_height ||
			m_linePitch != face->m_linePitch)
		{
			if (face->m_size > m_slicePitch)
			{
				m_slicePitch = face->m_size;
			}

			INFO("ERROR: faces of a cube map contain images of different size or format( %u vs. %u): %s", face->m_format, m_format, file);
			delete face;
			break;
		}

		m_mipcount = face->m_mipcount;

		images.push_back(face);
	}

	if (images.size()==0) return false;

	m_depth = (uint32)images.size();
	// Allocate memory for the image array
	// m_slicePitch is the size of a face with the mipmap leves
	m_size = m_slicePitch * m_depth;
	m_ptrUint8 = new KCL::uint8[m_size];

	// Copy all of the images with mipmaps
	KCL::uint8* dst = m_ptrUint8;
	for (KCL::uint32 i  = 0; i < images.size(); i++, dst += m_slicePitch)
	{
		Image* img = images[i];
		memcpy(dst, img->getData(), img->getDataSize());
		delete img;
	}

	return m_depth;
}

int KCL::Image::loadFaces(const std::vector<std::string> &filenames)
{
	if (filenames.empty())
	{
		return 0;
	}

	releaseData();

	std::vector<KCL::Image*> images;
	images.reserve(filenames.size());

	for (size_t i = 0; i < filenames.size(); i++)
	{
		if (KCL::g_os->LoadingCallback(0) != KCL_TESTERROR_NOERROR) break;

		Image *face = new Image;

		const char *filename = filenames[i].c_str();
		if (!face->load (filename))
		{
			INFO("ERROR: file not found=%s", filename);
			delete face;
			break;
		}

		// Store the size and format of the first image. The others will have to have the same.
		if (i == 0)
		{
			m_format = face->m_format;
			m_width = face->m_width;
			m_height = face->m_height;
			m_linePitch = face->m_linePitch;
			m_slicePitch = face->m_size;
			m_mipcount = face->m_mipcount;
		}
		else if (m_format != face->m_format ||
			m_width != face->m_width ||
			m_height != face->m_height ||
			m_linePitch != face->m_linePitch ||
			m_mipcount != face->m_mipcount)
		{
			if (face->m_size > m_slicePitch)
			{
				m_slicePitch = face->m_size;
			}

			INFO("ERROR: faces of a cube map contain images of different size or format( %u vs. %u): %s", face->m_format, m_format, filename);
			delete face;
			break;
		}

		images.push_back(face);
	}

	if (images.empty()) return 0;

	m_depth = (uint32)images.size();
	// Allocate memory for the image array
	// m_slicePitch is the size of a face with the mipmap leves
	m_size = m_slicePitch * m_depth;
	m_ptrUint8 = new KCL::uint8[m_size];

	// Copy all of the images with mipmaps
	KCL::uint8* dst = m_ptrUint8;
	for (KCL::uint32 i  = 0; i < images.size(); i++, dst += m_slicePitch)
	{
		Image* img = images[i];
		memcpy(dst, img->getData(), img->getDataSize());
		delete img;
	}

	return m_depth;
}

bool KCL::Image::loadTga(const char *filename)
{
	AssetFile file(filename);
	if (!file.Opened())
	{
		return false;
	}

	unsigned char* buf = NULL;//don't overwrite, this is the original file data's address
	unsigned char firstField;//rep field
	unsigned char cols[4];
	int len;
	int pos = 0;

	char id;
	char colorMap;
	char imageType;
	short xOrigin;
	short yOrigin;
	int pixelDepthBit;

	buf = (uint8*)file.GetBuffer();

	id = *buf++;
	colorMap = *buf++;
	imageType = *buf++;
	buf += 5;

	xOrigin = (short)* buf;
	buf += 2;
	yOrigin = (short)* buf;
	buf += 2;

	m_width = *(short*)buf;
	buf += 2;
	m_height = *(short*)buf;
	buf += 2;

	pixelDepthBit = *(char*)buf++;
	char imageDescriptorByte;
	imageDescriptorByte = *(char*)buf++;

	//2=rgb
	//10=rle rgb

	if (!(imageType == 10 || imageType == 2))
	{
		//error not supported file format
		return false;
	}

	if (pixelDepthBit == 24 && imageType == 2)	//uncompressed 24bit
	{
		m_ptrUint8 = new unsigned char[m_width * m_height * 3];
		for (int i = 0; i < m_width*m_height; ++i)
		{
			m_ptrUint8[i*3+0] = buf[i*3+2];
			m_ptrUint8[i*3+1] = buf[i*3+1];
			m_ptrUint8[i*3+2] = buf[i*3+0];
		}
	}
	else if (pixelDepthBit == 32 && imageType == 2)//uncompressed 32bit
	{
		m_ptrUint8 = new unsigned char[m_width * m_height * 4];
		for (int i = 0; i < m_width*m_height; ++i)
		{
			m_ptrUint8[i * 4 + 0] = buf[i * 4 + 2];
			m_ptrUint8[i * 4 + 1] = buf[i * 4 + 1];
			m_ptrUint8[i * 4 + 2] = buf[i * 4 + 0];
			m_ptrUint8[i * 4 + 3] = buf[i * 4 + 3];
		}
	}
	else if (imageType == 10)//RLE compressed 24 or 32bit
	{
		m_ptrUint8 = new unsigned char[m_width * m_height * 4];

		while (m_height * m_width > pos / 4)
		{
			firstField = *buf++;
			len = 1 + (firstField & 0x7f);

			if (firstField & 0x80) //rle packet
			{
				switch (pixelDepthBit)
				{

					case 24:
						cols[0] = *buf++;
						cols[1] = *buf++;
						cols[2] = *buf++;

						break;

					case 32:
						cols[0] = *buf++;
						cols[1] = *buf++;
						cols[2] = *buf++;
						cols[3] = *buf++;
						break;
				}

				for (int i = pos; i < pos + len * 4; i += 4)
				{
					m_ptrUint8[i] = cols[0];
					m_ptrUint8[i + 1] = cols[1];
					m_ptrUint8[i + 2] = cols[2];

					if (pixelDepthBit == 32)
						m_ptrUint8[i + 3] = cols[3];
					else
						m_ptrUint8[i + 3] = 0xFF;
				}

				pos += len * 4;
			}
			else //raw packet
			{
				for (int i = pos; i < pos + len * 4; i += 4)
				{
					switch (pixelDepthBit)
					{
						case 24:
							cols[0] = *buf++;
							cols[1] = *buf++;
							cols[2] = *buf++;
							break;

						case 32:
							cols[0] = *buf++;
							cols[1] = *buf++;
							cols[2] = *buf++;
							cols[3] = *buf++;
							break;
					}

					m_ptrUint8[i] = cols[0];
					m_ptrUint8[i + 1] = cols[1];
					m_ptrUint8[i + 2] = cols[2];

					if (pixelDepthBit == 32)
						m_ptrUint8[i + 3] = cols[3];
					else
						m_ptrUint8[i + 3] = 0xFF;
				}

				pos += len * 4;
			}
		}
	}//compressed end


	/*!Bit 5 - screen origin bit.
	*0 = Origin in lower left - hand corner.
	*1 = Origin in upper left - hand corner.
	*Must be 0 for Truevision images.
	*vertical flip
	*/
	if ( (imageDescriptorByte >> 5) == 0)
	{

		unsigned char bTemp;
		unsigned char *pLine1, *pLine2;
		int iLineLen, iIndex;

		iLineLen = m_width*(pixelDepthBit / 8);
		pLine1 = m_ptrUint8;
		pLine2 = &m_ptrUint8[iLineLen * (m_height - 1)];

		for (; pLine1 < pLine2; pLine2 -= (iLineLen * 2))
		{
			for (iIndex = 0; iIndex != iLineLen; pLine1++, pLine2++, iIndex++)
			{
				bTemp = *pLine1;
				*pLine1 = *pLine2;
				*pLine2 = bTemp;
			}
		}
	}

	m_size = m_width * m_height * (pixelDepthBit / 8);

	m_linePitch = m_width * (pixelDepthBit / 8);
	m_slicePitch = m_height * m_linePitch;

	if (pixelDepthBit == 24)
	{
		m_format = Image_RGB888;
	}
	else if (pixelDepthBit == 32)
	{
		m_format = Image_RGBA8888;
	}

	return true;
}


bool KCL::Image::loadBmp(const char *filename, bool flip)
{
	FILE *fp;
	BmpFileHdr	fHdr;
	BmpImgHdr	iHdr;

	fp = fopen(filename, "rb");
	if (fp == NULL) {
		return false;
	}

	KCL::uint16	isBmp = 0;
	fread (&isBmp, 2, 1, fp);
	fHdr.header = (isBmp == 0x4D42);
	fread (&fHdr.size, 4, 1, fp);
	fseek (fp, 4, SEEK_CUR);
	fread (&fHdr.offset, 4, 1, fp);

	if (!fHdr.header)
	{
		fclose (fp);
		return false;
	}

	fread (&iHdr.size, 4, 1, fp);
	fread (&iHdr.width, 4, 1, fp);
	fread (&iHdr.height, 4, 1, fp);
	fseek (fp, 2, SEEK_CUR);
	fread (&iHdr.bitCount, 2, 1, fp);
	if (!(iHdr.bitCount == 8 || iHdr.bitCount == 24 || iHdr.bitCount == 32))
	{
		fclose (fp);
		return false;
	}
	fread (&iHdr.compression, 4, 1, fp);
	if (iHdr.compression != 0)
	{
		fclose (fp);
		return false;
	}

	fseek (fp, 20, SEEK_CUR);

	//TODO: check if palette size is not less than 256, otherwise fseek skips too much
	KCL::uint16 bpp = iHdr.bitCount;
	switch (iHdr.bitCount)
	{
	case 8:
		fseek (fp, 0x400, SEEK_CUR);
		m_format = Image_LUMINANCE_L8;
		break;

	case 24:
		m_format = Image_RGB888;
		break;

	case 32:
		m_format = Image_RGBA8888;
		break;

	default:
		fclose (fp);
		return false;
	}

	m_width = iHdr.width;
	m_height = iHdr.height;

	int bytesPerPixel = bpp >> 3;
	m_size = m_width * m_height * bytesPerPixel;
	m_ptrUint8 = new KCL::uint8[m_size];

	m_linePitch = m_width * bytesPerPixel;
	m_slicePitch = m_height * m_linePitch;

	assert(m_ptrUint8);

	for (KCL::uint32 row = 0; row < m_height; row++)
	{
		KCL::uint32 i = flip ? (m_height - 1 - row) : row;
		KCL::uint32 j;

		for (j = 0; j < m_linePitch; j += bytesPerPixel)
		{
			switch (m_format)
			{
			case Image_LUMINANCE_L8:
			case Image_ALPHA_A8:
				fread (&m_ptrUint8[i*m_width+j], 1, 1, fp);
				break;

			case Image_RGB888:
				{
					KCL::uint32 rgb = 0;
					fread (&rgb, 3, 1, fp);
					m_ptrUint8[i*m_width*3+j] = (rgb>>16) & 0xff;
					m_ptrUint8[i*m_width*3+j+1] = (rgb>>8) & 0xff;
					m_ptrUint8[i*m_width*3+j+2] = rgb & 0xff;
					break;
				}

			case Image_RGBA8888:
				{
					KCL::uint32 rgba = 0;
					fread (&rgba, 4, 1, fp);
					m_ptrUint8[i*m_width*4+j] = (rgba>>16) & 0xff;
					m_ptrUint8[i*m_width*4+j+1] = (rgba>>8) & 0xff;
					m_ptrUint8[i*m_width*4+j+2] = rgba & 0xff;
					m_ptrUint8[i*m_width*4+j+3] = (rgba>>24) & 0xff;
					break;
				}

			default:
				break;
			}
		}

		if (iHdr.bitCount == 8 || iHdr.bitCount == 24)
		{
			int rowRemaining = (4 - (j & 3)) & 3;
			fseek (fp, rowRemaining, SEEK_CUR);
		}
	}

	fclose (fp);
	return true;
}

bool KCL::Image::loadASTC (const char *filename)
{
	size_t datasize = -1;
	int x;
	int y;
	int mipmaps;
	uint8* output_image = load_astc_file(filename, datasize, x, y, mipmaps);
	if(output_image==NULL)
	{
		return false;
	}

	m_mipcount = mipmaps;
	m_format = Image_RGBA_ASTC_8x8;
	m_width = x;
	m_height = y;
	m_size = datasize;
	m_ptrUint8 = output_image;
	return true;
}

bool KCL::Image::loadPvr2(const char *filename)
{
	AssetFile file(filename);

	if(file.GetLastError())
	{
		return false;
	}

	PVRHeader header;
	file.Read(&header.headSize, sizeof(unsigned int), 1);
	file.Read(&header.height, sizeof(unsigned int), 1);
	file.Read(&header.width, sizeof(unsigned int), 1);
	file.Read(&header.mipLevels, sizeof(unsigned int), 1);
	file.Read(&header.imageType, sizeof(unsigned int), 1);
	file.Read(&header.dataSize, sizeof(unsigned int), 1);
	file.Read(&header.bitsPPixel, sizeof(unsigned int), 1);
	file.Read(&header.redMask, sizeof(unsigned int), 1);
	file.Read(&header.greenMask, sizeof(unsigned int), 1);
	file.Read(&header.blueMask, sizeof(unsigned int), 1);
	file.Read(&header.alphaMask, sizeof(unsigned int), 1);
	file.Read(&header.PVRid, sizeof(unsigned int), 1);
	file.Read(&header.numOfSurfaces, sizeof(unsigned int), 1);

	//little/big-Endian test
	if(IsBigEndian())
	{
		header.headSize = convertUInt( header.headSize);
		header.height = convertUInt( header.height);
		header.width = convertUInt( header.width);
		header.mipLevels = convertUInt( header.mipLevels);
		header.imageType = convertUInt( header.imageType);
		header.dataSize = convertUInt( header.dataSize);
		header.bitsPPixel = convertUInt( header.bitsPPixel);
		header.redMask = convertUInt( header.redMask);
		header.greenMask = convertUInt( header.greenMask);
		header.blueMask = convertUInt( header.blueMask);
		header.alphaMask = convertUInt( header.alphaMask);
		header.PVRid = convertUInt( header.PVRid);
		header.numOfSurfaces = convertUInt( header.numOfSurfaces);
	}
	if(header.PVRid != 0x21525650)
	{
		return loadPvr3( filename);
	}

	m_height = header.height;
	m_width  = header.width;

	switch (header.imageType & 0xFF)
	{
		case 16:
			m_format=Image_RGBA4444;
			break;
		case 17:
			m_format=Image_RGBA5551;
			break;
		case 5:
		case 18:
			m_format=Image_RGBA8888;
			break;
		case 2:
		case 19:
			m_format=Image_RGB565;
			break;
		case 4:
		case 21:
			m_format=Image_RGB888;
			break;
		case 22:
			m_format = Image_LUMINANCE_L8;
			break;
		case 8:
		case 23:
			m_format = Image_LUMINANCE_ALPHA_LA88;
		break;
		case 24:
			m_format=Image_PVRTC2;
			break;
		case 13:
		case 25:
			m_format=Image_PVRTC4;
			break;
		case 32:
			if (header.imageType & 0x8000)
			{
				m_format = Image_DXT1_RGBA;
			}
			else
			{
				m_format = Image_DXT1;
			}
			break;
		case 33:
			m_format=Image_DXT2;
			break;
		case 34:
			m_format=Image_DXT3;
			break;
		case 35:
			m_format=Image_DXT4;
			break;
		case 36:
			m_format=Image_DXT5;
			break;
		case 54:
			m_format=Image_ETC1;
			break;
		default:
			m_format=ImageTypeAny;
			break;
	}

	m_size = header.dataSize;

    if (header.imageType & 0x100) // if has mipmaps
	{
        m_mipcount = header.mipLevels;
	}

	switch (m_format)
	{
	case Image_DXT1:
	case Image_DXT1_RGBA:
		m_linePitch = 2 * m_width;
		break;
	case Image_DXT5:
		m_linePitch = 4 * m_width;
		break;
	case Image_RGBA_ASTC_4x4:
	case Image_RGBA_ASTC_5x5:
	case Image_RGBA_ASTC_6x6:
	case Image_RGBA_ASTC_8x8:
		m_linePitch = 0;
		break;
	default:
		m_linePitch = (((header.bitsPPixel * m_width - 1) | 0x7) + 1) >> 3;
		break;
	}

	m_slicePitch = m_linePitch * m_height;
	m_ptrUint8 = new KCL::uint8[header.dataSize];
	file.Read(m_ptrUint8, 1, header.dataSize);


#if 0
	if( header.imageType & 0x200)
	{
		KCL::uint16 *src = (KCL::uint16*)m_data;
		KCL::uint16 *dst = new KCL::uint16[header.dataSize];
		detwiddle( 0, 0, m_width, m_width, dst, &src);

		delete m_data;
		m_data = (KCL::uint8*)dst;
	}
#endif

	return true;
}

bool KCL::Image::loadPvr3 (const char *filename)
{
	AssetFile file(filename);

	if(file.GetLastError())
	{
		return false;
	}

	PVRHeaderV3 header;

	file.Read( &header.m_version, sizeof(KCL::uint32), 1);
	file.Read( &header.m_flags, sizeof(KCL::uint32), 1);
	file.Read( &header.m_pixel_format, sizeof(KCL::uint64), 1);
	file.Read( &header.m_color_space, sizeof(KCL::uint32), 1);
	file.Read( &header.m_channel_type, sizeof(KCL::uint32), 1);
	file.Read( &header.m_height, sizeof(KCL::uint32), 1);
	file.Read( &header.m_width, sizeof(KCL::uint32), 1);
	file.Read( &header.m_depth, sizeof(KCL::uint32), 1);
	file.Read( &header.m_num_surfaces, sizeof(KCL::uint32), 1);
	file.Read( &header.m_num_faces, sizeof(KCL::uint32), 1);
	file.Read( &header.m_num_mipmaps, sizeof(KCL::uint32), 1);
	file.Read( &header.m_metadata_size, sizeof(KCL::uint32), 1);

	if( header.m_version != PVRTEX3_IDENT)
	{
		return false;
	}

	m_height = header.m_height;
	m_width  = header.m_width;

    m_sRGB_values = header.m_color_space;

	uint64 pixel_format = header.m_pixel_format;
	uint64 pixel_format_hi = pixel_format & PVRTEX_PFHIGHMASK;

        //NOTE: If the most significant 4 bytes contain a value, the full 8 bytes are used to determine the pixel format.
        //  The least significant 4 bytes contain the channel order, each byte containing a single character, or a null character
        //  if there are fewer than four channels, e.g., {r, g, b, a} or {r, g, b, \0}.
        //  The most significant 4 bytes state the bit rate for each channel in the same order, each byte containing a single 8bit
        //  unsigned integer value, or zero if there are fewer than four channels, e.g., {8, 8, 8, 8} or {5, 6, 5, 0}.

	m_linePitch = 0;
	if( pixel_format_hi == 0)
	{
		switch( pixel_format)
		{
		case PVR3_PIXEL_FORMAT_R9G9B9E5:
			{
				m_format = Image_RGB9E5;
				m_linePitch = 4 * m_width;
				break;
			}
		case PVR3_PIXEL_FORMAT_ETC1:
			{
				m_format=Image_ETC1;
				m_linePitch = 2 * m_width;
				break;
			}
		case PVR3_PIXEL_FORMAT_ETC2_RGB:
			{
				m_format=Image_ETC2_RGB;
				m_linePitch = 2 * m_width;
				break;
			}
		case PVR3_PIXEL_FORMAT_ETC2_RGBA:
			{
				m_format=Image_ETC2_RGBA8888;
				m_linePitch = 4 * m_width;
				break;
			}
		case PVR3_PIXEL_FORMAT_ETC2_RGB_A1:
		{
			m_format = Image_ETC2_RGB_A1;
			m_linePitch = 4 * m_width;
			break;
		}
		case PVR3_PIXEL_FORMAT_DXT1:
			{
				m_format=Image_DXT1;
				m_linePitch = 2 * m_width;
				break;
			}
		case PVR3_PIXEL_FORMAT_DXT5:
			{
				m_format=Image_DXT5;
				m_linePitch = 4 * m_width;
				break;
			}
		case PVR3_PIXEL_FORMAT_RGBA_ASTC_4x4_BACKWARD_COMP:
		case PVR3_PIXEL_FORMAT_RGBA_ASTC_4x4:
			{
				m_format=Image_RGBA_ASTC_4x4;
				m_linePitch = 0;
				break;
			}
		case PVR3_PIXEL_FORMAT_RGBA_ASTC_5x5_BACKWARD_COMP:
		case PVR3_PIXEL_FORMAT_RGBA_ASTC_5x5:
			{
				m_format=Image_RGBA_ASTC_5x5;
				m_linePitch = 0;
				break;
			}
		//case PVR3_PIXEL_FORMAT_RGBA_ASTC_6x6_BACKWARD_COMP:
		case PVR3_PIXEL_FORMAT_RGBA_ASTC_6x6:
			{
				m_format=Image_RGBA_ASTC_6x6;
				m_linePitch = 0;
				break;
			}
		case PVR3_PIXEL_FORMAT_RGBA_ASTC_8x8_BACKWARD_COMP:
		case PVR3_PIXEL_FORMAT_RGBA_ASTC_8x8:
			{
				m_format=Image_RGBA_ASTC_8x8;
				m_linePitch = 0;
				break;
			}
		default:
			m_format=ImageTypeAny;
			m_linePitch = 0;
		}
	}
    else if(pixel_format == 0x0808080861626772) //support uncompressed PVR files
    {
        m_format=Image_RGBA8888;
    }

	m_slicePitch = m_linePitch * m_height;
	m_mipcount = header.m_num_mipmaps - 1;

	m_size = file.GetLength() - PVRTEX3_HEADERSIZE - header.m_metadata_size;
	m_ptrUint8 = new KCL::uint8[m_size];

	// Read the image
	file.Seek( header.m_metadata_size, SEEK_CUR);
	file.Read( m_ptrUint8, 1, m_size);

	return true;
}

bool KCL::Image::loadPng(const char *filename)
{
	KCL::AssetFile file(filename);
	if(file.GetLastError())
	{
		return false;
	}

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	unsigned int i;
	unsigned char sig[8];
	int  bit_depth, color_type;
	png_uint_32  width, height;
	size_t  rowbytes;
	png_bytepp  row_pointers = NULL;

	file.Read(sig, sizeof(char), 8);
	if (!png_check_sig(sig, 8))
	{
		return false;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
	{
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);

	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return false;
	}
	png_set_option(png_ptr, PNG_SKIP_sRGB_CHECK_PROFILE, PNG_OPTION_OFF);

	png_set_read_fn(png_ptr, &file, assetRead);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);


	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_expand(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_expand(png_ptr);
	if (bit_depth == 16)
		png_set_strip_16(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	KCL::uint8 channels = png_get_channels( png_ptr, info_ptr);
	size_t bytesPerPixel;

	switch (channels) {
	case 1:
		m_format = Image_LUMINANCE_L8;
		bytesPerPixel = 1;
		break;
	case 2:
		m_format = Image_LUMINANCE_ALPHA_LA88;
		bytesPerPixel = 2;
		break;
	case 3:
		m_format = Image_RGB888;
		bytesPerPixel = 3;
		break;
	case 4:
		m_format = Image_RGBA8888;
		bytesPerPixel = 4;
		break;
	}

	m_width = width;
	m_height = height;
	m_linePitch = rowbytes;
	m_size = m_slicePitch = m_linePitch * m_height;
	m_ptrUint8 = new KCL::uint8[m_size];

	if ((row_pointers = (png_bytepp) new png_bytep[height]) == NULL) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		delete [] m_ptrUint8;
		m_ptrUint8 = NULL;
		return false;
	}


	for (i = 0;  i < height;  ++i)
		row_pointers[i] = m_ptrUint8 + i*rowbytes;

	png_read_image(png_ptr, row_pointers);

	delete [] row_pointers;

	row_pointers = NULL;

	png_read_end (png_ptr, NULL);

	if (png_ptr && info_ptr) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		png_ptr = NULL;
		info_ptr = NULL;
	}

	return true;
}


// TODO: floating point image support
bool Image::loadFloat3Raw(const char *filename)
{
	AssetFile file(filename);

	if(file.GetLastError())
	{
		return false;
	}

	m_width  = 1024;
	m_height = 512;

	m_linePitch = m_width*3*4;
	m_slicePitch = m_linePitch * m_height;

	m_mipcount = 0;
	m_size = m_width*m_height*3*4;

	m_ptrUint8 = new KCL::uint8[m_size];
	file.Read( m_ptrUint8, 1, m_size);

	m_format = Image_RGB_32F;

	return true;
}


bool KCL::Image::loadHdr(const char *file)
{
	bool loaded = false;
	HDRLoaderResult result;
	for (KCL::uint32 i = 0; i < File::GetScenePathCount(); i++)
	{
		std::string path = File::GetScenePath(i) + file;
		if (HDRLoader::load(path.c_str(), result))
		{
			loaded = true;
			break;
		}
	}

	if (!loaded)
	{
		return false;
	}

	m_width = result.width;
	m_height = result.height;

	m_linePitch = result.width * sizeof(float) * 4;
	m_slicePitch = m_linePitch * m_height;

	m_mipcount = 0;
	m_size = m_slicePitch;

	m_format = Image_RGBA_32F;

	m_ptrUint8 = (KCL::uint8*)result.cols;

	return true;
}


void KCL::Image::releaseData()
{
	if (m_ptrRawData)
	{
		delete[] (char*)m_ptrRawData;
		m_ptrRawData = NULL;
	}
	m_width = 0;
	m_height = 0;
}


KCL::Image* KCL::Image::cloneTo(KCL::ImageFormat target) const
{
    Image* img = new Image(*this, false);

	switch (m_format)
	{
	case KCL::Image_ETC1:
		{
			switch (target)
			{
			case KCL::Image_RGB565:
				decodeETC1toRGB565(img);
				break;

			case KCL::Image_RGB888:
				decodeETC1toRGB888(img);
				break;

			case KCL::Image_RGBA8888:
				decodeETC1toRGBA8888(img);
				break;

			default:
				assert(0);
				abort();
				break;
			}

			break;
		}

	case KCL::Image_RGBA_ASTC_8x8:
		{
			switch (target)
			{
			case KCL::Image_RGB888:
				decodeASTCtoRGB888(img);
				break;

			default:
				assert(0);
				abort();
				break;
			}

			break;
		}

	case KCL::Image_RGB888:
		{
			switch (target)
			{
			case KCL::Image_RGBA8888:
				decodeRGB888toRGBA8888(img);
				break;

			default:
				assert(0);
				abort();
				break;
			}

			break;
		}

	default:
        assert(0);
        abort();
		break;
	}

    img->setupMIPminXminY();
    return img;
}

bool KCL::Image::decodeASTCtoRGB888(KCL::Image* target) const
{
#ifdef DEFINE_ASTC_UNCOMPRESS
	int x, y, z;
	astc_header hdr;

	hdr.blockdim_x = 8;
	hdr.blockdim_y = 8;
	hdr.blockdim_z = 1;

	astc_decode_mode decode_mode = DECODE_HDR;
	swizzlepattern swz_decode = { 0, 1, 2, 3 };

	int bitness = 8;
	int xdim = hdr.blockdim_x;
	int ydim = hdr.blockdim_y;
	int zdim = hdr.blockdim_z;

	int xsize = m_width;
	int ysize = m_height;
	int zsize = 1;

	int xblocks = (xsize + xdim - 1) / xdim;
	int yblocks = (ysize + ydim - 1) / ydim;
	int zblocks = (zsize + zdim - 1) / zdim;


	build_quantization_mode_table();
	uint8* buffer = m_data;

	astc_codec_image *img = allocate_image(bitness, xsize, ysize, zsize, 0);
	initialize_image(img);

	imageblock pb;
	for (z = 0; z < zblocks; z++)
	{
		for (y = 0; y < yblocks; y++)
		{
			for (x = 0; x < xblocks; x++)
			{
				int offset = (((z * yblocks + y) * xblocks) + x) * 16;
				uint8_t *bp = buffer + offset;
				physical_compressed_block pcb = *(physical_compressed_block *) bp;
				symbolic_compressed_block scb;
				physical_to_symbolic(xdim, ydim, zdim, pcb, &scb);
				decompress_symbolic_block(decode_mode, xdim, ydim, zdim, x * xdim, y * ydim, z * zdim, &scb, &pb);
				write_imageblock(img, &pb, xdim, ydim, zdim, x * xdim, y * ydim, z * zdim, swz_decode);
			}
		}
	}
	free(buffer);



	int channel = determine_image_channels(img);

	target->m_width  = img->xsize;
	target->m_height = img->ysize;

	if(channel==1)
	{
		m_format = Image_ALPHA_A8;//TODO fix this?
	}
	else if(channel==3)
	{
		m_format = Image_RGB888;
	}
	else if(channel==4)
	{
		m_format = Image_RGBA8888;
	}

    KCL::uint8 *row_pointers8 = new uint8_t[target->m_width * target->m_height * channel];

	for (unsigned int y = 0; y < target->m_height; y++)
	{
		switch (channel)
		{
		case 1:
			for (unsigned int x = 0; x < target->m_width; x++)
			{
				row_pointers8[(target->m_width * channel * (target->m_height - 1 - y)) + x * channel] = img->imagedata8[0][y][4 * x];
			}
			break;
		case 2:
			for (unsigned int x = 0; x < target->m_width; x++)
			{
				row_pointers8[(target->m_width * channel * (target->m_height - 1 - y)) + x * channel + 1] = img->imagedata8[0][y][4 * x + 1];
				row_pointers8[(target->m_width * channel * (target->m_height - 1 - y)) + x * channel + 0] = img->imagedata8[0][y][4 * x + 0];
			}
			break;
		case 3:
			for (unsigned int x = 0; x < target->m_width; x++)
			{
				row_pointers8[(target->m_width * channel * (target->m_height - 1 - y)) + x * channel + 2] = img->imagedata8[0][y][4 * x + 2];
				row_pointers8[(target->m_width * channel * (target->m_height - 1 - y)) + x * channel + 1] = img->imagedata8[0][y][4 * x + 1];
				row_pointers8[(target->m_width * channel * (target->m_height - 1 - y)) + x * channel + 0] = img->imagedata8[0][y][4 * x + 0];
			}
			break;
		case 4:
			for (unsigned int x = 0; x < target->m_width; x++)
			{
				row_pointers8[(target->m_width * channel * (target->m_height - 1 -y)) + x * channel + 3] = img->imagedata8[0][y][4 * x + 3];
				row_pointers8[(target->m_width * channel * (target->m_height - 1 -y)) + x * channel + 2] = img->imagedata8[0][y][4 * x + 2];
				row_pointers8[(target->m_width * channel * (target->m_height - 1 -y)) + x * channel + 1] = img->imagedata8[0][y][4 * x + 1];
				row_pointers8[(target->m_width * channel * (target->m_height - 1 -y)) + x * channel + 0] = img->imagedata8[0][y][4 * x + 0];
			}
			break;
		}
	}

	destroy_image(img);
	target->m_mipcount = 0;
    target->m_ptrUint8 = row_pointers8;
#endif

    return false;
}

bool KCL::Image::decodeETC1toRGB888(KCL::Image* target) const
{
	if (m_format != Image_ETC1)
	{
		return false;
	}

	size_t newLinePitch = 3 * m_width;
	size_t newSlicePitch = newLinePitch * m_height;
	size_t newSize = newSlicePitch * m_depth;

    target->m_ptrUint8 = new KCL::uint8[newSize];

	KCL::uint8* dst = target->m_ptrUint8;
	KCL::uint8* src = m_ptrUint8;

	for (KCL::uint32 i = 0; i < m_depth; i++, src += m_slicePitch, dst += newSlicePitch)
	{
		DecodeETC1toRGB888(m_width, m_height, src, dst);
	}

	target->m_linePitch = newLinePitch;
	target->m_slicePitch = newSlicePitch;
	target->m_size = newSize;
	target->m_format = Image_RGB888;
    target->m_mipcount = 0;

	return true;
}

bool KCL::Image::decodeETC1toRGBA8888(KCL::Image* target) const
{
	if (m_format != Image_ETC1)
	{
		return false;
	}

	size_t newLinePitch = 4 * m_width;
	size_t newSlicePitch = newLinePitch * m_height;
	size_t newSize = newSlicePitch * m_depth;

    target->m_ptrUint8 = new KCL::uint8[newSize];

	KCL::uint8* dst = target->m_ptrUint8;
	KCL::uint8* src = m_ptrUint8;

	for (KCL::uint32 i = 0; i < m_depth; i++, src += m_slicePitch, dst += newSlicePitch)
	{
		DecodeETC1toRGBA8888(m_width, m_height, src, dst);
	}

	target->m_linePitch = newLinePitch;
	target->m_slicePitch = newSlicePitch;
	target->m_size = newSize;
	target->m_format = Image_RGBA8888;
    target->m_mipcount = 0;

	return true;
}

bool KCL::Image::decodeETC1toRGB565(KCL::Image* target) const
{
	if (m_format != Image_ETC1)
	{
		return false;
	}

	size_t newLinePitch = 2 * m_width;
	size_t newSlicePitch = newLinePitch * m_height;
	size_t newSize = newSlicePitch * m_depth;

    target->m_ptrUint8 = new KCL::uint8[newSize];

	KCL::uint8* dst = target->m_ptrUint8;
	KCL::uint8* src = m_ptrUint8;
	for (KCL::uint32 i = 0; i < m_depth; i++, src += m_slicePitch, dst += newSlicePitch)
	{
		DecodeETC1toRGB565(m_width, m_height, src, (KCL::uint16*)dst);
	}

	target->m_linePitch = newLinePitch;
	target->m_slicePitch = newSlicePitch;
	target->m_size = newSize;
	target->m_format = Image_RGB565;
    target->m_mipcount = 0;

	return true;
}

bool KCL::Image::decodeRGB888toRGBA8888 (KCL::Image* target) const
{
	if (m_format ==Image_RGB888 )
	{
        size_t newLinePitch = 4 * m_width;
	    size_t newSlicePitch = newLinePitch * m_height;
        size_t newSize = newSlicePitch * m_depth;

		target->m_ptrUint8 = new KCL::uint8[newSize];

	    KCL::uint8* dst = target->m_ptrUint8;
	    KCL::uint8* src = m_ptrUint8;

	    for (KCL::uint32 depthIdx = 0; depthIdx < m_depth; depthIdx++, src += m_slicePitch, dst += newSlicePitch)
	    {
            for(unsigned int i = 0;i < m_width * m_height;i++)
		    {
			    dst[i*4]   = src[i*3];
			    dst[i*4+1] = src[i*3+1];
			    dst[i*4+2] = src[i*3+2];
			    dst[i*4+3] = 255;
		    }
	    }

        target->m_size = newSize;
		target->m_mipcount = 0;
		target->m_format = Image_RGBA8888;
        target->m_linePitch = newLinePitch;
	    target->m_slicePitch = newSlicePitch;
	}
	if (m_format == Image_LUMINANCE_L8 )
	{
        size_t newLinePitch = 4 * m_width;
	    size_t newSlicePitch = newLinePitch * m_height;
        size_t newSize = newSlicePitch * m_depth;

        target->m_ptrUint8 = new KCL::uint8[newSize];

        KCL::uint8* dst = target->m_ptrUint8;
	    KCL::uint8* src = m_ptrUint8;

	    for (KCL::uint32 depthIdx = 0; depthIdx < m_depth; depthIdx++, src += m_slicePitch, dst += newSlicePitch)
	    {
            for(unsigned int i = 0;i < m_width * m_height;i++)
		    {
			    dst[i*4]   = src[i];
			    dst[i*4+1] = src[i];
			    dst[i*4+2] = src[i];
			    dst[i*4+3] = 255;
		    }
	    }

        target->m_size = newSize;
		target->m_mipcount = 0;
		target->m_format = Image_RGBA8888;
        target->m_linePitch = newLinePitch;
	    target->m_slicePitch = newSlicePitch;
	}

    return true;
}

typedef struct {
#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned int negative : 1;
	unsigned int biasedexponent : 8;
	unsigned int mantissa : 23;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int mantissa : 23;
	unsigned int biasedexponent : 8;
	unsigned int negative : 1;
#endif
#endif
} BitsOfIEEE754;

typedef union {
	unsigned int raw;
	float value;
	BitsOfIEEE754 field;
} float754;

typedef struct {
#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
	unsigned int biasedexponent : RGB9E5_EXPONENT_BITS;
	unsigned int b : RGB9E5_MANTISSA_BITS;
	unsigned int g : RGB9E5_MANTISSA_BITS;
	unsigned int r : RGB9E5_MANTISSA_BITS;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int r : RGB9E5_MANTISSA_BITS;
	unsigned int g : RGB9E5_MANTISSA_BITS;
	unsigned int b : RGB9E5_MANTISSA_BITS;
	unsigned int biasedexponent : RGB9E5_EXPONENT_BITS;
#endif
#endif
} BitsOfRGB9E5;

typedef union {
	unsigned int raw;
	BitsOfRGB9E5 field;
} rgb9e5;

static float ClampRange_for_rgb9e5(float x)
{
	if (x > 0.0) {
		if (x >= MAX_RGB9E5) {
			return MAX_RGB9E5;
		}
		else {
			return x;
		}
	}
	else {
		/* NaN gets here too since comparisons with NaN always fail! */
		return 0.0;
	}
}

static float MaxOf3(float x, float y, float z)
{
	if (x > y) {
		if (x > z) {
			return x;
		}
		else {
			return z;
		}
	}
	else {
		if (y > z) {
			return y;
		}
		else {
			return z;
		}
	}
}

/* Ok, FloorLog2 is not correct for the denorm and zero values, but we
are going to do a max of this value with the minimum rgb9e5 exponent
that will hide these problem cases. */
static int FloorLog2(float x)
{
	float754 f;

	f.value = x;
	return (f.field.biasedexponent - 127);
}

static int Max(int x, int y)
{
	if (x > y) {
		return x;
	}
	else {
		return y;
	}
}

static rgb9e5 float3_to_rgb9e5(const float rgb[3])
{
	rgb9e5 retval;
	float maxrgb;
	int rm, gm, bm;
	float rc, gc, bc;
	int exp_shared;
	double denom;

	rc = ClampRange_for_rgb9e5(rgb[0]);
	gc = ClampRange_for_rgb9e5(rgb[1]);
	bc = ClampRange_for_rgb9e5(rgb[2]);

	maxrgb = MaxOf3(rc, gc, bc);
	exp_shared = Max(-RGB9E5_EXP_BIAS - 1, FloorLog2(maxrgb)) + 1 + RGB9E5_EXP_BIAS;
	assert(exp_shared <= RGB9E5_MAX_VALID_BIASED_EXP);
	assert(exp_shared >= 0);
	/* This pow function could be replaced by a table. */
	denom = pow(2.0, exp_shared - RGB9E5_EXP_BIAS - RGB9E5_MANTISSA_BITS);

	int maxm = (int)floor(maxrgb / denom + 0.5);
	if (maxm == MAX_RGB9E5_MANTISSA + 1) {
		denom *= 2;
		exp_shared += 1;
		assert(exp_shared <= RGB9E5_MAX_VALID_BIASED_EXP);
	}
	else {
		assert(maxm <= MAX_RGB9E5_MANTISSA);
	}

	rm = (int)floor(rc / denom + 0.5);
	gm = (int)floor(gc / denom + 0.5);
	bm = (int)floor(bc / denom + 0.5);

	assert(rm <= MAX_RGB9E5_MANTISSA);
	assert(gm <= MAX_RGB9E5_MANTISSA);
	assert(bm <= MAX_RGB9E5_MANTISSA);
	assert(rm >= 0);
	assert(gm >= 0);
	assert(bm >= 0);

	retval.field.r = rm;
	retval.field.g = gm;
	retval.field.b = bm;
	retval.field.biasedexponent = exp_shared;

	return retval;
}

static void rgbe2float(float rgb[3], unsigned char rgbe[4])
{
	float f;

	if (rgbe[3])
	{
		f = ldexp(1.0, rgbe[3] - (int)(128 + 8));
		rgb[0] = rgbe[0] * f;
		rgb[1] = rgbe[1] * f;
		rgb[2] = rgbe[2] * f;
	}
	else
	{
		rgb[0] = rgb[1] = rgb[2] = 0.0;
	}
}

static void float2rgbe(unsigned char rgbe[4], float red, float green, float blue)
{
	float v;
	int e;

	v = red;
	if (green > v) v = green;
	if (blue > v) v = blue;
	if (v < 1e-32) {
		rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
	}
	else {
		v = frexp(v, &e) * 256.0 / v;
		rgbe[0] = (unsigned char)(red * v);
		rgbe[1] = (unsigned char)(green * v);
		rgbe[2] = (unsigned char)(blue * v);
		rgbe[3] = (unsigned char)(e + 128);
	}
}

static void convert_image_rgb_to_rgb9e5(const KCL::Image* img)
{
	unsigned char *data = (unsigned char*)img->getData();

	for (int i = 0; i < img->getWidth() * img->getHeight(); i++)
	{
		float pix[3];
		rgbe2float(pix, data + i * 4);
		rgb9e5 p = float3_to_rgb9e5(pix);
		((unsigned int*)data)[i] = p.raw;
	}
}

static void convert_image_rgb9e5_to_rgb(const KCL::Image* img)
{
	unsigned char *data = (unsigned char*)img->getData();

	for (int i = 0; i < img->getWidth() * img->getHeight(); i++)
	{
		unsigned char rgbe[4];
		unsigned char *rgb = data + i * 4;
		float2rgbe(rgbe, (float)rgb[0], (float)rgb[1], (float)rgb[2]);
		rgb[0] = rgbe[0];
		rgb[1] = rgbe[1];
		rgb[2] = rgbe[2];

	}
}

bool KCL::Image::decodeRGBA8888toRGB9E5()
{
	if (getFormat() == KCL::Image_RGBA8888)
	{
		m_format = KCL::Image_RGB9E5;
		convert_image_rgb_to_rgb9e5(this);
		
		return true;
	}

	return false;
}

bool KCL::Image::decodeRGB9E5toRGB888()
{
	if (getFormat() == KCL::Image_RGB9E5)
	{
		m_format = KCL::Image_RGB888;
		convert_image_rgb9e5_to_rgb(this);
		return true;
	}

	return false;
}


KCL::uint32 KCL::Image::Float3_To_RGB9E5(const float rgb[3])
{
	return float3_to_rgb9e5(rgb).raw;
}


KCL::uint8* KCL::Image::getMipmapData (KCL::uint32 level) const
{
    if (!m_mipcount) return 0;
	if (m_width != m_height) return m_ptrUint8;	// TODO:

	int offset = 0;
	int bpp = getBpp ()/8;

	for (KCL::uint32 i = 0; i < level; ++i)
	{
		offset += 1<<((m_mipcount-i-1)*2);
	}
	offset *= bpp;

	return m_ptrUint8 + offset;
}

void KCL::Image::setupMIPminXminY()
{
	const FormatDescription *fd = FormatDescriptions;
	while (fd->format != ImageTypeAny)
	{
		if ((unsigned)fd->format == m_format)
		{
			m_minx = fd->minx;
            m_miny = fd->miny;
			return;
		}
		++fd;
	}
}

void KCL::Image::getMipmapData (KCL::uint32 level, KCL::uint8 **data, KCL::uint32 *mipmapsize, KCL::uint32 *mipWidth, KCL::uint32 *mipHeight, KCL::uint32* mempitch) const
{
	if (!m_mipcount)
	{
		KCL::uint32 mipx = m_width;		/// calculate mipsize
		KCL::uint32 mipy = m_height;
		mipx = mipx > m_minx ? mipx : m_minx;
		mipy = mipy > m_miny ? mipy : m_miny;

		*data = m_ptrUint8;
		if(mipmapsize)
		{
			*mipmapsize = mipx*mipy*getBpp ()/8;
		}
		return;
	}

	int offset = 0;
	int bpp = getBpp ();

	KCL::uint32 mipx = m_width;		/// calculate mipsize
	KCL::uint32 mipy = m_height;
	KCL::uint32 mipx2 = m_width;		/// calculate image size
	KCL::uint32 mipy2 = m_height;

	for (KCL::uint32 i = 0; i <= level; ++i)
	{
		mipx = max(mipx, m_minx);
		mipy = max(mipy, m_miny);

		if (mipWidth) *mipWidth = mipx2>1?mipx2:1;
		if (mipHeight) *mipHeight = mipy2>1?mipy2:1;


		int size = mipx*mipy;
		size *= bpp;
		size /= 8;

		if(mipmapsize)
		{
			*mipmapsize = size;
		}

		if( mempitch)
		{
			*mempitch = size / mipy;
		}

		if (i < level)
			offset += size;

		mipx >>= 1;
		mipy >>= 1;
		mipx2 >>= 1;
		mipy2 >>= 1;
	}
	if (m_ptrUint8 != 0)
	{
		*data = m_ptrUint8 + offset;
	}
	else
	{
		*data = 0;
	}
}


void flipBlock(KCL::uint8* data, KCL::uint32 blockCount, KCL::uint32 blockSize)
{
	KCL::uint8 *tmp = new KCL::uint8[blockSize];
	KCL::uint8 *src=data;
	KCL::uint8 *dst=data+(blockCount-1)*blockSize;
	while (src<dst)
	{
		memcpy(tmp,src,blockSize);
		memcpy(src,dst,blockSize);
		memcpy(dst,tmp,blockSize);
		src+=blockSize;
		dst-=blockSize;
	}
	delete[] tmp;
}

bool KCL::Image::flipX()
{
	switch (m_type)
	{
		case Image_1D:
		case Image_2D:
		case Image_3D:
		case Image_Array:
			break;
		default:
			return false;
	}

	if (!isUncompressed()) return false;

	for (KCL::uint32 i=0;i<m_depth;++i)
	{
		for (KCL::uint32 j=0;j<m_height;++j)
		{
			::flipBlock((KCL::uint8*)getData(i)+j*m_linePitch,m_width,getBpp()/8);
		}
	}
	return true;
}

bool KCL::Image::flipY()
{
	switch (m_type)
	{
		case Image_2D:
		case Image_3D:
		case Image_Array:
			break;
		default:
			return false;
	}

	if (!isUncompressed()) return false;

	for (KCL::uint32 i = 0;i < m_depth;++i)
	{
		::flipBlock((KCL::uint8*)getData(i),m_height,(KCL::uint32)m_linePitch);
	}
	return true;
}

bool KCL::Image::flipZ()
{
	switch (m_type)
	{
		case Image_3D:
		case Image_Array:
			break;
		default:
			return false;
	}

	::flipBlock((KCL::uint8*)getData(0),m_depth, (KCL::uint32)m_slicePitch);

	return true;
}

bool KCL::Image::halfImage (uint8 (*func)(uint8,uint8,uint8,uint8))
{
	switch (m_type)
	{
		case Image_2D:
		case Image_Cube:
		case Image_Array:
			break;
		default:
			return false;
	}

	if (!isUncompressed()) return false;


	if (func==NULL) func = getAverageSuperPixel;

	KCL::uint8 blockSize = getBpp()>>3;
	KCL::uint32 newWidth = m_width >> 1;
	KCL::uint32 newHeight = m_height >> 1;
	size_t newLinePitch = (getBpp() * newWidth) >> 3;
	size_t newSlicePitch = newLinePitch * newHeight;
	size_t newSize = newSlicePitch * m_depth;
	KCL::uint8* newData = new KCL::uint8[newSize];

	KCL::uint8* dst = newData;
	KCL::uint8* src = m_ptrUint8;
	for (KCL::uint32 i = 0; i < m_depth; i++, src += m_slicePitch, dst += newSlicePitch)
	{
		for (KCL::uint32 y=0;y<m_height;++++y)
		{
			for (KCL::uint32 x=0;x<m_width;++++x)
			{
				KCL::uint32 DL=readColor(src + y*m_linePitch + x*blockSize, m_format);
				KCL::uint32 DR=readColor(src + y*m_linePitch + (x+1)*blockSize, m_format);
				KCL::uint32 UL=readColor(src + (y+1)*m_linePitch + x*blockSize, m_format);
				KCL::uint32 UR=readColor(src + (y+1)*m_linePitch + (x+1)*blockSize, m_format);
				KCL::uint32 newPix = 0;
				for (int i=0;i<4;i++)
				{
					newPix+=func(DL>>i*8 & 0xFF, DR>>i*8 & 0xFF, UL>>i*8 & 0xFF, UR>>i*8 & 0xFF)<<i*8;
				}
				writeColor(newPix,dst,m_format);
				dst+=blockSize;
			}
		}
	}

	delete[] m_ptrUint8;
	m_width = newWidth;
	m_height = newHeight;
	m_linePitch = newLinePitch;
	m_slicePitch = newSlicePitch;
	m_size = newSize;
	m_ptrUint8 = newData;
	return true;

}

bool KCL::Image::convertTo(ImageFormat newFormat)
{
	if (m_format == newFormat)
	{
		return true;
	}

	KCL::uint32 newBpp = ::getBpp(newFormat);
	size_t newLinePitch = (newBpp * m_width) >> 3;
	size_t newSlicePitch = newLinePitch * m_height;
	size_t newSize = newSlicePitch * m_depth;
	KCL::uint8* newData = new KCL::uint8[newSize];

	KCL::uint8* dst = newData;
	KCL::uint8* src = m_ptrUint8;
	for (KCL::uint32 i = 0; i < m_depth; ++i, src += m_slicePitch, dst += newSlicePitch)
	{
		bool result = ::convert(m_format, src, newFormat, dst, m_width, m_height);
		if (!result)
		{
			delete[] newData;
			return false;
		}
	}

	delete[] m_ptrUint8;
	m_linePitch = newLinePitch;
	m_slicePitch = newSlicePitch;
	m_size = newSize;
	m_ptrUint8 = newData;
	m_format = newFormat;

	return true;
}

// Required as callback
void kcl_img_png_error_callback(png_structp png_ptr, png_const_charp msg)
{
}

void* KCL::Image::getBgrData(int w, int h, const void* srcData, int srcLinePitch, ImageFormat srcFormat, int dstLinePitch)
{
	int dataSize = dstLinePitch > 0 ? dstLinePitch * h : -dstLinePitch * h;

	if (!srcLinePitch)
	{
		srcLinePitch = (::getBpp(srcFormat) * w) >> 3;
	}

	if (!dstLinePitch)
	{
		dstLinePitch = 3 * w;
	}

	KCL::uint8* result = new KCL::uint8[dataSize];
	KCL::uint8* srcStrideStart = (KCL::uint8*)srcData;
	KCL::uint8* dstStrideStart = dstLinePitch > 0?
		(KCL::uint8*)result :
		(KCL::uint8*)result + (1 - h) * dstLinePitch;

	switch (srcFormat)
	{
	case Image_BGR888:
		for (int y = 0; y < h; y++, srcStrideStart += srcLinePitch, dstStrideStart += dstLinePitch)
		{
			memcpy(dstStrideStart, srcStrideStart, srcLinePitch);
		}

		break;

	case Image_RGB888:
		for (int y = 0; y < h; y++, srcStrideStart += srcLinePitch, dstStrideStart += dstLinePitch)
		{
			KCL::uint8* src = srcStrideStart;
			KCL::uint8* dst = dstStrideStart;
			for (int x = 0; x < w; x++, src += 3, dst += 3)
			{
				dst[0] = src[2];
				dst[1] = src[1];
				dst[2] = src[0];
			}
		}

		break;

	case Image_RGBA8888:
		for (int y = 0; y < h; y++, srcStrideStart += srcLinePitch, dstStrideStart += dstLinePitch)
		{
			KCL::uint8* src = srcStrideStart;
			KCL::uint8* dst = dstStrideStart;
			for (int x = 0; x < w; x++, src += 4, dst += 3)
			{
				dst[0] = src[2];
				dst[1] = src[1];
				dst[2] = src[0];
			}
		}

		break;

	default:
		// not very elegant, but results in smaller code than setting the pointers in each 'case' block
		delete[] result;
		result = NULL;
		break;
	}

	return result;
}

bool KCL::Image::savePng(const char *filename, int w, int h, const void* data, ImageFormat format, bool flip)
{
	png_structp png_ptr = 0;
	png_infop info_ptr = 0;
	FILE *file = 0;
	png_byte **ppbRowPointers = 0;

	if (!filename)
		return false;

	file = fopen(filename, "wb");
	if (!file)
	{
		INFO("couldn't write a %s", filename);
		return false;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)kcl_img_png_error_callback, (png_error_ptr)NULL);
	if (!png_ptr)
	{
		fclose(file);
		return false;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		fclose(file);
		png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
		return false;
	}

#ifdef PNG_STDIO_SUPPORTED
	png_init_io(png_ptr, file);
#else
	png_set_write_fn(png_ptr, (png_voidp)file, png_write_data, png_flush);
#endif

	int channels = 0;

	switch (format)
	{
	case Image_RGB888:
		channels = 3;
		png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		break;

	case Image_BGR888:
		channels = 3;
		png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		png_set_bgr(png_ptr);
		break;

	case Image_RGBA8888:
		channels = 4;
		png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
		break;

	default:
		fclose(file);
		return false;
	}

	png_write_info(png_ptr, info_ptr);

	png_uint_32 ulRowBytes = w * channels;

	ppbRowPointers = new png_bytep[h];
	if(!ppbRowPointers)
		assert(0);

	int linePitch = (((ulRowBytes + 3) >> 2) << 2);
	if (flip)
	{
		for (int i = 0; i < h; i++)
		{
			ppbRowPointers[h - 1 - i] = (png_bytep)data + i * linePitch;
		}
	}
	else
	{
		for (int i = 0; i < h; i++)
		{
			ppbRowPointers[i] = (png_bytep)data + i * linePitch;
		}
	}

	png_write_image (png_ptr, ppbRowPointers);
	png_write_end(png_ptr, info_ptr);
	delete[] ppbRowPointers;
	ppbRowPointers = NULL;

	png_destroy_write_struct(&png_ptr, (png_infopp) NULL);

	fclose (file);

	return true;
}

/*
bool KCL::Image::saveBmp(const char *filename, int w, int h, const void* data, ImageFormat format, bool flip)
{
#if (_WIN32_WINNT >= 0x0603)
	return false;
#else
	int srcLinePitch = w * ::getBpp(format) >> 3;
	int dstLinePitch = w * 3;
	DWORD pixelDataSize = h * dstLinePitch;
	KCL::uint8* bgrFlippedData = (KCL::uint8*)getBgrData(w, h, data, srcLinePitch, format, flip ? dstLinePitch : -dstLinePitch);
	if (!bgrFlippedData)
	{
		return false;
	}

	BITMAPFILEHEADER bfh;
	memset(&bfh, 0, sizeof(BITMAPFILEHEADER));
	bfh.bfType = 'B' | ('M' << 8);
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pixelDataSize;
	bfh.bfOffBits = sizeof bfh + sizeof(BITMAPINFOHEADER);

	BITMAPINFOHEADER bih;
	memset(&bih, 0, sizeof(BITMAPINFOHEADER));
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = w;
	bih.biHeight = h;
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bih.biCompression = BI_RGB;

	FILE* fp = fopen(filename, "wb+");
	if (!fp)
	{
		INFO("Failed to open file for write: %s", filename);
		delete bgrFlippedData;
		return false;
	}

	fwrite(&bfh, sizeof(bfh), 1, fp);
	fwrite(&bih, sizeof(bih), 1, fp);
	fwrite(bgrFlippedData, pixelDataSize, 1, fp);
	fclose(fp);

	delete bgrFlippedData;
	return true;
#endif
}
*/

bool KCL::Image::saveTga(const char *filename, int w, int h, const void* data, ImageFormat format, bool flip)
{
	int srcLinePitch = w * ::getBpp(format) >> 3;
	int dstLinePitch = w * 3;
	KCL::uint32 pixelDataSize = h * dstLinePitch;
	KCL::uint8* bgrFlippedData = (KCL::uint8*)getBgrData(w, h, data, srcLinePitch, format, flip ? dstLinePitch : -dstLinePitch);
	if (!bgrFlippedData)
	{
		return false;
	}

	TTgaHeader tgaHdr;
	memset(&tgaHdr, 0, sizeof(TTgaHeader));
	tgaHdr.imagetype = 2;
	tgaHdr.width = (unsigned short)w;
	tgaHdr.height = (unsigned short)h;
	tgaHdr.bits = 24;

	FILE* fp = fopen(filename, "wb+");
	if (!fp)
	{
		INFO("Failed to open file for write: %s", filename);
		delete bgrFlippedData;
		return false;
	}

	fwrite((unsigned char*)&tgaHdr, sizeof(TTgaHeader),1, fp);
	fwrite(bgrFlippedData, pixelDataSize, 1, fp);
	fclose(fp);

	delete bgrFlippedData;
	return true;
}


const std::string& KCL::Image::GetImageFilename() const
{
	return m_image_filename;
}


bool convert(ImageFormat srcFormat, KCL::uint8* src, ImageFormat dstFormat, KCL::uint8 *dst, int width, int height)
{
	int srcStep = getBpp(srcFormat) >> 3;
	int dstStep = getBpp(dstFormat) >> 3;
	int pixCount = width * height;

	for (int i = 0; i < pixCount; i++, src += srcStep, dst += dstStep)
	{
		KCL::uint32 argb = readColor(src, srcFormat);
		int bytesWritten = writeColor(argb, dst, dstFormat);
		if (bytesWritten != dstStep)
		{
			return false;
		}
	}

	return true;
}

// Reads color information from a memory address
KCL::uint32 readColor(KCL::uint8* address, ImageFormat format)
{
	KCL::uint32 r, g, b, a;
	switch (format)
	{
	// Grayscale and alpha images...
	case Image_ALPHA_A8:
		r =
		g =
		b = 0;
		a = address[0];
		break;

	case Image_LUMINANCE_L8:
		r =
		g =
		b = address[0];
		a = 255;
		break;

	case Image_LUMINANCE_ALPHA_LA88:
		r =
		g =
		b = address[0];
		a = address[1];
		break;

	// 16 bit formats...
	case Image_RGB565:
		{
			uint8 firstByte = address[0];
			uint8 secondByte = address[1];
			r = firstByte & 0xf8;	// Mask the high 5 bytes
			g = (firstByte <<  5) || ((secondByte & 0xc0) >> 3);		// Three bits from the first, two from the second (0xC = 1100b)
			b = (secondByte << 2) & 0xf8;
			a = 255;
			break;
		}

	case Image_RGBA5551:
		{
			uint8 firstByte = address[0];
			uint8 secondByte = address[1];
			r = firstByte & 0xf8;	// Mask the high 5 bits
			g = (firstByte <<  5) || ((secondByte & 0xe0) >> 3);		// Three bits from the first, three from the second (0xE = 1110b)
			b = (secondByte << 3) & 0xf8;
			a = secondByte << 7;
			break;
		}

	case Image_RGBA4444:
		{
			uint8 firstByte = address[0];
			uint8 secondByte = address[1];
			r = firstByte & 0xf0;
			g = firstByte << 4;
			b = secondByte & 0xf0;
			a = secondByte << 4;
			break;
		}

	// 24 bits
	case Image_RGB888:
		r = address[0];
		g = address[1];
		b = address[2];
		a = 255;
		break;

	case Image_BGR888:
		b = address[0];
		g = address[1];
		r = address[2];
		a = 255;
		break;


	// 32 bits
	case Image_RGBA8888:
		r = address[0];
		g = address[1];
		b = address[2];
		a = address[3];
		break;

	default:
		return 0;
	}

	return (r << 24) | (g << 16) | (b << 8) | a;
}

/**
	Writes a color to the specified address.
	@color[in]		color to write in RGBA8888 format (as returned by readColor())
	@address[in]	memory address to start writing from
	@format[in]		output pixel format
	@return			number of bytes written, -1 if an error occurred (like a not supported format)
*/
int writeColor(KCL::uint32 color, KCL::uint8* address, ImageFormat format)
{
	KCL::uint8 r = (color >> 24) & 0xff;
	KCL::uint8 g = (color >> 16) & 0xff;
	KCL::uint8 b = (color >> 8) & 0xff;
	KCL::uint8 a = color & 0xff;

	switch (format)
	{
	// Grayscale and alpha images...
	case Image_ALPHA_A8:
		address[0] = a;
		return 1;

	case Image_LUMINANCE_L8:
		{
			float l = 0.3f * r + 0.59f * g + 0.11f * b;
			address[0] = (KCL::uint8)l;
			return 1;
		}

	case Image_LUMINANCE_ALPHA_LA88:
		{
			float l = 0.3f * r + 0.59f * g + 0.11f * b;
			address[0] = (KCL::uint8)l;
			address[1] = a;
			return 2;
		}

	// 16 bit formats...
	case Image_RGBA4444:
		address[0] = (r & 0xf0) | (g >> 4);
		address[1] = (b & 0xf0) | (a >> 4);
		return 2;

	// 24 bits
	case Image_RGB888:
		address[0] = r;
		address[1] = g;
		address[2] = b;
		return 3;

	case Image_BGR888:
		address[0] = b;
		address[1] = g;
		address[2] = r;
		return 3;

	// 32 bits
	case Image_RGBA8888:
		address[0] = r;
		address[1] = g;
		address[2] = b;
		address[3] = a;
		return 4;

	default:
		return -1;
	}
}
