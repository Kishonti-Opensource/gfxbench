/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GUI_TEST_H
#define GUI_TEST_H


#include "test_base.h"
#include "kcl_math3d.h"


namespace GLB
{
	class Texture2D;
	class FBO;
};


namespace GUIB 
{
	// declarations
	class GUI_Test; // : public TestBase --> init, animate, render, result, etc...

	enum Launcher2SimulationDrawMode {STATIC_MODE, DRAG_MODE, TILT_MODE};
	class Launcher2Simulation; // has GUI_Elements, simulates GUI environment

	//GUI elements
	class View; // drawable abstract GUI element
	class ViewWithShader; // drawable abstract GUI element with shader identifier
	class ImageView; // : public View
	class TextView; // : public View
	class BubbleTextView; // : public TextView

	class ViewGroup; // : public View
	//class LinearLayout; // : public ViewGroup -- arranges its children in a single row or column -- maybe not necessary
	//class FrameLayout; // : public ViewGroup -- maybe not necessary
	//class DragLayer; // : public FrameLayout -- maybe not necessary

	//UI helpers
	class ImageViewCell; //scissor size, transformation matrices
	class TextViewCell; //2 scissor sizes, 3 transformation matrices
	class IconCell; //ImageViewCell + TextViewCell
	
	enum IconPageGridType { FULLSCREEN_RELATIVE, ANIMATEDSCREEN_RELATIVE };
	class IconPageGrid; //IconCells
	class HotSeatArray; //ImageViewCells

	enum AttribPtrType {VERTEX, TEXTURECOORD};
	class ImageViewVBOAttribs; //vbo, ebo, vertexattribpointer paramaters + draw params
	class TextViewVBOAttribs; //vbos, ebos, vertexattribpointer paramaters + draw params

	class TextureReferer;

	enum TextureHolderFilter {NEAREST, LINEAR};
	enum TextureHolderUnpackAlginment {ONE = 1, FOUR = 4};
	class TextureHolder;
	
	class ColorHolder;

	enum FBO_Holder_ColorMode { RGB, RGBA };
	class FBO_Holder;
	
	enum Launcher2ShaderManagerPrograms { NONE, ONLY_TEXTURE_PROG, COLORA_TINT_TEXTURE_PROG, TEXTUREA_TINT_COLOR_PROG };
	enum Launcher2ShaderManagerMatrices { PROJECTION, TRANSFORMATION };
	class Launcher2ShaderManager;
	
	class Launcher2AttribAndVBORegistry;
	/*TODO*/ class FontPaletteBuffer; //Alpha texture for fonts


	// definitions
	class ImageViewCell
	{
		KCL::int32 m_scissor_x;
		KCL::int32 m_scissor_y;
		KCL::int32 m_scissor_w;
		KCL::int32 m_scissor_h;
		const KCL::Matrix4x4 *m_projection;
		KCL::Matrix4x4 m_transformation;
	public:
		ImageViewCell();

		KCL::int32 Scissor_x() const { return m_scissor_x; }
		KCL::int32 Scissor_y() const { return m_scissor_y; }
		KCL::int32 Scissor_w() const { return m_scissor_w; }
		KCL::int32 Scissor_h() const { return m_scissor_h; }
		const KCL::Matrix4x4& Projection() const { return *m_projection; }
		const KCL::Matrix4x4& Transformation() const { return m_transformation; }

		void SetScissor_x(KCL::int32 scissor_x) { m_scissor_x = scissor_x; }
		void SetScissor_y(KCL::int32 scissor_y) { m_scissor_y = scissor_y; }
		void SetScissor_w(KCL::int32 scissor_w) { m_scissor_w = scissor_w; }
		void SetScissor_h(KCL::int32 scissor_h) { m_scissor_h = scissor_h; }
		void ReferToProjection(const KCL::Matrix4x4& projection) { m_projection = &projection; }
		void SetTransformation(const KCL::Matrix4x4& transformation) { m_transformation = transformation; }

		KCL::Matrix4x4& AccessTransformation() { return m_transformation; }
	}; 
	

	class TextViewCell
	{
		KCL::int32 m_scissor_x;
		KCL::int32 m_scissor_y;
		KCL::int32 m_scissor_w;
		KCL::int32 m_scissor_h;

		KCL::int32 m_scissor_x_2;
		KCL::int32 m_scissor_y_2;
		KCL::int32 m_scissor_w_2;
		KCL::int32 m_scissor_h_2;
		
		const KCL::Matrix4x4 *m_projection;
		const KCL::Matrix4x4 *m_projection_2;
		const KCL::Matrix4x4 *m_projection_3;

		KCL::Matrix4x4 m_transformation;
		KCL::Matrix4x4 m_transformation_2;
		KCL::Matrix4x4 m_transformation_3;
	public:
		TextViewCell();

		KCL::int32 Scissor_x  () const { return m_scissor_x  ; }
		KCL::int32 Scissor_y  () const { return m_scissor_y  ; }
		KCL::int32 Scissor_w  () const { return m_scissor_w  ; }
		KCL::int32 Scissor_h  () const { return m_scissor_h  ; }
		KCL::int32 Scissor_x_2() const { return m_scissor_x_2; }
		KCL::int32 Scissor_y_2() const { return m_scissor_y_2; }
		KCL::int32 Scissor_w_2() const { return m_scissor_w_2; }
		KCL::int32 Scissor_h_2() const { return m_scissor_h_2; }
		const KCL::Matrix4x4& Projection  () const { return *m_projection  ; }
		const KCL::Matrix4x4& Projection_2() const { return *m_projection_2; }
		const KCL::Matrix4x4& Projection_3() const { return *m_projection_3; }
		const KCL::Matrix4x4& Transformation  () const { return m_transformation  ; }
		const KCL::Matrix4x4& Transformation_2() const { return m_transformation_2; }
		const KCL::Matrix4x4& Transformation_3() const { return m_transformation_3; }

		void SetScissor_x  (KCL::int32 scissor_x) { m_scissor_x   = scissor_x; }
		void SetScissor_y  (KCL::int32 scissor_y) { m_scissor_y   = scissor_y; }
		void SetScissor_w  (KCL::int32 scissor_w) { m_scissor_w   = scissor_w; }
		void SetScissor_h  (KCL::int32 scissor_h) { m_scissor_h   = scissor_h; }
		void SetScissor_x_2(KCL::int32 scissor_x) { m_scissor_x_2 = scissor_x; }
		void SetScissor_y_2(KCL::int32 scissor_y) { m_scissor_y_2 = scissor_y; }
		void SetScissor_w_2(KCL::int32 scissor_w) { m_scissor_w_2 = scissor_w; }
		void SetScissor_h_2(KCL::int32 scissor_h) { m_scissor_h_2 = scissor_h; }
		void SetProjection  (const KCL::Matrix4x4& projection) { m_projection   = &projection; }
		void SetProjection_2(const KCL::Matrix4x4& projection) { m_projection_2 = &projection; }
		void SetProjection_3(const KCL::Matrix4x4& projection) { m_projection_3 = &projection; }
		void SetTransformation  (const KCL::Matrix4x4& transformation) { m_transformation   = transformation; }
		void SetTransformation_2(const KCL::Matrix4x4& transformation) { m_transformation_2 = transformation; }
		void SetTransformation_3(const KCL::Matrix4x4& transformation) { m_transformation_3 = transformation; }
	};
	
	
	class IconCell
	{
		ImageViewCell m_imageViewCell;
		TextViewCell m_textViewCell;
	public:
		const ImageViewCell& GetImageViewCell() const { return m_imageViewCell; }
		const TextViewCell& GetTextViewCell() const { return m_textViewCell; }

		ImageViewCell& AccessImageViewCell() { return m_imageViewCell; }
		TextViewCell& AccessTextViewCell() { return m_textViewCell; }
	};
	
	
	class IconPageGrid
	{
		std::vector<IconCell> m_iconCells;
		KCL::uint32 m_rowCount;
		KCL::uint32 m_columnCount;
	public:
		IconPageGrid(IconPageGridType gridType);

		KCL::uint32 RowCount() const { return m_rowCount; }
		KCL::uint32 ColumnCount() const { return m_columnCount; }

		const IconCell& GetIconCell(KCL::uint32 rowIndex, KCL::uint32 columnIndex) const
		{
#ifdef WIN32
			if(RowCount() <= rowIndex || ColumnCount() <= columnIndex || m_iconCells.size() == 0 || m_iconCells.size() <= rowIndex * ColumnCount() + columnIndex)
			{
				printf("ERROR in IconPageGrid::GetIconCell! RowCount() <= rowIndex || ColumnCount() <= columnIndex || m_iconCells.size() == 0 || m_iconCells.size() <= rowIndex * ColumnCount() + columnIndex");
				exit(-1);
			}
#endif
			return m_iconCells[rowIndex * ColumnCount() + columnIndex];
		}
	};
	
	
	class HotSeatArray
	{
		std::vector<ImageViewCell> m_imageViewCells;
	public:
		HotSeatArray();

		size_t Count() const { return m_imageViewCells.size(); }

		const ImageViewCell& GetImageViewCell(KCL::uint32 index) const
		{
#ifdef WIN32
			if(0 == m_imageViewCells.size() || index >= m_imageViewCells.size())
			{
				printf("ERROR in HotSeatArray::GetImageViewCell! 0 == m_imageViewCells.size() || index >= m_imageViewCells.size()\n");
				exit(-1);
			}
#endif
			return m_imageViewCells[index];
		}
	};


	class ImageViewVBOAttribs //vbo, ebo, vertexattribpointer paramaters + draw params
	{
		KCL::uint32 m_vbo;
		KCL::int32 m_size[2];
		KCL::int32 m_type[2];
		bool m_normalized[2];
		KCL::int32 m_stride[2];
		const void* m_vertexattribpointer[2];

		KCL::uint32 m_mode; //Triangle or Strip
		KCL::uint32 m_first;
		KCL::uint32 m_count;
	public:
		ImageViewVBOAttribs()
		{
			m_vbo = 0;
			m_size[0] = 0;
			m_size[1] = 0;
			m_type[0] = 0;
			m_type[1] = 0;
			m_normalized[0] = 0;
			m_normalized[1] = 0;
			m_stride[0] = 0;
			m_stride[1] = 0;
			m_vertexattribpointer[0] = 0;
			m_vertexattribpointer[1] = 0;

			m_mode = 0;
			m_first = 0;
			m_count = 0;
		}

		void SetVbo (KCL::uint32 vbo) { m_vbo = vbo; }
		void SetSize (AttribPtrType t, KCL::int32 size) { m_size[t] = size; }
		void SetType (AttribPtrType t, KCL::int32 type) { m_type[t] = type; }
		void SetNormalized (AttribPtrType t, bool normalized) { m_normalized[t] = normalized; }
		void SetStride (AttribPtrType t, KCL::int32 stride) { m_stride[t] = stride; }
		void SetPointer (AttribPtrType t, const void* vertexattribpointer)
		{
			m_vertexattribpointer[t] = vertexattribpointer;
		}

		void SetMode (KCL::uint32 mode) { m_mode = mode; }
		void SetFirst (KCL::uint32 first) { m_first = first; }
		void SetCount (KCL::uint32 count) { m_count = count; }

		KCL::uint32 GetVbo () const { return m_vbo; }
		KCL::int32  GetSize (AttribPtrType t) const { return m_size[t]; }
		KCL::int32  GetType (AttribPtrType t) const { return m_type[t]; }
		bool        GetNormalized (AttribPtrType t) const { return m_normalized[t]; }
		KCL::int32  GetStride (AttribPtrType t) const { return m_stride[t]; }
		const void* GetPointer (AttribPtrType t) const { return m_vertexattribpointer[t]; }

		KCL::uint32 GetMode () const { return m_mode; }
		KCL::uint32 GetFirst () const { return m_first; }
		KCL::uint32 GetCount () const { return m_count; }
	};
	
	
	class TextViewVBOAttribs //vbos, ebos, vertexattribpointer paramaters
	{
		//TODO
	};
	

	class TextureReferer //this class does not own the texture!
	{
	protected:
		KCL::uint32 m_texture_id;
	public:
		explicit TextureReferer(KCL::uint32 texture_id) : m_texture_id(texture_id)
		{}

		virtual ~TextureReferer() {}

		KCL::uint32 Id() const { return m_texture_id; }
	};

	
	class TextureHolder : public TextureReferer //this class does own the texture!
	{
		GLB::Texture2D* m_texture;
		
		TextureHolder(const TextureHolder&);
		TextureHolder& operator=(const TextureHolder&);
	public:
		TextureHolder(std::string name, TextureHolderFilter filter, TextureHolderUnpackAlginment alignment);
		~TextureHolder();
	};
	
	
	class ColorHolder
	{
		KCL::Vector4D m_color;
	public:
		ColorHolder() {}
		ColorHolder(float r, float g, float b, float a) : m_color(r, g, b, a) {}

		void SetColor(const KCL::Vector4D &color) { m_color = color; }
		const KCL::Vector4D& GetColor() const { return m_color; }
		KCL::Vector4D& AccessColor() { return m_color; }
	};


	class FBO_Holder
	{
		GLB::FBO* m_FBO;
		TextureReferer* m_textureReferer;

		FBO_Holder(const FBO_Holder&);
		FBO_Holder& operator=(const FBO_Holder&);
	public:
		FBO_Holder(KCL::uint32 width, KCL::uint32 height, FBO_Holder_ColorMode mode);
		~FBO_Holder();

		bool Ready() const { return 0 != m_FBO; }

		void Bind();
		static void Unbind();

		const TextureReferer& GetTextureReferer() const
		{
#ifdef WIN32
			if(m_textureReferer == 0)
			{
				printf("ERROR in FBO_Holder::GetTextureReferer! m_textureReferer == 0");
				exit(-1);
			}
#endif
			return *m_textureReferer;
		}
	};


	class View
	{
		View(const View&);
		View& operator=(const View&);
	public:
		View() {}
		virtual ~View() = 0;
		virtual void Draw(const KCL::Matrix4x4 &transformation) const = 0;
	};


	class ViewGroup : public View
	{
	public:
		ViewGroup() {}
		virtual ~ViewGroup()
		{
			for(size_t i=0; i<m_views.size(); ++i)
			{
				delete m_views[i];
				m_views[i] = 0;
			}
		}

		void Draw(const KCL::Matrix4x4 &transformation) const
		{
			for(size_t i=0; i<m_views.size(); ++i)
			{
#ifdef WIN32
				if(0 == m_views[i])
				{
					printf("ERROR in ViewGroup::Draw! 0 == m_views[i]");
					exit(-1);
				}
#endif
				m_views[i]->Draw(transformation);
			}
		}

		void PushAndOwnNew(View* view)
		{
			m_views.push_back(view);
		}
		
	protected:
		std::vector<View*> m_views;
	};

	
	class ViewWithShader : public View
	{
	protected:
		Launcher2ShaderManagerPrograms m_program;
	public:
		ViewWithShader() : m_program(NONE) {}
		void SetProgram(Launcher2ShaderManagerPrograms program) { m_program = program; }
	};


	class ImageView : public ViewWithShader
	{
		const ImageViewCell* m_imageViewCell;
		const ImageViewVBOAttribs* m_imageViewVBOAttribs;
		const TextureReferer* m_textureReferer;
		const ColorHolder* m_colorHolder;
	public:
		
		ImageView() : m_imageViewCell (0), m_imageViewVBOAttribs (0), m_textureReferer (0), m_colorHolder (0)
		{}

		void SetImageViewCell (const ImageViewCell* imageViewCell) { m_imageViewCell = imageViewCell; }
		void SetImageViewVBOAttribs (const ImageViewVBOAttribs* imageViewVBOAttribs) { m_imageViewVBOAttribs = imageViewVBOAttribs; }
		void SetTextureReferer (const TextureReferer* textureReferer) { m_textureReferer = textureReferer; }
		void SetColorHolder (const ColorHolder* colorHolder) { m_colorHolder = colorHolder; }

		void Draw(const KCL::Matrix4x4 &transformation) const;
	};
	

	class TextView : public ViewWithShader
	{
		//4 color, 3 texture

		//const TextViewCell* m_textViewCell; // NOT USED
		//const TextViewVBOAttribs* m_textViewVBOAttribs; // NOT USED
	public:
		//TODO

		void Draw(const KCL::Matrix4x4 &transformation) const
		{
			//TODO
		}
	};
	

	class BubbleTextView : public View
	{
		ImageView* m_imageView;
		TextView* m_textView;
	public:

		BubbleTextView() : m_imageView(0), m_textView(0)
		{
		}

		~BubbleTextView()
		{
			delete m_imageView;
			delete m_textView;
		}

		void ReplaceImageView(ImageView* imageView)
		{
			delete m_imageView;
			m_imageView = imageView;
		}

		void ReplaceTextView(TextView* textView)
		{
			delete m_textView;
			m_textView = textView;
		}

		void Draw(const KCL::Matrix4x4 &transformation) const
		{
			if(m_imageView)
				m_imageView->Draw(transformation);
			if(m_textView)
				m_textView->Draw(transformation);
		}
	};
	

	class Launcher2ShaderManager
	{
		KCL::uint32 m_vertex_shader[3];
		KCL::uint32 m_fragment_shader[3];
		KCL::uint32 m_shader_program[4]; //0 == NONE
		KCL::int32 m_uniformMatrixLocations[4][2]; // 0-3 program, 0-1 projection/transformation
		KCL::int32 m_uniformColorLocations[4];
		KCL::int32 m_uniformSamplerLocations[4];
		mutable Launcher2ShaderManagerPrograms m_actual_program;
		
		Launcher2ShaderManager();
		Launcher2ShaderManager(const Launcher2ShaderManager&);
		Launcher2ShaderManager& operator=(const Launcher2ShaderManager&);

		static Launcher2ShaderManager* s_instance;
	public:
		~Launcher2ShaderManager();

		void UseProgram(Launcher2ShaderManagerPrograms program) const;
		void UniformMatrix4fv(Launcher2ShaderManagerMatrices which,const KCL::Matrix4x4& m) const;
		void Uniform4f(const KCL::Vector4D &v) const;
		void Uniform1i(KCL::int32 i) const;

		static void Init();
		static void Delete();
		static const Launcher2ShaderManager* Instance()
		{
			return s_instance;
		}
	};


	class Launcher2AttribAndVBORegistry
	{
		Launcher2AttribAndVBORegistry();
		Launcher2AttribAndVBORegistry(const Launcher2AttribAndVBORegistry&);
		Launcher2AttribAndVBORegistry& operator=(const Launcher2AttribAndVBORegistry&);

		static KCL::uint32 s_actual_vbo;
		static KCL::uint32 s_actual_ebo;
		static const void* s_actual_vertex_pointer[2];
		static bool s_attribpointer_dirty[2];
	public:
		static void Init();
		static void BindVBO(const KCL::uint32 id);
		static void BindEBO(const KCL::uint32 id);
		static void VertexAttribPointer(AttribPtrType t, KCL::int32 size, KCL::int32 type, bool normalized, KCL::int32 stride, const void* ptr);
	};





	//TODO
	class Launcher2Simulation
	{
	public:
		Launcher2Simulation(KCL::uint32 winwidth, KCL::uint32 winheight, KCL::uint32 testLengthMillisec);
		~Launcher2Simulation();
		
		void Simulate(unsigned int time); //simulates the scrolling back and forth + tilt
		void Render() const;

		bool FBOsReady() const;
		unsigned int ElapsedTime() const { return m_elapsed_time; }

	private:

		void createVBOs();
		void deleteVBOs();

		void FCreate_ImageViewVBOAttribs_01_Square_WithVBO();
		void FCreate_ImageViewVBOAttribs_01_Square_NOT_VBO();
		void FCreate_ImageViewVBOAttribs_Gradient();
		void FCreate_ImageViewVBOAttribs_NavigationStripe();
		void FCreate_ImageViewVBOAttribs_SearchBarBackground();
		
		void FCreateWallpaper();
		void FCreateGradient();
		void FCreateNavigationStripe();
		void FCreateSearchBar();
		void FCreateHotseat();
		void FCreatePagesOfIcons(const char *pageName);
		void FCreateVirtualScreen(KCL::uint32 winwidth, KCL::uint32 winheight);
		void FCreateVirtualPages();

		bool staticMode() const;
		bool dragMode() const;
		bool tiltMode() const;

		void updateWallpaperMatrix();
		void updateVirtualPages();


		KCL::uint32 m_winwidth;  //TODO maybe not needed
		KCL::uint32 m_winheight; //TODO maybe not needed

		size_t m_actual_page;
		unsigned int m_prev_time;
		unsigned int m_elapsed_time;
		unsigned int m_diff_accu;

		double m_drag_percent; //0.0 - 1.0 :: 0.0 == most left page is visible, 1.0 == most right page is visible
		
//		KCL::int32 m_scissor_x; //TODO maybe not needed // NOT USED
//		KCL::int32 m_scissor_y;	//TODO maybe not needed // NOT USED
//		KCL::int32 m_scissor_w;	//TODO maybe not needed // NOT USED
//		KCL::int32 m_scissor_h;	//TODO maybe not needed // NOT USED

		Launcher2SimulationDrawMode m_drawMode;

		KCL::uint32 m_zero_one_square_vbo; // [(0,0)x2 - (1,0)x2 - (0,1)x2 - (1,1)x2]
		KCL::uint32 m_gradient_vbo;
		KCL::uint32 m_navigationStripe_vbo;
		KCL::uint32 m_searchBarBackground_vbo;

		FBO_Holder *m_virtual_screen;
				
		FBO_Holder *m_virtual_page[2];

		IconPageGrid* m_iconPageGrid[2]; //0 fullscreen relative, 1 animated screen relative

		

		///////////////////////////////////////////////////////////////////////////////////////////////
		// initialized by own constructor
		HotSeatArray m_hotSeatArray;

		KCL::Matrix4x4 m_transformation_matrix;

		ImageViewVBOAttribs m_ImageViewVBOAttribs_01_Square_WithVBO; // 0,0 - 1,1 square with VBO
		ImageViewVBOAttribs m_ImageViewVBOAttribs_01_Square_NOT_VBO; // 0,0 - 1,1 square without VBO
		ImageViewVBOAttribs m_ImageViewVBOAttribs_Gradient;
		ImageViewVBOAttribs m_ImageViewVBOAttribs_NavigationStripe;
		ImageViewVBOAttribs m_ImageViewVBOAttribs_SearchBarBackground;

		ImageViewCell m_wallpaperImageViewCell;
		ImageViewCell m_gradientImageViewCell;
		ImageViewCell m_navigationStripeImageViewCell;
		ImageViewCell m_searchBarBackgroundImageViewCell;
		ImageViewCell m_searchBarImageViewCell;

		ImageView m_wallpaper;
		ImageView m_gradient;
		ImageView m_navigationStripe; //gray line between icons and hotseat
		ImageView m_blue_navigationStripe; //blue line over m_navigation_stripe - used when dragging
		ViewGroup m_searchBar; //google searchbar
		ViewGroup m_hotseat;
		
		ImageViewCell m_virtualScreenImageViewCell; //used to put the virtual screen's texture to the right place on the real screen
		ImageView m_virtual_screen_view;

		ColorHolder m_virtual_page_color[2];
		ImageViewCell m_virtualPageImageViewCell[2]; //used to put the animated page's texture to the right place on the virtual screen
		ImageView m_virtual_page_view[2];

		std::vector<View*> m_pages_of_icons[2]; //0 fullscreen relative, 1 animated screen relative
		std::vector<View*> m_scroll_decoratives; //stuff that is visible only while dragging
		
		std::vector<TextureReferer*> m_textureReferers; //lot of icon-textures

	};





	class GUI_Test : public GLB::TestBase
	{
	public:
		GUI_Test ( const GlobalTestEnvironment* const gte);
		virtual ~GUI_Test() ;

		virtual const char* getUom() const {return "frames";}

		virtual bool isWarmup() const { return false; }

		virtual KCL::uint32 indexCount() const { return 0; }
	
		virtual void resetEnv();

		virtual bool resize(int width, int height);
	
		/// Returns the result of the test.
		virtual float getScore () const
		{
			//TODO ?
			return getFrames ();
		}

		virtual int getScoreArraySize() const { return 0; }

		virtual float getPartialScore(int phase) const { return 0; }

		virtual bool isLowLevel() const { return true; }

		//virtual int GetFrameStepTime(); // imlpemented in TestBase

		virtual KCL::KCL_Status init ();

		virtual bool animate (const int time);

		virtual bool render ();

		virtual void FreeResources() {}

		//virtual uint32 getRenderedVerticesCount() const; // returns 0 // TODO ?
		//virtual uint32 getRenderedTrianglesCount() const; // returns 0 // TODO ?
		//virtual uint32 getDrawCalls() const; // returns 0 // TODO ?

		void rewriteFrameCounter() { m_frames = m_sum_rendered; }

	protected:
		Launcher2Simulation* m_launcher2Simulation;

		double m_diff_accu;
		size_t m_multiplier;
		size_t m_sum_rendered;
	};

}; //namespace GUIB 


#endif
