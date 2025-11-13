/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "newnotificationmanager_opengl.h"
#include "kcl_os.h"
#include "opengl/glbshader.h"
#include "opengl/ext.h"
#include "glb_texture.h"

namespace GLB {
		
	NewNotificationManager* NewNotificationManager::NewInstance()
	{
        bool needsCore = GLB::g_extension->hasFeature(GLB::GLBFEATURE_vertex_array_object);  

        return new NewNotificationManagerGL(needsCore);
	}
	
	const GLfloat NewNotificationManagerGL::m_vertices[8] = 
	{ 
		-1.0f,  1.0f,
		-1.0f, -1.0f,
		 1.0f,  1.0f,
		 1.0f, -1.0f 
	};
	const GLushort NewNotificationManagerGL::m_indices[4] = { 0,1,2,3 };
		
	NewNotificationManagerGL::NewNotificationManagerGL(bool needsCore)
	{		
        m_needsCore = needsCore;
		m_program = 0;

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

        std::string vssource;
        std::string fssource;

		GraphicsContext *ctx = GLB::g_extension->getGraphicsContext() ;
		if( ( (ctx->type() == GraphicsContext::OPENGL) && (ctx->versionMajor() >= 4) ) ||
			( (ctx->type() == GraphicsContext::GLES)   && (ctx->versionMajor() >= 3) ) )
        {
            std::string srcVert;
            std::string srcFrag;

			if(GLB::g_extension->getGraphicsContext()->type() == GraphicsContext::GLES)
			{
				vssource = "#version 300 es\n";
				fssource = "#version 300 es\n";
			}
			else
			{
				vssource = "#version 400 core\n";
				fssource = "#version 400 core\n";
			}

		vssource += 
			"\
			#ifdef GL_ES\n\
		    precision highp float;\n\
		    #endif\n\
			uniform vec2 texScale;\n\
            uniform int rotate;\n\
			in vec2 texPosition;\n\
			out vec2 v_texCoord;\n\
			\n\
			void main()\n\
			{\n\
				if (rotate>0)\n\
				{\n\
					v_texCoord = ( texPosition.yx * texScale.yx * vec2(-1.0,1.0) + vec2(1.0) ) * 0.5;\n\
					gl_Position = vec4(texPosition, 1.0, 1.0);\n\
				} else {\n\
					v_texCoord = (texPosition * texScale + vec2(1.0) ) * 0.5;\n\
					gl_Position = vec4(texPosition, 1.0, 1.0);\n\
				}\n\
			}\n\
			\n\
			"
			;

		fssource +=  
			"\
			#ifdef GL_ES\n\
		    precision mediump float;\n\
		    #endif\n\
			uniform sampler2D texLogo;\n\
			in vec2 v_texCoord;\n\
			out vec4 frag_color;\n\
			\n\
			void main() {\n\
				frag_color = texture(texLogo,v_texCoord);\n\
			}\n\
			" ; 
        }
        else
        {
		vssource = 
			"\
		    #ifdef GL_ES\n\
		    precision highp float;\n\
		    #endif\n\
			attribute vec2 texPosition;\n\
			uniform vec2 texScale;\n\
			varying vec2 v_texCoord;\n\
			uniform int rotate;\n\
			\n\
			void main()\n\
			{\n\
				if (rotate>0)\n\
				{\n\
					v_texCoord = ( texPosition.yx * texScale.yx * vec2(-1.0,1.0) + vec2(1.0) ) * 0.5;\n\
					gl_Position = vec4(texPosition, 1.0, 1.0);\n\
				} else {\n\
					v_texCoord = (texPosition * texScale + vec2(1.0) ) * 0.5;\n\
					gl_Position = vec4(texPosition, 1.0, 1.0);\n\
				}\n\
			}\
			\n\
			"
			;

		fssource =  
			"\
		    #ifdef GL_ES\n\
		    precision mediump float;\n\
		    #endif\n\
			varying vec2 v_texCoord;\n\
			uniform sampler2D texLogo;\n\
			\n\
			void main() {\n\
				gl_FragColor = texture2D(texLogo,v_texCoord);\n\
			}\n\
			" ; 
        }

        const char* vss = vssource.c_str();
        const char* fss = fssource.c_str();

        glShaderSource(vs,1,&vss,NULL);

		glShaderSource(fs,1,&fss,NULL);

		glCompileShader(vs);

		GLint compiled = 0;
		GLint len = 0;

		glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &len);
			char *str = new char[len+1];
			str[0] = 0;
			glGetShaderInfoLog(vs,len,&len,str);
			INFO("NNM VS Err: %s",str);
			delete[]str;
		}

		glCompileShader(fs);

		glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &len);
			char *str = new char[len+1];
			str[0] = 0;
			glGetShaderInfoLog(fs,len,&len,str);
			INFO("NNM FS Err: %s",str);
			delete[]str;
		}

		m_program = glCreateProgram();
		glAttachShader(m_program,vs);
		glAttachShader(m_program,fs);

        glBindAttribLocation(m_program, 0, "texPosition");

		glLinkProgram(m_program);

		GLint link_status = 0;
	
		GLint validate_status = 0;

		glGetProgramiv(m_program,GL_LINK_STATUS, &link_status);
	
		if (link_status==GL_FALSE)
		{
			len = 0;
			glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &len);
			char *str = new char[len + 1];
			str[0] = 0;
			glGetProgramInfoLog(m_program,len,&len,str);
			INFO("NNM PRG Err: %s",str);
			delete[]str;
		}

		glDeleteShader(vs);
		glDeleteShader(fs);

#if defined __glew_h__
        if(m_needsCore)
        {
		    m_vao = 0;
		    m_vertex_buffer = 0;
		    m_index_buffer = 0;
		
            GLint coord_loc = glGetAttribLocation(m_program,"texPosition");
            glGenBuffers(1, &m_vertex_buffer);
        
            glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
		    glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glGenBuffers(1, &m_index_buffer);
		    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
		    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		    glGenVertexArrays(1, &m_vao);
		    glBindVertexArray(m_vao);
	
		
		    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);		
		    glEnableVertexAttribArray(coord_loc);
		    glVertexAttribPointer(coord_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);

		    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
		
		    glBindVertexArray(0);
            glBindBuffer( GL_ARRAY_BUFFER, 0);
	        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
        }
#endif

	}

	NewNotificationManagerGL::~NewNotificationManagerGL()
	{
		glDeleteProgram(m_program);

#if defined __glew_h__
        if(m_needsCore)
        {
		    glDeleteVertexArrays(1, &m_vao);
		    glDeleteBuffers(1, &m_vertex_buffer);
		    glDeleteBuffers(1, &m_index_buffer);
        }
#endif
	}

	void NewNotificationManagerGL::ShowLogo(bool stretch, bool blend)
	{
		int texHeight = m_texture->getHeight();
		int texWidth = m_texture->getWidth();

		GLint params[4];
		
		glGetIntegerv(GL_VIEWPORT, params);
		GLint width = params[2];
		GLint height = params[3];

		GLfloat scalex = 1;
		GLfloat scaley = -1;

		if (!stretch)
		{
			scalex=((float)width)/texWidth;
			scaley=-((float)height)/texHeight;
		}

		GLint progid,eabid,abid,vaaEnabled,vaaSize, vaaStride, vaaNormalized,vaaType;
		GLboolean depthState,blendState,cullState;
		void* vaaPointer;
		glGetIntegerv(GL_CURRENT_PROGRAM,&progid);
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&abid);
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING,&eabid);
		depthState = glIsEnabled(GL_DEPTH_TEST);
		blendState = glIsEnabled(GL_BLEND);
		cullState = glIsEnabled(GL_CULL_FACE);
			
		glUseProgram(m_program);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		glDisable(GL_DEPTH_TEST);
		if (blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);
		GLint tex0_loc = glGetUniformLocation(m_program,"texLogo");
		GLint scale_loc = glGetUniformLocation(m_program,"texScale");
		GLint coord_loc = glGetAttribLocation(m_program,"texPosition");
		GLint rotate_loc = glGetUniformLocation(m_program, "rotate");
		glUniform1i(rotate_loc, width < height);
		glUniform2f(scale_loc,scalex,scaley);

		if (!m_needsCore)
		{
			glGetVertexAttribiv(coord_loc,GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vaaEnabled);
			glGetVertexAttribPointerv(coord_loc,GL_VERTEX_ATTRIB_ARRAY_POINTER, &vaaPointer);
			glGetVertexAttribiv(coord_loc,GL_VERTEX_ATTRIB_ARRAY_SIZE, &vaaSize);
			glGetVertexAttribiv(coord_loc,GL_VERTEX_ATTRIB_ARRAY_TYPE, &vaaType);
			glGetVertexAttribiv(coord_loc,GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &vaaNormalized);
			glGetVertexAttribiv(coord_loc,GL_VERTEX_ATTRIB_ARRAY_STRIDE, &vaaStride);
		}

		m_texture->bind(0);
		glUniform1i(tex0_loc, 0);

#if defined __glew_h__
        if(m_needsCore)
        {
		    glBindVertexArray(m_vao); //NOTE: glValidateProgram needs vao to be bound to success when running with core profile
        }
#endif
		glValidateProgram(m_program);
		GLint validate_status;
		glGetProgramiv(m_program,GL_VALIDATE_STATUS,&validate_status);
		if(validate_status!=GL_TRUE)
		{
			int len = 0;
			glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &len);
			char *str = new char[len + 1];
			str[0] = 0;
			glGetProgramInfoLog(m_program,len,&len,str);
			INFO("NNM Validation Err: %s",str);
			delete[]str;
		}

#if defined __glew_h__
        if(m_needsCore)
        {
            glBindVertexArray(0);
        }
#endif

		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if defined __glew_h__
        if(m_needsCore)
        {
		    glBindVertexArray(m_vao);
		    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
		    glBindVertexArray(0);
        }
        else
        {
		    glEnableVertexAttribArray(coord_loc);
		    glVertexAttribPointer(coord_loc, 2, GL_FLOAT, GL_FALSE, 0, m_vertices);
		    glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT, m_indices);
        }
#else
		glEnableVertexAttribArray(coord_loc);
		glVertexAttribPointer(coord_loc, 2, GL_FLOAT, GL_FALSE, 0, m_vertices);
		glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT, m_indices);
#endif
		if (depthState) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
		if (blendState) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		if (cullState) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
		glUseProgram(progid);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,eabid);
		glBindBuffer(GL_ARRAY_BUFFER,abid);

		if (!m_needsCore)
		{
			glEnableVertexAttribArray(coord_loc);
			glVertexAttribPointer(coord_loc,vaaSize,vaaType,vaaNormalized!=0,vaaStride,vaaPointer);
			glDisableVertexAttribArray(coord_loc);

			if (vaaEnabled)
			{
				glEnableVertexAttribArray(coord_loc);
			}
			else
			{
				glDisableVertexAttribArray(coord_loc);
			}
		}
	}
	
	KCL::Texture *NewNotificationManagerGL::CreateTexture(const KCL::Image* img, bool releaseUponCommit)
	{
		GLB::GLBTextureFactory f;
		return f.CreateTexture(img, releaseUponCommit);
	}
}
