/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <string>

#include <kcl_os.h>

#include "opengl/glbshader.h"
#include "zlib.h"
#include "stdc.h"

using namespace std;

namespace GLB
{

	static bool save_shaders=false;

	void SaveShaders( bool yes)
	{
		save_shaders = yes;
	}


	void deleteProgram(KCL::uint32 program)
	{
#ifndef ZUNE_HD 
		GLuint shd[2]={0,0};
		if( program)
		{
			glGetAttachedShaders (program, 2, NULL, shd);
			if(shd[0])
			{
				glDeleteShader (shd[0]);
				shd[0] = 0;
			}
			if(shd[1])
			{
				glDeleteShader (shd[1]);
				shd[1] = 0;
			}
			if(program)
			{
				glDeleteProgram(program);
				program = 0;
			}
		}
#endif
	}


	/// Initilizes binary or source shaders
	KCL::uint32 initShader(GLenum shaderType, const char * shaderSource)
	{
		if(!shaderSource)
		{
			INFO("shadreSource is null!!!");
			return 0;
		}

		KCL::uint32 shaderId = 0;
		char fileName[256] = {0};
		GLboolean does_shader_compiler_exist = true;

		KCL::uint32 shaderKey = adler32(0,(const unsigned char *)shaderSource, (uInt)strlen(shaderSource));

		
		if(shaderType==GL_VERTEX_SHADER)
		{
			sprintf (fileName, "%08x.%s", (unsigned int)shaderKey, "glslv");
		}
		else
		{
			sprintf (fileName, "%08x.%s", (unsigned int)shaderKey, "glslf");
		}
		if(save_shaders)
		{
			GLBFILE *fp = glb_fopen (fileName, "w");

			if(fp)
			{
				glb_fwrite (shaderSource, sizeof (KCL::uint8), strlen(shaderSource), fp);
				glb_fclose (fp);
			}
		}

#if defined GL_SHADER_COMPILER && !defined EMSCRIPTEN
		glGetBooleanv(GL_SHADER_COMPILER,&does_shader_compiler_exist);
#endif	

		shaderId = glCreateShader (shaderType);

		if( does_shader_compiler_exist)
		{
			GLint compiled = 0;
			GLint len = 0;

			while (glGetError());

			glShaderSource (shaderId, 1, (const char **)&shaderSource, NULL);
			glCompileShader (shaderId);

			glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compiled);
			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &len);
							
			
			if(len > 8192)
			{
				INFO("GL_INFO_LOG_LENGTH have to chunked to 8192 %d",len);
				len = 8192;				
			}
				
			if(len < 1)
			{
				len = 512;
			}
			char *str = new char[len+1];
			if(!str)
			{
				return 0;
			}
			str[0] = 0;
			glGetShaderInfoLog(shaderId,len,&len,str);
			delete[]str;

			if (!compiled) 
			{
				string error;
				int logLength, charsWritten;
				glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
				char* infoLog = new char[logLength];
				glGetShaderInfoLog(shaderId, logLength, &charsWritten, infoLog);
				if(shaderType==GL_FRAGMENT_SHADER)
				{
					error+="FragmentShader: (";
					error+=shaderSource;
					error+=") ";
				}
				else
				{
					error+="VertexShader: (";
					error+=shaderSource;
					error+=") ";
				}
				error += infoLog;
				delete [] infoLog;
				INFO(" %s\n", error.c_str());
				return 0;
			}
		}
		else
		{
#ifdef ZUNE_HD
			char vs_ext[]="nvbv";
			char fs_ext[]="nvbf";
			GLenum binaryFormat=GL_NVIDIA_PLATFORM_BINARY_NV;
#else
			char vs_ext[]="glslv.bin";
			char fs_ext[]="glslf.bin";
#ifdef GL_SHADER_COMPILER
			GLenum binaryFormat=0;
#endif
#endif

#ifdef OPENKODE
			if(shaderType==GL_VERTEX_SHADER)
			{
				sprintf (fileName, "%s%08x.%s", GLB::g_os->GetShaderDirectory(), (unsigned int)shaderKey, "nvbv");
			}
			else
			{
				sprintf (fileName, "%s%08x.%s", GLB::g_os->GetShaderDirectory(), (unsigned int)shaderKey, "nvbf");
			}
			char *data;
			KCL::int32 len;
			struct KDStat st;
			GLBFILE *fp = glb_fopen (fileName, "rb");
			if(!fp || kdFstat(fp, &st))
			{
				return ~0;
			}
			len=st.st_size;
			data = (char *)glb_malloc(len + 1);
			glb_fread(data, st.st_size, 1, fp);
			*(data + st.st_size) = '\0';
			glb_fclose(fp);
			glShaderBinary(1,&shaderId,GL_NVIDIA_PLATFORM_BINARY_NV,data,len);
#else
			if(shaderType==GL_VERTEX_SHADER)
			{
				sprintf (fileName, "%08x.%s", (unsigned int)shaderKey, vs_ext);
			}
			else
			{
				sprintf (fileName, "%08x.%s", (unsigned int)shaderKey, fs_ext);
			}
			char *data;
			KCL::int32 len;
			GLBFILE *fp = glb_fopen (fileName, "rb");
			if(!fp)
			{
				INFO("file not found:%s.\n%s",fileName,shaderSource);
				return 0;
			}
			fseek(fp,0,SEEK_END);
			len=ftell(fp);
			fseek(fp,0,SEEK_SET);
			data = (char *)glb_malloc(len + 1);
			glb_fread(data, len, 1, fp);
			data[len]=0;
			glb_fclose(fp);
#ifdef GL_SHADER_COMPILER
			glShaderBinary(1,&shaderId,binaryFormat,data,len);
#endif

#endif
		}
		if(!shaderId)
		{
			GLint len = 0;
			glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &len);
			char *str = 0;
			str = new char[len+1];

			if(!str)
			{
				INFO("GL_INFO_LOG_LENGTH len %d",len);
				return 0;
			}

			str[0]=0;
			glGetShaderInfoLog(shaderId,len,&len,str);
			//if(len)
			//	INFO("shader log:%s.\n",str);
			delete[]str;
			return 0;
		}
		return shaderId;
	}


	KCL::uint32 initProgram (const char *vertSrc, const char *fragSrc, bool lowlevel, bool forceHighp)
	{
        std::string vertSrcString(vertSrc);
        std::string fragSrcString(fragSrc);
        if(forceHighp)
        {
            vertSrcString += "#ifdef GL_ES\nprecision highp float;\n#endif\n";   
            fragSrcString += "#ifdef GL_ES\nprecision highp float;\n#endif\n";
        }

        GLuint vertShader = initShader(GL_VERTEX_SHADER,vertSrcString.c_str());
        GLuint fragShader = initShader(GL_FRAGMENT_SHADER,fragSrcString.c_str());

		if(!vertShader || !fragShader)
		{
			return 0;
		}

		GLuint program = glCreateProgram();
		glAttachShader(program, vertShader);
		glAttachShader(program, fragShader);

		glBindAttribLocation(program, 0, "myVertex");
		if (lowlevel)
		{
			glBindAttribLocation(program, 1, "myVertex1");
			glBindAttribLocation(program, 2, "myVertex2");
			glBindAttribLocation(program, 3, "myVertex3");
			glBindAttribLocation(program, 4, "myVertex4");
			glBindAttribLocation(program, 5, "myTexCoord");
			glBindAttribLocation(program, 6, "myNormal");
		}
		else
		{
			glBindAttribLocation(program, 1, "myTexCoord");
		}
		glLinkProgram (program);


		GLint link_status = 0;
		GLint validate_status = 0;


		glGetProgramiv(program,GL_LINK_STATUS,&link_status);
		glValidateProgram(program);

		if(vertShader)
		{
			glDeleteShader(vertShader);
			vertShader = 0;
		}

		if(fragShader)
		{
			glDeleteShader(fragShader);
			fragShader = 0;
		}

		glGetProgramiv(program,GL_VALIDATE_STATUS,&validate_status);

		GLint len = 0;
		char *str = 0;

		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

		str = new char[len + 1];
		str[0] = 0;

		glGetProgramInfoLog(program,len,&len,str);

		if(validate_status!=GL_TRUE)
		{
			//ignore Tegra's "P1202: Texture's gl states do not match with shader's"
			if( !strstr(str,"P1202") && !strstr(str,"P1203"))
			{
				INFO("link status:%d, validate status:%d, program log:%s.\n",link_status,validate_status, str);
				glDeleteProgram(program);
				delete[]str;
				return 0;			
			}
			else
			{
				INFO("link status:%d, validate status:%d, program log:%s.\n",link_status,validate_status, str);
			}
		}

		delete[]str;
		str = 0;


		return program;
	}



	int glsl_log(GLuint obj, GLenum check_compile, const char *op)
	{
		int success = 0;

		// log output.
		int len = 0;
		char *str = 0;

		if(check_compile == GL_COMPILE_STATUS)
		{
			glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
			if(len > 0)
			{
				str = new char[len];
				glGetShaderInfoLog(obj, len, NULL, str);
			}
		}
		else
		{
			// LINK or VALIDATE
			glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
			if(len > 0)
			{
				str = new char[len];
				glGetProgramInfoLog(obj, len, NULL, str);
			}
		}

		if(str != NULL && *str != '\0')
		{
			INFO("%s",str);
			//			INFO(str);
		}

		if(str)
		{
			delete[] str;
			str = 0;
		}

		// check the compile / link status.
		if(check_compile == GL_COMPILE_STATUS)
		{
			glGetShaderiv(obj, check_compile, &success);
			if(!success)
			{
				glGetShaderiv(obj, GL_SHADER_SOURCE_LENGTH, &len);
				if(len > 0)
				{
					str = new char[len];
					glGetShaderSource(obj, len, NULL, str);
					if(str != NULL && *str != '\0')
					{
						//INFO(str);
						INFO("%s",str);
					}
					delete[] str;
					str = 0;
				}
			}
		}
		else
		{ // LINK or VALIDATE
			glGetProgramiv(obj, check_compile, &success);
		}

		if(!success && str)
		{
			INFO("%s",str);
		}
		return success;
	}
}
