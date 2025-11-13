/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <sstream>

#include "gui_test.h"
#include "platform.h"
#include "opengl/glb_image.h"
#include "opengl/texture.h"
#include "opengl/fbo.h"
#include "opengl/glb_opengl_state_manager.h"
#include "kcl_io.h"



using namespace GUIB;
using namespace KCL;


static const KCL::uint32 VIRTUAL_WIN_WIDTH  = 2560;
static const KCL::uint32 VIRTUAL_WIN_HEIGHT = 1504;

static const KCL::uint32 PAGE_WIN_WIDTH  = 2240;
static const KCL::uint32 PAGE_WIN_HEIGHT = 1088;
static const KCL::uint32 PAGE_VIEWPORT_WIDTH  = 2236;
static const KCL::uint32 PAGE_VIEWPORT_HEIGHT = 1088;

static const double GRID_ICON_SZ = 144.0;
static const double HOTSEAT_APPS_ICON_SZ = 102.4;
static const double HOTSEAT_ICON_SZ = 115.200005;


static const KCL::uint32 GRID_ICON_SCISSOR_START_X[2] = { 162 , 0 };
static const KCL::uint32 GRID_ICON_SCISSOR_START_Y[2] = { 1097 , 896 };
static const KCL::uint32 GRID_ICON_SCISSOR_SZ   = 192;
static const KCL::uint32 GRID_ICON_SCISSOR_STEP_X  = 292;
static const KCL::uint32 GRID_ICON_SCISSOR_STEP_Y  = 224;

//TODO TEXT STUFF

static const KCL::uint32 WALLPAPER_WIDTH = 3000;

static const KCL::uint32 HOTSEAT_ICON_SCISSOR_START_X = 1224; //start from middle !!! this is the start of "Apps Icon"
static const KCL::uint32 HOTSEAT_ICON_SCISSOR_Y = 26;
static const KCL::uint32 HOTSEAT_ICON_SCISSOR_SZ = 112;
static const KCL::uint32 HOTSEAT_ICON_SCISSOR_STEP_X = 256;


static const KCL::uint32 GRID_ROW_COUNT = 5;
static const KCL::uint32 GRID_COL_COUNT = 8;
static const KCL::uint32 HOTSEAT_COUNT	= 7;



static const KCL::Matrix4x4 FULL_SCREEN_PROJ_MAT ( 2.0 / (double)VIRTUAL_WIN_WIDTH,  0,                                 0,  0,
												   0,                               -2.0 / (double)VIRTUAL_WIN_HEIGHT,  0,  0,
												   0,                                0,                                -1,  0,
												  -1,                                1,                                 0,  1);



static const KCL::Matrix4x4 FULL_SCREEN_PROJ_MAT2 ( 2.0 / (double)VIRTUAL_WIN_WIDTH,  0,                                 0,  0,
												   0,                                2.0 / (double)VIRTUAL_WIN_HEIGHT,  0,  0,
												   0,                                0,                                -1,  0,
												  -1,                                -1,                                 0,  1);



static const KCL::Matrix4x4 ANIMATED_SCREEN_PROJ_MAT ( 2.0 / (double)PAGE_VIEWPORT_WIDTH,  0,                                   0,  0,
													   0,                                 -2.0 / (double)PAGE_VIEWPORT_HEIGHT,  0,  0,
													   0,                                  0,                                  -1,  0,
													  -1,                                  1,                                   0,  1);



static const std::string GUIB_DIRECTORY = "GUIBenchmark/";
//TODO: background
static const std::string HOTSEAT_ICON_FILE_PREFIX = "hotseat_"; //hotseat_00.png, hotseat_01.png, ...
static const std::string GRID_ICON_FILE_PREFIX = "icon_"; // icon_a_00.png, ... icon_a_39.png, icon_b_00.png, ... icon_b_39.png, ...
static const std::string WALLPAPER_FILE_NAME = "wallpaper.png";
static const std::string GRADIENT_FILE_NAME = "gradient.png";
static const std::string NAVIGATION_STRIPE_FILE_NAME = "navigation_stripe.png";
static const std::string SEARCH_BAR_BG_FILE_NAME = "search_bar_bg.png";
static const std::string SEARCH_BAR_FILE_NAME = "search_bar.png";
static const std::string BLUE_NAVIGATION_STRIPE_FILE_NAME = "blue_navigation_stripe.png";


static const ColorHolder ZERO_COLOR ( 0,0,0,0 );
static const KCL::Matrix4x4 ID_MATRIX;

Launcher2ShaderManager* Launcher2ShaderManager::s_instance = 0;


KCL::uint32 Launcher2AttribAndVBORegistry::s_actual_vbo = 0;
KCL::uint32 Launcher2AttribAndVBORegistry::s_actual_ebo = 0;
const void* Launcher2AttribAndVBORegistry::s_actual_vertex_pointer[2] = {0, 0};
bool Launcher2AttribAndVBORegistry::s_attribpointer_dirty[2] = {true, true};


//float zero_one_square
const float ZERO_ONE_SQUARE[] =
{
	0.0f, // x0
	0.0f, // y0
	0.0f, // u0
	0.0f, // v0
	
	1.0f, // x1
	0.0f, // y1
	1.0f, // u1
	0.0f, // v1
	
	0.0f, // x2
	1.0f, // y2
	0.0f, // u2
	1.0f, // v2
	
	1.0f, // x3
	1.0f, // y3
	1.0f, // u3
	1.0f  // v3
};


//directly copied from tracer, Nexus 10's trace in landscape mode (JellyBean 4.2.1)
const float GRADIENT_VBO_DATA[] =
{
	0.0000f,
	0.0000f,
	0.4998f,
	0.0000f,
	2560.0000f,
	0.0000f,
	0.5002f,
	0.0000f,
	0.0000f,
	197.0000f,
	0.4998f,
	0.4925f,
	0.0000f,
	197.0000f,
	0.4998f,
	0.4925f,
	2560.0000f,
	0.0000f,
	0.5002f,
	0.0000f,
	2560.0000f,
	197.0000f,
	0.5002f,
	0.4925f,
	0.0000f,
	1255.0000f,
	0.4998f,
	0.5025f,
	2560.0000f,
	1255.0000f,
	0.5002f,
	0.5025f,
	0.0000f,
	1454.0000f,
	0.4998f,
	1.0000f,
	0.0000f,
	1454.0000f,
	0.4998f,
	1.0000f,
	2560.0000f,
	1255.0000f,
	0.5002f,
	0.5025f,
	2560.0000f,
	1454.0000f,
	0.5002f,
	1.0000f
};


//directly copied from tracer, Nexus 10's trace in landscape mode (JellyBean 4.2.1)
const float NAVIGATION_STRIPE_VBO_DATA[] =
{
	0.0000f,
	0.0000f,
	0.0000f,
	0.0000f,
	4.0000f,
	0.0000f,
	0.3333f,
	0.0000f,
	0.0000f,
	4.0000f,
	0.0000f,
	0.3333f,
	0.0000f,
	4.0000f,
	0.0000f,
	0.3333f,
	4.0000f,
	0.0000f,
	0.3333f,
	0.0000f,
	4.0000f,
	4.0000f,
	0.3333f,
	0.3333f,
	4.0000f,
	0.0000f,
	0.3749f,
	0.0000f,
	1848.0000f,
	0.0000f,
	0.6251f,
	0.0000f,
	4.0000f,
	4.0000f,
	0.3749f,
	0.3333f,
	4.0000f,
	4.0000f,
	0.3749f,
	0.3333f,
	1848.0000f,
	0.0000f,
	0.6251f,
	0.0000f,
	1848.0000f,
	4.0000f,
	0.6251f,
	0.3333f,
	1848.0000f,
	0.0000f,
	0.6667f,
	0.0000f,
	1852.0000f,
	0.0000f,
	1.0000f,
	0.0000f,
	1848.0000f,
	4.0000f,
	0.6667f,
	0.3333f,
	1848.0000f,
	4.0000f,
	0.6667f,
	0.3333f,
	1852.0000f,
	0.0000f,
	1.0000f,
	0.0000f,
	1852.0000f,
	4.0000f,
	1.0000f,
	0.3333f,
	0.0000f,
	4.0000f,
	0.0000f,
	0.3333f,
	4.0000f,
	4.0000f,
	0.3333f,
	0.3333f,
	0.0000f,
	8.0000f,
	0.0000f,
	0.6667f,
	0.0000f,
	8.0000f,
	0.0000f,
	0.6667f,
	4.0000f,
	4.0000f,
	0.3333f,
	0.3333f,
	4.0000f,
	8.0000f,
	0.3333f,
	0.6667f,
	4.0000f,
	4.0000f,
	0.3749f,
	0.3333f,
	1848.0000f,
	4.0000f,
	0.6251f,
	0.3333f,
	4.0000f,
	8.0000f,
	0.3749f,
	0.6667f,
	4.0000f,
	8.0000f,
	0.3749f,
	0.6667f,
	1848.0000f,
	4.0000f,
	0.6251f,
	0.3333f,
	1848.0000f,
	8.0000f,
	0.6251f,
	0.6667f,
	1848.0000f,
	4.0000f,
	0.6667f,
	0.3333f,
	1852.0000f,
	4.0000f,
	1.0000f,
	0.3333f,
	1848.0000f,
	8.0000f,
	0.6667f,
	0.6667f,
	1848.0000f,
	8.0000f,
	0.6667f,
	0.6667f,
	1852.0000f,
	4.0000f,
	1.0000f,
	0.3333f,
	1852.0000f,
	8.0000f,
	1.0000f,
	0.6667f,
	0.0000f,
	8.0000f,
	0.0000f,
	0.6667f,
	4.0000f,
	8.0000f,
	0.3333f,
	0.6667f,
	0.0000f,
	12.0000f,
	0.0000f,
	1.0000f,
	0.0000f,
	12.0000f,
	0.0000f,
	1.0000f,
	4.0000f,
	8.0000f,
	0.3333f,
	0.6667f,
	4.0000f,
	12.0000f,
	0.3333f,
	1.0000f,
	4.0000f,
	8.0000f,
	0.3749f,
	0.6667f,
	1848.0000f,
	8.0000f,
	0.6251f,
	0.6667f,
	4.0000f,
	12.0000f,
	0.3749f,
	1.0000f,
	4.0000f,
	12.0000f,
	0.3749f,
	1.0000f,
	1848.0000f,
	8.0000f,
	0.6251f,
	0.6667f,
	1848.0000f,
	12.0000f,
	0.6251f,
	1.0000f,
	1848.0000f,
	8.0000f,
	0.6667f,
	0.6667f,
	1852.0000f,
	8.0000f,
	1.0000f,
	0.6667f,
	1848.0000f,
	12.0000f,
	0.6667f,
	1.0000f,
	1848.0000f,
	12.0000f,
	0.6667f,
	1.0000f,
	1852.0000f,
	8.0000f,
	1.0000f,
	0.6667f,
	1852.0000f,
	12.0000f,
	1.0000f,
	1.0000f
};


//directly copied from tracer, Nexus 10's trace in landscape mode (JellyBean 4.2.1)
const float SEARCH_BAR_BG_VBO_DATA[] =
{
	0.0000f,
	0.0000f,
	0.0000f,
	0.0000f,
	12.0000f,
	0.0000f,
	0.3750f,
	0.0000f,
	0.0000f,
	12.0000f,
	0.0000f,
	0.3750f,
	0.0000f,
	12.0000f,
	0.0000f,
	0.3750f,
	12.0000f,
	0.0000f,
	0.3750f,
	0.0000f,
	12.0000f,
	12.0000f,
	0.3750f,
	0.3750f,
	12.0000f,
	0.0000f,
	0.3906f,
	0.0000f,
	1840.0000f,
	0.0000f,
	0.6094f,
	0.0000f,
	12.0000f,
	12.0000f,
	0.3906f,
	0.3750f,
	12.0000f,
	12.0000f,
	0.3906f,
	0.3750f,
	1840.0000f,
	0.0000f,
	0.6094f,
	0.0000f,
	1840.0000f,
	12.0000f,
	0.6094f,
	0.3750f,
	1840.0000f,
	0.0000f,
	0.6250f,
	0.0000f,
	1852.0000f,
	0.0000f,
	1.0000f,
	0.0000f,
	1840.0000f,
	12.0000f,
	0.6250f,
	0.3750f,
	1840.0000f,
	12.0000f,
	0.6250f,
	0.3750f,
	1852.0000f,
	0.0000f,
	1.0000f,
	0.0000f,
	1852.0000f,
	12.0000f,
	1.0000f,
	0.3750f,
	0.0000f,
	12.0000f,
	0.0000f,
	0.3891f,
	12.0000f,
	12.0000f,
	0.3750f,
	0.3891f,
	0.0000f,
	92.0000f,
	0.0000f,
	0.6109f,
	0.0000f,
	92.0000f,
	0.0000f,
	0.6109f,
	12.0000f,
	12.0000f,
	0.3750f,
	0.3891f,
	12.0000f,
	92.0000f,
	0.3750f,
	0.6109f,
	12.0000f,
	12.0000f,
	0.3906f,
	0.3891f,
	1840.0000f,
	12.0000f,
	0.6094f,
	0.3891f,
	12.0000f,
	92.0000f,
	0.3906f,
	0.6109f,
	12.0000f,
	92.0000f,
	0.3906f,
	0.6109f,
	1840.0000f,
	12.0000f,
	0.6094f,
	0.3891f,
	1840.0000f,
	92.0000f,
	0.6094f,
	0.6109f,
	1840.0000f,
	12.0000f,
	0.6250f,
	0.3891f,
	1852.0000f,
	12.0000f,
	1.0000f,
	0.3891f,
	1840.0000f,
	92.0000f,
	0.6250f,
	0.6109f,
	1840.0000f,
	92.0000f,
	0.6250f,
	0.6109f,
	1852.0000f,
	12.0000f,
	1.0000f,
	0.3891f,
	1852.0000f,
	92.0000f,
	1.0000f,
	0.6109f,
	0.0000f,
	92.0000f,
	0.0000f,
	0.6250f,
	12.0000f,
	92.0000f,
	0.3750f,
	0.6250f,
	0.0000f,
	104.0000f,
	0.0000f,
	1.0000f,
	0.0000f,
	104.0000f,
	0.0000f,
	1.0000f,
	12.0000f,
	92.0000f,
	0.3750f,
	0.6250f,
	12.0000f,
	104.0000f,
	0.3750f,
	1.0000f,
	12.0000f,
	92.0000f,
	0.3906f,
	0.6250f,
	1840.0000f,
	92.0000f,
	0.6094f,
	0.6250f,
	12.0000f,
	104.0000f,
	0.3906f,
	1.0000f,
	12.0000f,
	104.0000f,
	0.3906f,
	1.0000f,
	1840.0000f,
	92.0000f,
	0.6094f,
	0.6250f,
	1840.0000f,
	104.0000f,
	0.6094f,
	1.0000f,
	1840.0000f,
	92.0000f,
	0.6250f,
	0.6250f,
	1852.0000f,
	92.0000f,
	1.0000f,
	0.6250f,
	1840.0000f,
	104.0000f,
	0.6250f,
	1.0000f,
	1840.0000f,
	104.0000f,
	0.6250f,
	1.0000f,
	1852.0000f,
	92.0000f,
	1.0000f,
	0.6250f,
	1852.0000f,
	104.0000f,
	1.0000f,
	1.0000f
};





template<class T>
void DeletePointerVector(std::vector<T*> &vec)
{
	for(size_t i=0; i<vec.size(); ++i)
	{
		delete vec[i];
		vec[i] = 0;
	}
}


ImageViewCell::ImageViewCell() : m_scissor_x( 0 ), m_scissor_y( 0 ), m_scissor_w( 0 ), m_scissor_h( 0 ), m_projection(&ID_MATRIX)
{
}


TextViewCell::TextViewCell() :
	m_scissor_x  ( 0 ), m_scissor_y  ( 0 ), m_scissor_w  ( 0 ), m_scissor_h  ( 0 ),
	m_scissor_x_2( 0 ), m_scissor_y_2( 0 ), m_scissor_w_2( 0 ), m_scissor_h_2( 0 ),
	m_projection ( &ID_MATRIX ), m_projection_2 ( &ID_MATRIX ), m_projection_3 ( &ID_MATRIX )
{
}


IconPageGrid::IconPageGrid(IconPageGridType gridType)
{
	m_rowCount = GRID_ROW_COUNT;
	m_columnCount = GRID_COL_COUNT;

	KCL::int32 scissor_x = GRID_ICON_SCISSOR_START_X[gridType];
	KCL::int32 scissor_y = GRID_ICON_SCISSOR_START_Y[gridType];
	
	const double tx = scissor_x + (GRID_ICON_SCISSOR_SZ - (KCL::uint32)GRID_ICON_SZ)/2;

	//*
	const double ty = (FULLSCREEN_RELATIVE == gridType ? VIRTUAL_WIN_HEIGHT : PAGE_VIEWPORT_HEIGHT) - GRID_ICON_SCISSOR_SZ - GRID_ICON_SCISSOR_START_Y[gridType];
	//*/
	/*
	const double ty = (FULLSCREEN_RELATIVE == gridType ? VIRTUAL_WIN_HEIGHT : PAGE_WIN_HEIGHT) - GRID_ICON_SCISSOR_SZ - GRID_ICON_SCISSOR_START_Y[gridType];
	//*/

	for(size_t row = 0; row < m_rowCount; ++row)
	{
		scissor_x = GRID_ICON_SCISSOR_START_X[gridType];
		for(size_t col = 0; col < m_columnCount; ++col)
		{
			IconCell iconCell;
			KCL::Matrix4x4 matrix;
			
			iconCell.AccessImageViewCell().SetScissor_x( scissor_x );
			iconCell.AccessImageViewCell().SetScissor_y( scissor_y );
			iconCell.AccessImageViewCell().SetScissor_w( GRID_ICON_SCISSOR_SZ );
			iconCell.AccessImageViewCell().SetScissor_h( GRID_ICON_SCISSOR_SZ );
			
			matrix.v11 = GRID_ICON_SZ;
			matrix.v22 = GRID_ICON_SZ;
			matrix.v41 = tx + col * GRID_ICON_SCISSOR_STEP_X;
			matrix.v42 = ty + row * GRID_ICON_SCISSOR_STEP_Y;
										
			iconCell.AccessImageViewCell().SetTransformation(matrix);
			iconCell.AccessImageViewCell().ReferToProjection( FULLSCREEN_RELATIVE == gridType ? FULL_SCREEN_PROJ_MAT : ANIMATED_SCREEN_PROJ_MAT);

			//TODO set iconCell.m_textViewCell

			m_iconCells.push_back(iconCell);

			scissor_x += GRID_ICON_SCISSOR_STEP_X;
		}
		scissor_y -= GRID_ICON_SCISSOR_STEP_Y;
	}

}


HotSeatArray::HotSeatArray()
{
	//idx 0 -> apps button
	{
		ImageViewCell imageViewCell;
		KCL::Matrix4x4 matrix;

		imageViewCell.SetScissor_x( HOTSEAT_ICON_SCISSOR_START_X );
		imageViewCell.SetScissor_y( HOTSEAT_ICON_SCISSOR_Y );
		imageViewCell.SetScissor_w( HOTSEAT_ICON_SCISSOR_SZ );
		imageViewCell.SetScissor_h( HOTSEAT_ICON_SCISSOR_SZ );

		matrix.v11 = HOTSEAT_APPS_ICON_SZ;
		matrix.v22 = HOTSEAT_APPS_ICON_SZ;
		matrix.v41 = (double)HOTSEAT_ICON_SCISSOR_START_X + ((double)HOTSEAT_ICON_SCISSOR_SZ - HOTSEAT_APPS_ICON_SZ)/2.0;
		matrix.v42 = VIRTUAL_WIN_HEIGHT - HOTSEAT_ICON_SCISSOR_SZ - HOTSEAT_ICON_SCISSOR_Y;
		
		imageViewCell.SetTransformation(matrix);
		imageViewCell.ReferToProjection(FULL_SCREEN_PROJ_MAT);

		m_imageViewCells.push_back(imageViewCell);
	}
	for(size_t i=0; i< HOTSEAT_COUNT; ++i)
	{
		if(i == HOTSEAT_COUNT / 2)
			continue;

		ImageViewCell imageViewCell;
		KCL::Matrix4x4 matrix;

		imageViewCell.SetScissor_x( HOTSEAT_ICON_SCISSOR_START_X + (i - HOTSEAT_COUNT / 2) * HOTSEAT_ICON_SCISSOR_STEP_X );
		imageViewCell.SetScissor_y( HOTSEAT_ICON_SCISSOR_Y );
		imageViewCell.SetScissor_w( HOTSEAT_ICON_SCISSOR_SZ );
		imageViewCell.SetScissor_h( HOTSEAT_ICON_SCISSOR_SZ );

		matrix.v11 = HOTSEAT_ICON_SZ;
		matrix.v22 = HOTSEAT_ICON_SZ;
		matrix.v41 = (double)imageViewCell.Scissor_x() + ((double)HOTSEAT_ICON_SCISSOR_SZ - HOTSEAT_ICON_SZ)/2;
		matrix.v42 = VIRTUAL_WIN_HEIGHT - HOTSEAT_ICON_SCISSOR_SZ - HOTSEAT_ICON_SCISSOR_Y;
				 
		imageViewCell.SetTransformation(matrix);
		imageViewCell.ReferToProjection(FULL_SCREEN_PROJ_MAT);

		m_imageViewCells.push_back(imageViewCell);
	}
}


TextureHolder::TextureHolder(std::string name, TextureHolderFilter filter, TextureHolderUnpackAlginment alignment) : TextureReferer(0), m_texture(0)
{
	GLB::Image2D *img = 0;
	img = new GLB::Image2D;

	if( !img->load( name.c_str()))
	{
		delete img;
		img = 0;
#ifdef WIN32
		printf("ERROR in TextureHolder::TextureHolder! !img->load( name.c_str()), %s file couldn't loaded", name.c_str());
		exit(-1);
#endif
		//TODO: some sensible way to report error and quit
		abort();
	}

	m_texture = new GLB::Texture2D(img);
	
	glPixelStorei( GL_UNPACK_ALIGNMENT, alignment);

	img->commit();

	if(NEAREST == filter)
		m_texture->setFiltering (GLB::Texture::FILTER_BASE_LEVEL, GLB::Texture::FILTER_NEAREST);
	else
		m_texture->setFiltering (GLB::Texture::FILTER_BASE_LEVEL, GLB::Texture::FILTER_LINEAR);

	m_texture->setWrapping(GLB::Texture::WRAP_CLAMP, GLB::Texture::WRAP_CLAMP); //this is the default

	m_texture->bind();
	glBindTexture (GL_TEXTURE_2D, 0);

	m_texture_id = img->getId ();

	//glPixelStorei( GL_UNPACK_ALIGNMENT, 4); //TODO : set it back to 4 or 1 or leave it as is?
}


TextureHolder::~TextureHolder()
{
	delete m_texture;
}


FBO_Holder::FBO_Holder(KCL::uint32 width, KCL::uint32 height, FBO_Holder_ColorMode mode) : m_FBO(0), m_textureReferer(0)
{

	GLB::FBO_COLORMODE colormode = (mode == RGB ? GLB::RGB888_Linear : GLB::RGBA8888_Nearest);

	try
	{
		m_FBO = new GLB::FBO(width, height, 0, colormode, GLB::DEPTH_None, "FBO_holder") ;
	}
	catch (...)
	{
		m_FBO = 0;
	}

	m_textureReferer = new TextureReferer(m_FBO->getTextureName());
}


FBO_Holder::~FBO_Holder()
{
	GLB::FBO::bind(0);
	delete m_FBO;
	delete m_textureReferer;
}


void FBO_Holder::Bind()
{
	GLB::FBO::bind(m_FBO);
}


void FBO_Holder::Unbind()
{
	GLB::FBO::bind(0);
}


View::~View() {}


void View::Draw(const KCL::Matrix4x4 &transformation) const {}


void ImageView::Draw(const KCL::Matrix4x4 &transformation) const
{
#ifdef WIN32
	if(m_program == NONE)
	{
		printf("ERROR in ImageView::Draw! m_program == NONE");
		exit(-1);
	}

	if(0 == m_imageViewCell)
	{
		printf("ERROR in ImageView::Draw! 0 == m_imageViewCell");
		exit(-1);
	}

	if(0 == m_imageViewVBOAttribs)
	{
		printf("ERROR in ImageView::Draw! 0 == m_imageViewVBOAttribs");
		exit(-1);
	}

	if(0 == m_textureReferer)
	{
		printf("ERROR in ImageView::Draw! 0 == m_textureReferer");
		exit(-1);
	}

	if(0 == m_colorHolder)
	{
		printf("ERROR in ImageView::Draw! 0 == m_colorHolder");
		exit(-1);
	}
#endif

	if(0 == m_imageViewVBOAttribs->GetVbo())
	{
		Launcher2AttribAndVBORegistry::BindVBO( 0 );
	}
	glScissor (m_imageViewCell->Scissor_x(), m_imageViewCell->Scissor_y(), m_imageViewCell->Scissor_w(), m_imageViewCell->Scissor_h());
	
	/*DEBUG*/
	/*
	if(transformation.v33 > 100)
	{

		glScissor(100, 200, 100, 100);
		glClearColor(1,0,0,1);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glScissor(100, 100, 100, 100);
		glClearColor(1,1,1,1);
		glClear(GL_COLOR_BUFFER_BIT);

		glScissor(100, 0, 100, 100);
		glClearColor(0,1,0,1);
		glClear(GL_COLOR_BUFFER_BIT);

		glClearColor(0,0,0,0);
	}
	//*/
	
	/*
	if(transformation.v33 > 100 && transformation.v33 < 2000)
	{
		glClearColor(1,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0,0,0,0);
		//return;
	}
	if(transformation.v33 > 1000)
	{
		glClearColor(0,1,0,0);
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0,0,0,0);
		//return;
	}
	//*/
	/**/
	
	Launcher2ShaderManager::Instance()->UseProgram(m_program);
	Launcher2ShaderManager::Instance()->UniformMatrix4fv(PROJECTION, m_imageViewCell->Projection());
	Launcher2ShaderManager::Instance()->UniformMatrix4fv(TRANSFORMATION, m_imageViewCell->Transformation() * transformation);
	Launcher2ShaderManager::Instance()->Uniform4f(m_colorHolder->GetColor());
	glBindTexture(GL_TEXTURE_2D, m_textureReferer->Id());
	
	if(m_imageViewVBOAttribs->GetVbo())
	{
		Launcher2AttribAndVBORegistry::BindVBO( m_imageViewVBOAttribs->GetVbo() );
	}
	Launcher2AttribAndVBORegistry::VertexAttribPointer(VERTEX, m_imageViewVBOAttribs->GetSize(VERTEX), m_imageViewVBOAttribs->GetType (VERTEX), m_imageViewVBOAttribs->GetNormalized (VERTEX), m_imageViewVBOAttribs->GetStride (VERTEX) , m_imageViewVBOAttribs->GetPointer (VERTEX));
	Launcher2AttribAndVBORegistry::VertexAttribPointer(TEXTURECOORD, m_imageViewVBOAttribs->GetSize(TEXTURECOORD), m_imageViewVBOAttribs->GetType (TEXTURECOORD), m_imageViewVBOAttribs->GetNormalized (TEXTURECOORD), m_imageViewVBOAttribs->GetStride (TEXTURECOORD) , m_imageViewVBOAttribs->GetPointer (TEXTURECOORD));

	/*DEBUG*///Launcher2AttribAndVBORegistry::VertexAttribPointer(VERTEX, m_imageViewVBOAttribs->GetSize(VERTEX), m_imageViewVBOAttribs->GetType (VERTEX), m_imageViewVBOAttribs->GetNormalized (VERTEX), m_imageViewVBOAttribs->GetStride (VERTEX) , (const void *)ZERO_ONE_SQUARE);
	/*DEBUG*///Launcher2AttribAndVBORegistry::VertexAttribPointer(TEXTURECOORD, m_imageViewVBOAttribs->GetSize(TEXTURECOORD), m_imageViewVBOAttribs->GetType (TEXTURECOORD), m_imageViewVBOAttribs->GetNormalized (TEXTURECOORD), m_imageViewVBOAttribs->GetStride (TEXTURECOORD) , (const void *)&(ZERO_ONE_SQUARE[2]));
	
	Launcher2AttribAndVBORegistry::BindEBO(0);
	glDrawArrays(m_imageViewVBOAttribs->GetMode(), m_imageViewVBOAttribs->GetFirst(), m_imageViewVBOAttribs->GetCount());
}


/* TODO IMPLEMENT TextView's stuff
TextView::
//*/


Launcher2ShaderManager::Launcher2ShaderManager()
{
	m_vertex_shader[0] = 0;
	m_vertex_shader[1] = 0;
	m_vertex_shader[2] = 0;
	
	m_fragment_shader[0] = 0;
	m_fragment_shader[1] = 0;
	m_fragment_shader[2] = 0;
	
	m_shader_program[0] = 0;
	m_shader_program[1] = 0;
	m_shader_program[2] = 0;
	m_shader_program[3] = 0;

	m_uniformMatrixLocations[0][0] = -1;
	m_uniformMatrixLocations[1][0] = -1;
	m_uniformMatrixLocations[2][0] = -1;
	m_uniformMatrixLocations[3][0] = -1;

	m_uniformMatrixLocations[0][1] = -1;
	m_uniformMatrixLocations[1][1] = -1;
	m_uniformMatrixLocations[2][1] = -1;
	m_uniformMatrixLocations[3][1] = -1;

	m_uniformColorLocations[0] = -1;
	m_uniformColorLocations[1] = -1;
	m_uniformColorLocations[2] = -1;
	m_uniformColorLocations[3] = -1;

	m_uniformSamplerLocations[0] = -1;
	m_uniformSamplerLocations[1] = -1;
	m_uniformSamplerLocations[2] = -1;
	m_uniformSamplerLocations[3] = -1;

	m_actual_program = NONE;

	KCL::AssetFile vs0("GUIBenchmark/gui0.vs");
	KCL::AssetFile fs0("GUIBenchmark/gui0.fs");
	KCL::AssetFile fs1("GUIBenchmark/gui1.fs");
	KCL::AssetFile fs2("GUIBenchmark/gui2.fs");
	
	const char* vs_str = vs0.GetBuffer();
	const char* fs_str[3] =
	{
		fs0.GetBuffer(),
		fs1.GetBuffer(),
		fs2.GetBuffer()
	};

	for(size_t i=0; i<3; ++i)
	{
		
		m_vertex_shader[i] = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource (m_vertex_shader[i], 1, (const char **) (&vs_str)  , 0);
		glCompileShader (m_vertex_shader[i]);
				
		m_fragment_shader[i] = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource (m_fragment_shader[i], 1, (const char **) (&(fs_str[i]))  , 0);
		glCompileShader (m_fragment_shader[i]);
				
		m_shader_program[i+1] = glCreateProgram();
		glAttachShader(m_shader_program[i+1], m_vertex_shader[i]);
		glAttachShader(m_shader_program[i+1], m_fragment_shader[i]);
		
		glBindAttribLocation(m_shader_program[i+1], 0, "position");
		glBindAttribLocation(m_shader_program[i+1], 1, "texCoords");
		
		glLinkProgram (m_shader_program[i+1]);
				
		m_uniformMatrixLocations[i+1][PROJECTION] = glGetUniformLocation(m_shader_program[i+1], "projection");
		m_uniformMatrixLocations[i+1][TRANSFORMATION] = glGetUniformLocation(m_shader_program[i+1], "transform");
		m_uniformColorLocations[i+1]  = glGetUniformLocation(m_shader_program[i+1], "color");
		m_uniformSamplerLocations[i+1] = glGetUniformLocation(m_shader_program[i+1], "sampler");
	}
}


Launcher2ShaderManager::~Launcher2ShaderManager()
{
	glUseProgram(0);

	glDeleteProgram(m_shader_program [1]);
	glDeleteShader (m_vertex_shader  [0]);
	glDeleteShader (m_fragment_shader[0]);

	glDeleteProgram(m_shader_program [2]);
	glDeleteShader (m_vertex_shader  [1]);
	glDeleteShader (m_fragment_shader[1]);

	glDeleteProgram(m_shader_program [3]);
	glDeleteShader (m_vertex_shader  [2]);
	glDeleteShader (m_fragment_shader[2]);
}


void Launcher2ShaderManager::UseProgram(const Launcher2ShaderManagerPrograms program) const
{
	if(program != m_actual_program)
	{
		glUseProgram(m_shader_program[program]);

		m_actual_program = program;
		Uniform1i(0);
	}
}


void Launcher2ShaderManager::UniformMatrix4fv(Launcher2ShaderManagerMatrices which, const KCL::Matrix4x4& m) const
{
	if( -1 != m_uniformMatrixLocations[m_actual_program][which])
	{
		glUniformMatrix4fv(m_uniformMatrixLocations[m_actual_program][which], 1, 0, m);
	}
}


void Launcher2ShaderManager::Uniform4f(const KCL::Vector4D &v) const
{
	if( -1 != m_uniformColorLocations[m_actual_program])
	{
		glUniform4f(m_uniformColorLocations[m_actual_program], v.x, v.y, v.z, v.w);
	}
}


void Launcher2ShaderManager::Uniform1i(KCL::int32 i) const
{
	if( -1 != m_uniformSamplerLocations[m_actual_program])
	{
		glUniform1i(m_uniformSamplerLocations[m_actual_program], i);
	}
}


void Launcher2ShaderManager::Init()
{
	delete s_instance;
	s_instance = new Launcher2ShaderManager;
}


void Launcher2ShaderManager::Delete()
{
	delete s_instance;
	s_instance = 0;
}


void Launcher2AttribAndVBORegistry::Init()
{
	s_actual_vbo = 0;
	s_actual_ebo = 0;
	s_actual_vertex_pointer[0] = 0;
	s_actual_vertex_pointer[1] = 0;
	s_attribpointer_dirty[0] = true;
	s_attribpointer_dirty[1] = true;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void Launcher2AttribAndVBORegistry::BindVBO(const KCL::uint32 id)
{
	if(id != s_actual_vbo)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id);
		s_actual_vbo = id;
		s_attribpointer_dirty[0] = true;
		s_attribpointer_dirty[1] = true;
	}
}


void Launcher2AttribAndVBORegistry::BindEBO(const KCL::uint32 id)
{
	if(id != s_actual_ebo)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
		s_actual_ebo = id;
	}
}


void Launcher2AttribAndVBORegistry::VertexAttribPointer(AttribPtrType t, KCL::int32 size, KCL::int32 type, bool normalized, KCL::int32 stride, const void* ptr)
{
	if(ptr != s_actual_vertex_pointer[t] || s_attribpointer_dirty[t])
	{
		glVertexAttribPointer(t, size, type, normalized, stride, ptr);
		s_actual_vertex_pointer[t] = ptr;
		s_attribpointer_dirty[t] = false;
	}
}









Launcher2Simulation::Launcher2Simulation(KCL::uint32 winwidth, KCL::uint32 winheight, KCL::uint32 testLengthMillisec):
	m_winwidth (winwidth),
	m_winheight (winheight),
	m_actual_page (0),
	m_prev_time (0),
	m_elapsed_time (0),
	m_diff_accu (0),
	m_drag_percent (0.0),
	m_drawMode (DRAG_MODE),
	m_zero_one_square_vbo (0),
	m_gradient_vbo (0),
	m_navigationStripe_vbo (0),
	m_searchBarBackground_vbo (0),
	m_virtual_screen (0)
{
	m_virtual_page[0] = 0;
	m_virtual_page[1] = 0;
	
	m_iconPageGrid[0] = 0;
	m_iconPageGrid[1] = 0;


	//glClearColor(0,0,0,1);

	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);
	//glEnable(GL_SCISSOR_TEST);
	//glEnable(GL_BLEND);
	//glBlendEquation(GL_FUNC_ADD);
	//glBlendFunc(1, GL_ONE_MINUS_SRC_ALPHA);

	//glActiveTexture(GL_TEXTURE0);

	//glEnableVertexAttribArray(0);
	//glEnableVertexAttribArray(1);

	Launcher2ShaderManager::Init();
	Launcher2AttribAndVBORegistry::Init();

	m_virtual_screen = new FBO_Holder(VIRTUAL_WIN_WIDTH, VIRTUAL_WIN_HEIGHT, RGB);
	m_virtual_page[0] = new FBO_Holder(PAGE_WIN_WIDTH, PAGE_WIN_HEIGHT, RGBA);
	m_virtual_page[1] = new FBO_Holder(PAGE_WIN_WIDTH, PAGE_WIN_HEIGHT, RGBA);

	if(FBOsReady())
	{
		//TODO: call factories

		createVBOs();

		m_iconPageGrid[0] = new IconPageGrid(FULLSCREEN_RELATIVE);
		m_iconPageGrid[1] = new IconPageGrid(ANIMATEDSCREEN_RELATIVE );

		FCreate_ImageViewVBOAttribs_01_Square_WithVBO();
		FCreate_ImageViewVBOAttribs_01_Square_NOT_VBO();
		FCreate_ImageViewVBOAttribs_Gradient();
		FCreate_ImageViewVBOAttribs_NavigationStripe();
		FCreate_ImageViewVBOAttribs_SearchBarBackground();

		FCreateWallpaper();
		FCreateGradient();
		FCreateNavigationStripe();
		FCreateSearchBar();
		FCreateHotseat();
		
		FCreatePagesOfIcons("a");
		FCreatePagesOfIcons("b");
		
		FCreateVirtualScreen(winwidth, winheight);
		FCreateVirtualPages();
	}
}


Launcher2Simulation::~Launcher2Simulation()
{
	Launcher2ShaderManager::Delete();
	
	DeletePointerVector(m_pages_of_icons[0]);
	DeletePointerVector(m_pages_of_icons[1]);
	DeletePointerVector(m_scroll_decoratives);	
	DeletePointerVector(m_textureReferers);
	
	delete m_virtual_screen;

	delete m_virtual_page[0];
	delete m_virtual_page[1];
	
	delete m_iconPageGrid[0];
	delete m_iconPageGrid[1];

	deleteVBOs();
}


void Launcher2Simulation::Simulate(unsigned int time)
{
	if(m_prev_time)
	{
		m_diff_accu += time - m_prev_time;
		m_elapsed_time += time - m_prev_time;
		m_prev_time = time;
	}
	else
	{
		m_prev_time = time;
	}


	//TODO

	/*DEBUG -- PREALPHA*/

	const unsigned int drag_time = 1000;
	const unsigned int static_time = 200; 
	
	if(dragMode())
	{
		if(m_diff_accu > drag_time)
		{
			m_diff_accu = m_diff_accu % drag_time;

			m_drawMode = STATIC_MODE;

			m_actual_page = 1 - m_actual_page;
			m_drag_percent = m_actual_page;
		}
		else
		{
			m_drag_percent = m_actual_page ? 1-(double)m_diff_accu / (double)drag_time : (double)m_diff_accu / (double)drag_time;
		}
	}	
	else if(staticMode())
	{
		if(m_diff_accu > static_time)
		{
			m_diff_accu = m_diff_accu % static_time;

			m_drawMode = DRAG_MODE;

			m_drag_percent = m_actual_page ? 1-(double)m_diff_accu / (double)drag_time : (double)m_diff_accu / (double)drag_time;
		}
	}
	
	if(dragMode())
	{
		updateWallpaperMatrix();
		updateVirtualPages();
	}

	/*DEBUG -- PREALPHA*/

}


void Launcher2Simulation::Render() const
{
	/*DEBUG*/
	//GLB::OpenGLStateManager::Save();
	/*DEBUG*/


	static const KCL::Matrix4x4 identity;

	/* RESET ENVIRONMENT */
	Launcher2ShaderManager::Instance()->UseProgram(NONE);
	Launcher2AttribAndVBORegistry::Init();
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);	
	glEnable(GL_SCISSOR_TEST);
	
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		
	glActiveTexture(GL_TEXTURE0);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glClearColor(0,0,0,0);

	//TODO: set everything back as it was at the end of Render() !!!

	/* RESET ENVIRONMENT -- END */
	


	m_virtual_screen->Bind();
	glViewport(0, 0, VIRTUAL_WIN_WIDTH, VIRTUAL_WIN_HEIGHT);

	glEnable(GL_BLEND);

	glScissor(0, 0, VIRTUAL_WIN_WIDTH, VIRTUAL_WIN_HEIGHT);
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	/*debug*/
	//glClearColor(0,1,0,0);
	//glClear(GL_COLOR_BUFFER_BIT);
	//glClearColor(0,0,0,0);
	/*debug*/

	
	m_wallpaper.Draw(ID_MATRIX);

	m_gradient.Draw(ID_MATRIX);

	m_navigationStripe.Draw(ID_MATRIX);

	if(staticMode())
	{
		m_pages_of_icons[FULLSCREEN_RELATIVE][m_actual_page]->Draw(ID_MATRIX);
	}
	
	if(dragMode())
	{
		//TODO

		//1.) BLUE NAV STRIPE

		//2.) VIRTUAL PAGES with FRAMES
		for(size_t i=0; i<2; ++i)
		{
			//TODO: draw the blueframes somewhere

			m_virtual_page[i]->Bind();

			glViewport(0, 0, PAGE_WIN_WIDTH, PAGE_WIN_HEIGHT);
			glScissor(0, 0, PAGE_WIN_WIDTH, PAGE_WIN_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT);
			
			m_pages_of_icons[ANIMATEDSCREEN_RELATIVE][i]->Draw(ID_MATRIX);
		}


		//3.) DRAW VIRTUAL PAGES
		m_virtual_screen->Bind();
		glViewport(0, 0, VIRTUAL_WIN_WIDTH, VIRTUAL_WIN_HEIGHT);
		
		m_virtual_page_view[0].Draw(ID_MATRIX);
		m_virtual_page_view[1].Draw(ID_MATRIX);
		//TODO ??
	}
	
	if (tiltMode())
	{
		//TODO
	}


	m_hotseat.Draw(ID_MATRIX);
	m_searchBar.Draw(ID_MATRIX);


	FBO_Holder::Unbind();
	glViewport(0, 0, m_winwidth, m_winheight);
	glScissor(0, 0, m_winwidth, m_winheight);
	glClear(GL_COLOR_BUFFER_BIT);
	m_virtual_screen_view.Draw(ID_MATRIX);


	/*DEBUG*/
	Launcher2ShaderManager::Instance()->UseProgram(NONE);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisable(GL_SCISSOR_TEST);
	
	GLB::OpenGLStateManager::Reset();
	//GLB::OpenGLStateManager::Restore();
	//GLB::OpenGLStateManager::Commit();

	/*DEBUG*/
}


bool Launcher2Simulation::FBOsReady() const
{
	return
		m_virtual_screen->Ready()
		&& m_virtual_page[0]->Ready()
		&& m_virtual_page[1]->Ready();
}


void Launcher2Simulation::createVBOs()
{
	glGenBuffers(1, &m_zero_one_square_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_zero_one_square_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ZERO_ONE_SQUARE), (const void*)ZERO_ONE_SQUARE, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_gradient_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_gradient_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GRADIENT_VBO_DATA), (const void*)GRADIENT_VBO_DATA, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &m_navigationStripe_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_navigationStripe_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(NAVIGATION_STRIPE_VBO_DATA), (const void*)NAVIGATION_STRIPE_VBO_DATA, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &m_searchBarBackground_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_searchBarBackground_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SEARCH_BAR_BG_VBO_DATA), (const void*)SEARCH_BAR_BG_VBO_DATA, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//TODO OTHER BUFFERS 
}


void Launcher2Simulation::deleteVBOs()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if(m_zero_one_square_vbo)
		glDeleteBuffers(1, &m_zero_one_square_vbo);

	if(m_gradient_vbo)
		glDeleteBuffers(1, &m_gradient_vbo);

	if(m_navigationStripe_vbo)
		glDeleteBuffers(1, &m_navigationStripe_vbo);

	if(m_searchBarBackground_vbo)
		glDeleteBuffers(1, &m_searchBarBackground_vbo);
	
	//TODO OTHER BUFFERS
}


void Launcher2Simulation::FCreate_ImageViewVBOAttribs_01_Square_WithVBO()
{
	m_ImageViewVBOAttribs_01_Square_WithVBO.SetVbo (m_zero_one_square_vbo);
	
	m_ImageViewVBOAttribs_01_Square_WithVBO.SetSize (VERTEX, 2);
	m_ImageViewVBOAttribs_01_Square_WithVBO.SetSize (TEXTURECOORD, 2);

	m_ImageViewVBOAttribs_01_Square_WithVBO.SetType (VERTEX, GL_FLOAT);
	m_ImageViewVBOAttribs_01_Square_WithVBO.SetType (TEXTURECOORD, GL_FLOAT);

	m_ImageViewVBOAttribs_01_Square_WithVBO.SetNormalized (VERTEX, false);
	m_ImageViewVBOAttribs_01_Square_WithVBO.SetNormalized (TEXTURECOORD, false);
	
	m_ImageViewVBOAttribs_01_Square_WithVBO.SetStride (VERTEX, 16);
	m_ImageViewVBOAttribs_01_Square_WithVBO.SetStride (TEXTURECOORD, 16);

	m_ImageViewVBOAttribs_01_Square_WithVBO.SetPointer (VERTEX, (const void*)0);
	m_ImageViewVBOAttribs_01_Square_WithVBO.SetPointer (TEXTURECOORD, (const void*)(sizeof(float)*2));

	m_ImageViewVBOAttribs_01_Square_WithVBO.SetMode (GL_TRIANGLE_STRIP);
	m_ImageViewVBOAttribs_01_Square_WithVBO.SetFirst (0);
	m_ImageViewVBOAttribs_01_Square_WithVBO.SetCount (4);
}


void Launcher2Simulation::FCreate_ImageViewVBOAttribs_01_Square_NOT_VBO()
{
	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetVbo (0);
	
	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetSize (VERTEX, 2);
	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetSize (TEXTURECOORD, 2);

	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetType (VERTEX, GL_FLOAT);
	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetType (TEXTURECOORD, GL_FLOAT);

	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetNormalized (VERTEX, false);
	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetNormalized (TEXTURECOORD, false);
	
	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetStride (VERTEX, 16);
	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetStride (TEXTURECOORD, 16);

	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetPointer (VERTEX, (const void*)ZERO_ONE_SQUARE );
	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetPointer (TEXTURECOORD, (const void*)&(ZERO_ONE_SQUARE[2]));

	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetMode (GL_TRIANGLE_STRIP);
	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetFirst (0);
	m_ImageViewVBOAttribs_01_Square_NOT_VBO.SetCount (4);
}


void Launcher2Simulation::FCreate_ImageViewVBOAttribs_Gradient()
{
	m_ImageViewVBOAttribs_Gradient.SetVbo (m_gradient_vbo);
	
	m_ImageViewVBOAttribs_Gradient.SetSize (VERTEX, 2);
	m_ImageViewVBOAttribs_Gradient.SetSize (TEXTURECOORD, 2);

	m_ImageViewVBOAttribs_Gradient.SetType (VERTEX, GL_FLOAT);
	m_ImageViewVBOAttribs_Gradient.SetType (TEXTURECOORD, GL_FLOAT);

	m_ImageViewVBOAttribs_Gradient.SetNormalized (VERTEX, false);
	m_ImageViewVBOAttribs_Gradient.SetNormalized (TEXTURECOORD, false);
	
	m_ImageViewVBOAttribs_Gradient.SetStride (VERTEX, 16);
	m_ImageViewVBOAttribs_Gradient.SetStride (TEXTURECOORD, 16);

	m_ImageViewVBOAttribs_Gradient.SetPointer (VERTEX, (const void*)0);
	m_ImageViewVBOAttribs_Gradient.SetPointer (TEXTURECOORD, (const void*)(sizeof(float)*2));

	m_ImageViewVBOAttribs_Gradient.SetMode (GL_TRIANGLES);
	m_ImageViewVBOAttribs_Gradient.SetFirst (0);
	m_ImageViewVBOAttribs_Gradient.SetCount (12);
}


void Launcher2Simulation::FCreate_ImageViewVBOAttribs_NavigationStripe()
{
	m_ImageViewVBOAttribs_NavigationStripe.SetVbo (m_navigationStripe_vbo);
	
	m_ImageViewVBOAttribs_NavigationStripe.SetSize (VERTEX, 2);
	m_ImageViewVBOAttribs_NavigationStripe.SetSize (TEXTURECOORD, 2);

	m_ImageViewVBOAttribs_NavigationStripe.SetType (VERTEX, GL_FLOAT);
	m_ImageViewVBOAttribs_NavigationStripe.SetType (TEXTURECOORD, GL_FLOAT);

	m_ImageViewVBOAttribs_NavigationStripe.SetNormalized (VERTEX, false);
	m_ImageViewVBOAttribs_NavigationStripe.SetNormalized (TEXTURECOORD, false);
	
	m_ImageViewVBOAttribs_NavigationStripe.SetStride (VERTEX, 16);
	m_ImageViewVBOAttribs_NavigationStripe.SetStride (TEXTURECOORD, 16);

	m_ImageViewVBOAttribs_NavigationStripe.SetPointer (VERTEX, (const void*)0);
	m_ImageViewVBOAttribs_NavigationStripe.SetPointer (TEXTURECOORD, (const void*)(sizeof(float)*2));

	m_ImageViewVBOAttribs_NavigationStripe.SetMode (GL_TRIANGLES);
	m_ImageViewVBOAttribs_NavigationStripe.SetFirst (0);
	m_ImageViewVBOAttribs_NavigationStripe.SetCount (54);
}


void Launcher2Simulation::FCreate_ImageViewVBOAttribs_SearchBarBackground()
{
	m_ImageViewVBOAttribs_SearchBarBackground.SetVbo (m_searchBarBackground_vbo);
	
	m_ImageViewVBOAttribs_SearchBarBackground.SetSize (VERTEX, 2);
	m_ImageViewVBOAttribs_SearchBarBackground.SetSize (TEXTURECOORD, 2);

	m_ImageViewVBOAttribs_SearchBarBackground.SetType (VERTEX, GL_FLOAT);
	m_ImageViewVBOAttribs_SearchBarBackground.SetType (TEXTURECOORD, GL_FLOAT);

	m_ImageViewVBOAttribs_SearchBarBackground.SetNormalized (VERTEX, false);
	m_ImageViewVBOAttribs_SearchBarBackground.SetNormalized (TEXTURECOORD, false);
	
	m_ImageViewVBOAttribs_SearchBarBackground.SetStride (VERTEX, 16);
	m_ImageViewVBOAttribs_SearchBarBackground.SetStride (TEXTURECOORD, 16);

	m_ImageViewVBOAttribs_SearchBarBackground.SetPointer (VERTEX, (const void*)0);
	m_ImageViewVBOAttribs_SearchBarBackground.SetPointer (TEXTURECOORD, (const void*)(sizeof(float)*2));

	m_ImageViewVBOAttribs_SearchBarBackground.SetMode (GL_TRIANGLES);
	m_ImageViewVBOAttribs_SearchBarBackground.SetFirst (0);
	m_ImageViewVBOAttribs_SearchBarBackground.SetCount (54);
}


void Launcher2Simulation::FCreateWallpaper()
{
	std::string name = GUIB_DIRECTORY + WALLPAPER_FILE_NAME;

	TextureHolder *textureH = new TextureHolder(name, LINEAR, ONE);

	m_wallpaperImageViewCell.SetScissor_x( 0 );
	m_wallpaperImageViewCell.SetScissor_y( 0 );
	m_wallpaperImageViewCell.SetScissor_w( WALLPAPER_WIDTH );
	m_wallpaperImageViewCell.SetScissor_h( VIRTUAL_WIN_HEIGHT );
	m_wallpaperImageViewCell.ReferToProjection(FULL_SCREEN_PROJ_MAT);
	
	m_wallpaperImageViewCell.AccessTransformation().v11 = WALLPAPER_WIDTH;
	m_wallpaperImageViewCell.AccessTransformation().v22 = VIRTUAL_WIN_HEIGHT;

	m_wallpaper.SetProgram(ONLY_TEXTURE_PROG);
	m_wallpaper.SetColorHolder(&ZERO_COLOR);
	m_wallpaper.SetTextureReferer(textureH);
	m_wallpaper.SetImageViewCell( &m_wallpaperImageViewCell );
	m_wallpaper.SetImageViewVBOAttribs(&m_ImageViewVBOAttribs_01_Square_WithVBO);

	m_textureReferers.push_back(textureH);
}


void Launcher2Simulation::FCreateGradient()
{
	std::string name = GUIB_DIRECTORY + GRADIENT_FILE_NAME;

	TextureHolder *textureH = new TextureHolder(name, LINEAR, ONE);

	m_gradientImageViewCell.SetScissor_x( 0 );
	m_gradientImageViewCell.SetScissor_y( 0 );
	m_gradientImageViewCell.SetScissor_w( VIRTUAL_WIN_WIDTH );
	m_gradientImageViewCell.SetScissor_h( VIRTUAL_WIN_HEIGHT );
	m_gradientImageViewCell.ReferToProjection(FULL_SCREEN_PROJ_MAT);
	
	m_gradientImageViewCell.AccessTransformation().v22 = (double)VIRTUAL_WIN_HEIGHT / (double)1454.0;
	//m_gradientImageViewCell.AccessTransformation().v42 = 25;

	m_gradient.SetProgram(ONLY_TEXTURE_PROG);
	m_gradient.SetColorHolder(&ZERO_COLOR);
	m_gradient.SetTextureReferer(textureH);
	m_gradient.SetImageViewCell( &m_gradientImageViewCell );
	m_gradient.SetImageViewVBOAttribs(&m_ImageViewVBOAttribs_Gradient);

	m_textureReferers.push_back(textureH);
}


void Launcher2Simulation::FCreateNavigationStripe()
{
	std::string name = GUIB_DIRECTORY + NAVIGATION_STRIPE_FILE_NAME;

	TextureHolder *textureH = new TextureHolder(name, LINEAR, ONE);

	m_navigationStripeImageViewCell.SetScissor_x( 0 );
	m_navigationStripeImageViewCell.SetScissor_y( 164 );
	m_navigationStripeImageViewCell.SetScissor_w( VIRTUAL_WIN_WIDTH );
	m_navigationStripeImageViewCell.SetScissor_h( 12 );
	m_navigationStripeImageViewCell.ReferToProjection(FULL_SCREEN_PROJ_MAT);
	
	m_navigationStripeImageViewCell.AccessTransformation().v41 = 354;  // yes, magic numbers! from trace of Nexus 10
	m_navigationStripeImageViewCell.AccessTransformation().v42 = 1328; // yes, magic numbers! from trace of Nexus 10

	m_navigationStripe.SetProgram(ONLY_TEXTURE_PROG);
	m_navigationStripe.SetColorHolder(&ZERO_COLOR);
	m_navigationStripe.SetTextureReferer(textureH);
	m_navigationStripe.SetImageViewCell( &m_navigationStripeImageViewCell );
	m_navigationStripe.SetImageViewVBOAttribs(&m_ImageViewVBOAttribs_NavigationStripe);

	m_textureReferers.push_back(textureH);
}


void Launcher2Simulation::FCreateSearchBar()
{
	{
		std::string name = GUIB_DIRECTORY + SEARCH_BAR_BG_FILE_NAME;

		TextureHolder *textureH = new TextureHolder(name, LINEAR, ONE);

		m_searchBarBackgroundImageViewCell.SetScissor_x( 354 );
		m_searchBarBackgroundImageViewCell.SetScissor_y( 1334 );
		m_searchBarBackgroundImageViewCell.SetScissor_w( 1852 );
		m_searchBarBackgroundImageViewCell.SetScissor_h( 104 );
		m_searchBarBackgroundImageViewCell.ReferToProjection(FULL_SCREEN_PROJ_MAT);
	
		m_searchBarBackgroundImageViewCell.AccessTransformation().v41 = 354;  // yes, magic numbers! from trace of Nexus 10
		m_searchBarBackgroundImageViewCell.AccessTransformation().v42 = 66;   // yes, magic numbers! from trace of Nexus 10

		ImageView *imageView = new ImageView();
		imageView->SetProgram(ONLY_TEXTURE_PROG);
		imageView->SetColorHolder(&ZERO_COLOR);
		imageView->SetTextureReferer(textureH);
		imageView->SetImageViewCell( &m_searchBarBackgroundImageViewCell );
		imageView->SetImageViewVBOAttribs( &m_ImageViewVBOAttribs_SearchBarBackground );

		m_textureReferers.push_back(textureH);
		m_searchBar.PushAndOwnNew(imageView);
	}
	{
		std::string name = GUIB_DIRECTORY + SEARCH_BAR_FILE_NAME;

		TextureHolder *textureH = new TextureHolder(name, LINEAR, ONE);

		m_searchBarImageViewCell.SetScissor_x( 378 );
		m_searchBarImageViewCell.SetScissor_y( 1358 );
		m_searchBarImageViewCell.SetScissor_w( 196 );
		m_searchBarImageViewCell.SetScissor_h( 56 );
		m_searchBarImageViewCell.ReferToProjection(FULL_SCREEN_PROJ_MAT);

		m_searchBarImageViewCell.AccessTransformation().v11 = 196; // yes, magic numbers! from trace of Nexus 10
		m_searchBarImageViewCell.AccessTransformation().v22 = 56;  // yes, magic numbers! from trace of Nexus 10
		m_searchBarImageViewCell.AccessTransformation().v41 = 378; // yes, magic numbers! from trace of Nexus 10
		m_searchBarImageViewCell.AccessTransformation().v42 = 90;  // yes, magic numbers! from trace of Nexus 10
		
		ImageView *imageView = new ImageView();
		imageView->SetProgram(ONLY_TEXTURE_PROG);
		imageView->SetColorHolder(&ZERO_COLOR);
		imageView->SetTextureReferer(textureH);
		imageView->SetImageViewCell( &m_searchBarImageViewCell );
		imageView->SetImageViewVBOAttribs(&m_ImageViewVBOAttribs_01_Square_WithVBO);

		m_textureReferers.push_back(textureH);
		m_searchBar.PushAndOwnNew(imageView);
	}
}


void Launcher2Simulation::FCreateHotseat()
{
	std::string name = GUIB_DIRECTORY + HOTSEAT_ICON_FILE_PREFIX;
	std::string suffix = ".png";

	for(size_t i=0; i<m_hotSeatArray.Count(); ++i)
	{
		ImageView *imageView = new ImageView();

		std::ostringstream os;

		os << name;
		if(i<10) //TODO: this works only if hotseat icons are below 101, named from 00 till 99
			os << "0";
		os << i << suffix;

		std::string filename = os.str();
		
		TextureHolder *textureH = new TextureHolder(filename, NEAREST, FOUR);

		imageView->SetProgram(ONLY_TEXTURE_PROG);
		imageView->SetColorHolder(&ZERO_COLOR);
		imageView->SetTextureReferer(textureH);
		imageView->SetImageViewCell( &(m_hotSeatArray.GetImageViewCell(i)) );

		if(i)
		{
			imageView->SetImageViewVBOAttribs(&m_ImageViewVBOAttribs_01_Square_WithVBO);
		}
		else
		{
			imageView->SetImageViewVBOAttribs(&m_ImageViewVBOAttribs_01_Square_NOT_VBO);
		}

		m_textureReferers.push_back(textureH);
		m_hotseat.PushAndOwnNew(imageView);
	}
}


void Launcher2Simulation::FCreatePagesOfIcons(const char *pageName)
{
	std::string name = GUIB_DIRECTORY + GRID_ICON_FILE_PREFIX;
	name += pageName;
	name += "_";
	std::string suffix = ".png";

	ViewGroup *pages[2];
	pages[FULLSCREEN_RELATIVE] = new ViewGroup();
	pages[ANIMATEDSCREEN_RELATIVE] = new ViewGroup();

	size_t i = 0;

	
	for(size_t row=0; row < m_iconPageGrid[0]->RowCount(); ++row)
	{
		for(size_t col = 0; col < m_iconPageGrid[0]->ColumnCount(); ++col)
		{
			BubbleTextView* bubbleTextViews[2];
			bubbleTextViews[FULLSCREEN_RELATIVE] = new BubbleTextView();
			bubbleTextViews[ANIMATEDSCREEN_RELATIVE] = new BubbleTextView();

			ImageView *imageViews[2];
			imageViews[FULLSCREEN_RELATIVE] = new ImageView();
			imageViews[ANIMATEDSCREEN_RELATIVE] = new ImageView();
			{
				std::ostringstream os;

				os << name;
				if(i<10) //TODO: this works only if icons are below 101, named from 00 till 99
					os << "0";
				os << i << suffix;
				++i;

				std::string filename = os.str();
		
				TextureHolder *textureH = new TextureHolder(filename, NEAREST, FOUR);

				imageViews[FULLSCREEN_RELATIVE]->SetProgram(ONLY_TEXTURE_PROG);
				imageViews[FULLSCREEN_RELATIVE]->SetColorHolder(&ZERO_COLOR);
				imageViews[FULLSCREEN_RELATIVE]->SetTextureReferer(textureH);
				imageViews[FULLSCREEN_RELATIVE]->SetImageViewCell( &(m_iconPageGrid[FULLSCREEN_RELATIVE]->GetIconCell(row, col).GetImageViewCell()) );
				imageViews[FULLSCREEN_RELATIVE]->SetImageViewVBOAttribs(&m_ImageViewVBOAttribs_01_Square_WithVBO);

				imageViews[ANIMATEDSCREEN_RELATIVE]->SetProgram(ONLY_TEXTURE_PROG);
				imageViews[ANIMATEDSCREEN_RELATIVE]->SetColorHolder(&ZERO_COLOR);
				imageViews[ANIMATEDSCREEN_RELATIVE]->SetTextureReferer(textureH);
				imageViews[ANIMATEDSCREEN_RELATIVE]->SetImageViewCell( &(m_iconPageGrid[ANIMATEDSCREEN_RELATIVE]->GetIconCell(row, col).GetImageViewCell()) );
				imageViews[ANIMATEDSCREEN_RELATIVE]->SetImageViewVBOAttribs(&m_ImageViewVBOAttribs_01_Square_WithVBO);

				m_textureReferers.push_back(textureH);
			}
			bubbleTextViews[FULLSCREEN_RELATIVE]->ReplaceImageView(imageViews[FULLSCREEN_RELATIVE]);
			bubbleTextViews[ANIMATEDSCREEN_RELATIVE]->ReplaceImageView(imageViews[ANIMATEDSCREEN_RELATIVE]);

			//TODO: TextView
			
			pages[FULLSCREEN_RELATIVE]->PushAndOwnNew(bubbleTextViews[FULLSCREEN_RELATIVE]);
			pages[ANIMATEDSCREEN_RELATIVE]->PushAndOwnNew(bubbleTextViews[ANIMATEDSCREEN_RELATIVE]);
		}
	}


	m_pages_of_icons[FULLSCREEN_RELATIVE].push_back( pages[FULLSCREEN_RELATIVE] );
	m_pages_of_icons[ANIMATEDSCREEN_RELATIVE].push_back( pages[ANIMATEDSCREEN_RELATIVE] );
}


void Launcher2Simulation::FCreateVirtualScreen(KCL::uint32 winwidth, KCL::uint32 winheight)
{
	const bool rotate = winwidth < winheight;
	if(rotate)
	{
		KCL::uint32 tmp = winwidth;
		winwidth = winheight;
		winheight = tmp;
	}

	double scale = 1.0;
	bool horizontal_black_bars = true;
	KCL::Matrix4x4 mat;

	{
		const double virtual_aspect = (double)VIRTUAL_WIN_WIDTH / (double)VIRTUAL_WIN_HEIGHT;
		const double target_aspect = (double)winwidth / (double)winheight;

		if(virtual_aspect >= target_aspect)
		{
			scale = (double)winwidth / (double)VIRTUAL_WIN_WIDTH;
		}
		else
		{
			horizontal_black_bars = false;
			scale = (double)winheight / (double)VIRTUAL_WIN_HEIGHT;
		}
	}

	if(scale > 1.0)
	{
		m_virtualScreenImageViewCell.SetScissor_x( (winwidth -VIRTUAL_WIN_WIDTH ) / 2  * (1-rotate) + (winheight-VIRTUAL_WIN_HEIGHT) / 2  * rotate );
		m_virtualScreenImageViewCell.SetScissor_y( (winheight-VIRTUAL_WIN_HEIGHT) / 2  * (1-rotate) + (winwidth -VIRTUAL_WIN_WIDTH ) / 2  * rotate );
		m_virtualScreenImageViewCell.SetScissor_w( VIRTUAL_WIN_WIDTH  * (1-rotate) + VIRTUAL_WIN_HEIGHT * rotate );
		m_virtualScreenImageViewCell.SetScissor_h( VIRTUAL_WIN_HEIGHT * (1-rotate) + VIRTUAL_WIN_WIDTH  * rotate );

		scale = 1.0;
	}
	else
	{
		if(horizontal_black_bars)
		{
			m_virtualScreenImageViewCell.SetScissor_w(winwidth * (1-rotate)                   + scale * VIRTUAL_WIN_HEIGHT * rotate);
			m_virtualScreenImageViewCell.SetScissor_h(scale * VIRTUAL_WIN_HEIGHT * (1-rotate) + winwidth * rotate  );
			m_virtualScreenImageViewCell.SetScissor_x((winheight-m_virtualScreenImageViewCell.Scissor_w())/2 * rotate);
			m_virtualScreenImageViewCell.SetScissor_y((winheight-m_virtualScreenImageViewCell.Scissor_h())/2 * (1-rotate));
		}
		else
		{
			m_virtualScreenImageViewCell.SetScissor_w(scale * VIRTUAL_WIN_WIDTH * (1-rotate) + winheight * rotate);
			m_virtualScreenImageViewCell.SetScissor_h(winheight * (1-rotate)                 + scale * VIRTUAL_WIN_WIDTH * rotate);
			m_virtualScreenImageViewCell.SetScissor_x((winwidth-m_virtualScreenImageViewCell.Scissor_w())/2 * (1-rotate));
			m_virtualScreenImageViewCell.SetScissor_y((winwidth-m_virtualScreenImageViewCell.Scissor_h())/2 * rotate);
		}
	}
	
	if(rotate)
	{	
		KCL::Matrix4x4 rot_tr;

		rot_tr.v11 =  0;
		rot_tr.v22 =  0;
		rot_tr.v21 = -1;
		rot_tr.v12 =  1;
		rot_tr.v41 =  1;

		mat.v11 =  (2.0 / winheight) * scale * VIRTUAL_WIN_HEIGHT;
		mat.v22 =  (2.0 / winwidth ) * scale * VIRTUAL_WIN_WIDTH ;
		mat.v41 = -1.0 + (2.0 / winheight) * m_virtualScreenImageViewCell.Scissor_x();
		mat.v42 = -1.0 + (2.0 / winwidth ) * m_virtualScreenImageViewCell.Scissor_y();

		mat = rot_tr*mat;
	}
	else
	{
		mat.v11 =  (2.0 / winwidth ) * scale * VIRTUAL_WIN_WIDTH;
		mat.v22 =  (2.0 / winheight) * scale * VIRTUAL_WIN_HEIGHT;
		mat.v41 = -1.0 + (2.0 / winwidth ) * m_virtualScreenImageViewCell.Scissor_x();
		mat.v42 = -1.0 + (2.0 / winheight) * m_virtualScreenImageViewCell.Scissor_y();
	}

	m_virtualScreenImageViewCell.ReferToProjection(ID_MATRIX);
	m_virtualScreenImageViewCell.SetTransformation(mat);
	
	m_virtual_screen_view.SetProgram(ONLY_TEXTURE_PROG);
	m_virtual_screen_view.SetColorHolder(&ZERO_COLOR);
	m_virtual_screen_view.SetTextureReferer(  &(m_virtual_screen->GetTextureReferer())  );
	m_virtual_screen_view.SetImageViewVBOAttribs(&m_ImageViewVBOAttribs_01_Square_WithVBO);
	m_virtual_screen_view.SetImageViewCell(&m_virtualScreenImageViewCell);
}


void Launcher2Simulation::FCreateVirtualPages()
{
	for(size_t i=0; i<2; ++i)
	{
		m_virtualPageImageViewCell[i].SetScissor_x( 0 ); //TODO: update that everytime
		m_virtualPageImageViewCell[i].SetScissor_y( 201 );
		//m_virtualPageImageViewCell[i].SetScissor_w( PAGE_WIN_WIDTH ); //TODO: update that everytime
		m_virtualPageImageViewCell[i].SetScissor_w( VIRTUAL_WIN_WIDTH ); //TODO: this is a temporary solution
		m_virtualPageImageViewCell[i].SetScissor_h( PAGE_WIN_HEIGHT );
		
		m_virtualPageImageViewCell[i].ReferToProjection( FULL_SCREEN_PROJ_MAT2 );

		m_virtualPageImageViewCell[i].AccessTransformation().v11 = PAGE_WIN_WIDTH;
		m_virtualPageImageViewCell[i].AccessTransformation().v22 = PAGE_WIN_HEIGHT;
		
		m_virtualPageImageViewCell[i].AccessTransformation().v42 = 201;

		m_virtual_page_view[i].SetProgram(COLORA_TINT_TEXTURE_PROG);
		
		m_virtual_page_view[i].SetColorHolder(&m_virtual_page_color[i]);
		m_virtual_page_view[i].SetTextureReferer(  &(m_virtual_page[i]->GetTextureReferer())  );
		m_virtual_page_view[i].SetImageViewVBOAttribs(&m_ImageViewVBOAttribs_01_Square_WithVBO);
		m_virtual_page_view[i].SetImageViewCell( &(m_virtualPageImageViewCell[i]) );
	}
}


bool Launcher2Simulation::staticMode() const
{
	//TODO

	return m_drawMode == STATIC_MODE;

	return false;
}


bool Launcher2Simulation::dragMode() const
{
	//TODO

	return m_drawMode == DRAG_MODE;

	return true;
}


bool Launcher2Simulation::tiltMode() const
{
	//TODO

	return false;
}


void Launcher2Simulation::updateWallpaperMatrix()
{
	double tx = -(double)(WALLPAPER_WIDTH-VIRTUAL_WIN_WIDTH) * m_drag_percent;
	m_wallpaperImageViewCell.AccessTransformation().v41 = tx;
}


void Launcher2Simulation::updateVirtualPages()
{
	//TODO
	//m_virtualPageImageViewCell[i].SetScissor_x( 0 ); //TODO: update that everytime
	//m_virtualPageImageViewCell[i].SetScissor_w( PAGE_WIN_WIDTH ); //TODO: update that everytime
	
	double tx[2];
	tx[0] = -m_drag_percent;
	tx[1] = 1-m_drag_percent;
	
	m_virtualPageImageViewCell[0].AccessTransformation().v41 = (-m_drag_percent) * (double)VIRTUAL_WIN_WIDTH + (VIRTUAL_WIN_WIDTH - PAGE_VIEWPORT_WIDTH) / 2 - 2;
	m_virtual_page_color[0].AccessColor().x = (1-m_drag_percent);
	m_virtual_page_color[0].AccessColor().y = (1-m_drag_percent);
	m_virtual_page_color[0].AccessColor().z = (1-m_drag_percent);
	m_virtual_page_color[0].AccessColor().w = (1-m_drag_percent);

	m_virtualPageImageViewCell[1].AccessTransformation().v41 = (1-m_drag_percent) * (double)VIRTUAL_WIN_WIDTH + (VIRTUAL_WIN_WIDTH - PAGE_VIEWPORT_WIDTH) / 2 - 2;
	m_virtual_page_color[1].AccessColor().x = m_drag_percent;
	m_virtual_page_color[1].AccessColor().y = m_drag_percent;
	m_virtual_page_color[1].AccessColor().z = m_drag_percent;
	m_virtual_page_color[1].AccessColor().w = m_drag_percent;
}






GUI_Test::GUI_Test (const GlobalTestEnvironment* const gte) :
	GLB::TestBase (gte),
	m_launcher2Simulation (0),
	m_diff_accu (0.0),
	m_multiplier (1),
	m_sum_rendered (0)
{
}


GUI_Test::~GUI_Test()
{
	delete m_launcher2Simulation;
}


void GUI_Test::resetEnv()
{
	//TODO ???
}


bool GUI_Test::resize(int width, int height)
{
	return false;
}


KCL::KCL_Status GUI_Test::init ()
{
	m_launcher2Simulation = new Launcher2Simulation(m_settings->m_viewport_width, m_settings->m_viewport_height, m_settings->m_play_time);

	if( !m_launcher2Simulation->FBOsReady() )
	{
		delete m_launcher2Simulation;
		m_launcher2Simulation = 0;

		return KCL_TESTERROR_GUI_BENCHMARK_NOT_SUPPORTED;
	}

	return KCL_TESTERROR_NOERROR;
}


bool GUI_Test::animate (const int time)
{ 
	SetAnimationTime(time);
	
	m_launcher2Simulation->Simulate(time);

	/*debug*/
	//return true;
	/*debug*/

	return time <= m_settings->m_play_time;
}


bool GUI_Test::render ()
{
	if( m_launcher2Simulation->ElapsedTime() < 10000)
	{
		m_diff_accu += 10;//DiffTime();
		if(m_diff_accu > 1100)
		{
			m_diff_accu = 0;

//			if(CurrentFPS() > 50.0f)
//			{
//				++m_multiplier;
//			}
		}
	}

	for(size_t i=0; i<m_multiplier; ++i)
	{
		m_launcher2Simulation->Render();
	}
	
	m_sum_rendered += m_multiplier;


	return true;
}

