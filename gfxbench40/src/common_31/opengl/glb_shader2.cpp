/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_shader2.h"

#include <kcl_io.h>
#include <kcl_os.h>

#include "platform.h"
#include "zlib.h"
#include "ng/log.h"
#include "opengl/glb_discard_functions.h"
#include "opengl/glb_opengl_limits.h"
#include "opengl/ext.h"

#include <sstream>
#include <iomanip>

using namespace GLB;
using namespace KCL;

static const int MAXLOGLEN = 32768;
static const int FLOAT_LITERAL_PRECISION = 10;

std::map<KCL::uint32, GLBShader2::CachedShader> GLBShader2::m_shader_cache;
std::vector<GLBShader2*> GLBShader2::m_invalid_shaders;

GLBShaderBuilder::BuilderDescriptor GLBShaderBuilder::m_build_descriptor;

GLBShaderBuilder::BuilderDescriptor::BuilderDescriptor()
{
    m_target_limits = NULL;
}

GLBShaderBuilder::GLBShaderBuilder()
{
	Clear();
}

GLBShaderBuilder::~GLBShaderBuilder()
{
}

void GLBShaderBuilder::Clear()
{
    m_checksum = 0;
    m_shader_dirs = m_build_descriptor.m_global_shader_dirs;	

    for (KCL::uint32 i = 0; i < INDEX_COUNT; i++)
	{
		m_files[i].clear();
		m_sources[i].clear();
	}

	m_shader_file.clear();
	m_shader_source.clear();

	m_defines_set.clear();    

	m_transform_feedback = false;
}

GLBShader2 *GLBShaderBuilder::Build(KCL::KCL_Status& error)
{	   
    error = KCL::KCL_TESTERROR_NOERROR;

    // Search the shader in the cache
    GLBShader2 *shader = LookupShader();
    if (shader)
    {
        Clear();
        return shader;
    }

	error = LoadFiles();
	if (error != KCL_TESTERROR_NOERROR)
	{
		Clear();
        return new GLBShader2();
	}
		
	// Parse the sources (resolve .shader defines, handle includes...)
	error = ParseSources();
    if (error != KCL_TESTERROR_NOERROR)
	{
        NGLOG_ERROR("GLBShaderBuilder: There was an error parsing shader sources:\n%s", GetSourceList());        
		Clear();
		return new GLBShader2();
	}

	// Collect the defines	
	if (!m_defines_set.empty())
	{
		std::stringstream defines;
		for (std::set<std::string>::const_iterator it = m_defines_set.begin(); it != m_defines_set.end(); it++)
		{
			defines << "#define " << *it << std::endl;
		}
		std::string defines_str = defines.str();
        for (KCL::uint32 i = 0; i < INDEX_COUNT; i++)
		{
			if (!m_sources[i].empty())
			{
                m_sources[i].insert(m_sources[i].begin(), defines_str);
			}
		}
	}
		
	shader = CreateShader(error);
	Clear();
	if (shader == NULL)
	{
		return new GLBShader2();
	}    
    
	return shader;
}

KCL::KCL_Status GLBShaderBuilder::LoadFiles()
{
	KCL::KCL_Status result = KCL_TESTERROR_NOERROR;
	if (!m_shader_file.empty())
	{
		// Load shader as .shader file
		result = LoadFile(m_shader_file, m_shader_source, false);
		if (result != KCL_TESTERROR_NOERROR)
		{
			return result;
		}
	}	

	// Load the shader files
	std::string dst;
    for (KCL::uint32 i = 0; i < INDEX_COUNT; i++)
	{
		const std::vector<std::string> &file_list = m_files[i];
        for (KCL::uint32 j = 0; j < file_list.size(); j++)
        {
            result = LoadFile(file_list[j], dst, false);
			if (result != KCL_TESTERROR_NOERROR)
			{
				return result;
			}
			m_sources[i].push_back(dst);
        }
	}

	return result;
}

KCL::KCL_Status GLBShaderBuilder::LoadFile(const std::string &file, std::string &dst, bool isHeader)
{
    for (KCL::uint32 i = 0; i < m_shader_dirs.size(); i++)
	{
		const std::string &dir = m_shader_dirs[i];

        if (!KCL::AssetFile::Exists(dir + file))
        {
            continue;
        }

		KCL::AssetFile shader_file(dir + file);
		if(shader_file.GetLastError() == KCL::KCL_IO_NO_ERROR)
		{
			dst = shader_file.GetBuffer();
			return KCL_TESTERROR_NOERROR;		
		}

	}

	// header files are optional
	if (!isHeader)
	{
		NGLOG_ERROR("GLBShaderBuilder: File %s not found!", file.c_str());
		for (KCL::uint32 i = 0; i < m_shader_dirs.size(); i++)
		{
			NGLOG_ERROR("GLBShaderBuilder: Searched in directory: %s", m_shader_dirs[i]);
		}
	}

	return KCL_TESTERROR_FILE_NOT_FOUND;
}

KCL::KCL_Status GLBShaderBuilder::ParseSources()
{    
	KCL::KCL_Status result = KCL_TESTERROR_NOERROR;
	if (!m_shader_source.empty())
	{
		// Parse the .shader file
		// Look for shader type defines
		if (m_shader_source.find("TYPE_vertex") != std::string::npos)
		{
			m_sources[VERTEX_INDEX].push_back(m_shader_source);	
		}		
		if (m_shader_source.find("TYPE_fragment") != std::string::npos)
		{
			m_sources[FRAGMENT_INDEX].push_back(m_shader_source);	
		}
		if (m_shader_source.find("TYPE_compute") != std::string::npos)
		{
			m_sources[COMPUTE_INDEX].push_back(m_shader_source);	
		}
		if (m_shader_source.find("TYPE_tess_control") != std::string::npos)
		{
			m_sources[TESS_CONTROL_INDEX].push_back(m_shader_source);	
		}
		if (m_shader_source.find("TYPE_tess_eval") != std::string::npos)
		{
			m_sources[TESS_EVALUATION_INDEX].push_back(m_shader_source);	
		}
		if (m_shader_source.find("TYPE_geometry") != std::string::npos)
		{
			m_sources[GEOMETRY_INDEX].push_back(m_shader_source);	
		}
	}	

	// Parse the shader files, include additional files if needed
	std::set<std::string> included_files;
    for (KCL::uint32 i = 0; i < INDEX_COUNT; i++)
	{
		std::vector<std::string> &source_list = m_sources[i];		
        included_files.clear();   
        for (KCL::uint32 i = 0; i < source_list.size(); i++)
		{
			result = ParseSource(source_list[i], included_files);
			if (result != KCL_TESTERROR_NOERROR)
			{
				return result;
			}
		}
	}

	return result;
}

static bool ReadLine(const char *src, KCL::uint32 src_length, KCL::uint32 &offset, std::string &dst)
{   
    if (offset >= src_length)
    {
        return false;
    }

    // Search the end of the line
    int dst_length = 0;   
    while(dst_length + offset < src_length && src[offset + dst_length] != '\n')
    {
        char c = src[offset + dst_length];
        dst_length++;
    }    

    // Copy the line to the destination string
    dst.resize(dst_length + 1);
    memcpy(&dst[0], &src[offset], dst_length);
    dst[dst_length] = '\n';

    offset += dst_length + 1;

    return true;
}

KCL::KCL_Status GLBShaderBuilder::ParseSource(std::string &source, std::set<std::string> &included_files)
{
    const char *input_str = source.c_str();
    KCL::uint32 input_size = source.size();

    //std::stringstream input_stream(source);
	std::stringstream output_stream;		

	std::size_t first_pos;
	std::size_t second_pos;

	std::string line;
    KCL::uint32 offset = 0;
    //while(std::getline(input_stream, line))
    while(ReadLine(input_str, input_size, offset, line))   // Read the line in C way, std::getline is really slow in DEBUG mode
	{
		if (line.find("#include") != std::string::npos)
		{	
			// Search for the first " charachter
			first_pos = line.find('"');
			// Also check if it is not the last one
			if (first_pos == std::string::npos || first_pos >= line.size() - 1)
			{
				NGLOG_ERROR("GLBShaderBuilder: Shader error! #include \"filename\" expected in line: %s", line.c_str());
				return KCL_TESTERROR_SHADER_ERROR;
			}

			// Search for the second " charachter
			second_pos = line.find('"', first_pos + 1);
			// Also check if there is someting between them
			if (second_pos == std::string::npos || second_pos - first_pos - 1 < 1)
			{
				NGLOG_ERROR("GLBShaderBuilder: Shader error! #include \"filename\" expected in line: %s", line.c_str());
				return KCL_TESTERROR_SHADER_ERROR;
			}

			// Include the file
			std::string header_file_name = line.substr(first_pos + 1, second_pos - first_pos - 1);
            if (included_files.find(header_file_name) != included_files.end())
            {
                // This file is already included, now we just skip this include directive
                continue;
            }

			std::string included_file;
			if(LoadFile(header_file_name, included_file, true) != KCL_TESTERROR_NOERROR)
			{
				included_file = "" ;
			}            

            included_files.insert(header_file_name);
            KCL::KCL_Status result = ParseSource(included_file, included_files);
            if (result != KCL_TESTERROR_NOERROR)
            {
                return result;
            }
			output_stream << included_file << std::endl;   
		}
		else if (GLBShaderCommon::is_highp)
		{
			size_t medium_pos = line.find("mediump");
			if (medium_pos != std::string::npos)
			{
				line.replace(medium_pos, 7, "highp");
			}
			
			size_t low_pos = line.find("lowp");
			if (low_pos != std::string::npos)
			{
				line.replace(low_pos, 4, "highp");
			}
			output_stream << line;
		}
		else
		{
			// Just copy this line
			output_stream << line;// << std::endl;
		}		
	}		
	source = output_stream.str();	
    return KCL::KCL_TESTERROR_NOERROR;
}

GLBShader2 *GLBShaderBuilder::CreateShader(KCL::KCL_Status& error)
{
	error = KCL::KCL_TESTERROR_NOERROR;
    
    // Create the new shader
	KCL::uint32 shader_flags = 0;
    GLBShader2 *shader = new GLBShader2();

	// Collect the shader source files
    std::vector<std::string> source_files;
	std::list<std::string>::iterator it;
    for (KCL::uint32 i = 0; i < INDEX_COUNT; i++)
	{
        for (KCL::uint32 j = 0; j < m_files[i].size(); j++)
		{
			source_files.push_back(m_files[i][j]);
		}
	}	
			
	// push back the name of source files
	if (!m_shader_file.empty())
	{
		source_files.push_back(m_shader_file);
    }	

	shader->m_source_files = source_files;
    shader->m_glsl_version_string = m_build_descriptor.m_common_headers; // Ensures version string is the first line

	std::vector<std::string> &vs_sources = m_sources[GetShaderTypeIndex(GLBShader2::ShaderTypes::VertexShader)];
	if (!vs_sources.empty())
	{	        
        vs_sources.insert(vs_sources.begin(), m_build_descriptor.m_stage_headers[VERTEX_INDEX]);

		// Compile the vertex shader
		shader_flags |= GLBShader2::ShaderTypes::VertexShader;
        shader->m_vs_shader = glCreateShader(GL_VERTEX_SHADER);

		std::string vs_source = ConcatenateShaderSourceFromContainer(vs_sources);
		GLBShaderCommon::AddPrecisionQualifierToUniforms(vs_source);
		error = shader->CompileShader(shader->m_vs_shader, vs_source);
	}

	std::vector<std::string> &tcs_sources = m_sources[GetShaderTypeIndex(GLBShader2::ShaderTypes::TessControlShader)];
    if (error == KCL::KCL_TESTERROR_NOERROR && !tcs_sources.empty() && (m_defines_set.find("USE_TESSELLATION") != m_defines_set.end()))
	{
        tcs_sources.insert(tcs_sources.begin(), m_build_descriptor.m_stage_headers[TESS_CONTROL_INDEX]);

		// Compile the tessellation control shader
		shader_flags |= GLBShader2::ShaderTypes::TessControlShader;
		shader->m_tcs_shader = glCreateShader(GL_TESS_CONTROL_SHADER);

		std::string tcs_source = ConcatenateShaderSourceFromContainer(tcs_sources);
		GLBShaderCommon::AddPrecisionQualifierToUniforms(tcs_source);
		error = shader->CompileShader(shader->m_tcs_shader, tcs_source);
	}

	std::vector<std::string> &tes_sources = m_sources[GetShaderTypeIndex(GLBShader2::ShaderTypes::TessEvaluationShader)];
	if (error == KCL::KCL_TESTERROR_NOERROR && !tes_sources.empty() && (m_defines_set.find("USE_TESSELLATION") != m_defines_set.end()))
	{
	    tes_sources.insert(tes_sources.begin(), m_build_descriptor.m_stage_headers[TESS_EVALUATION_INDEX]);

		// Compile the tessellation control shader
		shader_flags |= GLBShader2::ShaderTypes::TessEvaluationShader;
		shader->m_tes_shader = glCreateShader(GL_TESS_EVALUATION_SHADER);

		std::string tes_source = ConcatenateShaderSourceFromContainer(tes_sources);
		GLBShaderCommon::AddPrecisionQualifierToUniforms(tes_source);
		error = shader->CompileShader(shader->m_tes_shader, tes_source);
	}

    std::vector<std::string> &gs_sources = m_sources[GetShaderTypeIndex(GLBShader2::ShaderTypes::GeometryShader)];
    if ( (error == KCL::KCL_TESTERROR_NOERROR) && !gs_sources.empty() && (m_defines_set.find("USE_GEOMSHADER") != m_defines_set.end()))
	{
		gs_sources.insert(gs_sources.begin(), m_build_descriptor.m_stage_headers[GEOMETRY_INDEX]);

		// Compile the geometry shader
		shader_flags |= GLBShader2::ShaderTypes::GeometryShader;
		shader->m_gs_shader = glCreateShader(GL_GEOMETRY_SHADER);

		std::string gs_source = ConcatenateShaderSourceFromContainer(gs_sources);
		GLBShaderCommon::AddPrecisionQualifierToUniforms(gs_source);
		error = shader->CompileShader(shader->m_gs_shader, gs_source);
	}
        
	std::vector<std::string> &fs_sources = m_sources[GetShaderTypeIndex(GLBShader2::ShaderTypes::FragmentShader)];
	if (error == KCL::KCL_TESTERROR_NOERROR && !fs_sources.empty())
	{
        fs_sources.insert(fs_sources.begin(), m_build_descriptor.m_stage_headers[FRAGMENT_INDEX]);

		// Compile the fragment shader
		shader_flags |= GLBShader2::ShaderTypes::FragmentShader;
		shader->m_fs_shader = glCreateShader(GL_FRAGMENT_SHADER);

		std::string fs_source = ConcatenateShaderSourceFromContainer(fs_sources);
		GLBShaderCommon::AddPrecisionQualifierToUniforms(fs_source);
		error = shader->CompileShader(shader->m_fs_shader, fs_source);
	}

    std::vector<std::string> &cs_sources = m_sources[GetShaderTypeIndex(GLBShader2::ShaderTypes::ComputeShader)];
	if (error == KCL::KCL_TESTERROR_NOERROR && !cs_sources.empty())
	{
        cs_sources.insert(cs_sources.begin(), m_build_descriptor.m_stage_headers[COMPUTE_INDEX]);

		// Compile the compute shader
		shader_flags |= GLBShader2::ShaderTypes::ComputeShader;
		shader->m_cs_shader = glCreateShader(GL_COMPUTE_SHADER);

		std::string cs_source = ConcatenateShaderSourceFromContainer(cs_sources);
		GLBShaderCommon::AddPrecisionQualifierToUniforms(cs_source);
		error = shader->CompileShader(shader->m_cs_shader, cs_source);
	}
	
	if (error == KCL::KCL_TESTERROR_NOERROR)
	{
		// Attach a value of 0 for shader is invalid.
		if (shader->m_vs_shader != 0)
		{
			glAttachShader(shader->m_p, shader->m_vs_shader);
		}
		if (shader->m_tcs_shader != 0)
		{
			glAttachShader(shader->m_p, shader->m_tcs_shader);
		}
		if (shader->m_tes_shader != 0)
		{
			glAttachShader(shader->m_p, shader->m_tes_shader);
		}
        if (shader->m_gs_shader != 0)
		{
			glAttachShader(shader->m_p, shader->m_gs_shader) ;
		}
		if (shader->m_fs_shader != 0)
		{
			glAttachShader(shader->m_p, shader->m_fs_shader);
		}
		if (shader->m_cs_shader != 0)
		{
			glAttachShader(shader->m_p, shader->m_cs_shader);
		}
		
		if(m_transform_feedback)
		{
			shader_flags |= GLBShader2::ShaderTypes::TransformFeedback;
			// Output data from the advocation vertex shader
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
			glTransformFeedbackVaryings(shader->m_p, COUNT_OF(varyings), varyings, GL_INTERLEAVED_ATTRIBS);
#endif			
		}

		error = shader->LinkProgram();

		if (error == KCL::KCL_TESTERROR_NOERROR)
		{
			// Set the shader type and init the locations/binding points before the conformance test
			shader->m_shader_type = shader_flags;
			shader->InitShaderLocations(shader_flags &GLBShader2::ShaderTypes::VertexShader);

#ifdef DEBUG
            if (m_build_descriptor.m_target_limits != NULL)
			{
				error = m_build_descriptor.m_target_limits->ShaderConformanceTest(g_extension->getGraphicsContext(), shader->m_p);
				if (error != KCL_TESTERROR_NOERROR)
				{
					NGLOG_ERROR("Shader source file(s):");
					shader->DumpSourceFileNames();
					assert(false);
				}
			}
#endif
		}

		if (error == KCL::KCL_TESTERROR_NOERROR)
		{			
			// We successfully created the program						
            GLBShader2::CacheShader(shader, m_checksum);
		}
		else
		{
			delete shader;
		}
	}

	if (error == KCL::KCL_TESTERROR_NOERROR)
	{	
		return shader;
	} 
	else
	{
		return NULL;
	}
}

 std::string GLBShaderBuilder::GetSourceList()
 {
     std::stringstream sstream;
     if (!m_shader_file.empty())
     {
         sstream << m_shader_file;// << std::endl;
     }

    for (KCL::uint32 i = 0; i < INDEX_COUNT; i++)
	{
		const std::vector<std::string> &file_list = m_files[i];
		if (file_list.empty())
		{
			continue;
		}

        for (KCL::uint32 i = 0; i < file_list.size(); i++)
		{
			sstream << file_list[i];// << std::endl;
		}
	}
    return sstream.str();
 }

KCL::uint32 GLBShaderBuilder::GetShaderTypeIndex(GLBShader2::ShaderTypes::Enum type)
{
	switch (type)
	{
	case GLBShader2::ShaderTypes::VertexShader:
		return VERTEX_INDEX;
	case GLBShader2::ShaderTypes::FragmentShader:
		return FRAGMENT_INDEX;
	case GLBShader2::ShaderTypes::ComputeShader:
		return COMPUTE_INDEX;
	case GLBShader2::ShaderTypes::TessControlShader:
		return TESS_CONTROL_INDEX;
	case GLBShader2::ShaderTypes::TessEvaluationShader:
		return TESS_EVALUATION_INDEX;
	case GLBShader2::ShaderTypes::GeometryShader:
		return GEOMETRY_INDEX;
	default:
		NGLOG_ERROR("ShaderBuilder - Unknown shader type: %s", type);
		return -1;
	}
}

GLBShader2 * GLBShaderBuilder::LookupShader()
{
    m_checksum = 0;     
    std::string src_list = GetSourceList();
    m_checksum = CalculateChecksum(m_checksum, src_list.c_str(), src_list.size());
    
    if (!m_defines_set.empty())
	{		
		for (std::set<std::string>::const_iterator it = m_defines_set.begin(); it != m_defines_set.end(); it++)
		{
			 m_checksum = CalculateChecksum(m_checksum, (*it).c_str(), (*it).size());
		}
	}

    for (KCL::uint32 i = 0; i < INDEX_COUNT; i++)
	{
        const std::vector<std::string> &sources = m_sources[i];
		if (sources.empty())
		{
            m_checksum = CalculateChecksum(m_checksum, sources);
		}
    }
	return GLBShader2::LookupShader(m_checksum);
}

KCL::uint32 GLBShaderBuilder::CalculateChecksum(KCL::uint32 current, const char *str, KCL::uint32 len)
{
    if (str && len)
    {
        return adler32(current, (const unsigned char*)str, len);
    }
    return current;
}

KCL::uint32 GLBShaderBuilder::CalculateChecksum(KCL::uint32 current, const std::vector<std::string> &sources)
{
	KCL::uint32 result = current;
	std::list<std::string>::const_iterator it;
    for (KCL::uint32 i = 0; i < sources.size(); i++)
    {
		result = adler32(result, (const unsigned char*)(sources[i].c_str()), sources[i].length());	
	}	
	return result;
}

GLBShaderBuilder &GLBShaderBuilder::AddShaderDir(const std::string &dir)
{	
    m_shader_dirs.insert(m_shader_dirs.begin(), dir);
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::AddDefines(const std::set<std::string> *defines)
{
	if (defines != NULL)
	{
		m_defines_set.insert(defines->begin(),defines->end());
	}
	return *this;
}


GLBShaderBuilder &GLBShaderBuilder::AddDefineInt(const std::string &name, int value)
{
	std::stringstream sstream;
	sstream << name << " " << value;
	m_defines_set.insert(sstream.str());
	return *this;
}

 GLBShaderBuilder &GLBShaderBuilder::AddDefineFloat(const std::string &name, float value)
 {
    std::stringstream sstream;
	sstream<<std::fixed<<std::setprecision(FLOAT_LITERAL_PRECISION);
	sstream << name << " " << value;
	m_defines_set.insert(sstream.str());
	return *this;
 }

GLBShaderBuilder &GLBShaderBuilder::AddDefineVec2(const std::string &name, const KCL::Vector2D &value)
{    
    std::stringstream sstream;
	sstream<<std::fixed<<std::setprecision(FLOAT_LITERAL_PRECISION);
    sstream << name << " vec2(" << value.x << ", " << value.y << ")";
	m_defines_set.insert(sstream.str());
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::AddDefineVec3(const std::string &name, const KCL::Vector3D &value)
{
    std::stringstream sstream;
	sstream<<std::fixed<<std::setprecision(FLOAT_LITERAL_PRECISION);
    sstream << name << " vec3(" << value.x << ", " << value.y << ", " << value.z << ")";
	m_defines_set.insert(sstream.str());
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::AddDefineVec4(const std::string &name, const KCL::Vector4D &value)
{
    std::stringstream sstream;
	sstream<<std::fixed<<std::setprecision(FLOAT_LITERAL_PRECISION) ;
    sstream << name << " vec4(" << value.x << ", " << value.y << ", " << value.z << ", " << value.w << ")";
	m_defines_set.insert(sstream.str());
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::AddDefineIVec2(const std::string &name, KCL::int32 x, KCL::int32 y)
{
    std::stringstream sstream;
    sstream << name << " ivec2(" << x << ", " << y << ")";
    m_defines_set.insert(sstream.str());
    return *this;
}

GLBShaderBuilder &GLBShaderBuilder::AddDefine(const std::string &define)
{
	m_defines_set.insert(define);
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::ShaderFile(const char *file)
{
	if (file)
	{
		m_shader_file = file;
	}
	else
	{
		m_shader_file.clear();
	}
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::File(GLBShader2::ShaderTypes::Enum type, const char *file)
{
	if (file)
	{
		m_files[GetShaderTypeIndex(type)].push_back(file);
	}
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::FileVs(const char *file)
{
	if (file)
	{
		m_files[VERTEX_INDEX].push_back(file);
	}
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::FileFs(const char *file)
{
	if (file)
	{
		m_files[FRAGMENT_INDEX].push_back(file);
	}
	return *this;
}
GLBShaderBuilder &GLBShaderBuilder::FileCs(const char *file)
{
	if (file)
	{
		m_files[COMPUTE_INDEX].push_back(file);
	}
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::Source(GLBShader2::ShaderTypes::Enum type, const char *source)
{
	if (source)
	{
		m_sources[GetShaderTypeIndex(type)].push_back(source);
	}
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::SourceVs(const char *source)
{
	if (source)
	{
		m_sources[VERTEX_INDEX].push_back(source);
	}
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::SourceFs(const char *source)
{
	if (source)
	{
		m_sources[FRAGMENT_INDEX].push_back(source);
	}
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::SourceCs(const char *source)
{
	if (source)
	{
		m_sources[COMPUTE_INDEX].push_back(source);
	}
	return *this;
}

GLBShaderBuilder &GLBShaderBuilder::TransformFeedback(bool value)
{
	m_transform_feedback = value;
	return *this;
}

void GLBShaderBuilder::AddGlobalDefine(const std::string &name, int value)
{
    std::stringstream sstream;
    sstream << "#define " << name << " " << value << std::endl;
    m_build_descriptor.m_common_headers += sstream.str();
}

void GLBShaderBuilder::RemoveGlobalDefine(const std::string &name)
{
    std::size_t start_pos = m_build_descriptor.m_common_headers.find("#define " + name);
    if (start_pos == std::string::npos)
    {
        return;
    }

    std::string tmp_str;
    if (start_pos > 0)
    {
        tmp_str = m_build_descriptor.m_common_headers.substr(0, start_pos);
    }
    
    std::size_t end_pos = m_build_descriptor.m_common_headers.find('\n', start_pos);  
    if (end_pos != std::string::npos && end_pos != m_build_descriptor.m_common_headers.length() - 1)
    {
        tmp_str += m_build_descriptor.m_common_headers.substr(end_pos + 1, m_build_descriptor.m_common_headers.length() - end_pos);
    }
    
    m_build_descriptor.m_common_headers = tmp_str;    
}

GLBShader2::GLBShader2() : GLBShaderCommon()
{
    m_vs_shader = 0;    
	m_tcs_shader = 0;
	m_tes_shader = 0;
	m_fs_shader = 0;
	m_cs_shader = 0;
	m_gs_shader = 0;
}

GLBShader2::~GLBShader2()
{
    if (m_vs_shader)
    {
        glDetachShader(m_p, m_vs_shader);
    }
    if (m_tcs_shader)
    {
        glDetachShader(m_p, m_tcs_shader);
    }
    if (m_tes_shader)
    {
        glDetachShader(m_p, m_tes_shader);
    }
    if (m_fs_shader)
    {
        glDetachShader(m_p, m_fs_shader);
    }   
    if (m_cs_shader)
    {
        glDetachShader(m_p, m_cs_shader);
    }
	if (m_gs_shader)
	{
		glDetachShader(m_p, m_gs_shader) ;
	}
    glDeleteShader(m_vs_shader);
    glDeleteShader(m_tcs_shader);
    glDeleteShader(m_tes_shader);
    glDeleteShader(m_fs_shader);
    glDeleteShader(m_cs_shader);
	glDeleteShader(m_gs_shader);
}

KCL::uint32 GLBShader2::GetShaderType() const
{
	return m_shader_type;
}

KCL::uint32 GLBShader2::HasShader(ShaderTypes::Enum shaderType) const
{
	return (m_shader_type &shaderType);
}

bool GLBShader2::IsComputeShader() const
{
	return (m_shader_type &ShaderTypes::ComputeShader);
}

// Shader cache
void GLBShader2::CacheShader(GLBShader2 *shader, KCL::uint32 checksum)
{
  	shader->m_checksum = checksum;

    CachedShader casched_shader;
    casched_shader.m_shader = shader;
    casched_shader.m_dirty = false;
	m_shader_cache[checksum] = casched_shader;
}

GLBShader2 *GLBShader2::LookupShader(KCL::uint32 checksum)
{
	std::map<KCL::uint32, CachedShader>::const_iterator it = m_shader_cache.find(checksum);
	if (it != m_shader_cache.end())
	{
        if (it->second.m_dirty)
        {
            // We found the shader but it is dirty. We have to keep the shader, maybe somebody is still using it
            m_invalid_shaders.push_back(it->second.m_shader);
            m_shader_cache.erase(it);
            return NULL;
        }
        else
        {
            return it->second.m_shader;
        }       
	}
	return NULL;
}

void GLBShader2::InvalidateShaderCache()
{
    for (std::map<KCL::uint32, CachedShader>::iterator it = m_shader_cache.begin(); it != m_shader_cache.end(); it++)
	{
        it->second.m_dirty = true;
	}  
}

void GLBShader2::DeleteShaders()
{
	for (std::map<KCL::uint32, CachedShader>::const_iterator it = m_shader_cache.begin(); it != m_shader_cache.end(); it++)
	{
		delete it->second.m_shader;
	}
    for (KCL::uint32 i = 0; i < m_invalid_shaders.size(); i++)
    {
        delete m_invalid_shaders[i];
    }

    m_invalid_shaders.clear();
	m_shader_cache.clear();
}

std::string GLBShader2::GLSLTextureFormat(KCL::uint32 textureFormat)
{
	switch (textureFormat)
	{
	case GL_RGBA8:
		return "rgba8";
	case GL_RG8:
		return "rg8";
	case GL_RG16F:
		return "rg16f";
	case GL_RGBA16F:
		return "rgba16f";
	case GL_R11F_G11F_B10F:
		return "r11f_g11f_b10f";	
	default:
		NGLOG_ERROR("Shader - Unknown texture format: %s", textureFormat);
		return "Unknown texture format";
	}
}
