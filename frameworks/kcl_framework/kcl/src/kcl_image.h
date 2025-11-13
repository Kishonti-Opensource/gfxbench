/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file image.h
	Definition of GLB::Image2D.
*/
#ifndef KCL_IMAGE_H
#define KCL_IMAGE_H

#include <kcl_base.h>
#include <string>
#include <vector>
#include <limits.h>
#include <algorithm>

namespace KCL
{
	#define	Image_First_Paletted_Format 128

	#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
	#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
	#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
	#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
	#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
	#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
	#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
	#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
	#define GL_ETC1_RGB8_OES                  0x8D64

	#define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT 0x8C4C
	#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
	#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
	#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F

	#ifndef GL_COMPRESSED_RGB8_ETC2
	#define GL_COMPRESSED_RGB8_ETC2                          0x9274
	#endif
	#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
	#define GL_COMPRESSED_RGBA8_ETC2_EA						0x9278 
	#endif
	#ifndef GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
	#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC				0x9279
	#endif

	#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR                         0x93B0
	#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR                         0x93B1
	#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR                         0x93B2
	#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR                         0x93B3
	#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR                         0x93B4
	#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR                         0x93B5
	#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR                         0x93B6
	#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR                         0x93B7
	#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR                        0x93B8
	#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR                        0x93B9
	#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR                        0x93BA
	#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR                       0x93BB
	#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR                       0x93BC
	#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR                       0x93BD
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR                 0x93D0
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR                 0x93D1
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR                 0x93D2
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR                 0x93D3
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR                 0x93D4
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR                 0x93D5
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR                 0x93D6
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR                 0x93D7
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR                0x93D8
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR                0x93D9
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR                0x93DA
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR               0x93DB
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR               0x93DC
	#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR               0x93DD

	typedef enum _ImageFormat
	{
		ImageTypeAny = 0,
		POI_Entertainment,
		Image_DEPTH_16,
		Image_RGB_32F,
		Image_RGBA_32F,
		Image_RGB888,
		Image_BGR888,
		Image_RGBA8888,
		Image_RGB565,
		Image_RGBA5551,
		Image_RGBA4444,
		Image_RGBA_ASTC_4x4,
		Image_RGBA_ASTC_5x5,
		Image_RGBA_ASTC_6x6,
		Image_RGBA_ASTC_8x8,
		Image_RGB9E5,
		Image_ETC1,
		Image_ETC2_RGB,
		Image_ETC2_RGB_A1,
		Image_ETC2_RGBA8888,
		Image_DXT1,
		Image_DXT1_RGBA,
		Image_DXT2,
		Image_DXT3,
		Image_DXT4,
		Image_DXT5,
		Image_PVRTC2,
		Image_PVRTC4,
		Image_ATC_RGB,
		Image_ATC_RGBA_EA,
		Image_ATC_RGBA_IA,
		Image_FLXTC_RGB565,
		Image_FLXTC_RGBA4444,
		Image_LUMINANCE_L8,
		Image_ALPHA_A8,
		Image_LUMINANCE_ALPHA_LA88,
		Image_PAL4_RGB888 = Image_First_Paletted_Format,
		Image_PAL4_RGBA8888,
		Image_PAL4_RGB565,
		Image_PAL4_RGBA4444,
		Image_PAL4_RGBA5551,
		Image_PAL8_RGB888,
		Image_PAL8_RGBA8888,
		Image_PAL8_RGB565,
		Image_PAL8_RGBA4444,
		Image_PAL8_RGBA5551
	} ImageFormat;

	typedef enum _ImageType
	{
		Image_Unknown = 0,
		Image_1D = 1,
		Image_2D = 2,
		Image_3D = 3,
		Image_Cube = 4,
		Image_Array = 5,
		Image_CubeArray = 6
	} ImageType;

	typedef struct _FormatDescription
	{
		ImageFormat format;
		KCL::int32 minx;
		KCL::int32 miny;
		KCL::int32 bpp;
		KCL::int32 alphaBits;
		const char *name;
	} FormatDescription;

	class KCL_API Image
	{
	private:
		std::string m_name;
		size_t m_size;
		size_t m_linePitch;
		size_t m_slicePitch;
		ImageType m_type;
		ImageFormat m_format;
        bool m_sRGB_values;
		KCL::uint32 m_width;
		KCL::uint32 m_height;
		KCL::uint32 m_depth;
        KCL::uint32 m_mipcount;
        KCL::uint32 m_minx;
        KCL::uint32 m_miny;

		// Pixel data. Pointers to retrieve different data formats from the same memory address.
		union
		{
			// Pointers to 1D and 2D images. Use these if m_depth is zero
			void*			m_ptrRawData;
			KCL::uint8*		m_ptrUint8;
			KCL::uint16*	m_ptrUint16;
			KCL::uint32*	m_ptrUint32;
		};

	public:
        Image();

		Image(const KCL::Image &source, bool deepCopy = true);

		Image(KCL::uint32 width, KCL::uint32 height, ImageFormat format);

		Image(KCL::uint32 width, KCL::uint32 height, KCL::uint32 depth, ImageFormat format);

		Image(KCL::uint32 width, KCL::uint32 height, KCL::uint32 depth, size_t linePitch, size_t slicePitch, ImageType type, ImageFormat format);

		virtual ~Image();

		void Allocate2D(KCL::uint32 width, KCL::uint32 height, ImageFormat format);

		KCL::uint32 getBpp() const;

        bool isDataInSRGB() const { return m_sRGB_values; }

		/**
			Gets the dimensions of a block in pixels for compressed textures.
		*/
		void getBlockDimensions(KCL::uint32 &x, KCL::uint32 &y, KCL::uint32 &z) const;

		/**
			Gets the size of a block in bits for compressed textures.
		*/
		KCL::uint32 getBlockSize() const;

		/**
			Gets a pointer to the raw image data.
		*/
		inline void* getData() const	{ return m_ptrRawData; }

        /**
			Sets a pointer to the raw image data.
		*/
        inline void setData(void* dataIn) { m_ptrRawData = dataIn; }

		/**
			Gets a pointer to the raw data of an image layer. Returns NULL for 1D and 2D images
			depth[in]	index of the layer or cube face to return.
		*/
		inline void* getData(unsigned int depth) const	{ return (depth < m_depth) ? (m_ptrUint8 + (m_slicePitch * depth)) : NULL; }

        /**
			Gets the name of the image.
		*/
		inline std::string getName() const	{ return m_name; }

		/**
			Gets the format of the image.
		*/
		inline KCL::ImageFormat getFormat() const	{ return m_format; }

		/**
			Gets the type of the image.
		*/
		inline KCL::ImageType getType() const		{ return m_type; }

		/**
			Gets the width of the image.
			@return Width of the image.
		*/
		inline KCL::uint32 getWidth() const		{ return m_width; }

		/**
			Gets the height of the image.
			@return Height of the image. Returns 0 for 1D images.
		*/
		inline KCL::uint32 getHeight() const	{ return m_height; }

		/**
			Gets the depth of the image.
			@return Depth of the image. Returns 0 for 1D and 2D images, 6 for cube images and number of layers for image arrays.
		*/
		inline KCL::uint32 getDepth() const		{ return m_depth; }

        /**
			Gets the number of mip levels.
			@return Number of mip levels in the image. 0 if only top-level image is available.
		*/
        inline KCL::uint32 getMipCount() const		{ return m_mipcount; }

		/**
			Gets the distance in bytes from the beginning of one line of a texture to the next line.
		*/
		inline size_t getLinePitch() const	{ return m_linePitch; }

		/**
			Gets the distance in bytes from the beginning of one depth level to the next.
		*/
		inline size_t getSlicePitch() const { return m_slicePitch; }

		/**
			Gets the size of the image data in bytes.
		*/
		inline size_t getDataSize() const	{ return m_size; }

		/**
			Converts image data to BGR uncompressed image format. This method is faster than the generic convert() method.
			@w[in]				image width
			@h[in]				image height
			@srcData[in]		pointer to the image data
			@srcLinePitch[in]	distance in bytes from the beginning of one line of a texture to the next line in the source image
			@srcFormat[in]		format of the input image
			@dstLinePitch[in]	distance in bytes from the beginning of one line of a texture to the next line in the output image. Specify negative number to flip along Y axis.
			@return				the resulting output image or NULL, if conversion failed
		*/
		static void* getBgrData(int w, int h, const void* srcData, int srcLinePitch, ImageFormat srcFormat, int dstLinePitch);

		/**
			Tests if an image file is present, with all the possible extensions.
			@file[in]	path to the file to load
			@return		true if the image file is present; otherwise false
		*/
        static bool isPresent(const char *file);

		/**
			Loads image from a file.
			@file[in]	path to the file to load
			@flip[in]	specifies whether to flip the image vertically or not
			@return		true if loading succeeded; otherwise false
		*/
		bool load(const char *file, bool flip = false);

		/**
			Loads image from a file.
			@fileBase[in]	file name prefix
			@return			number of textures loaded.
		*/
		int load3D(const char *fileBase);

		/**
			Loads image from a file.
			@fileBase[in]	file name prefix
			@return			true if loading succeeded; otherwise false
		*/
		bool loadCube(const char *fileBase);

		/**
			Loads image from a file.
			@fileBase[in]	file name prefix
			@return			number of textures loaded.
		*/
		int loadCubeArray(const char *fileBase);

		/**
			Loads image from a file.
			@fileBase[in]	file name prefix
			@return			number of textures loaded.
		*/
		int loadArray(const char *fileBase);

		/**
			Loads image from a file.
			@fileBase[in]	file name prefix
			@startIdx[in]	index of the first image to load
			@maxCount[in]	maximum number of images to load
			@pattern[in]	pattern to generate filenames. The first argument will be filebase, the secont an integer counter starting from firstIdx
			@return			number of textures loaded.
		*/
		int loadArray(const char *fileBase, const char *pattern, int startIdx = 0, int maxCount = INT_MAX);

		/**
			Saves an image as PNG image file. Works only with RGB888 images.
			@fileName[in]	path of the destination file
			@width[in]		width of the image in pixels
			@height[in]		height of the image in pixels
			@data[in]		pointer to the raw image data
			@format[in]		format of the image
			@flip[in]		true to flip image along Y axis before saving
			@returns		true if saving succeeded, false otherwise
		*/
		static bool savePng(const char* filename, int width, int height, const void* data, ImageFormat format, bool flip = false);

		/**
			Saves an image as BMP image file. Works only with RGB888 images.
			@fileName[in]	path of the destination file
			@width[in]		width of the image in pixels
			@height[in]		height of the image in pixels
			@data[in]		pointer to the raw image data
			@format[in]		format of the image
			@flip[in]		true to flip image along Y axis before saving
			@returns		true if saving succeeded, false otherwise
		*/
		/*static bool saveBmp(const char* filename, int width, int height, const void* data, ImageFormat format, bool flip = false);*/

		/**
			Saves an image as TGA image file. Works only with RGB888 images.
			@fileName[in]	path of the destination file
			@width[in]		width of the image in pixels
			@height[in]		height of the image in pixels
			@data[in]		pointer to the raw image data
			@format[in]		format of the image
			@flip[in]		true to flip image along Y axis before saving
			@returns		true if saving succeeded, false otherwise
		*/
		static bool saveTga(const char* filename, int width, int height, const void* data, ImageFormat format, bool flip = false);

		/**
			Saves an image as PNG image file. Works only with RGB888 2D images.
			@fileName[in]	path of the destination file
			@flip[in]		true to flip image along Y axis before saving
			@returns		true if saving succeeded, false otherwise
		*/
		inline bool savePng(const char* filename, bool flip = false) const
		{
			switch (m_type)
			{
			case Image_1D:
				return savePng(filename, m_width, 1, m_ptrRawData, m_format, flip);

			case Image_2D:
				return savePng(filename, m_width, m_height, m_ptrRawData, m_format, flip);

			default:
				return false;
			}			
		}

		/**
			Saves an image as BMP image file. Works only with RGB888 2D images.
			@fileName[in]	path of the destination file
			@flip[in]		true to flip image along Y axis before saving
			@returns		true if saving succeeded, false otherwise
		*/
		/*inline bool saveBmp(const char* filename, bool flip = false) const
		{
			switch (m_type)
			{
			case Image_1D:
				return saveBmp(filename, m_width, 1, m_ptrRawData, m_format, flip);

			case Image_2D:
				return saveBmp(filename, m_width, m_height, m_ptrRawData, m_format, flip);

			default:
				return false;
			}			
		}*/

		/**
			Saves an image as TGA image file. Works only with RGB888 2D images.
			@fileName[in]	path of the destination file
			@flip[in]		true to flip image along Y axis before saving
			@returns		true if saving succeeded, false otherwise
		*/
		inline bool saveTga(const char* filename, bool flip = false) const
		{
			switch (m_type)
			{
			case Image_1D:
				return saveTga(filename, m_width, 1, m_ptrRawData, m_format, flip);

			case Image_2D:
				return saveTga(filename, m_width, m_height, m_ptrRawData, m_format, flip);

			default:
				return false;
			}			
		}

		/**
			Saves an image as PNG image file. Works only with RGB888 2D images.
			@fileName[in]	path of the destination file
			@layer[in]		specifies which layer to save
			@flip[in]		true to flip image along Y axis before saving
			@returns		true if saving succeeded, false otherwise
		*/
		inline bool savePng(const char* filename, KCL::uint8 layer, bool flip = false) const
		{
			switch (m_type)
			{
			case Image_1D:
			case Image_2D:
				return false;

			default:
				return savePng(filename, m_width, m_height, getData(layer), m_format, flip);
			}			
		}

		/**
			Saves an image as BMP image file. Works only with RGB888 2D images.
			@fileName[in]	path of the destination file
			@layer[in]		specifies which layer to save
			@flip[in]		true to flip image along Y axis before saving
			@returns		true if saving succeeded, false otherwise
		*/
		/*inline bool saveBmp(const char* filename, KCL::uint8 layer, bool flip = false) const
		{
			switch (m_type)
			{
			case Image_1D:
			case Image_2D:
				return false;

			default:
				return saveBmp(filename, m_width, m_height, getData(layer), m_format, flip);
			}			
		}*/

		/**
			Saves an image as TGA image file. Works only with RGB888 2D images.
			@fileName[in]	path of the destination file
			@layer[in]		specifies which layer to save
			@flip[in]		true to flip image along Y axis before saving
			@returns		true if saving succeeded, false otherwise
		*/
		inline bool saveTga(const char* filename, KCL::uint8 layer, bool flip = false) const
		{
			switch (m_type)
			{
			case Image_1D:
			case Image_2D:
				return false;

			default:
				return saveTga(filename, m_width, m_height, getData(layer), m_format, flip);
			}			
		}

		/**
			Releases the data buffer.
		*/
		void releaseData();

		/**
			Flips the image on the X axis.
			@return			true if conversion succeeded
		*/
		bool flipX();

		/**
			Flips the image on the Y axis if applicable.
			@return			true if conversion succeeded
		*/
		bool flipY();

		/**
			Flips the image on the Z axis if applicable.
			@return			true if conversion succeeded
		*/
		bool flipZ();

		/**
			Returns the average of the four pixels
			@return			the average of the four pixels
		*/
		static KCL::uint8 getAverageSuperPixel(KCL::uint8 p0,KCL::uint8 p1,KCL::uint8 p2,KCL::uint8 p3)
		{
			return (KCL::uint8)(((KCL::uint16)p0+p1+p2+p3)/4);
		}

		/**
			Returns the average of the middle two pixels
			@return			the average of the middle two pixels
		*/
		static KCL::uint8 getMedianSuperPixel(KCL::uint8 p0,KCL::uint8 p1,KCL::uint8 p2,KCL::uint8 p3)
		{
			KCL::uint16 minmax=std::max<KCL::uint8>(std::min<KCL::uint8>(p0,p1),std::min<KCL::uint8>(p2,p3));
			KCL::uint16 maxmin=std::min<KCL::uint8>(std::max<KCL::uint8>(p0,p1),std::max<KCL::uint8>(p2,p3));
			return (KCL::uint8)((minmax+maxmin)/2);
		}

		/**
			Halves the image using the given averaging function. Works on 2D images, Cube images and Image arrays.
			@func[in]		the function used for averaging the pixels
			@return			true if conversion succeeded
		*/
		bool halfImage(uint8 (*func)(uint8,uint8,uint8,uint8)=NULL);

		/**
			Converts the image to the desired format.
			@format[in]		desired image format
			@return			true if conversion succeeded
		*/
		bool convertTo(ImageFormat format);

        KCL::Image* cloneTo(KCL::ImageFormat format) const;

        bool decodeASTCtoRGB888(KCL::Image* target) const;
		bool decodeETC1toRGB888(KCL::Image* target) const;
		bool decodeETC1toRGBA8888(KCL::Image* target) const;
		bool decodeETC1toRGB565(KCL::Image* target) const;
        bool decodeRGB888toRGBA8888 (KCL::Image* target) const;
		bool decodeRGBA8888toRGB9E5();
		bool decodeRGB9E5toRGB888();

		static KCL::uint32 Float3_To_RGB9E5(const float rgb[3]);

        void setupMIPminXminY ();

        KCL::uint8* getMipmapData (KCL::uint32 level) const;
        void getMipmapData (KCL::uint32 level, KCL::uint8 **data, KCL::uint32 *mipmapsize, KCL::uint32 *mipWidth = 0, KCL::uint32 *mipHeight = 0, KCL::uint32* mempitch = 0) const;

		inline bool isUncompressed() const
		{
			return isUncompressed(m_format);
		}

		const std::string& GetImageFilename() const;

		static inline bool isUncompressed(ImageFormat f)
		{
			switch (f)
			{
				case Image_ALPHA_A8:
				case Image_DEPTH_16:
				case Image_LUMINANCE_ALPHA_LA88:
				case Image_LUMINANCE_L8:
				case Image_PAL4_RGB565:
				case Image_PAL4_RGB888:
				case Image_PAL4_RGBA4444:
				case Image_PAL4_RGBA5551:
				case Image_PAL4_RGBA8888:
				case Image_PAL8_RGB565:
				case Image_PAL8_RGB888:
				case Image_PAL8_RGBA4444:
				case Image_PAL8_RGBA5551:
				case Image_PAL8_RGBA8888:
				case Image_RGB565:
				case Image_RGB888:
				case Image_RGBA4444:
				case Image_RGBA5551:
				case Image_RGBA8888:
				case Image_RGBA_32F:
					return true;
				default:
					return false;
			}
		}
	private:
		std::string m_image_filename;
		bool loadTga(const char *filename);
		bool loadBmp (const char *file, bool flip = false);
        bool loadASTC (const char *filename);
		bool loadPvr2 (const char *file);
		bool loadPvr3 (const char *file);
		bool loadPng (const char *file);
		bool loadFloat3Raw(const char *file);
		bool loadHdr(const char *file);

		int loadFaces(const std::vector<std::string> &filenames);
	};

	///Base class for loading, or storing images 
	class ImageBase 
	{
		friend class M3GBufferReader;
	public:
		virtual ~ImageBase ();

		inline KCL::uint32 getWidth () const	{ return m_width; }
		inline KCL::uint32 getHeight () const	{ return m_height; }
		inline KCL::uint32 getFormat () const	{ return m_format; }
		inline KCL::uint32 isMutable () const	{ return m_mutable; }
		inline KCL::uint32 getId () const		{ return m_id; }

		virtual void commit (bool repeatS, bool repeatT) = 0;
		KCL::uint32 getBpp () const;
		virtual bool isFormatSupported () const { return false; }

	protected:
		ImageBase ();

		void getDescription ();
		//virtual void uploadMipmap (int format) {}
		//virtual void generateMipmap () = 0;

		KCL::uint32 m_id;
		KCL::uint32 m_width;
		KCL::uint32 m_height;
		KCL::ImageFormat m_format;
		KCL::uint32 m_mipmaps;
		bool m_mutable;
		const FormatDescription *m_desc;
	};


	/// \class Image2D
	/// Object for loading, or storing 2D images.
	/// Supported image formats: BMP, PNG.
	class KCL_API Image2D : public ImageBase
	{
	friend class ImageCube;
	friend class CubeEnvMap;
	public:
		/*override*/ ~Image2D ();

		/*override*/ void commit_debug_mipmap (bool repeatS = false, bool repeatT = false) {} //!!!
		/*override*/ void commit (bool repeatS = false, bool repeatT = false) {} //!!!

		void reset ();
		bool load (const char *file, bool flip = false);
		bool release_data ();
		void setData (KCL::uint32 size, const KCL::uint8 *data);

		inline KCL::uint8 *data ()					{ return m_data; }
		inline void EnableMipmap()					{ mm_enabled = true; }
		inline void setName(const char* imageName)	{ m_name = imageName; }
		inline const char* getName() const			{ return m_name.c_str(); }

		void setDimensions(KCL::uint32 width, KCL::uint32 height, KCL::uint32 format);
		void allocBuffer(KCL::uint32 width, KCL::uint32 height, KCL::uint32 format);

		bool init ();
		bool loadASTC (const char *filename);
		bool loadBmp (const char *file, bool flip = false);
		bool loadPvr (const char *file);
		bool loadPvr3 (const char *file);
		bool loadPng (const char *file);
		void generateFalseColorMipmapDataWithId (KCL::uint32 imageId, int size);

		bool HasAlphaPvr()
		{
			return m_has_pvr_alpha;
		}
		bool GetMutable()
		{
			return m_mutable;
		}

		void convertRGB ();
	
		void getMipmapData (KCL::uint32 level, KCL::uint8 **data, KCL::uint32 *mipmapsize, KCL::uint32 *mipWidth = 0, KCL::uint32 *mipHeight = 0, KCL::uint32* mempitch = 0) const;

		void decodeASTCtoRGB888 ();
		void decodeETC1toRGB888 ();
		void decodeETC1toRGB565 ();
		void decodeRGB888toRGBA8888 ();

		Image2D ();

	protected:
		//virtual void uploadMipmap (int format);
		//virtual void generateMipmap ();
		KCL::uint8 *getMipmapData(KCL::uint32 level);
		//void generateMipmap (int internalformat, int format, int type);

		void commitETC1 ();

		std::string m_name;
		size_t m_size;
		KCL::uint8 *m_data;
		bool mm_enabled;
		bool m_has_pvr_alpha;
	};

	///Object for loading, or storing cube map images.
	class ImageCube : public ImageBase
	{
	public:
		~ImageCube ();

		/*override*/ void commit (bool repeatS = false, bool repeatT = false) {} //!!!
		void setData (int k, size_t size, KCL::uint8 *data);
		void setDimensions (KCL::uint32 width, KCL::uint32 height, KCL::uint32 format);
		void allocBuffer (KCL::uint32 width, KCL::uint32 height, KCL::uint32 format);
		bool load (const char *fileBase);
		KCL::uint8 m_filename[6][1024];

	protected:
		ImageCube ();

		//virtual void generateMipmap ();
		//void commitETC1 (int face);
		void convertRGB ();
		void decodeETC1 ();

		KCL::uint8 *m_data[6];
	};
}//namespace KCL

#endif
