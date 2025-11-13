/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file image.h
	Definition of GLB::Image2D.
*/
#ifndef IMAGE_H
#define IMAGE_H

#include "kcl_image.h"
#include <kcl_base.h>
#include <string>

#include <d3d11_1.h>
#include "DXUtils.h"
#include "DX.h"

using namespace std;

namespace DXB
{
	/// \class Image2D
	/// Object for loading, or storing 2D images.
	/// Supported image formats: BMP, PNG.
	class Image2D : public KCL::Image2D
	{
	friend class ImageCube;
	public:
		virtual ~Image2D ();

		/*override*/ void commit_debug_mipmap (bool repeatS = false, bool repeatT = false);
		/*override*/ void commit (bool repeatS = false, bool repeatT = false);
		/*override*/ void convertRGB ();

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getD3D11Id() { return m_d3d11_id; }

        Image2D ();
	protected:

		virtual void uploadMipmap (int format);
		virtual void generateMipmap ();
		void generateMipmap (int internalformat, int format, int type);

		void commitETC1 ();

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_d3d11_id;
	};

	///Object for loading, or storing cube map images.
	class ImageCube : public KCL::ImageCube
	{
	public:
		/*override*/ void commit (bool repeatS = false, bool repeatT = false);

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getD3D11Id() { return m_d3d11_id; }

	protected:
		/*override*/ void commitETC1 (int face);
		/*override*/ void generateMipmap ();

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_d3d11_id;
	};

}//namespace GLB

#endif
