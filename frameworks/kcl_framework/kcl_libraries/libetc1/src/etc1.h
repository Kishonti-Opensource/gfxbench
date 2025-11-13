/** \file etc1.h
	Header file that contains declarations of supperting functions for ETC1 texture compression.
	ETC1 is not yet supported on all platforms.
*/
#ifndef ETC1_H
#define ETC1_H

#include "kcl_base.h"

#define ETC1 0x36		//ETC1
#define RGB888 0x04		//RGB888
#define RGBA8888 0x05	//RGBA8888

/// Header structure used in PVR files.
struct _PVRHeader
{
	unsigned int headSize;
	unsigned int height;
	unsigned int width;
	unsigned int mipLevels;
	unsigned int imageType;
	unsigned int dataSize;
	unsigned int bitsPPixel;
	unsigned int redMask;
	unsigned int greenMask;
	unsigned int blueMask;
	unsigned int alphaMask;
	unsigned int PVRid;
	unsigned int numOfSurfaces;
} typedef PVRHeader;


#define PVRTEX3_HEADERSIZE		52
#define PVRTEX3_IDENT			0x03525650
#define PVRTEX3_IDENT_REV		0x50565203
#define PVRTEX_PFHIGHMASK		uint64(0xffffffff00000000ull)

struct PVRHeaderV3
{
	KCL::uint32 m_version;
	KCL::uint32 m_flags;
	KCL::uint64 m_pixel_format;
	KCL::uint32 m_color_space;
	KCL::uint32 m_channel_type;
	KCL::uint32 m_height;
	KCL::uint32 m_width;
	KCL::uint32 m_depth;
	KCL::uint32 m_num_surfaces;
	KCL::uint32 m_num_faces;
	KCL::uint32 m_num_mipmaps;
	KCL::uint32 m_metadata_size;

	PVRHeaderV3() : m_version(PVRTEX3_IDENT), m_flags(0), m_pixel_format(-1), m_color_space(0),m_channel_type(0), m_height(1),m_width(1),m_depth(1), m_num_surfaces(1), m_num_faces(1), m_num_mipmaps(1), m_metadata_size(0)
	{
	}
};

#define PVR3_PIXEL_FORMAT_ETC1				6
#define PVR3_PIXEL_FORMAT_DXT1				7
#define PVR3_PIXEL_FORMAT_DXT5				11
#define PVR3_PIXEL_FORMAT_R9G9B9E5			19
#define PVR3_PIXEL_FORMAT_ETC2_RGB			22
#define PVR3_PIXEL_FORMAT_ETC2_RGBA			23
#define PVR3_PIXEL_FORMAT_ETC2_RGB_A1		24
#define PVR3_PIXEL_FORMAT_RGBA_ASTC_4x4		27
#define PVR3_PIXEL_FORMAT_RGBA_ASTC_5x5		29
#define PVR3_PIXEL_FORMAT_RGBA_ASTC_6x6		31
#define PVR3_PIXEL_FORMAT_RGBA_ASTC_8x8		34
#define PVR3_PIXEL_FORMAT_RGBA_ASTC_4x4_BACKWARD_COMP		0xFF0000
#define PVR3_PIXEL_FORMAT_RGBA_ASTC_5x5_BACKWARD_COMP		0xFF0002
#define PVR3_PIXEL_FORMAT_RGBA_ASTC_8x8_BACKWARD_COMP		0xFF0009

///Transforms an ETC1 compressed image into RGB values
void DecodeETC1toRGB888 (KCL::uint32 width, KCL::uint32 height, const KCL::uint8 *etc1, KCL::uint8 *rgb);

///Transforms an ETC1 compressed image into RGBA values (alpha set to constant 1)
void DecodeETC1toRGBA8888 (KCL::uint32 width, KCL::uint32 height, const KCL::uint8 *etc1, KCL::uint8 *rgba);

///Transforms an ETC1 compressed image into RGB565 values
void DecodeETC1toRGB565 (KCL::uint32 width, KCL::uint32 height, const KCL::uint8 *etc1, KCL::uint16 *rgb);

///Transforms an RGB image into ETC1 compressed form
void EncodeRGBtoETC (KCL::uint32 width, KCL::uint32 height, const KCL::uint8 *rgb, KCL::uint8 *etc);

#endif
