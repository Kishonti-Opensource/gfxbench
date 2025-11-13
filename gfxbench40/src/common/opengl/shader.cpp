/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_base.h"
#include "kcl_os.h"
#include "kcl_io.h"


#include "shader.h"
#include "zlib.h"
#include "platform.h"

#include <stdio.h>
#include <ctype.h>
#include "assert.h"
#include "opengl/ext.h"

using namespace KCL;


extern KCL::OS *KCL::g_os;

Shader* Shader::m_last_shader = 0;


std::set< std::string> Shader::m_defines_debug;

std::vector<Shader*> Shader::m_shader_cache;


const char *Shader::m_uniform_block_names[UNIFORM_BLOCK_COUNT];
std::string Shader::m_uniform_block_data[UNIFORM_BLOCK_COUNT];

std::string Shader::m_shader_dir;
std::string Shader::m_shader_header;

bool Shader::m_ubo_support_enabled = false;

const int MAXLOGLEN = 4096;



#ifdef LOG_SHADERS

#include "misc2.h"

namespace LOGSHADER
{
	static const std::string uscore = "_";

	static const char* filename_begin = "GLB_SHADERLOG/shader_";
	static const char* shaderInfosLogFileName = "GLB_SHADERLOG/shader_infos.log";
	static const char* perShaderAluInfoFileName = "GLB_SHADERLOG/PerShaderAluInfo.log";
	static const char* perFrameAluInfoFileName = "GLB_SHADERLOG/PerFrameAluInfo.log";
	
	#ifdef SAVE_ALU_INFO_EXCEL
	static const char* perShaderAluInfoFileNameEXCEL = "GLB_SHADERLOG/PerShaderAluInfo.xml";
	static const char* perFrameAluInfoFileNameEXCEL = "GLB_SHADERLOG/PerFrameAluInfo.xml";
	#endif //SAVE_ALU_INFO_EXCEL


	static const char* glsl_ext = ".glsl";
	static const char* txt_ext = "_info.txt";


/* shader_infos.log contains N count paths of vertex and fragment shaderinfos:
[vertex shader info file path 0]
[fragment shader info file path 0]
...
[vertex shader info file path N-1]
[fragment shader info file path N-1]

//*/
	class ShaderInfoPaths
	{
		static ShaderInfoPaths* _instance;
		std::vector<std::string> _vertexShaderInfoPaths;
		std::vector<std::string> _fragmentShaderInfoPaths;
	public:
		static void Delete()
		{
			delete _instance;
			_instance = 0;
		}

		static const std::string VertexShaderInfoPath(int i)
		{
			if(i < 0) return "";
			ShaderInfoPaths *sip = Instance();

			if(i >= sip->_vertexShaderInfoPaths.size() ) return "";

			return sip->_vertexShaderInfoPaths[i];
		}
		
		static const std::string FragmentShaderInfoPath(int i)
		{
			if(i<0) return "";
			ShaderInfoPaths *sip = Instance();

			if(i >= sip->_fragmentShaderInfoPaths.size() ) return "";

			return sip->_fragmentShaderInfoPaths[i];
		}

	private:
		static ShaderInfoPaths* Instance()
		{
			if(_instance == 0)
			{
				_instance = new ShaderInfoPaths();
			}
			return _instance;
		}
		ShaderInfoPaths()
		{
			FILE *file;
			{
				std::string fileName = GLB::g_os->GetDataRWDirectory();
				fileName += shaderInfosLogFileName;
				file = fopen(fileName.c_str(), "r");
			}

			const size_t bufSz = 1024;
			char buf1[bufSz];
			char buf2[bufSz];
			char *result;
			std::string buf;
			while(true)
			{
				result = fgets(buf1, bufSz, file);
				if(result == 0) break;
				result = fgets(buf2, bufSz, file);
				if(result == 0) break;

				buf1[strlen(buf1)-1] = 0; // delete newline character
				buf2[strlen(buf2)-1] = 0; // delete newline character

				buf = buf1;
				_vertexShaderInfoPaths.push_back(buf);

				buf = buf2;
				_fragmentShaderInfoPaths.push_back(buf);
			}


			fclose(file);
		}
		ShaderInfoPaths(const ShaderInfoPaths&);
		ShaderInfoPaths& operator=(const ShaderInfoPaths&);
	};
	ShaderInfoPaths* ShaderInfoPaths::_instance = 0;


/* File format for ShaderInfo: *.txt
ALU Instructions
int
ALU/TEX Ratio if Bilinear
double
ALU/TEX Ratio if Trilinear
double
ALU/TEX Ratio if Aniso
double

//*/
	class ShaderInfo
	{
	public:
		ShaderInfo() : ALU_Instructions(0), ALU_TEX_Ratio_if_Bilinear(0.0), ALU_TEX_Ratio_if_Trilinear(0.0), ALU_TEX_Ratio_if_Aniso(0.0)
		{
		}
		virtual ~ShaderInfo()
		{
		}

		uint32 ALU_Instructions;
		double ALU_TEX_Ratio_if_Bilinear;
		double ALU_TEX_Ratio_if_Trilinear;
		double ALU_TEX_Ratio_if_Aniso;

		void loadShaderInfo(const char* path) // NO ERROR HANDLING !!!
		{
			FILE *file = fopen(path, "r");	

			_loadShaderInfo(file);

			fclose(file);
		}

		void serializeALU(std::string &result) const
		{
			result = "ALU Instructions\n";
			result += lexical_cast<std::string>(ALU_Instructions);
			result +="\nALU/TEX Ratio if Bilinear\n";
			result += lexical_cast<std::string>(ALU_TEX_Ratio_if_Bilinear);
			result +="\nALU/TEX Ratio if Trilinear\n";
			result += lexical_cast<std::string>(ALU_TEX_Ratio_if_Trilinear);
			result +="\nALU/TEX Ratio if Aniso\n";
			result += lexical_cast<std::string>(ALU_TEX_Ratio_if_Aniso);
			result += "\n";
		}

	protected:
		virtual void _loadShaderInfo(FILE *file) // NO ERROR HANDLING !!!
		{
			const int bufSz = 64;
			char buf[bufSz];
			std::string buf2;

			fgets(buf, bufSz, file); // skip line
			fgets(buf, bufSz, file);
			buf[strlen(buf)-1] = 0; // delete newline character
			buf2 = buf;
			ALU_Instructions = lexical_cast<uint32>(buf2);

			fgets(buf, bufSz, file); // skip line
			fgets(buf, bufSz, file);
			buf[strlen(buf)-1] = 0; // delete newline character
			buf2 = buf;
			ALU_TEX_Ratio_if_Bilinear = lexical_cast<double>(buf2);

			fgets(buf, bufSz, file); // skip line
			fgets(buf, bufSz, file);
			buf[strlen(buf)-1] = 0; // delete newline character
			buf2 = buf;
			ALU_TEX_Ratio_if_Trilinear = lexical_cast<double>(buf2);

			fgets(buf, bufSz, file); // skip line
			fgets(buf, bufSz, file);
			buf[strlen(buf)-1] = 0; // delete newline character
			buf2 = buf;
			ALU_TEX_Ratio_if_Aniso = lexical_cast<double>(buf2);
		}
	};


/* File format for FragmentShaderInfo: *.txt
ALU Instructions
int
ALU/TEX Ratio if Bilinear
double
ALU/TEX Ratio if Trilinear
double
ALU/TEX Ratio if Aniso
double
sampler2D fetch texture_unit0
int
sampler2D fetch texture_unit1
int
sampler2D fetch texture_unit2
int
sampler2D fetch texture_unit3
int
samplerCube fetch envmap0
int
samplerCube fetch envmap1
int
sampler2D fetch shadow_unit
int
samplerCube fetch normalization_cubemap
int
sampler2D fetch planar_reflection
int

//*/
	class FragmentShaderInfo : public ShaderInfo
	{
	protected:
		static const uint32 SamplerFetchesSz = 16;
	public:
		FragmentShaderInfo()
		{
			for(uint32 i=0; i<SamplerFetchesSz; SamplerFetches[i++] = 0);
			for(uint32 i=0; i<SamplerFetchesSz; BitPerSampler[i++] = 0);

			BitPerSampler[0] = 4; //texture_unit0
			BitPerSampler[1] = 4; //texture_unit1
			BitPerSampler[2] = 4; //texture_unit2
			BitPerSampler[3] = 4; //texture_unit3
			BitPerSampler[4] = 4; //envmap0
			BitPerSampler[5] = 4; //envmap1
			BitPerSampler[6] = 16; //shadow_unit
			BitPerSampler[7] = 4; //normalization_cubemap
			BitPerSampler[8] = 16; //planar_reflection

			sumbit = 0;
			sumfetch = 0;
		}

		uint32 TextureFetchesBitSize() const { return sumbit; }
		uint32 TextureFetches() const { return sumfetch; }

		uint32 SamplerFetches[SamplerFetchesSz];
	protected:
		
		uint32 BitPerSampler[SamplerFetchesSz];
		uint32 sumbit;
		uint32 sumfetch;

		virtual void _loadShaderInfo(FILE *file) // NO ERROR HANDLING !!!
		{
			ShaderInfo::_loadShaderInfo(file);
			
			const int bufSz = 128;
			char buf[bufSz];
			std::string buf2;
			char *check = 0;

			sumbit = 0;
			sumfetch = 0;
			for(int i=0; i<SamplerFetchesSz; ++i)
			{
				check = fgets(buf, bufSz, file); // skip line
				if(!check) return;
				check = fgets(buf, bufSz, file);
				if(!check) return;
				buf[strlen(buf)-1] = 0; // delete newline character
				buf2 = buf;
				SamplerFetches[i] = lexical_cast<uint32>(buf2);


				sumbit += (SamplerFetches[i] * BitPerSampler[i]);
				sumfetch += SamplerFetches[i];
			}
		}
	};


	struct FrameAluInfo
	{
		FrameAluInfo() : vertexALU(0), fragmentALU(0), fetchSum(0), fetchedTexBits(0)
		{
		}

		uint32 vertexALU;
		uint32 fragmentALU;
		uint32 fetchSum;
		uint32 fetchedTexBits;

		void serialize(std::string &result) const
		{
			result = "Vertex shader ALU Instructions sum\n";
			result += lexical_cast<std::string>(vertexALU);
			result += "\nFragment shader ALU Instructions sum\n";
			result += lexical_cast<std::string>(fragmentALU);
			result += "\nTexture-fetch Instructions sum\n";
			result += lexical_cast<std::string>(fetchSum);
			result += "\nTexture-fetch Fetched bits sum\n";
			result += lexical_cast<std::string>(fetchedTexBits);
			result += "\n";
		}
	};


	void SaveShaderAluInfo(FILE *file, int i, const ShaderInfo &vertexShaderInfo,  const FragmentShaderInfo &fragmentShaderInfo)
	{
		fprintf(file, "Shader %d\n", i);

		/*
		fprintf(file, "Vertex Shader\n");

		std::string tmp;
		vertexShaderInfo.serializeALU(tmp);
		fprintf(file, "%s", tmp.c_str());

		fprintf(file, "Fragment Shader\n");
		
		fragmentShaderInfo.serializeALU(tmp);
		fprintf(file, "%s", tmp.c_str());
		//*/

		std::string tmp;

		fprintf(file, "Vertex shader ALU Instructions\n");
		tmp = lexical_cast<std::string>(vertexShaderInfo.ALU_Instructions);
		fprintf(file, "%s\n", tmp.c_str());
		
		fprintf(file, "Fragment shader ALU Instructions\n");
		tmp = lexical_cast<std::string>(fragmentShaderInfo.ALU_Instructions);
		fprintf(file, "%s\n", tmp.c_str());
		
		fprintf(file, "Texture-fetch Instructions\n");
		tmp = lexical_cast<std::string>(fragmentShaderInfo.TextureFetches());
		fprintf(file, "%s\n", tmp.c_str());

		
	}

	
	void SaveFrameAluInfo(FILE *file, int i, const FrameAluInfo &frameAluInfo)
	{
		fprintf(file, "Frame %d\n", i);

		std::string tmp;
		frameAluInfo.serialize(tmp);
		fprintf(file, "%s", tmp.c_str());
	}



	//SAVE Shaders and create info files
	//this section is used when we save every shader's source and create info files for later use
	void replaceCharsInStr(std::string &source, const char* replaceThese, const std::string &toThat)
	{
		const char* it = replaceThese;
		
		while(*it)
		{
			size_t pos = 0;
			char c = *it++;

			while( std::string::npos != (pos = source.find(c, pos)) )
			{
				source.replace( pos, 1, toThat );
			}
		}
	}
	
	enum saveToTextFileMode { SAVEFILE, SAVEFILE_AND_GENERATEINFOLOG, GENERATEINFOLOG };
	void saveToTextFile(const std::string &fileName, const std::string &text, saveToTextFileMode mode = SAVEFILE)
	{
		FILE* outFile;

		std::string path = GLB::g_os->GetDataRWDirectory();
		path += fileName;

		if( SAVEFILE == mode || SAVEFILE_AND_GENERATEINFOLOG == mode )
		{
			outFile = fopen(path.c_str(), "w");
			fprintf(outFile, "%s\n", text.c_str() );
			fclose(outFile);
			outFile = 0;
		}

		if( GENERATEINFOLOG == mode || SAVEFILE_AND_GENERATEINFOLOG == mode )
		{
			static bool firstTime = true;
			std::string pathInfoLog = GLB::g_os->GetDataRWDirectory();
			pathInfoLog += shaderInfosLogFileName;

			if(firstTime)
			{
				outFile = fopen(pathInfoLog.c_str(), "w");
				firstTime = false;
			}
			else
			{
				outFile = fopen(pathInfoLog.c_str(), "a");
			}


			fprintf(outFile, "%s\n", path.c_str() );
			fclose(outFile);
		}
	}

	void LogShader(const char *vsfile, const std::string &vs_source, const char *fsfile, const std::string &fs_source, bool includeOriginalFileName = true)
	{
		static int counter = 0;
		++counter;

		std::string counterStr = "";
		if(counter < 10)
		{
			counterStr += "00";
		}
		else if(counter < 100)
		{
			counterStr += "0";
		}
		counterStr += lexical_cast<std::string>(counter);
		counterStr += uscore;

		std::string vertex_file_name   = filename_begin + counterStr + "00_Vert" + uscore;
		std::string fragment_file_name = filename_begin + counterStr + "01_Frag" + uscore;
		
		std::string vertex_file_nameALU   = filename_begin + counterStr + "10_Vert" + uscore;
		std::string fragment_file_nameALU = filename_begin + counterStr + "11_Frag" + uscore;

		if(includeOriginalFileName)
		{
			static const char* replaceThese = "/.\\";
			std::string tmp = vsfile;
			
			replaceCharsInStr(tmp, replaceThese, uscore);		
			vertex_file_name += tmp;
			vertex_file_nameALU += tmp;


			tmp = fsfile;

			replaceCharsInStr(tmp, replaceThese, uscore);
			fragment_file_name += tmp;
			fragment_file_nameALU += tmp;
		}

		saveToTextFile(vertex_file_name + glsl_ext, vs_source);
		saveToTextFile(fragment_file_name + glsl_ext, fs_source);

		saveToTextFile(vertex_file_nameALU + txt_ext, "", SAVEFILE_AND_GENERATEINFOLOG);
		saveToTextFile(fragment_file_nameALU + txt_ext, "", SAVEFILE_AND_GENERATEINFOLOG);
	}

	//SAVE Shaders and create info files -- END
}


#ifdef PER_FRAME_ALU_INFO
std::vector<LOGSHADER::FrameAluInfo*> Shader::FrameAluInfo;

void Shader::Push_New_FrameAluInfo()
{
	LOGSHADER::FrameAluInfo* newFrameAluInfo = new LOGSHADER::FrameAluInfo;
	FrameAluInfo.push_back(newFrameAluInfo);
}


void Shader::Add_FrameAluInfo(Shader* shader, uint32 samplesPassed, uint32 triangleCount)
{
	const size_t idx = FrameAluInfo.size()-1;
	
	FrameAluInfo[idx]->vertexALU += shader->VertexShaderInfo->ALU_Instructions * triangleCount * 3;
	FrameAluInfo[idx]->fragmentALU += shader->FragmentShaderInfo->ALU_Instructions * samplesPassed;
	FrameAluInfo[idx]->fetchSum += shader->FragmentShaderInfo->TextureFetches() * samplesPassed;
	FrameAluInfo[idx]->fetchedTexBits += shader->FragmentShaderInfo->TextureFetchesBitSize() * samplesPassed;
}


uint32 Shader::s_queryId=0;
bool Shader::s_queryCreated=false;

void Shader::CreateQuery()
{
	if(s_queryCreated) return;
	s_queryCreated = true;
	glGenQueries(1, &s_queryId);
}

void Shader::DeleteQuery()
{
	if(!s_queryCreated) return;
	s_queryCreated = false;
	glDeleteQueries(1, &s_queryId);
}


#ifdef SAVE_ALU_INFO_EXCEL
#include "xml_utils.h"
#include <cstdarg>

void CreateRow(TiXmlElement *table, int rownum, char*s, ...)
{
	TiXmlElement *row, *cell, *data;
	char *type, *value;
	int cellnum = 1;
	bool is_type = true;

	va_list ap;
	va_start(ap,s);
	
	row = addelem( table, "Row", "");
	row->SetAttribute( "ss:Index", rownum);

	while (s)
	{       
		if (is_type)
		{
			type = s;
		}
		else
		{
			value = s;
			cell = addelem( row, "Cell", "");
			cell->SetAttribute( "ss:Index", cellnum++);

			data = addelem( cell, "Data", value);
			data->SetAttribute( "ss:Type",type);
		}
		is_type = ! is_type;
		s = va_arg(ap, char *);
	}

	va_end(ap);
}
#endif //SAVE_ALU_INFO_EXCEL

void Shader::SaveShaderAluInfo_Excel()
{
#ifdef SAVE_ALU_INFO_EXCEL
	TiXmlElement *elem;
	TiXmlElement *table;
	TiXmlElement *row;
	TiXmlElement *cell;
	TiXmlElement *data;
	TiXmlElement *column;
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "no" );
	doc.LinkEndChild( decl );

	TiXmlUnknown *u = new TiXmlUnknown;
	u->SetValue( "?mso-application progid=\"Excel.Sheet\"?");
	doc.LinkEndChild( u);

	TiXmlElement *root = addelem( doc, "Workbook", "");
	root->SetAttribute( "xmlns","urn:schemas-microsoft-com:office:spreadsheet");
	root->SetAttribute( "xmlns:o","urn:schemas-microsoft-com:office:office");
	root->SetAttribute( "xmlns:x","urn:schemas-microsoft-com:office:excel");
	root->SetAttribute( "xmlns:ss","urn:schemas-microsoft-com:office:spreadsheet");
	root->SetAttribute( "xmlns:html","http://www.w3.org/TR/REC-html40");
	
	elem = addelem( root, "Worksheet", "");
	elem->SetAttribute( "ss:Name", "GFXBench Per Shader ALU INFO");
	table = addelem( elem, "Table", "");

	column = addelem( table, "Column", "");
	column->SetAttribute( "ss:Index", "1");
	column->SetAttribute( "ss:Width", "100");

	column = addelem( table, "Column", "");
	column->SetAttribute( "ss:Index", "2");
	column->SetAttribute( "ss:Width", "200");

	column = addelem( table, "Column", "");
	column->SetAttribute( "ss:Index", "3");
	column->SetAttribute( "ss:Width", "200");

	column = addelem( table, "Column", "");
	column->SetAttribute( "ss:Index", "4");
	column->SetAttribute( "ss:Width", "200");

	//First row
	row = addelem( table, "Row", "");
	row->SetAttribute( "ss:Index", "1");

	cell = addelem( row, "Cell", "");
	cell->SetAttribute( "ss:Index", "2");

	data = addelem( cell, "Data", "Vertex shader ALU Instructions" );
	data->SetAttribute( "ss:Type","String");


	cell = addelem( row, "Cell", "");
	cell->SetAttribute( "ss:Index", "3");

	data = addelem( cell, "Data", "Fragment shader ALU Instructions" );
	data->SetAttribute( "ss:Type","String");


	cell = addelem( row, "Cell", "");
	cell->SetAttribute( "ss:Index", "4");

	data = addelem( cell, "Data", "Texture-fetch Instructions" );
	data->SetAttribute( "ss:Type","String");


	//log:
	std::string head;
	std::string vertexALU;
	std::string fragmentALU;
	std::string textureFetch;
	for(size_t i=0; i<m_shader_cache.size(); ++i)
	{
		head         = "Shader ";
		head        += lexical_cast<std::string>(  i  );
		vertexALU    = lexical_cast<std::string>(  m_shader_cache[i]->VertexShaderInfo->ALU_Instructions  );
		fragmentALU  = lexical_cast<std::string>(  m_shader_cache[i]->FragmentShaderInfo->ALU_Instructions  );
		textureFetch = lexical_cast<std::string>(  m_shader_cache[i]->FragmentShaderInfo->TextureFetches()  );
		
		CreateRow(table, i+2, "String", head.c_str(), "Number", vertexALU.c_str(), "Number", fragmentALU.c_str(), "Number", textureFetch.c_str(), NULL);
	}

	std::string fileName = GLB::g_os->GetDataRWDirectory();
	fileName += LOGSHADER::perShaderAluInfoFileNameEXCEL;
	doc.SaveFile( fileName.c_str() );


#endif
}


void Shader::SaveFrameAluInfo_Excel()
{
#ifdef SAVE_ALU_INFO_EXCEL
	TiXmlElement *elem;
	TiXmlElement *table;
	TiXmlElement *row;
	TiXmlElement *cell;
	TiXmlElement *data;
	TiXmlElement *column;
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "no" );
	doc.LinkEndChild( decl );

	TiXmlUnknown *u = new TiXmlUnknown;
	u->SetValue( "?mso-application progid=\"Excel.Sheet\"?");
	doc.LinkEndChild( u);

	TiXmlElement *root = addelem( doc, "Workbook", "");
	root->SetAttribute( "xmlns","urn:schemas-microsoft-com:office:spreadsheet");
	root->SetAttribute( "xmlns:o","urn:schemas-microsoft-com:office:office");
	root->SetAttribute( "xmlns:x","urn:schemas-microsoft-com:office:excel");
	root->SetAttribute( "xmlns:ss","urn:schemas-microsoft-com:office:spreadsheet");
	root->SetAttribute( "xmlns:html","http://www.w3.org/TR/REC-html40");

	elem = addelem( root, "Worksheet", "");
	elem->SetAttribute( "ss:Name", "GFXBench Per Frame ALU INFO");
	table = addelem( elem, "Table", "");

	column = addelem( table, "Column", "");
	column->SetAttribute( "ss:Index", "1");
	column->SetAttribute( "ss:Width", "100");

	column = addelem( table, "Column", "");
	column->SetAttribute( "ss:Index", "2");
	column->SetAttribute( "ss:Width", "200");

	column = addelem( table, "Column", "");
	column->SetAttribute( "ss:Index", "3");
	column->SetAttribute( "ss:Width", "200");

	column = addelem( table, "Column", "");
	column->SetAttribute( "ss:Index", "4");
	column->SetAttribute( "ss:Width", "200");

	column = addelem( table, "Column", "");
	column->SetAttribute( "ss:Index", "5");
	column->SetAttribute( "ss:Width", "200");

	//First row
	row = addelem( table, "Row", "");
	row->SetAttribute( "ss:Index", "1");

	cell = addelem( row, "Cell", "");
	cell->SetAttribute( "ss:Index", "2");

	data = addelem( cell, "Data", "Vertex shader ALU Instructions sum" );
	data->SetAttribute( "ss:Type","String");


	cell = addelem( row, "Cell", "");
	cell->SetAttribute( "ss:Index", "3");

	data = addelem( cell, "Data", "Fragment shader ALU Instructions sum" );
	data->SetAttribute( "ss:Type","String");


	cell = addelem( row, "Cell", "");
	cell->SetAttribute( "ss:Index", "4");

	data = addelem( cell, "Data", "Texture-fetch Instructions sum" );
	data->SetAttribute( "ss:Type","String");


	cell = addelem( row, "Cell", "");
	cell->SetAttribute( "ss:Index", "5");

	data = addelem( cell, "Data", "Texture-fetch Fetched bits sum" );
	data->SetAttribute( "ss:Type","String");


	//log:
	std::string head;
	std::string vertexALU;
	std::string fragmentALU;
	std::string textureFetch;
	std::string textureFetchBits;
	for(size_t i=0; i<FrameAluInfo.size(); ++i)
	{
		head             = "Frame ";
		head            += lexical_cast<std::string>(  i  );
		vertexALU        = lexical_cast<std::string>(  FrameAluInfo[i]->vertexALU  );
		fragmentALU      = lexical_cast<std::string>(  FrameAluInfo[i]->fragmentALU  );
		textureFetch     = lexical_cast<std::string>(  FrameAluInfo[i]->fetchSum  );
		textureFetchBits = lexical_cast<std::string>(  FrameAluInfo[i]->fetchedTexBits  );
		
		CreateRow(table, i+2, "String", head.c_str(), "Number", vertexALU.c_str(), "Number", fragmentALU.c_str(), "Number", textureFetch.c_str(), "Number", textureFetchBits.c_str(), NULL);
	}

	std::string fileName = GLB::g_os->GetDataRWDirectory();
	fileName += LOGSHADER::perFrameAluInfoFileNameEXCEL;
	doc.SaveFile( fileName.c_str() );
	

#endif
}

#endif //PER_FRAME_ALU_INFO

#endif




using namespace std;


bool openfile( const char *filename, string &result);

char* findalnumdigit( bool yes, char* s)
{
	if( !s)
	{
		return 0;
	}

	char *it3 = s;

	while( *it3)
	{
		bool c0 = isalnum(*it3) || isdigit(*it3) || *it3 == '_';
		if( c0 == yes)
		{
			break;
		}
		it3++;
	}
	return it3;
}


void Shader::AddPrecisionQualifierToUniforms( std::string &vs_source, std::string &fs_source)
{
	GLB::GLBShaderCommon::AddPrecisionQualifierToUniforms( vs_source);
	if (m_ubo_support_enabled)
	{
		if(m_shader_dir == "shaders_40/shaders.30/")
		{
			AddUniformBlocks(vs_source);
		}
	}
	
	if(fs_source != "") //support nofs codepath
	{
		GLB::GLBShaderCommon::AddPrecisionQualifierToUniforms( fs_source);
		if (m_ubo_support_enabled)
		{
			if(m_shader_dir == "shaders_40/shaders.30/")
			{
				AddUniformBlocks(fs_source);
			}
		}
	}
}

void Shader::AddMediumPrecisionQualifierToVaryings( std::string &vs_source)
{
	const std::string findThat = "varying ";
	const std::string replWith = "varying mediump ";
	size_t pos = 0;

	while(std::string::npos != (pos = vs_source.find(findThat, pos)) )
	{
		vs_source.replace(pos, findThat.length(), replWith);
		pos += replWith.length();
	}
}

void Shader::AddHighPrecisionQualifierToVaryings( std::string &vs_source)
{
	const std::string findThat = "varying ";
	const std::string replWith = "varying highp ";
	size_t pos = 0;

	while(std::string::npos != (pos = vs_source.find(findThat, pos)) )
	{
		vs_source.replace(pos, findThat.length(), replWith);
		pos += replWith.length();
	}
}

void Shader::AddUniformBlocks( std::string &source)
{
	for(KCL::uint32 i=0; i<UNIFORM_BLOCK_COUNT; ++i)
	{
		if(0 == m_uniform_block_names[i]) continue;

		while( 1)
		{
			std::string findThat = "#include " + std::string(m_uniform_block_names[i]) + ";";

			size_t pos;

			if(std::string::npos == (pos = source.find(findThat)) )
			{
				break;
			}

			std::string replWith = m_uniform_block_data[i];

			source.replace(pos, findThat.length(), replWith);
		}
	}
}

Shader* Shader::CreateShader( const char* vsfile, const char* fsfile, const std::set<std::string> *defines_set, KCL::KCL_Status& error, ShaderTypes::Enum shaderType)
{	
	string vs_source = "";
	string fs_source = "";
	Shader *r = 0;
	KCL::uint32 adler;
	error = KCL_TESTERROR_NOERROR;
	string defines;

	if (m_ubo_support_enabled)
	{
		defines += string("#define USE_UBOs\n");
	}

	if(defines_set)
	{
		std::set<std::string>::const_iterator it = defines_set->begin();
		
		while(it != defines_set->end())
		{
			defines+=string("#define ") + *it + string("\n");
			it++;
		}
	}

	//debug defines
	std::set<std::string>::const_iterator it = m_defines_debug.begin();		
	while(it != m_defines_debug.end())
	{
		defines+=string("#define ") + *it + string("\n");
		it++;
	}

	KCL::AssetFile vshader_file( m_shader_dir + vsfile);
	if(vshader_file.GetLastError())
	{
		INFO("ERROR: file %s not found!\n", vsfile);
		error = KCL_TESTERROR_FILE_NOT_FOUND;
		return 0;
	}
	vs_source = vshader_file.GetBuffer();

	KCL::AssetFile fshader_file( m_shader_dir + fsfile);
	if(fshader_file.GetLastError())
	{
		INFO("ERROR: file %s not found!\n", fsfile);
		error = KCL_TESTERROR_FILE_NOT_FOUND;
		return 0;
	}
	fs_source = fshader_file.GetBuffer();


	vs_source = defines + GetVertexPrecisionHeader() + vs_source;
	fs_source = defines + GetFragmentPrecisionHeader() + fs_source;

	AddPrecisionQualifierToUniforms(vs_source, fs_source);
	if (is_highp)
		AddHighPrecisionQualifierToVaryings(vs_source);
	else
	AddMediumPrecisionQualifierToVaryings(vs_source);

	adler = adler32( 0, (const unsigned char*)vs_source.c_str(), vs_source.length());
	adler = adler32( adler, (const unsigned char*)fs_source.c_str(), fs_source.length());

	for( KCL::uint32 j=0; j<m_shader_cache.size(); j++)
	{
		Shader *s = m_shader_cache[j];

		if( s->m_adler == adler)
		{
			r = s;
			break;
		}
	}
	if( !r)
	{
		r = new Shader;
		r->m_glsl_version_string = m_shader_header ;

		std::vector<std::string> source_files ;
		source_files.push_back(vsfile) ;
		source_files.push_back(fsfile) ;

		r->m_source_files = source_files ;

		KCL::uint32 m_vs = glCreateShader(GL_VERTEX_SHADER);
		KCL::uint32 m_fs = glCreateShader(GL_FRAGMENT_SHADER);

		glAttachShader( r->m_p, m_vs);
		glAttachShader( r->m_p, m_fs);

		error = r->CompileShader( m_vs, vs_source);
		if(error != KCL_TESTERROR_NOERROR)
		{
			INFO("vertex shader compile error %s:", vsfile);
			//INFO("%s",vs_source.c_str());
		}

		error = r->CompileShader( m_fs, fs_source);
		if(error != KCL_TESTERROR_NOERROR)
		{
			INFO("fragment shader compile error %s:", fsfile);
			//INFO("%s",fs_source.c_str());
		}

		//INFO("Linking: %s %s %s\n", vsfile, fsfile, defines.c_str());

		if(shaderType == ShaderTypes::TransformFeedback)
		{
			//////////////////////////////////////////////////////////////////////////
			//output data from the advocation vertex shader
			const char* varyings[] = 
			{ 
				"out_Pos",
				"out_Age01_Speed_Accel",
				"out_Amplitude",
				"out_Phase",
				"out_Frequency",
				"out_T",
				"out_B",
				"out_N",
				"out_Velocity"
			};
#if defined HAVE_GLES3 || defined __glew_h__
			glTransformFeedbackVaryings(r->m_p, COUNT_OF(varyings), varyings, GL_INTERLEAVED_ATTRIBS);
#endif
			//////////////////////////////////////////////////////////////////////////
		}

		error = r->LinkProgram();
		
		glDetachShader(r->m_p,m_vs) ;
		glDetachShader(r->m_p,m_fs) ;

		glDeleteShader(m_vs) ;
		glDeleteShader(m_fs) ;

		if(error != KCL_TESTERROR_NOERROR)
		{
			INFO("Shader link error %s %s:", vsfile, fsfile);
		}
			
		if(error != KCL_TESTERROR_NOERROR)
		{
			r = new Shader() ;
		}

		//error = SaveBinaryForCaching(r->m_p);
		m_shader_cache.push_back( r);

		r->m_adler = adler;

		r->InitShaderLocations(true) ;

		for(KCL::uint32 i=0; i<UNIFORM_BLOCK_COUNT; ++i)
		{
			r->m_has_uniform_block[i] = false;
		}
		//uniform blocks: es3 and above only
#if defined HAVE_GLES3 || defined __glew_h__
		if(m_shader_dir == "shaders_40/shaders.30/")
		{
			for(KCL::uint32 i=0; i<UNIFORM_BLOCK_COUNT; ++i)
			{
				if(m_uniform_block_names[i])
				{
					KCL::int32 blockIdx = glGetUniformBlockIndex(r->m_p, m_uniform_block_names[i]);
					if(blockIdx != -1)
					{
						glUniformBlockBinding(r->m_p, blockIdx, i); //bind the shader's uniform block to the unique binding point corresponding to the block's name
						r->m_has_uniform_block[i] = true;
					}
				}
			}
		}
#endif		

#ifdef LOG_SHADERS

#ifdef WRITE_LOG_SHADERS
		LOGSHADER::LogShader(vsfile, vs_source, fsfile, fs_source);
#endif

#ifdef READ_LOG_SHADERS
		
		r->VertexShaderInfo = new LOGSHADER::ShaderInfo();
		r->FragmentShaderInfo = new LOGSHADER::FragmentShaderInfo();

		r->VertexShaderInfo->loadShaderInfo( LOGSHADER::ShaderInfoPaths::VertexShaderInfoPath(m_shader_cache.size()-1).c_str() );
		r->FragmentShaderInfo->loadShaderInfo( LOGSHADER::ShaderInfoPaths::FragmentShaderInfoPath(m_shader_cache.size()-1).c_str() );

#ifdef PER_FRAME_ALU_INFO
		CreateQuery();
#endif //PER_FRAME_ALU_INFO

#endif //READ_LOG_SHADERS

#endif

	}
	
	return r;
}

void Shader::DeleteShader(Shader* s)
{
    if(s)
    {
        delete s;
        s = NULL;
    }
}

void Shader::DeleteShaders()
{
#ifdef LOG_SHADERS
	LOGSHADER::ShaderInfoPaths::Delete();

	#ifdef READ_LOG_SHADERS

		#ifdef PER_SHADER_ALU_INFO
		{
			SaveShaderAluInfo_Excel();

			std::string fname = GLB::g_os->GetDataRWDirectory();
			fname += LOGSHADER::perShaderAluInfoFileName;
			FILE* file = fopen(fname.c_str(), "w");

			for(int i=0; i<m_shader_cache.size(); ++i)
			{
				LOGSHADER::SaveShaderAluInfo(file, i, *(m_shader_cache[i]->VertexShaderInfo) , *(m_shader_cache[i]->FragmentShaderInfo) );
			}

			fclose(file);
		}
		#endif //PER_SHADER_ALU_INFO

		#ifdef PER_FRAME_ALU_INFO
		{
			DeleteQuery();

			SaveFrameAluInfo_Excel();

			std::string fname = GLB::g_os->GetDataRWDirectory();
			fname += LOGSHADER::perFrameAluInfoFileName;
			FILE* file = fopen(fname.c_str(), "w");

			for(int i=0; i<FrameAluInfo.size(); ++i)
			{
				LOGSHADER::SaveFrameAluInfo(file, i, *(FrameAluInfo[i]) );

				delete FrameAluInfo[i];
			}

			fclose(file);
		}
		#endif //PER_FRAME_ALU_INFO

	#endif // READ_LOG_SHADERS

#endif //LOG_SHADERS


	for( KCL::uint32 j=0; j<m_shader_cache.size(); j++)
	{
		Shader *s = m_shader_cache[j];
		if(s)
		{
			delete s;
		}
		m_shader_cache[j] = 0;
	}

	m_shader_cache.clear();
}


Shader::Shader() : GLBShaderCommon() 
{


#ifdef LOG_SHADERS
	
	VertexShaderInfo = 0;
	FragmentShaderInfo = 0;
	
#endif
}


Shader::~Shader()
{

#ifdef LOG_SHADERS
	
	delete VertexShaderInfo;
	delete FragmentShaderInfo;
	
#endif
}

// KCL::KCL_Status Shader::SaveBinaryForCaching(KCL::uint32 program)
// {
// 
// }
// 
// KCL::KCL_Status Shader::LoadBinaryFromCache(KCL::uint32 program)
// {
// 
// }


void Shader::FillUniformBlockData(KCL::uint32 idx)
{
	KCL::AssetFile vshader_file( m_shader_dir + "ubo_" + std::string(m_uniform_block_names[idx]) + ".h");
	if(vshader_file.GetLastError())
	{
		INFO("ERROR: file %s not found!\n", vshader_file.getFilename().c_str());
		return;
	}
	m_uniform_block_data[idx] = vshader_file.GetBuffer();
}


void Shader::SetForceHIGHP( bool force_highp)
{
	is_highp=force_highp;
}

void Shader::InitShaders( const std::string &requiredRenderAPI, bool force_fs_mediump)
{
	Shader::m_ubo_support_enabled = false;
	if( requiredRenderAPI == "es2")
	{
		m_shader_dir = "shaders_40/shaders.20/";
		m_shader_header.clear();
	}
	else if( requiredRenderAPI == "es3")
	{
		m_shader_dir = "shaders_40/shaders.30/";
		m_shader_header = GLB::g_extension->isES()?"#version 300 es\n":"#version 400 core\n";
	}
	else if( requiredRenderAPI == "es31")
	{
		m_shader_dir = "shaders_40/shaders.30/";
		m_shader_header = GLB::g_extension->isES()?"#version 300 es\n":"#version 400 core\n";
#ifdef UBO31
		Shader::m_ubo_support_enabled = true;
#endif
	}
	else if( requiredRenderAPI == "es4")
	{
		m_shader_dir = "shaders_40/shaders.4/";
		m_shader_header = GLB::g_extension->isES()?"#version 310 es\n":"#version 430 core\n";
	}

	//NOTE: uniform blocks: must use (i/u)vec4-s and member access, floats/vec2/vec3-s won't work on some platforms
	m_uniform_block_names[sUBOnames::Frame] = "frame";
	m_uniform_block_names[sUBOnames::Camera] = "cameraConsts";
	m_uniform_block_names[sUBOnames::Mat] = "matConsts";
	m_uniform_block_names[sUBOnames::Mesh] = "meshConsts";
	m_uniform_block_names[sUBOnames::StaticMesh] = "staticMeshConsts";
	m_uniform_block_names[sUBOnames::Light] = "lightConsts";
	m_uniform_block_names[sUBOnames::EmitterAdvect] = "emitterAdvectConsts";
	m_uniform_block_names[sUBOnames::EmitterRender] = "emitterRenderConsts";
	m_uniform_block_names[sUBOnames::LightShaftConsts] = "lightShaftConsts";
	m_uniform_block_names[sUBOnames::LightLensFlareConsts] = "lightLensFlareConsts";
	m_uniform_block_names[sUBOnames::FilterConsts] = "filterConsts";
	m_uniform_block_names[sUBOnames::TranslateUVConsts] = "translateConsts";
	m_uniform_block_names[sUBOnames::EnvmapsInterpolatorConsts] = "envmapsInterpolatorConsts";
	m_uniform_block_names[sUBOnames::LightInstancingConsts] = "lightInstancingConsts";

	InitShaderCommon() ;

	if( requiredRenderAPI == "es3" || requiredRenderAPI == "es31" )
	{
		for(int i=0; i<UNIFORM_BLOCK_COUNT; ++i)
		{
			FillUniformBlockData(i);
		}
	}
}

