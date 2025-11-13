/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_OPENGL_STATE_MANAGER_H
#define GLB_OPENGL_STATE_MANAGER_H

#include "platform.h"

#if defined ENABLE_FRAME_CAPTURE
#define OPENGL_STATE_MANAGER_NULL
#endif

#if !defined OPENGL_STATE_MANAGER_NULL
#include <stdio.h>
#include "opengl/ext.h"

#ifndef STRIP_REDUNDANT_CLEARS
//#define STRIP_REDUNDANT_CLEARS
#endif
#endif


namespace GLB
{
	class OpenGLStateManager
	{
	public:

		static void Reset()
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			for(size_t i = 0; i < OpenGLStateManager::max_CAPABILITY_IDX; ++i)
			{
				m_desired_capabilities[i][0] = false;
				m_actual_capabilities[i] = true;
		
				//GL_DITHER is set to GL_FALSE, but its default value is GL_TRUE in the standard
			}
			
			
			for(size_t i = 0; i < m_vertex_attrib_arrays_count; ++i)
			{
				m_desired_enabled_vertex_attrib_arrays[i][0] = false;
				m_actual_enabled_vertex_attrib_arrays[i] = true;
			}
			

			m_actual_shader_program[0] = 1;
			OpenGLStateManager::GlUseProgram(0);
		
			m_actual_texture_unit[0] = GL_TEXTURE1;
			OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
			
			m_actual_depth_mask[0] = GL_FALSE;
			OpenGLStateManager::GlDepthMask(GL_TRUE);
		
		
			m_desired_blendfunc_sfactor[0] = GL_ONE;
			m_actual_blendfunc_sfactor = GL_ZERO;
		
			m_desired_blendfunc_dfactor[0] = GL_ZERO;
			m_actual_blendfunc_dfactor = GL_ONE;
		
			m_desired_cullface_mode[0] = GL_BACK;
			m_actual_cullface_mode = GL_FRONT;
			
			m_desired_depth_func[0] = GL_LESS;
			m_actual_depth_func = GL_GREATER;
		
			OpenGLStateManager::Commit();
#endif
		}

#if !defined OPENGL_STATE_MANAGER_NULL
		static void Print()
		{
			for(size_t i=0; i<OpenGLStateManager::max_CAPABILITY_IDX; ++i)
			{
				printf( "%s - ", GlCapabilityToStr( m_capabilities_table[i]));
				GLboolean b = glIsEnabled( m_capabilities_table[i]);
				if (b)
					printf( "enabled\n");
				else
					printf( "disabled\n");
			}
		}
#endif

		static void Commit()
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			for (size_t i = 0; i<OpenGLStateManager::max_CAPABILITY_IDX; ++i)
			{
				if(m_actual_capabilities[i] != m_desired_capabilities[i][0])
				{
					m_actual_capabilities[i] = m_desired_capabilities[i][0];
		
					if (m_actual_capabilities[i])
						::glEnable(m_capabilities_table[i]);
					else
						::glDisable(m_capabilities_table[i]);
				}
			}
			
			if (!GLB::g_extension->hasFeature(GLB::GLBFEATURE_vertex_array_object))
			{
				for(unsigned int i = 0; i < m_vertex_attrib_arrays_count; ++i)
				{
					if(m_actual_enabled_vertex_attrib_arrays[i] != m_desired_enabled_vertex_attrib_arrays[i][0])
					{
						m_actual_enabled_vertex_attrib_arrays[i] = m_desired_enabled_vertex_attrib_arrays[i][0];
					
						if (m_actual_enabled_vertex_attrib_arrays[i])
							::glEnableVertexAttribArray(i);
						else
							::glDisableVertexAttribArray(i);
					}
				}
			}
			
		
			bool changeBlendFunc = false;
			if(m_actual_blendfunc_sfactor != m_desired_blendfunc_sfactor[0])
			{
				m_actual_blendfunc_sfactor = m_desired_blendfunc_sfactor[0];
				changeBlendFunc = true;
			}
			if(m_actual_blendfunc_dfactor != m_desired_blendfunc_dfactor[0])
			{
				m_actual_blendfunc_dfactor = m_desired_blendfunc_dfactor[0];
				changeBlendFunc = true;
			}
			if(changeBlendFunc)
				::glBlendFunc(m_actual_blendfunc_sfactor, m_actual_blendfunc_dfactor);
			
		
			if(m_actual_cullface_mode != m_desired_cullface_mode[0])
			{
				m_actual_cullface_mode = m_desired_cullface_mode[0] ;
				::glCullFace(m_actual_cullface_mode);
			}
			
		
			if(m_actual_depth_func != m_desired_depth_func[0])
			{
				m_actual_depth_func = m_desired_depth_func[0];
				::glDepthFunc(m_actual_depth_func);
			}
#endif
		}

		
		static void GlEnable(unsigned int capability) //committable
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			m_desired_capabilities[OpenGLStateManager::GlCapabilityToIdx(capability)][0] = true;
#else
			glEnable(capability);
#endif
		}
		

		static void GlDisable(unsigned int capability) //committable
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			m_desired_capabilities[OpenGLStateManager::GlCapabilityToIdx(capability)][0] = false;
#else
			glDisable(capability);
#endif
		}


		static void DisableAllCapabilites() //committable
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			for (size_t i = 0; i<max_CAPABILITY_IDX; ++i)
				m_desired_capabilities[i][0] = false;
#endif
		}

		
		static void GlEnableVertexAttribArray(unsigned int index) //committable
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			//support unexported attribs when input is "-1"
            if(index > m_vertex_attrib_arrays_count)
                return;

			m_desired_enabled_vertex_attrib_arrays[index][0] = true;
#else
			::glEnableVertexAttribArray(index);
#endif
		}

		static void GlDisableVertexAttribArray(unsigned int index) //committable
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			//support unexported attribs when input is "-1"
            if(index > m_vertex_attrib_arrays_count)
                return;

			m_desired_enabled_vertex_attrib_arrays[index][0] = false;
#else
			::glDisableVertexAttribArray(index);
#endif
		}
		
		static void GlEnableVertexAttribArrayInstantCommit(unsigned int index) //commits itself
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			//support unexported attribs when input is "-1"
            if(index > m_vertex_attrib_arrays_count)
                return;

			m_desired_enabled_vertex_attrib_arrays[index][0] = true;

			if( !m_actual_enabled_vertex_attrib_arrays[index] )
			{
				m_actual_enabled_vertex_attrib_arrays[index] = true;
				::glEnableVertexAttribArray(index);
			}
#else
			::glEnableVertexAttribArray(index);
#endif
		}

		static void GlDisableVertexAttribArrayInstantCommit(unsigned int index) //commits itself
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			//support unexported attribs when input is "-1"
            if(index > m_vertex_attrib_arrays_count)
                return;

			m_desired_enabled_vertex_attrib_arrays[index][0] = false;

			if( m_actual_enabled_vertex_attrib_arrays[index] )
			{
				m_actual_enabled_vertex_attrib_arrays[index] = false;
				::glDisableVertexAttribArray(index);
			}
#else
			::glDisableVertexAttribArray(index);
#endif
		}

		static void DisableAllVertexAttribs()
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			for(unsigned int i=0; i<m_vertex_attrib_arrays_count; ++i)
			{
				m_desired_enabled_vertex_attrib_arrays[i][0] = false;
			}
#endif
		}

		static void DisableAllVertexAttribsInstantCommit()
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			for (unsigned int i = 0; i<m_vertex_attrib_arrays_count; ++i)
			{
				m_desired_enabled_vertex_attrib_arrays[i][0] = false;

				if( m_actual_enabled_vertex_attrib_arrays[i] )
				{
					m_actual_enabled_vertex_attrib_arrays[i] = false;
					::glDisableVertexAttribArray(i);
				}
			}
#endif
		}
		

		static void GlActiveTexture(unsigned int texture)	//commits itself
		{		
#if !defined OPENGL_STATE_MANAGER_NULL
			if (m_actual_texture_unit[0] != texture)
			{
				m_actual_texture_unit[0] = texture;
				::glActiveTexture(texture);
			}
#else
			::glActiveTexture(texture);
#endif
		}
		
		static void GlBlendFunc(unsigned int sfactor, unsigned int dfactor) //committable
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			m_desired_blendfunc_sfactor[0] = sfactor;
			m_desired_blendfunc_dfactor[0] = dfactor;
#else
			glBlendFunc(sfactor, dfactor);
#endif
		}
		
		static void GlCullFace(unsigned int mode) //committable
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			m_desired_cullface_mode[0] = mode;
#else
			glCullFace(mode);
#endif
		}

        static void InvertCullFace()
        {
#if !defined OPENGL_STATE_MANAGER_NULL
            if(m_desired_capabilities[OpenGLStateManager::GlCapabilityToIdx(GL_CULL_FACE)][0])
            {
                if(m_desired_cullface_mode[0] == GL_FRONT)
                {
                    m_desired_cullface_mode[0] = GL_BACK;
                }
                else
                {
                    m_desired_cullface_mode[0] = GL_FRONT;
                }
            }
#else
            if(glIsEnabled(GL_CULL_FACE))
            {
                if(glGet(CULL_FACE_MODE) == GL_FRONT)
                {
                    m_desired_cullface_mode[0] = GL_BACK;
                }
                else
                {
                    m_desired_cullface_mode[0] = GL_FRONT;
                }
            }
#endif
        }
		
		static void GlDepthFunc(unsigned int func) //committable
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			m_desired_depth_func[0] = func;
#else
			glDepthFunc(func);
#endif
		}
		
		static void GlDepthMask(unsigned char mask) //commits itself
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			if(m_actual_depth_mask[0] != mask)
			{
				m_actual_depth_mask[0] = mask;
				::glDepthMask(mask);
			}
#else
			::glDepthMask(mask);
#endif
		}

		static void GlUseProgram(unsigned int program)
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			if (m_actual_shader_program[0] != program)
			{
				m_actual_shader_program[0] = program;
				::glUseProgram(program);
			}
#else
			::glUseProgram(program);
#endif
		}

		static void Save()
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			for (size_t i = 0; i<OpenGLStateManager::max_CAPABILITY_IDX; ++i)
			{
				m_desired_capabilities[i][1] = m_desired_capabilities[i][0];
			}
			
			
			for(size_t i=0; i<m_vertex_attrib_arrays_count; ++i)
			{
				m_desired_enabled_vertex_attrib_arrays[i][1] = m_desired_enabled_vertex_attrib_arrays[i][0];
			}
			

			m_actual_shader_program[1] = m_actual_shader_program[0];			
		
			m_actual_texture_unit[1] = m_actual_texture_unit[0];
						
			m_actual_depth_mask[1] = m_actual_depth_mask[0];
			

			m_desired_blendfunc_sfactor[1] = m_desired_blendfunc_sfactor[0];
			m_desired_blendfunc_dfactor[1] = m_desired_blendfunc_dfactor[0];
			m_desired_cullface_mode[1] = m_desired_cullface_mode[0];
			m_desired_depth_func[1] = m_desired_depth_func[0];
#endif
		}

		static void Restore()
		{
#if !defined OPENGL_STATE_MANAGER_NULL
			for (size_t i = 0; i<OpenGLStateManager::max_CAPABILITY_IDX; ++i)
			{
				m_desired_capabilities[i][0] = m_desired_capabilities[i][1];
			}
			
			
			for(size_t i=0; i<m_vertex_attrib_arrays_count; ++i)
			{
				m_desired_enabled_vertex_attrib_arrays[i][0] = m_desired_enabled_vertex_attrib_arrays[i][1];
			}
			

			OpenGLStateManager::GlUseProgram(m_actual_shader_program[1]);
			
			OpenGLStateManager::GlActiveTexture(m_actual_texture_unit[1]);
			
			OpenGLStateManager::GlDepthMask(m_actual_depth_mask[1]);
						
			m_desired_blendfunc_sfactor[0] = m_desired_blendfunc_sfactor[1];			
			m_desired_blendfunc_dfactor[0] = m_desired_blendfunc_dfactor[1];			
			m_desired_cullface_mode[0] = m_desired_cullface_mode[1];			
			m_desired_depth_func[0] = m_desired_depth_func[1];
#endif
		}
		
	private:

		enum CAPABILITY_IDX {
			CAP_GL_BLEND,
			CAP_GL_CULL_FACE,
			CAP_GL_DEPTH_TEST,
			CAP_GL_DITHER,
			CAP_GL_POLYGON_OFFSET_FILL,
			CAP_GL_SAMPLE_ALPHA_TO_COVERAGE,
			CAP_GL_SAMPLE_COVERAGE,
			CAP_GL_SCISSOR_TEST,
			CAP_GL_STENCIL_TEST,

			max_CAPABILITY_IDX
		};

		static CAPABILITY_IDX GlCapabilityToIdx(unsigned int capability)
		{
			switch(capability)
			{
			case GL_BLEND:					  return CAP_GL_BLEND;
			case GL_CULL_FACE:				  return CAP_GL_CULL_FACE;
			case GL_DEPTH_TEST:				  return CAP_GL_DEPTH_TEST;
			case GL_DITHER:					  return CAP_GL_DITHER;
			case GL_POLYGON_OFFSET_FILL:	  return CAP_GL_POLYGON_OFFSET_FILL;
			case GL_SAMPLE_ALPHA_TO_COVERAGE: return CAP_GL_SAMPLE_ALPHA_TO_COVERAGE;
			case GL_SAMPLE_COVERAGE:		  return CAP_GL_SAMPLE_COVERAGE;
			case GL_SCISSOR_TEST:			  return CAP_GL_SCISSOR_TEST;
			case GL_STENCIL_TEST:			  return CAP_GL_STENCIL_TEST;
			default: return CAP_GL_STENCIL_TEST; //this can happen only if someone uses invalid value...
			}
		}
		static const char* GlCapabilityToStr(unsigned int capability)
		{
			switch(capability)
			{
			case GL_BLEND:					  return "GL_BLEND";
			case GL_CULL_FACE:				  return "GL_CULL_FACE";
			case GL_DEPTH_TEST:				  return "GL_DEPTH_TEST";
			case GL_DITHER:					  return "GL_DITHER";
			case GL_POLYGON_OFFSET_FILL:	  return "GL_POLYGON_OFFSET_FILL";
			case GL_SAMPLE_ALPHA_TO_COVERAGE: return "GL_SAMPLE_ALPHA_TO_COVERAGE";
			case GL_SAMPLE_COVERAGE:		  return "GL_SAMPLE_COVERAGE";
			case GL_SCISSOR_TEST:			  return "GL_SCISSOR_TEST";
			case GL_STENCIL_TEST:			  return "GL_STENCIL_TEST";
			default: return "GL_STENCIL_TEST"; //this can happen only if someone uses invalid value...
			}
		}

		static unsigned int m_capabilities_table[max_CAPABILITY_IDX];

		static bool m_actual_capabilities[max_CAPABILITY_IDX];
		static bool m_desired_capabilities[max_CAPABILITY_IDX][2];

		static const size_t m_vertex_attrib_arrays_count = 16;
		static bool m_actual_enabled_vertex_attrib_arrays[m_vertex_attrib_arrays_count];
		static bool m_desired_enabled_vertex_attrib_arrays[m_vertex_attrib_arrays_count][2];
		
		static unsigned int m_actual_texture_unit[2]; //standalone, GlActiveTexture() commits itself

		static unsigned int m_actual_blendfunc_sfactor;
		static unsigned int m_actual_blendfunc_dfactor;
		static unsigned int m_desired_blendfunc_sfactor[2];
		static unsigned int m_desired_blendfunc_dfactor[2];

		static unsigned int m_actual_cullface_mode;
		static unsigned int m_desired_cullface_mode[2];

		static unsigned int m_actual_depth_func;
		static unsigned int m_desired_depth_func[2];

		static unsigned char m_actual_depth_mask[2]; //standalone, GlDepthMask() commits itself

		static unsigned int m_actual_shader_program[2]; //standalone, GlUseProgram() commits itself
	};

}//namespace GLB

#endif // GLB_OPENGL_STATE_MANAGER_H
