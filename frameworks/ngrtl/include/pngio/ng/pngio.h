/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NG_PNGCPP_INCLUDED
#define NG_PNGCPP_INCLUDED

#include <stdint.h>
#include <vector>

#include "ng/macros.h"

namespace ng
{
	class IStream;
	class OStream;

	class PngReaderImp;
	class PngWriterImp;

	class PngIOBase
	{
	public:
		enum ColorType
		{
			ct_gray, ct_grayAlpha,
			ct_rgb, ct_rgba
		};
	};

	class PngReader
		: public PngIOBase
	{
	public:
		PngReader();
		~PngReader();
		void read(ng::IStream& istream);
		int width() const;
		int height() const;
		int bpc() const; //bits per channel
		ColorType colorType() const;
		void getRow(int y, uint8_t* row) const; //returns the row in (width * n_of_channels * bpc + 7 )/ 8 bytes. Sub-byte pixels are packed msb-to-lsb
	private:
		PngReaderImp* p;
	};

	class PngWriter
		: public PngIOBase
	{
	public:
		PngWriter();
		~PngWriter();
		void reset(int width, int height, ColorType ct, int bpc);
		void write(ng::OStream& ostream);
		void setRow(int y, const uint8_t* row); //sets the row using the bytes row..row + (width * n_of_channels * bpc + 7 )/ 8 bytes. Sub-byte pixels are packed msb-to-lsb
	private:
		PngWriterImp* p;
	};
}

#endif

