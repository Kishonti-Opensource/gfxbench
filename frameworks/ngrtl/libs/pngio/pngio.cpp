/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/pngio.h"

#include <png.h>

#include "ng/require.h"
#include "ng/stream/stream.h"
#include "ng/format.h"

namespace {
#if PNG_LIBPNG_VER < 10500
void png_longjmp(png_structp png_ptr, int val)
{
#if PNG_LIBPNG_VER > 10249
	require(png_ptr != NULL && png_ptr->longjmp_fn != NULL);
	png_ptr->longjmp_fn(png_ptr->jmpbuf, val);
#endif
}
#endif
}

namespace ng
{

class PngRWImpBase
{
public:
	PngRWImpBase()
		: png(0)
		, info(0)
		, pngCreationType(pct_null)
	{}
	~PngRWImpBase()
	{
		clear();
	}
	void clear()
	{
		if ( png != NULL && info != NULL )
			png_destroy_info_struct(png, &info);
		if ( png != NULL )
		{
			switch(pngCreationType)
			{
			case pct_read_struct:
				png_destroy_read_struct(&png, NULL, NULL); break;
			case pct_write_struct:
				png_destroy_write_struct(&png, NULL); break;
			default:
				require(false);
			}
			pngCreationType = pct_null;
		}
	}
	int width() const
	{
		return png_get_image_width(png, info);
	}
	int height() const
	{
		return png_get_image_height(png, info);
	}
	int bitDepth() const
	{
		return png_get_bit_depth(png, info);
	}
	PngIOBase::ColorType colorType() const
	{
		png_byte ct = png_get_color_type(png, info);
		switch(ct)
		{
		case PNG_COLOR_TYPE_GRAY:
			return PngIOBase::ct_gray;
		case PNG_COLOR_TYPE_PALETTE:
			require(false, "Png color type palette is not supported");
			break;
		case PNG_COLOR_TYPE_RGB:
			return PngIOBase::ct_rgb;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			return PngIOBase::ct_rgba;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			return PngIOBase::ct_grayAlpha;
		default:
			require(false, ng::cstr(ng::format("Unknown png color type: %s") % (int)ct));
		}
		return (PngIOBase::ColorType)0; //unreachable
	}
	int rowBytes() const
	{
		return (int)png_get_rowbytes(png, info);
	}
	uint8_t* rowPointer(int y)
	{
		return png_get_rows(png, info)[y];
	}
	const uint8_t* rowPointer(int y) const
	{
		return png_get_rows(png, info)[y];
	}
	int nChannels() const
	{
		int ct = colorType();
		switch(ct)
		{
		case PngIOBase::ct_gray:
			return 1;
		case PngIOBase::ct_grayAlpha:
			return 2;
		case PngIOBase::ct_rgb:
			return 3;
		case PngIOBase::ct_rgba:
			return 4;
		default:
			require(false, ng::cstr(ng::format("Invalid color type: ") % (int)ct));
		}
		return 0; //unreachable
	}
	void createReadStruct();
	void createWriteStruct();

	png_structp png;
	png_infop info;
	std::string errorMessage;
	std::vector<std::string> warningMessages;
	enum PngCreationType { pct_null, pct_read_struct, pct_write_struct } pngCreationType;
};

namespace {

void user_error_fn(png_structp png_ptr, png_const_charp error_message)
{
	PngRWImpBase* p = (PngRWImpBase*)png_get_error_ptr(png_ptr);
	p->errorMessage = error_message;
	png_longjmp(png_ptr, 1);
}

void user_warning_fn(png_structp png_ptr, png_const_charp warning_message)
{
	PngRWImpBase* p = (PngRWImpBase*)png_get_error_ptr(png_ptr);
	p->warningMessages.push_back(std::string(warning_message));
}

}

void PngRWImpBase::
	createReadStruct()
{
	require(png == NULL);
#if PNG_LIBPNG_VER > 10210
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, this, user_error_fn, user_warning_fn);
#else
	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, this, 0, user_warning_fn);
#endif
	require(png != NULL);
	pngCreationType = pct_read_struct;
}

void PngRWImpBase::
	createWriteStruct()
{
	require(png == NULL);
#if PNG_LIBPNG_VER > 10210
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, this, user_error_fn, user_warning_fn);
#else
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, this, 0, user_warning_fn);
#endif
	require(png != NULL);
	pngCreationType = pct_write_struct;
}



class PngReaderImp
	: public PngRWImpBase
{
public:
	typedef PngRWImpBase Super;

	PngReaderImp()
		: istream(0)
	{}

	void clear()
	{
		Super::clear();
		istream = 0;
	}

	ng::IStream* istream;
};

namespace {
void user_read_fn(png_structp png_ptr, png_bytep data, png_size_t size)
{
	((PngReaderImp*)png_get_io_ptr(png_ptr))->istream->read(data, size);
}
}

class PngWriterImp
	: public PngRWImpBase
{
public:
	typedef PngRWImpBase Super;

	PngWriterImp()
		: ostream(0)
	{}

	void clear()
	{
		Super::clear();
		ostream = 0;
	}

	ng::OStream* ostream;
	std::vector<uint8_t> data;
	std::vector<png_bytep> rowPointers;
};

namespace {
void user_write_fn(png_structp png_ptr, png_bytep data, png_size_t size)
{
	((PngWriterImp*)png_get_io_ptr(png_ptr))->ostream->write(data, size);
}

void user_IO_flush_function(png_structp png_ptr)
{
	((PngWriterImp*)png_get_io_ptr(png_ptr))->ostream->flush();
}
}

PngReader::
	PngReader()
	: p(new PngReaderImp)
{}

PngReader::
	~PngReader()
{
	delete p;
}

void PngReader::
	read(ng::IStream& istream)
{
	p->clear();
	p->istream = &istream;

	p->createReadStruct();

	int setJmpResult = setjmp(png_jmpbuf(p->png));
	require(setJmpResult == 0, ng::cstr(ng::format("libpng error: %s") % p->errorMessage));

	p->info = png_create_info_struct(p->png);
	require(p->info != NULL);

	png_set_read_fn(p->png, p, user_read_fn);

	png_read_png(p->png, p->info, PNG_TRANSFORM_IDENTITY, NULL);
}


int PngReader:: width() const { return p->width(); }
int PngReader::height() const { return p->height(); }
int PngReader::bpc() const { return p->bitDepth(); }
PngIOBase::ColorType PngReader::colorType() const { return p->colorType(); }

PngWriter::
	PngWriter()
	: p(new PngWriterImp)
{
}

PngWriter::
	~PngWriter()
{
	delete p;
}

void PngWriter::
	reset(int width, int height, ColorType ct, int bpc)
{
	p->clear();

	p->createWriteStruct();
	
	int setJmpResult = setjmp(png_jmpbuf(p->png));
	require(setJmpResult == 0, ng::cstr(ng::format("libpng error: %s") % p->errorMessage));

	p->info = png_create_info_struct(p->png);
	require(p->info != NULL);

	int color_type;
	switch(ct)
	{
	case ct_gray:
		color_type = PNG_COLOR_TYPE_GRAY; break;
	case ct_grayAlpha:
		color_type = PNG_COLOR_TYPE_GRAY_ALPHA; break;
	case ct_rgb:
		color_type = PNG_COLOR_TYPE_RGB; break;
	case ct_rgba:
		color_type = PNG_COLOR_TYPE_RGB_ALPHA; break;
	default:
		require(false, ng::cstr(ng::format("Invalid color type: ") % (int)ct));
	}

	png_set_IHDR(p->png, p->info, width, height, bpc, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	p->data.assign(height * p->rowBytes(), (uint8_t)0);
	p->rowPointers.resize(height);

	int rowBytes = p->rowBytes();

	for(int y = 0; y < height; ++y)
		p->rowPointers[y] = &(p->data[y * rowBytes]);

	png_set_rows(p->png, p->info, &p->rowPointers.front());
}

void PngWriter::
	write(ng::OStream& ostream)
{
	p->ostream = &ostream;

	png_set_write_fn(p->png, p, user_write_fn, user_IO_flush_function);

	png_write_png(p->png, p->info, PNG_TRANSFORM_IDENTITY, NULL);
}

void PngReader::
	getRow(int y, uint8_t* row) const
{
	uint8_t* r = p->rowPointer(y);
	std::copy(r, r + p->rowBytes(), row);
}

void PngWriter::
	setRow(int y, const uint8_t* row)
{
	std::copy(row, row + p->rowBytes(), p->rowPointer(y));
}

}
