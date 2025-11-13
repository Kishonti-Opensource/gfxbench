/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GL_CAPTURE_HELPER_H
#define GL_CAPTURE_HELPER_H

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>
#include <set>
#include <cstdarg>
#include <exception>
#include <io.h>
#include <direct.h>

namespace Original_GL
{
#ifdef ANDROID
#ifdef HAVE_GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#elif HAVE_GLES2
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GLES/gl.h>
#include <GLES/glext.h>
#endif
#elif defined __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES2/gl.h>
// TODO: if IOS7 include ES3
#else
#if defined HAVE_GLEW
#include <GL/glew.h>
#elif defined HAVE_GL
#include <OpenGL/gl.h>
#else
#error "Cannot find OpenGL headers"
#endif
#endif
#elif defined HAVE_DX
// Nothing to include
#else
#if defined HAVE_GLEW
#include <GL/glew.h>
#elif defined HAVE_GL
#include <GL/gl.h>
#elif HAVE_GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#elif HAVE_GLES2
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#error "Cannot find OpenGL headers"
#endif
#endif
}

#ifndef EXPATH
#define EXPATH ""
#endif

// glGetError calls during capture, printing to the console
#define FRAMECAPTURE_CHECK_ERRORS

// write glGetError calls to the captured file after each GL call.
// #define FRAMEPLAYER_CHECK_ERRORS

// print data access/write statistics (useful for debugging why framecapture is slow)
//#define DEBUG_DATAWRITE_STATS

#define MAX_INSTRUCTION_PER_FUNCTION 300
#define MAX_FUNCTIONS_PER_CPP 20

bool enable_logging = false;
int glids_padding_to = 16;
int instr_in_func = -1;
int func_ctr=0;
int local_func_ctr=0;
int cpp_ctr=0;
int rendercounter = 0;
std::vector<std::string> functions_to_call;
GLuint current_vbo = 0;
GLuint current_ebo = 0;
GLuint current_pbo = 0;
GLuint current_to = 0;
std::vector<GLuint> g_GLids;
std::string testName;

std::vector<GLfloat> floats;
std::set<int> floatIndexes;
std::vector<GLenum> enums;
std::set<int> enumIndexes;

FILE *headerfile = NULL;
FILE *sourcefile = NULL;

inline float float_sane(const float *f)
{
	if ((*f) != (*f))  return 0;
	if (*f<-FLT_MAX) return -FLT_MAX;
	if (*f>FLT_MAX) return FLT_MAX;
	return *f;
}

inline double float_sane(const double *f)
{
	if ((*f) != (*f))  return 0;
	if (*f<-DBL_MAX) return -DBL_MAX;
	if (*f>DBL_MAX) return DBL_MAX;
	return *f;
}

int getSizeOf(int width, int height, int depth, GLenum format, GLenum type)
{
	switch( type)
	{
		case GL_UNSIGNED_BYTE_3_3_2:
		case GL_UNSIGNED_BYTE_2_3_3_REV:
			return width * height * depth;
			break;
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_5_6_5_REV:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:
		{
			return width * height * depth * 2;
		}
		case GL_UNSIGNED_INT_8_8_8_8:
		case GL_UNSIGNED_INT_8_8_8_8_REV:
		case GL_UNSIGNED_INT_10_10_10_2:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		{
			return width *height * depth * 4;
		}
	default:
		{
			int bpp = 1;
			switch (format)
			{
				case GL_RED:
				case GL_RED_INTEGER:
				case GL_DEPTH_COMPONENT:
				case GL_STENCIL_INDEX:
				case GL_LUMINANCE:
					break;
				case GL_RG:
				case GL_RG_INTEGER:
				case GL_DEPTH_STENCIL:
					bpp *=2;
					break;
				case GL_RGB:
				case GL_BGR:
				case GL_RGB_INTEGER:
				case GL_BGR_INTEGER:
					bpp *=3;
					break;
				case GL_RGBA:
				case GL_BGRA:
				case GL_RGBA_INTEGER:
				case GL_BGRA_INTEGER:
					bpp *=4;
					break;
			}
			switch (type)
			{
				case GL_UNSIGNED_BYTE: 
				case GL_BYTE:
					break;
				case GL_UNSIGNED_SHORT:
				case GL_SHORT:
					bpp*=2;
					break;
				case GL_UNSIGNED_INT:
				case GL_INT:
				case GL_FLOAT:
					bpp*=4;
					break;
			}
			return bpp*width*height*depth;
		}
	}
}

inline std::string getGlErrorStr(GLenum errcode)
{
	switch (errcode)
	{
		case 0x0500: return "GL_INVALID_ENUM";
		case 0x0501: return "GL_INVALID_VALUE";
		case 0x0502: return "GL_INVALID_OPERATION";
		case 0x0505: return "GL_OUT_OF_MEMORY";
		default:
			char code[10];
			sprintf(code, "0x%04X", errcode);
			return code;
			break;
	}
}

inline void getGlError(std::string str)
{
	GLenum errint = Original_GL::glGetError();
	if (errint)
	{
		printf("%s at %s\n",getGlErrorStr(errint).c_str(),str.c_str());
	}
}

enum FramePlayerStatus {
	FP_NONE,
	FP_MAIN,
	FP_INIT,
	FP_INITDONE,
	FP_RENDER
} status = FP_NONE;

template <typename ItemType>
int put(std::vector<ItemType>& to, std::set<int>& indexes, GLsizei count, const ItemType* v)
{
	int startidx = to.size();
	for (std::set<int>::iterator iter= indexes.begin();iter!=indexes.end();iter++)
	{
		bool found = true;
		for ( int i=0;i<count && *iter+i<floats.size();i++)
		{
			if (to[*iter+i]!=v[i])
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			startidx = *iter;
			break;
		}
	}
	int skip = to.size()-startidx;
	for (int i=skip;i<count;++i) to.push_back(v[i]);
	indexes.insert(startidx);
	return startidx;
}

int putFloats(GLsizei count, const GLfloat* v)
{
	return put(floats,floatIndexes,count,v);
}

int putEnums(GLsizei count, const GLenum* v)
{
	return put(enums,enumIndexes,count,v);
}

void startcpp()
{
	if (sourcefile) fclose(sourcefile);
	cpp_ctr++;
	local_func_ctr=0;
	char filename[2048];
	sprintf( filename, "%s%s_%d.cpp", EXPATH, testName.c_str(),cpp_ctr);

	sourcefile = fopen ( filename, "wb");

	fprintf( sourcefile, "#include \"%s.h\"\n\n", testName.c_str());
}

void printzerocpp()
{
	char filename[2048];
	sprintf( filename, "%s%s_0.cpp", EXPATH, testName.c_str());

	sourcefile = fopen ( filename, "wb");

	fprintf( sourcefile, "#include \"%s.h\"\n",testName.c_str());
	fprintf( sourcefile, "#include \"frameplayer.h\"\n\n");

	fprintf( sourcefile, "CREATE_FACTORY(%s, FramePlayer<%s>)\n",testName.c_str(), testName.c_str());

	fclose(sourcefile);
}

void startPlayerInstance(std::string tName)
{
	testName = tName;

	if (status!=FP_NONE) throw std::exception("startPlayerInstance called when a player capture already in progress");

	glids_padding_to = 16;
	rendercounter = 0;
	func_ctr = 0;
	local_func_ctr = 0;
	cpp_ctr = 0;
	instr_in_func = -1;
	enable_logging = false;
	functions_to_call.clear();
	floats.clear();
	enums.clear();

	char filename[2048];
	sprintf( filename, "%s%s.h", EXPATH, testName.c_str());
	headerfile = fopen( filename, "wb");

	printzerocpp();
	startcpp();

	fprintf( headerfile, "#ifndef %s_H\n#define %s_H\n\n",testName.c_str(),testName.c_str());
	fprintf( headerfile, "#include \"gl_player_helper.h\"\n\n");
	fprintf( headerfile, "class %s : public GLB::FramePlayer\n{\n",testName.c_str());
	fprintf( headerfile, "private:\n");
	fprintf( headerfile, "\tvirtual void initFP ();\n");
	fprintf( headerfile, "\tvirtual void renderFP ();\n");
	status = FP_MAIN;
}

void start_function()
{
	if (instr_in_func!=-1) throw std::exception("start_function called twice");
	++func_ctr;
	++local_func_ctr;

	if (local_func_ctr>MAX_FUNCTIONS_PER_CPP) { startcpp(); ++local_func_ctr; }

	char funcname[2048];
	sprintf( funcname, "func%d()", func_ctr);

	fprintf( headerfile, "\tvoid %s;\n",funcname);
	fprintf( sourcefile, "void %s::%s\n{\n", testName.c_str(),funcname);
	functions_to_call.push_back(funcname);
	instr_in_func=0;
}

void end_function()
{
	if (instr_in_func==-1) return;
	fprintf( sourcefile, "}\n\n");
	instr_in_func=-1;
}

void logplain_cpp(const char* str)
{
	if (instr_in_func==-1) start_function();
	fprintf( sourcefile,"%s",str);
	++instr_in_func;
	if (instr_in_func>=MAX_INSTRUCTION_PER_FUNCTION) end_function();
}

void startInit()
{
	if (status!=FP_MAIN) throw std::exception("startInit called when init capture already in progress, init already captured or not a valid object");
	status = FP_INIT;
	glids_padding_to = g_GLids.size();
	enable_logging = true;
}

void endInit()
{
	if (status!=FP_INIT) throw std::exception("endInit called but currently not capturing initialization");
	status = FP_INITDONE;
	end_function();
	fprintf( sourcefile, "void %s::initFP()\n{\n",testName.c_str());
	if (glids_padding_to>16) fprintf( sourcefile, "\tfor (int i=16;i<%d;++i) g_GLids.push_back(i);\n", glids_padding_to);
	for (int i=0;i<functions_to_call.size();i++)
	{
		fprintf(sourcefile,"\t%s;\n",functions_to_call[i].c_str());
	}
	fprintf( sourcefile, "}\n\n");
	functions_to_call.clear();
	enable_logging = false;
}

void startRender()
{
	if (status!=FP_INITDONE) throw std::exception("startRender called when render capture already in progress, initialization not captured or not a valid object");
	status = FP_RENDER;
	enable_logging = true;
}

void endRender()
{
	if (status!=FP_RENDER) throw std::exception("endRender called but currently not capturing render");
	status = FP_INITDONE;
	end_function();
	++rendercounter;
	fprintf( headerfile, "\tvoid render%d();\n",rendercounter);
	fprintf( sourcefile, "void %s::render%d()\n{\n",testName.c_str(),rendercounter);
	for (int i=0;i<functions_to_call.size();i++)
	{
		fprintf(sourcefile,"\t%s;\n",functions_to_call[i].c_str());
	}
	fprintf( sourcefile, "}\n\n");
	functions_to_call.clear();
	enable_logging = false;
}

void endPlayerInstance()
{
	if (status!=FP_INITDONE) throw std::exception("endPlayerInstance called when render or initialization capture is in progress, initialization not captured or not a valid object");
	fprintf( sourcefile, "void %s::renderFP()\n{\n", testName.c_str());
	fprintf(sourcefile,"\tswitch (getFrames()%%%d)\n\t{\n",rendercounter);
	for (int i=1;i<=rendercounter;i++)
	{
		fprintf( sourcefile, "\t\tcase %d: render%d();break;\n",i-1,i);
	}
	fprintf(sourcefile,"\t}\n");
	fprintf( sourcefile, "}\n\n");
	if (floats.size()>0)
	{
		fprintf( headerfile,"\tstatic const GLfloat floats[%d];\n",floats.size());
		fprintf( sourcefile,"const GLfloat %s::floats[%d] = { %#gf",testName.c_str(), floats.size(),float_sane(&floats[0]));
		for (int i=1;i<floats.size();++i)
		{
			fprintf( sourcefile, ", %#gf",float_sane(&floats[i]));
		}
		fprintf( sourcefile," };\n");
	}
	if (enums.size()>0)
	{
		fprintf( headerfile,"\tstatic const GLenum enums[%d];\n", enums.size());
		fprintf( sourcefile,"const GLenum %s::enums[%d] = { 0x%x", testName.c_str(), enums.size(), enums[0]);
		for (int i=1;i<enums.size();++i)
		{
			fprintf( sourcefile, ", 0x%x",enums[i]);
		}
		fprintf( sourcefile," };\n");
	}
	fprintf( headerfile, "};\n");
	fprintf( headerfile, "#endif");
	fclose(headerfile);
	fclose(sourcefile);
}

#ifdef DEBUG_DATAWRITE_STATS
int fileid_access[32768];
#endif

void glewInit()
{
	g_GLids.clear();
	g_GLids.push_back( 0);
	g_GLids.push_back( 1);
	g_GLids.push_back( 2);
	g_GLids.push_back( 3);
	g_GLids.push_back( 4);
	g_GLids.push_back( 5);
	g_GLids.push_back( 6);
	g_GLids.push_back( 7);
	g_GLids.push_back( 8);
	g_GLids.push_back( 9);
	g_GLids.push_back( 10);
	g_GLids.push_back( 11);
	g_GLids.push_back( 12);
	g_GLids.push_back( 13);
	g_GLids.push_back( 14);
	g_GLids.push_back( 15);

	Original_GL::glewInit();
#ifdef DEBUG_DATAWRITE_STATS
	for (int i=0;i<32768;i++)
	{
		fileid_access[i]=0;
	}
#endif
}

int DataWrite( int idx, int size, const char *data )
{
	FILE *file;
	char tmp[512];
	char dirname[512]={0};
	int savepos=0;
	if (idx<0) return -1;
	if( !data) return -1;
	if (size<=0) return -1;

	srand(idx);
	int ctr;

#ifdef DEBUG_DATAWRITE_STATS
	fileid_access[idx]++;
	int skipped=0;
#endif

	while (1)
	{
		ctr=rand();
		sprintf( tmp, "%sbin/%d.bin", EXPATH, ctr);

#ifdef WIN32
		if(_access(tmp,0) == -1) {
#else
		if(stat(tmp,&status)!=0) {
#endif
			for(unsigned int i = 0; i < strlen(tmp); i++)
			{
				if(tmp[i]=='\\' || tmp[i]=='/')
				{
					savepos = i;
				}
			}
			if(savepos != 0)
			{
				strncpy(dirname, tmp, savepos + 1);
				dirname[savepos+1] = 0;
			}
			
			_mkdir(dirname);
		}
		file = fopen( tmp, "r+b");
		if (!file)
		{
			file = fopen( tmp, "w+b");
			if (!file) return -1;
			break;
		}
		
		fseek(file,0,SEEK_END);
		int fs = ftell(file);
		if (fs!=size) {
			fclose(file);
#ifdef DEBUG_DATAWRITE_STATS
			skipped++;
#endif
			continue;
		}
		fseek(file,0,SEEK_SET);
		char *mem = new char[size];
		fread(mem,size,1,file);
		fclose(file);
		if (memcmp(mem,data,size)==0)
		{
#ifdef DEBUG_DATAWRITE_STATS
			printf("Id %5d MapTo %5d Access %5d Skipped %5d\n",idx,ctr, fileid_access[idx],skipped);
#endif
			delete mem;
			return ctr;
		}
		delete mem;
#ifdef DEBUG_DATAWRITE_STATS
		skipped++;
#endif
	}

#ifdef DEBUG_DATAWRITE_STATS
	printf("Id %5d MapTo %5d Access %5d Skipped %5d\n",idx,ctr, fileid_access[idx],skipped);
#endif

	fseek(file,0,SEEK_SET);

	fwrite( data, size, 1, file);
	fclose( file);
	return ctr;
}

void log_cpp(const char* format, ...)
{
	va_list args;
	va_start(args,format);

	size_t charbufsize= vsnprintf(0,0,format,args);
	char* charbuf=new char[charbufsize+1];
	charbuf[charbufsize]=0;
	vsnprintf(charbuf,charbufsize,format,args);

	logplain_cpp(charbuf);

	delete charbuf;

	va_end( args);

}

void glDrawBuffer (Original_GL::GLenum mode)
{
	Original_GL::glDrawBuffer( mode);
}


void glDepthRange (float zNear, float zFar)
{
	Original_GL::glDepthRange( zNear, zFar);
}


void glPolygonMode (Original_GL::GLenum face, Original_GL::GLenum mode)
{
	Original_GL::glPolygonMode ( face, mode);
}

#undef GL_TEXTURE_1D

#endif