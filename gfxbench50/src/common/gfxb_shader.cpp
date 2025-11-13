/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_shader.h"

#include <iomanip>
#include <sstream>
#include <ngl.h>

#include "ksl_compiler.h"
#include <Poco/Mutex.h>

#include <kcl_io.h>
#include <kcl_math3d.h>

#define FLOAT_LITERAL_PRECISION		11
#define LOG_WARNINGS 1

using namespace GFXB;

void GFXB::LoadShader(NGL_job_descriptor &jd, uint32_t pass, uint32_t shader_code, NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES], std::vector<NGL_shader_uniform> &application_uniforms)
{
	ShaderFactory::GetInstance()->Create(jd, pass, shader_code, ssd, application_uniforms);
}


ShaderDescriptor::ShaderDescriptor()
{
	m_workgroup_size_x = 0;
	m_workgroup_size_y = 0;
	m_workgroup_size_z = 0;
}


ShaderDescriptor::ShaderDescriptor(const char *filename_compute)
{
	m_filenames[NGL_COMPUTE_SHADER] = filename_compute;

	m_workgroup_size_x = 0;
	m_workgroup_size_y = 0;
	m_workgroup_size_z = 0;
}


ShaderDescriptor::ShaderDescriptor(const char *filename_vert, const char *filename_frag)
{
	m_filenames[NGL_VERTEX_SHADER] = filename_vert;
	m_filenames[NGL_FRAGMENT_SHADER] = filename_frag;

	m_workgroup_size_x = 0;
	m_workgroup_size_y = 0;
	m_workgroup_size_z = 0;
}


ShaderDescriptor &ShaderDescriptor::SetName(const char *name)
{
	if (name == nullptr)
	{
		m_name.clear();
	}
	else
	{
		m_name = name;
	}
	return *this;
}


ShaderDescriptor &ShaderDescriptor::AddHeaderFile(const char* filename)
{
	m_header_files.push_back(filename);
	return *this;
}


ShaderDescriptor &ShaderDescriptor::SetVSFile(const char *filename)
{
	m_filenames[NGL_VERTEX_SHADER] = filename;
	return *this;
}


ShaderDescriptor &ShaderDescriptor::SetTCSFile(const char *filename)
{
	m_filenames[NGL_TESS_CONTROL_SHADER] = filename;
	return *this;
}


ShaderDescriptor &ShaderDescriptor::SetTESFile(const char *filename)
{
	m_filenames[NGL_TESS_EVALUATION_SHADER] = filename;
	return *this;
}


ShaderDescriptor &ShaderDescriptor::SetGSFile(const char *filename)
{
	m_filenames[NGL_GEOMETRY_SHADER] = filename;
	return *this;
}


ShaderDescriptor &ShaderDescriptor::SetFSFile(const char *filename)
{
	m_filenames[NGL_FRAGMENT_SHADER] = filename;
	return *this;
}


ShaderDescriptor &ShaderDescriptor::SetCSFile(const char *filename)
{
	m_filenames[NGL_COMPUTE_SHADER] = filename;
	return *this;
}


ShaderDescriptor &ShaderDescriptor::AddDefine(const char *define)
{
	AddDefineInt(define, 1);
	return *this;
}


ShaderDescriptor &ShaderDescriptor::AddDefineInt(const char *define, KCL::int32 value)
{
	std::stringstream sstream;
	sstream << value;

	m_defines[define] = sstream.str();
	return *this;
}


ShaderDescriptor &ShaderDescriptor::AddDefineUInt(const char* define, KCL::uint32 value)
{
	std::stringstream sstream;
	sstream << value;
	sstream << 'u';

	m_defines[define] = sstream.str();
	return *this;
}


ShaderDescriptor &ShaderDescriptor::AddDefineFloat(const char* define, float value)
{
	std::stringstream sstream;
	sstream << std::fixed << std::setprecision(FLOAT_LITERAL_PRECISION) << value;

	m_defines[define] = sstream.str();
	return *this;
}


ShaderDescriptor &ShaderDescriptor::AddDefineHalf(const char* define, float value)
{
	std::stringstream sstream;
	sstream << std::fixed << std::setprecision(FLOAT_LITERAL_PRECISION) << value << "h";

	m_defines[define] = sstream.str();
	return *this;
}


ShaderDescriptor &ShaderDescriptor::AddDefineString(const char* define, const char* value)
{
	m_defines[define] = value;
	return *this;
}


ShaderDescriptor &ShaderDescriptor::AddDefineVec2(const char* define, const KCL::Vector2D &value)
{
	std::stringstream sstream;
	sstream << std::fixed << std::setprecision(FLOAT_LITERAL_PRECISION);
	sstream << "float2(" << value.x << ", " << value.y << ")";

	m_defines[define] = sstream.str();
	return *this;
}


void ShaderDescriptor::SetUniforms(const std::vector<NGL_shader_uniform> &uniforms)
{
	m_uniforms = uniforms;
}


const std::vector<NGL_shader_uniform> &ShaderDescriptor::GetUniforms() const
{
	return m_uniforms;
}


ShaderDescriptor &ShaderDescriptor::SetWorkgroupSize(KCL::uint32 x, KCL::uint32 y, KCL::uint32 z)
{
	m_workgroup_size_x = x;
	m_workgroup_size_y = y;
	m_workgroup_size_z = z;
	return *this;
}


KCL::uint32 ShaderDescriptor::GetWorkgroupSizeX() const
{
	return m_workgroup_size_x;
}


KCL::uint32 ShaderDescriptor::GetWorkgroupSizeY() const
{
	return m_workgroup_size_y;
}


KCL::uint32 ShaderDescriptor::GetWorkgroupSizeZ() const
{
	return m_workgroup_size_z;
}


const std::vector<std::string> &ShaderDescriptor::GetHeaderFiles() const
{
	return m_header_files;
}


const std::string &ShaderDescriptor::GetFilename(NGL_shader_type type) const
{
	return m_filenames[type];
}


const std::string &ShaderDescriptor::GetName() const
{
	return m_name;
}


std::string ShaderDescriptor::GetDefinesString() const
{
	std::stringstream sstream;

	for (std::map<std::string, std::string>::const_iterator it = m_defines.begin(); it != m_defines.end(); it++)
	{
		sstream << "#define " << it->first << ' ' << it->second << std::endl;
	}

	return sstream.str();
}


void ShaderDescriptor::Serialize(JsonSerializer& s)
{
	s.Serialize("name", m_name);

	s.Serialize("shader_vs", m_filenames[NGL_VERTEX_SHADER]);
	s.Serialize("shader_tcs", m_filenames[NGL_TESS_CONTROL_SHADER]);
	s.Serialize("shader_tes", m_filenames[NGL_TESS_EVALUATION_SHADER]);
	s.Serialize("shader_gs", m_filenames[NGL_GEOMETRY_SHADER]);
	s.Serialize("shader_fs", m_filenames[NGL_FRAGMENT_SHADER]);
	s.Serialize("shader_cs", m_filenames[NGL_COMPUTE_SHADER]);

	s.Serialize("header_files", m_header_files);

	s.Serialize("defines", m_defines);

	s.Serialize("compute_workgroup_size_x", m_workgroup_size_x);
	s.Serialize("compute_workgroup_size_y", m_workgroup_size_y);
	s.Serialize("compute_workgroup_size_z", m_workgroup_size_z);
}


std::string ShaderDescriptor::GetParameterFilename() const
{
	return "engine/" + m_name + ".json";
}


const bool ShaderDescriptor::operator==(const ShaderDescriptor& other)const
{
	/*
	if (m_name != other.m_name)
	{
		return false;
	}
	*/

	for (KCL::uint32 i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		if (m_filenames[i] != other.m_filenames[i])
		{
			return false;
		}
	}

	if (m_header_files != other.m_header_files)
	{
		return false;
	}

	if (GetDefinesString() != other.GetDefinesString())
	{
		return false;
	}

	if (m_uniforms.size() != other.m_uniforms.size())
	{
		return false;
	}

	for (size_t i = 0; i < m_uniforms.size(); i++)
	{
		if (m_uniforms[i].m_name != other.m_uniforms[i].m_name)
		{
			return false;
		}

		if (m_uniforms[i].m_group != other.m_uniforms[i].m_group)
		{
			return false;
		}
	}

	if (m_workgroup_size_x != other.m_workgroup_size_x ||
		m_workgroup_size_y != other.m_workgroup_size_y ||
		m_workgroup_size_z != other.m_workgroup_size_z)
	{
		return false;
	}

	return true;
}

ShaderFactory *ShaderFactory::instance = nullptr;


void ShaderFactory::CreateInstance()
{
	if (instance == nullptr)
	{
		instance = new ShaderFactory();
	}
    else
    {
        INFO("Shader factory already exists.");
    }
}


ShaderFactory *ShaderFactory::GetInstance()
{
    if (instance == nullptr)
    {
        INFO("WARNING! Shader factory not exists error or shaders shouldn't be compiled here.");
    }
	return instance;
}


void ShaderFactory::Release()
{
	delete instance;
	instance = nullptr;
}


void ShaderFactory::Serialize(JsonSerializer& s)
{
	if (s.IsWriter)
	{
		for (auto it = m_shader_descriptors.begin(); it != m_shader_descriptors.end(); it++)
		{
			ShaderDescriptor &desc = it->second;
			if (desc.GetName().empty())
			{
				// Only serialize the named descriptors
				continue;
			}

			JsonSerializer desc_json(true);
			desc.Serialize(desc_json);
			s.JsonValue.push_back(desc_json.JsonValue);
		}
	}
	else
	{
		for (size_t i = 0; i < s.JsonValue.size(); i++)
		{
			JsonSerializer desc_json(false);
			desc_json.JsonValue = s.JsonValue[i];

			ShaderDescriptor desc;
			desc.Serialize(desc_json);
			AddDescriptor(desc);
		}
	}
}


std::string ShaderFactory::GetParameterFilename() const
{
	return "shader_descriptors.json";
}


ShaderFactory &ShaderFactory::ClearGlobals()
{
	SetGlobalHeader("");
	ClearShaderSubdirectories();
	ClearGlobalHeaderFiles();
	ClearGlobalDefines();

	return *this;
}


KCL::uint32 ShaderFactory::AddDescriptor(const ShaderDescriptor &shader_descriptor)
{
	std::map<KCL::uint32, ShaderDescriptor>::iterator it;
	for (it = m_shader_descriptors.begin(); it != m_shader_descriptors.end(); ++it)
	{
		if (it->second == shader_descriptor)
		{
			return it->first;
		}
	}

	//generate code
	KCL::uint32 code = (KCL::uint32)(m_generated_shader_code_counter);
	m_shader_descriptors.insert(std::make_pair(code, shader_descriptor));
	m_generated_shader_code_counter++;
	return code;
}


const ShaderDescriptor *ShaderFactory::GetDescriptor(const char *name) const
{
	std::map<KCL::uint32, ShaderDescriptor>::const_iterator it;
	for (it = m_shader_descriptors.begin(); it != m_shader_descriptors.end(); ++it)
	{
		if (it->second.GetName() == name)
		{
			return &it->second;
		}
	}
	return nullptr;
}


void ShaderFactory::GetNamedDescriptors(std::vector<ShaderDescriptor> &descriptors)
{
	descriptors.clear();

	std::map<KCL::uint32, ShaderDescriptor>::iterator it;
	for (it = m_shader_descriptors.begin(); it != m_shader_descriptors.end(); ++it)
	{
		if (it->second.GetName().empty() == false)
		{
			descriptors.push_back(it->second);
		}
	}
}


ShaderFactory &ShaderFactory::ClearShaderSubdirectories()
{
	m_directories.clear();
	return *this;
}


ShaderFactory &ShaderFactory::AddShaderSubdirectory(const char *directory)
{
	if (directory == nullptr)
	{
		return *this;
	}

	size_t length = strlen(directory);
	if (length == 0)
	{
		return *this;
	}

	if (directory[length - 1] != '/')
	{
		// Append '/' at the end of the string
		std::stringstream sstream;
		sstream << directory << '/';
		m_directories.push_back(sstream.str());
	}
	else
	{
		m_directories.push_back(directory);
	}
	return *this;
}


ShaderFactory &ShaderFactory::ClearGlobalDefines()
{
	m_global_defines.clear();
	return *this;
}


ShaderFactory &ShaderFactory::AddGlobalDefine(const char *define)
{
	AddGlobalDefineInt(define, 1);
	return *this;
}


ShaderFactory &ShaderFactory::AddGlobalDefineInt(const char *define, KCL::int32 value)
{
	std::stringstream sstream;
	sstream << value;
	m_global_defines[define] = sstream.str();
	return *this;
}


ShaderFactory &ShaderFactory::AddGlobalDefineFloat(const char* define, float value)
{
	std::stringstream sstream;
	sstream << std::fixed << std::setprecision(FLOAT_LITERAL_PRECISION) << value;

	m_global_defines[define] = sstream.str();
	return *this;
}


std::string ShaderFactory::GetGlobalDefinesString() const
{
	std::stringstream sstream;
	for (auto it = m_global_defines.begin(); it != m_global_defines.end(); it++)
	{
		sstream << "#define " << it->first << ' ' << it->second << std::endl;
	}
	return sstream.str();
}


ShaderFactory &ShaderFactory::SetGlobalHeader(const char *header)
{
	m_global_header = header;
	return *this;
}


ShaderFactory &ShaderFactory::ClearGlobalHeaderFiles()
{
	m_global_header_files.clear();
	return *this;
}


ShaderFactory &ShaderFactory::AddGlobalHeaderFile(const char* filename)
{
	m_global_header_files.push_back(filename);
	return *this;
}


const std::string &ShaderFactory::GetGlobalHeader() const
{
	return m_global_header;
}


ShaderFactory &ShaderFactory::SetForceHighp(bool force_highp)
{
	m_force_highp = force_highp;
	return *this;
}


bool ShaderFactory::AppendShaderFile(const std::string &filename, std::stringstream &sstream)
{
	for (size_t i = 0; i < m_directories.size(); i++)
	{
		KCL::AssetFile shader_file(m_directories[i] + filename);
		if (shader_file.GetLastError() == KCL::KCL_IO_NO_ERROR)
		{
			sstream << shader_file.GetBuffer() << std::endl; // Ensure the last character is new line
			return true;
		}
	}

	INFO("Unable to load shaderfile: %s", filename.c_str());
	INFO("Searched in directories:");
	for (size_t i = 0; i < m_directories.size(); i++)
	{
		INFO("%s", m_directories[i].c_str());
	}
	return false;
}


const char* ShaderFactory::GetShaderTypeDefine(NGL_shader_type type)
{
	switch (type)
	{
	case NGL_VERTEX_SHADER:
		return "TYPE_vertex";

	case NGL_TESS_CONTROL_SHADER:
		return "TYPE_tessellation_control";

	case NGL_TESS_EVALUATION_SHADER:
		return "TYPE_tessellation_evalulation";

	case NGL_GEOMETRY_SHADER:
		return "TYPE_geometry";

	case NGL_FRAGMENT_SHADER:
		return "TYPE_fragment";

	case NGL_COMPUTE_SHADER:
		return "TYPE_compute";

	default:
		INFO("Unknown shader type: %d", type);
		assert(0);
		return nullptr;
	}
}


void ShaderFactory::Create(NGL_job_descriptor &jd, uint32_t pass, uint32_t shader_code, NGL_shader_source_descriptor descriptors[NGL_NUM_SHADER_TYPES], std::vector<NGL_shader_uniform> &uniforms)
{
	auto it = m_shader_descriptors.find(shader_code);
	if (it == m_shader_descriptors.end())
	{
		INFO("Shader descriptor not found for shader code: %d!", (uint32_t)shader_code);
		return;
	}

	const ShaderDescriptor &shader_descriptor = it->second;

	// Setup the uniforms
	uniforms = shader_descriptor.GetUniforms();
	if (uniforms.empty())
	{
		InitCommonUniforms(uniforms);
	}

	// Collect shader source
	for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		NGL_shader_source_descriptor &ssd = descriptors[shader_type];

		const std::string &shader_filename = shader_descriptor.GetFilename((NGL_shader_type)shader_type);

		if (shader_filename.empty())
		{
			continue;
		}

		// Add the defines
		std::stringstream sstream;

		if (m_force_highp)
		{
			sstream << "force_highp;\n";
		}

		sstream << GetGlobalDefinesString();

		sstream << "#define " << GetShaderTypeDefine((NGL_shader_type)shader_type) << " 1" << std::endl;

		if (shader_type == NGL_COMPUTE_SHADER)
		{
			sstream << "#define WORKGROUP_SIZE_X " << shader_descriptor.GetWorkgroupSizeX() << 'u' << std::endl;
			sstream << "#define WORKGROUP_SIZE_Y " << shader_descriptor.GetWorkgroupSizeY() << 'u' << std::endl;
			sstream << "#define WORKGROUP_SIZE_Z " << shader_descriptor.GetWorkgroupSizeZ() << 'u' << std::endl;

			ssd.m_work_group_size[0] = shader_descriptor.GetWorkgroupSizeX();
			ssd.m_work_group_size[1] = shader_descriptor.GetWorkgroupSizeY();
			ssd.m_work_group_size[2] = shader_descriptor.GetWorkgroupSizeZ();
		}

		// Append compute kernel workgoup size
		if (shader_type == NGL_COMPUTE_SHADER)
		{
			// Check the compute workgroup size is specified
			if (shader_descriptor.GetWorkgroupSizeX() == 0 || shader_descriptor.GetWorkgroupSizeY() == 0 || shader_descriptor.GetWorkgroupSizeZ() == 0)
			{
				INFO("ShaderFactory - Error! Incorrectly defined workgroup size: %d %d %d",
					shader_descriptor.GetWorkgroupSizeX(),
					shader_descriptor.GetWorkgroupSizeY(),
					shader_descriptor.GetWorkgroupSizeZ());
				assert(false);
				continue;
			}
		}

		sstream << shader_descriptor.GetDefinesString();

		if (m_parser_enabled)
		{
			if( shader_type != NGL_TESS_CONTROL_SHADER && shader_type != NGL_TESS_EVALUATION_SHADER )
			{
				// Load the include the global header files
				for( size_t i = 0; i < m_global_header_files.size(); i++ )
				{
					AppendShaderFile( m_global_header_files[i], sstream );
				}
			}
		}

		// Load and include the header files
		ssd.m_info_string = "File(s): ";
		const std::vector<std::string> &header_files = shader_descriptor.GetHeaderFiles();
		for (size_t i = 0; i < header_files.size(); i++)
		{
			AppendShaderFile(header_files[i], sstream);

			ssd.m_info_string += header_files[i] + ", ";
		}

		// Load the shader file
		if (AppendShaderFile(shader_filename, sstream) == false)
		{
			continue;
		}

		ssd.m_info_string += shader_filename;

		ssd.m_source_data = sstream.str();
	}

	bool is_metal = (nglGetApi() == NGL_METAL_IOS) || (nglGetApi() == NGL_METAL_MACOS);
	if ((jd.m_subpasses.size() > 1) && !is_metal)
	{
		std::stringstream sstream;

		int last_subpass_idx = 0;

		for (size_t j = 0; j < jd.m_subpasses[pass].m_usages.size(); j++)
		{
			NGL_resource_state &usage = jd.m_subpasses[pass].m_usages[j];

			if (usage == NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE)
			{
				sstream << "#define JOB_ATTACHMENT" << j << " " << last_subpass_idx++ << std::endl;
			}
			if (usage == NGL_READ_ONLY_DEPTH_ATTACHMENT || usage == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE)
			{
				sstream << "#define JOB_ATTACHMENT" << j << " " << last_subpass_idx++ << std::endl;
			}
		}

		descriptors[NGL_FRAGMENT_SHADER].m_source_data = sstream.str() + descriptors[NGL_FRAGMENT_SHADER].m_source_data;
	}
	else if ((jd.m_subpasses.size() > 1) && is_metal)
	{
		std::stringstream sstream;
		for (size_t j = 0; j < jd.m_attachments.size(); j++)
		{
			sstream << "#define JOB_ATTACHMENT" << j << " " << j << std::endl;
		}
		sstream << descriptors[NGL_FRAGMENT_SHADER].m_source_data;
		descriptors[NGL_FRAGMENT_SHADER].m_source_data = sstream.str();
	}

	//	compile pipeline
	if (m_parser_enabled)
	{
		bool s = false;
		switch (nglGetApi())
		{
		case NGL_VULKAN:
			s = CompileWithPipelineCache(descriptors, shader_code, shader_descriptor, uniforms);
			break;

		default:
			s = CompileWithShaderCache(descriptors, shader_descriptor, uniforms);
			break;
		}

		// empty sources if compile failed
		if (!s)
		{
			for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
			{
				descriptors[shader_type].m_source_data = "";
			}
		}
	}
	else // !m_parser_enabled
	{
		for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			NGL_shader_source_descriptor &ssd = descriptors[shader_type];
			if (ssd.m_source_data.empty()) continue;

			if (nglGetApi() == NGL_OPENGL || nglGetApi() == NGL_OPENGL_ES)
			{
				switch (shader_type)
				{
					case NGL_VERTEX_SHADER:
					{
						ssd.m_source_data = GetGlobalHeader() + "#define vertex_main\n" + ssd.m_source_data;
						break;
					}
					case NGL_FRAGMENT_SHADER:
					{
						ssd.m_source_data = GetGlobalHeader() + "#define fragment_main\n" + ssd.m_source_data;
						break;
					}
					default:
					{
						assert(0);
						break;
					}
				}
			}
			else
			{
				ssd.m_source_data = GetGlobalHeader() + ssd.m_source_data;
			}
		}
	}

	//  platform specific setup
	for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		NGL_shader_source_descriptor &ssd = descriptors[shader_type];
		if (ssd.m_source_data.empty()) continue;

		if (nglGetApi() == NGL_DIRECT3D_12 || nglGetApi() == NGL_DIRECT3D_11)
		{
			switch (shader_type)
			{
			case NGL_VERTEX_SHADER:
				ssd.m_entry_point = "vertex_main";
				ssd.m_version = "vs_5_0";
				break;
			case NGL_FRAGMENT_SHADER:
				ssd.m_entry_point = "fragment_main";
				ssd.m_version = "ps_5_0";
				break;
			case NGL_COMPUTE_SHADER:
				ssd.m_entry_point = "compute_main";
				ssd.m_version = "cs_5_0";
				break;
			default:
				assert(0);
				break;
			}
		}

		if ((nglGetApi() == NGL_METAL_IOS) || (nglGetApi() == NGL_METAL_MACOS))
		{
			switch (shader_type)
			{
			case NGL_VERTEX_SHADER:	  ssd.m_entry_point = "vertex_main";   break;
			case NGL_FRAGMENT_SHADER: ssd.m_entry_point = "fragment_main"; break;
			case NGL_COMPUTE_SHADER:  ssd.m_entry_point = "compute_main";  break;
			default:				  assert(0);						   break;
			}
		}
	}
}


bool ShaderFactory::CompileWithPipelineCache(NGL_shader_source_descriptor descriptors[NGL_NUM_SHADER_TYPES], const uint32_t sc, const ShaderDescriptor &shader_descriptor, const std::vector<NGL_shader_uniform> &uniforms)
{
	PipelineCache::Key key(sc, descriptors);

	if (m_pipeline_cache.Search(key,descriptors))
	{
		return true;
	}

	const std::vector<std::string> &header_files = shader_descriptor.GetHeaderFiles();
	std::vector<KSLCompiler*> stage_compilers(NGL_NUM_SHADER_TYPES, NULL);

	// Analyze
	for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		NGL_shader_source_descriptor &ssd = descriptors[shader_type];
		if (ssd.m_source_data.empty()) continue;

		stage_compilers[shader_type] = new KSLCompiler();

		const std::string &shader_filename = shader_descriptor.GetFilename((NGL_shader_type)shader_type);

		// Setup the compiler
		KSLCompiler &c = *stage_compilers[shader_type];
		c.SetSource(ssd.m_source_data);
		c.SetShaderType((NGL_shader_type)shader_type);
		c.SetTargetApi(nglGetApi());
		c.SetShaderUniformInfo(uniforms);
		c.SetUseSubpass(nglGetInteger(NGL_SUBPASS_ENABLED) != 0);

		INFO("compile %s...", shader_filename.c_str());
		bool s = c.Analyze();
		INFO("done\n");

		if (!s || (LOG_WARNINGS && c.GetErrors().size() > 0))
		{
			INFO("compile fail: ");
			for (size_t i = 0; i < header_files.size(); i++)
			{
				INFO("%s ", header_files[i].c_str());
			}
			INFO("%s\n\n", shader_filename.c_str());
			INFO("%s\n", c.GetLog().c_str());
		}

		if (!s)
		{
			return false;
		}
	}

	KSLCompiler::SetVulkanPushConstantCount(KCL::Min(nglGetInteger(NGL_PIPELINE_MAX_PUSH_CONSTANT_SIZE), 128));
	bool s = KSLCompiler::ProcessStageInfo(stage_compilers);
	if (!s) return false;

	// Generate
	for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		if (stage_compilers[shader_type] == NULL) continue;

		NGL_shader_source_descriptor &ssd = descriptors[shader_type];
		const std::string &shader_filename = shader_descriptor.GetFilename((NGL_shader_type)shader_type);

		KSLCompiler &c = *stage_compilers[shader_type];
		bool s = c.Generate();

		if (!s || (LOG_WARNINGS && c.GetErrors().size() > 0))
		{
			INFO("generate fail: ");
			for (size_t i = 0; i < header_files.size(); i++)
			{
				INFO("%s ", header_files[i].c_str());
			}
			INFO("%s\n\n", shader_filename.c_str());
			INFO("%s\n", c.GetLog().c_str());
		}

		if (s)
		{
			bool create_reflection = false;
			create_reflection |= (shader_type == NGL_VERTEX_SHADER) && ((nglGetApi() == NGL_METAL_IOS) || (nglGetApi() == NGL_METAL_MACOS));
			create_reflection |= nglGetApi() == NGL_OPENGL || nglGetApi() == NGL_OPENGL_ES;
			if (create_reflection)
			{
				c.CreateReflection(ssd);
			}
		}
		else
		{
			return false;
		}

		ssd.m_source_data = GetGlobalHeader() + c.GetResult();
		delete stage_compilers[shader_type];
	}

	stage_compilers.clear();
	m_pipeline_cache.Add(key, descriptors);

	return true;
}


bool ShaderFactory::CompileWithShaderCache(NGL_shader_source_descriptor descriptors[NGL_NUM_SHADER_TYPES], const ShaderDescriptor &shader_descriptor, const std::vector<NGL_shader_uniform> &uniforms)
{
	const std::vector<std::string> &header_files = shader_descriptor.GetHeaderFiles();

	for (uint32_t shader_type = 0; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		NGL_shader_source_descriptor &ssd = descriptors[shader_type];
		if (ssd.m_source_data.empty()) continue;

		if (m_shader_cache.Search(ssd))
		{
			continue;
		}
		const NGL_shader_source_descriptor key = ssd;

		const std::string &shader_filename = shader_descriptor.GetFilename((NGL_shader_type)shader_type);

		// Setup the compiler
		KSLCompiler c;
		c.SetSource(ssd.m_source_data);
		c.SetShaderType((NGL_shader_type)shader_type);
		c.SetTargetApi(nglGetApi());
		c.SetShaderUniformInfo(uniforms);
		c.SetUseSubpass(nglGetInteger(NGL_SUBPASS_ENABLED) != 0);

		INFO("compile %s...", shader_filename.c_str());
		bool s;
		if( shader_type != NGL_TESS_CONTROL_SHADER && shader_type != NGL_TESS_EVALUATION_SHADER )
		{
			s = c.Compile();
			INFO( "done\n" );
		}
		else
		{
			s = true;
			INFO( "skipped KSL compilation of %s \n", shader_filename.c_str() );
		}

		if (!s || (LOG_WARNINGS && c.GetErrors().size() > 0))
		{
			INFO("compile fail: ");
			for (size_t i = 0; i < header_files.size(); i++)
			{
				INFO("%s ", header_files[i].c_str());
			}
			INFO("%s\n\n", shader_filename.c_str());
			INFO("%s\n", c.GetLog().c_str());
		}

		if (s)
		{
			bool create_reflection = false;
			create_reflection |= (shader_type == NGL_VERTEX_SHADER) && ((nglGetApi() == NGL_METAL_IOS) || (nglGetApi() == NGL_METAL_MACOS));
			create_reflection |= nglGetApi() == NGL_OPENGL || nglGetApi() == NGL_OPENGL_ES;
			if (create_reflection)
			{
				c.CreateReflection(ssd);
			}
		}
		else
		{
			return false;
		}

		if( shader_type != NGL_TESS_CONTROL_SHADER && shader_type != NGL_TESS_EVALUATION_SHADER )
		{
			ssd.m_source_data = GetGlobalHeader() + c.GetResult();
		}
		else
		{
			std::string header = GetGlobalHeader();
			std::string version = header.substr( 0, header.find( "\n" )+1 );
			std::string rest = header.substr( header.find( "\n" ) );
			ssd.m_source_data = 
				version
				+ (nglGetApi() == NGL_OPENGL_ES ? 
					"#extension GL_EXT_tessellation_shader : enable\n"
					"#extension GL_OES_tessellation_shader : enable\n" : "" )
				+ rest
				+ ssd.m_source_data;
		}

		m_shader_cache.Add( key, ssd );
	}

	return true;
}


void ShaderFactory::InitCommonUniforms(std::vector<NGL_shader_uniform> &uniforms)
{
	uniforms.resize(UNIFORM_MAX);

	// Frame
	uniforms[UNIFORM_TIME] = NGL_shader_uniform( "time", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_DELTA_TIME] = NGL_shader_uniform( "delta_time", NGL_GROUP_PER_RENDERER_CHANGE );

	// Camera
	uniforms[UNIFORM_VIEW_POS] = NGL_shader_uniform( "view_pos", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_VIEW_DIR] = NGL_shader_uniform( "view_dir", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_VIEW] = NGL_shader_uniform( "view", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_VP] = NGL_shader_uniform( "vp", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_PREV_VP] = NGL_shader_uniform("prev_vp", NGL_GROUP_PER_RENDERER_CHANGE);
	uniforms[UNIFORM_INV_VP] = NGL_shader_uniform( "vp_inv", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_DEPTH_PARAMETERS] = NGL_shader_uniform( "depth_parameters", NGL_GROUP_PER_RENDERER_CHANGE );

	uniforms[UNIFORM_FRUSTUM_PLANES] = NGL_shader_uniform( "frustum_planes", NGL_GROUP_PER_RENDERER_CHANGE );	
	uniforms[UNIFORM_CORNERS] = NGL_shader_uniform( "corners", NGL_GROUP_PER_RENDERER_CHANGE );

	// Mesh
	uniforms[UNIFORM_MODEL] = NGL_shader_uniform( "model", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_INV_MODEL] = NGL_shader_uniform( "inv-model", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_MV] = NGL_shader_uniform( "mv", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_MVP] = NGL_shader_uniform( "mvp", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_PREV_MVP] = NGL_shader_uniform( "prev_mvp", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_BONES] = NGL_shader_uniform( "bones",NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_PREV_BONES] = NGL_shader_uniform("prev_bones", NGL_GROUP_PER_DRAW);

	// Material
	uniforms[UNIFORM_COLOR_TEX] = NGL_shader_uniform( "color_texture", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_NORMAL_TEX] = NGL_shader_uniform( "normal_texture", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_SPECULAR_TEX] = NGL_shader_uniform( "specular_texture", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_EMISSIVE_TEX] = NGL_shader_uniform( "emissive_texture", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_EMISSIVE_INTENSITY] = NGL_shader_uniform( "emissive_intensity", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_AUX_TEX0] = NGL_shader_uniform("aux_texture0", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_AUX_TEX1] = NGL_shader_uniform("aux_texture1", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_ALPHA_TEST_THRESHOLD] = NGL_shader_uniform( "alpha_test_threshold", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_ROUGHNESS] = NGL_shader_uniform( "roughness", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_DITHER_VALUE] = NGL_shader_uniform("dither_value", NGL_GROUP_PER_DRAW);

	// Lights
	uniforms[UNIFORM_LIGHT_VP] = NGL_shader_uniform( "light_vp", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_LIGHT_POS] = NGL_shader_uniform( "light_pos", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_LIGHT_DIR] = NGL_shader_uniform( "light_dir", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_LIGHT_COLOR] = NGL_shader_uniform( "light_color", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_SPOT_COS] = NGL_shader_uniform( "spot_cos", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_ATTENUATION_PARAMETERS] = NGL_shader_uniform( "attenuation_parameters", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_LIGHT_FUSTUM_PLANES] = NGL_shader_uniform("light_frustum_planes", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_SKY_COLOR] = NGL_shader_uniform( "sky_color", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_GROUND_COLOR] = NGL_shader_uniform( "ground_color", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SHADOW_LIGHT_POS] = NGL_shader_uniform("shadow_light_pos", NGL_GROUP_PER_DRAW);

	// Light shaft
	uniforms[UNIFORM_LIGHTSHAFT_PARAMETERS] = NGL_shader_uniform("lightshaft_parameters", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_VERTEX_ARRAY] = NGL_shader_uniform( "vertex_array", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_BAYER_ARRAY] = NGL_shader_uniform("bayer_array", NGL_GROUP_PER_RENDERER_CHANGE);

	// OVR Tessellation
	uniforms[UNIFORM_CAM_NEAR] = NGL_shader_uniform( "cam_near", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_DISPLACEMENT_TEX] = NGL_shader_uniform( "displacement_tex", NGL_GROUP_PER_DRAW );

	// Fire
	uniforms[UNIFORM_FIRE_TIME] = NGL_shader_uniform( "fire_time", NGL_GROUP_PER_DRAW );

	// Shadow
	uniforms[UNIFORM_SHADOW_MAP] = NGL_shader_uniform( "shadow_map", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_SHADOW_MAP0] = NGL_shader_uniform( "shadow_map0", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_SHADOW_MAP1] = NGL_shader_uniform( "shadow_map1", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SHADOW_MAP2] = NGL_shader_uniform( "shadow_map2", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SHADOW_MAP_SIZE0] = NGL_shader_uniform( "shadow_map_size0", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SHADOW_MAP_SIZE1] = NGL_shader_uniform( "shadow_map_size1", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SHADOW_MAP_SIZE2] = NGL_shader_uniform( "shadow_map_size2", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SHADOW_MATRIX] = NGL_shader_uniform( "shadow_matrix", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_SHADOW_MATRIX0] = NGL_shader_uniform( "shadow_matrix0", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_SHADOW_MATRIX1] = NGL_shader_uniform( "shadow_matrix1", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SHADOW_MATRIX2] = NGL_shader_uniform( "shadow_matrix2", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SHADOW_MATRICES] = NGL_shader_uniform( "shadow_matrices", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SHADOW_FRUSTUM_DISTANCES] = NGL_shader_uniform( "shadow_frustum_distances", NGL_GROUP_PER_RENDERER_CHANGE );

	// SSAO
	uniforms[UNIFORM_world2_and_screen_radius] = NGL_shader_uniform( "world2_and_screen_radius", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SAO_PROJECTION_SCALE] = NGL_shader_uniform( "sao_projection_scale", NGL_GROUP_PER_RENDERER_CHANGE );

	// IBL
	uniforms[UNIFORM_BRDF] = NGL_shader_uniform( "environment_brdf", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_IBL_DIFFUSE_INTENSITY] = NGL_shader_uniform( "ibl_diffuse_intensity",  NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_IBL_REFLECTION_INTENSITY] = NGL_shader_uniform( "ibl_reflection_intensity",  NGL_GROUP_PER_RENDERER_CHANGE );

	// GI
	uniforms[UNIFORM_INDIRECT_LIGHTING_FACTOR] = NGL_shader_uniform( "indirect_lighting_factor", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_direct_lightmap] = NGL_shader_uniform( "m_direct_lightmap", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_envprobe_indirect_uv_map] = NGL_shader_uniform( "m_envprobe_indirect_uv_map", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_envprobe_sh_atlas] = NGL_shader_uniform( "m_envprobe_sh_atlas", NGL_GROUP_PER_DRAW, NGL_BUFFER_SUBRESOURCE);
	uniforms[UNIFORM_envprobe_sh_atlas_texture] = NGL_shader_uniform("m_envprobe_sh_atlas_texture", NGL_GROUP_PER_RENDERER_CHANGE);
	uniforms[UNIFORM_envprobe_envmap_atlas] = NGL_shader_uniform( "m_envprobe_envmap_atlas", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_envprobe_index] = NGL_shader_uniform( "envprobe_index", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_envprobe_inv_half_extent] = NGL_shader_uniform("envprobe_inv_half_extent", NGL_GROUP_PER_DRAW);

	// Fog
	uniforms[UNIFORM_FOG_COLOR] = NGL_shader_uniform( "fog_color", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_FOG_PARAMETERS1] = NGL_shader_uniform( "fog_parameters1", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_FOG_PARAMETERS2] = NGL_shader_uniform( "fog_parameters2", NGL_GROUP_PER_RENDERER_CHANGE );

	// Tone mapper
	uniforms[UNIFORM_HDR_REDUCTION_VALUES] = NGL_shader_uniform( "values", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_HDR_ABCD] = NGL_shader_uniform( "hdr_abcd", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_HDR_EFW_TAU] = NGL_shader_uniform( "hdr_efw_tau", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_HDR_TONEMAP_WHITE] = NGL_shader_uniform( "hdr_tonemap_white", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_HDR_AUTO_EXPOSURE_VALUES] = NGL_shader_uniform( "hdr_auto_exposure_values", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_HDR_EXPOSURE] = NGL_shader_uniform( "hdr_exposure", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_HDR_PREDEFINED_LUMINANACE] = NGL_shader_uniform("hdr_predefined_luminance", NGL_GROUP_PER_RENDERER_CHANGE);

	uniforms[UNIFORM_HDR_UV_TEST_TEXTURE] = NGL_shader_uniform("hdr_uv_test_texture", NGL_GROUP_PER_DRAW);

	// Bloom
	uniforms[UNIFORM_BLOOM_PARAMETERS] = NGL_shader_uniform( "bloom_parameters", NGL_GROUP_PER_RENDERER_CHANGE );

	// Color correction
	uniforms[UNIFORM_COLOR_CORRECTION] = NGL_shader_uniform( "color_correction", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SHARPEN_FILTER] = NGL_shader_uniform( "sharpen_filter", NGL_GROUP_PER_RENDERER_CHANGE );

	// Motion blur
	uniforms[UNIFORM_VELOCITY_MIN_MAX_SCALE_FACTOR] = NGL_shader_uniform( "velocity_min_max_scale_factor", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_TILE_MAX_BUFFER] = NGL_shader_uniform( "tile_max_buffer", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_NEIGHBOR_MAX_TEXTURE] = NGL_shader_uniform( "neighbor_max_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_MB_TILE_UV_TEST_TEXTURE] = NGL_shader_uniform("mb_uv_test_texture", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_TAP_OFFSETS] = NGL_shader_uniform( "tap_offsets", NGL_GROUP_PER_RENDERER_CHANGE );

	// DoF
	uniforms[UNIFORM_DOF_INPUT_TEXTURE] = NGL_shader_uniform("dof_input_texture", NGL_GROUP_PER_RENDERER_CHANGE);
	uniforms[UNIFORM_DOF_PARAMETERS] = NGL_shader_uniform( "dof_parameters", NGL_GROUP_PER_RENDERER_CHANGE );

	// Gauss blur
	uniforms[UNIFORM_GAUSS_LOD_LEVEL] = NGL_shader_uniform( "gauss_lod_level", NGL_GROUP_PER_RENDERER_CHANGE );

	// Debug
	uniforms[UNIFORM_MIP_TEX] = NGL_shader_uniform( "mip_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_MIN_IDEAL_MAX_TEXTURE_DENSITY] = NGL_shader_uniform( "min_ideal_max_texture_density", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_TEXSIZE_SIZE] = NGL_shader_uniform( "orig_texture_size", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_PERIMETER_THRESHOLD] = NGL_shader_uniform( "perimeter_threshold", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_SHADOW_MAP_DIRECT_DEBUG] = NGL_shader_uniform( "shadow_map_direct_debug", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SHADOW_MAP_INDIRECT_DEBUG] = NGL_shader_uniform( "shadow_map_indirect_debug", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_POS0] = NGL_shader_uniform("pos0", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_POS1] = NGL_shader_uniform("pos1", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_COLOR] = NGL_shader_uniform("color", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_COLOR0] = NGL_shader_uniform("color0", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_COLOR1] = NGL_shader_uniform("color1", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_DEBUG_FRUSTUM_IDS] = NGL_shader_uniform("debug_frusum_ids", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_DEBUG_WIREFRAME_MODE] = NGL_shader_uniform("debug_wireframe_mode", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_DEBUG_MESH_LOD_INDEX] = NGL_shader_uniform("debug_mesh_lod_index", NGL_GROUP_PER_DRAW);

	// Render targets
	uniforms[UNIFORM_GBUFFER_COLOR_TEX] = NGL_shader_uniform( "gbuffer_color_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_GBUFFER_NORMAL_TEX] = NGL_shader_uniform( "gbuffer_normal_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_GBUFFER_SPECULAR_TEX] = NGL_shader_uniform( "gbuffer_specular_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_GBUFFER_EMISSIVE_TEX] = NGL_shader_uniform( "gbuffer_emissive_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_GBUFFER_VELOCITY_TEX] = NGL_shader_uniform( "gbuffer_velocity_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_GBUFFER_DEPTH_TEX] = NGL_shader_uniform( "gbuffer_depth_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_DEPTH_DOWNSAMPLE_TEX] = NGL_shader_uniform("depth_downsample_texture", NGL_GROUP_PER_RENDERER_CHANGE);
	uniforms[UNIFORM_LIGHTING_TEX] = NGL_shader_uniform( "lighting_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_SSAO_TEXTURE] = NGL_shader_uniform( "ssao_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_BLURRED_SSAO_TEX] = NGL_shader_uniform( "blurred_ssao_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_ENVMAP0] = NGL_shader_uniform( "envmap0", NGL_GROUP_PER_RENDERER_CHANGE );	uniforms[UNIFORM_ENVMAP1] = NGL_shader_uniform( "envmap1", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_BLOOM_TEXTURE] = NGL_shader_uniform( "bloom_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_BLOOM_TEXTURE0] = NGL_shader_uniform( "bloom_texture0", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_BLOOM_TEXTURE1] = NGL_shader_uniform( "bloom_texture1", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_BLOOM_TEXTURE2] = NGL_shader_uniform( "bloom_texture2", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_BLOOM_TEXTURE3] = NGL_shader_uniform( "bloom_texture3", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_BRIGHT_TEXTURE] = NGL_shader_uniform( "bright_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_BRIGHT_TEXTURE0] = NGL_shader_uniform( "bright_texture0", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_BRIGHT_TEXTURE1] = NGL_shader_uniform( "bright_texture1", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_BRIGHT_TEXTURE2] = NGL_shader_uniform( "bright_texture2", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_BRIGHT_TEXTURE3] = NGL_shader_uniform( "bright_texture3", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_FINAL_TEXTURE] = NGL_shader_uniform( "final_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_MOTION_BLUR_TEXTURE] = NGL_shader_uniform("motion_blur_texture", NGL_GROUP_PER_RENDERER_CHANGE);
	uniforms[UNIFORM_DOF_BLUR_TEXTURE] = NGL_shader_uniform( "dof_blur_texture", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_HALF_RES_TRANSPARENT_TEXTURE] = NGL_shader_uniform("half_res_transparent_texture", NGL_GROUP_PER_RENDERER_CHANGE);
	uniforms[UNIFORM_INDIRECT_WEIGHT_TEXTURE] = NGL_shader_uniform("indirect_weight_texture", NGL_GROUP_PER_RENDERER_CHANGE);

	// Particle system
	uniforms[UNIFORM_PARTICLE_UPLOAD_POOL] = NGL_shader_uniform("particle_upload_pool", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_PARTICLE_UPLOAD_SIZE] = NGL_shader_uniform("particle_upload_size", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_PARTICLE_POSITION] = NGL_shader_uniform("particle_position", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_PARTICLE_SIZE_ROTATION_OPACITY] = NGL_shader_uniform("particle_size_rotation_opacity", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_FLIPBOOK_FRAME] = NGL_shader_uniform("flipbook_frame", NGL_GROUP_PER_DRAW);

	// FSAA
	uniforms[UNIFORM_g_Depth].m_name = "g_Depth";
	uniforms[UNIFORM_g_screenTexture].m_name = "g_screenTexture";
	uniforms[UNIFORM_g_resultTextureFlt4Slot1].m_name = "g_resultTextureFlt4Slot1";
	uniforms[UNIFORM_g_resultTexture].m_name = "g_resultTexture";
	uniforms[UNIFORM_g_src0Texture4Uint].m_name = "g_src0Texture4Uint";
	uniforms[UNIFORM_g_src0TextureFlt].m_name = "g_src0TextureFlt";
	uniforms[UNIFORM_g_resultTextureSlot2].m_name = "g_resultTextureSlot2";
	uniforms[UNIFORM_OneOverScreenSize].m_name = "OneOverScreenSize";
	uniforms[UNIFORM_albedo_tex].m_name = "albedo_tex";
	uniforms[UNIFORM_area_tex].m_name = "area_tex";
	uniforms[UNIFORM_edge_tex].m_name = "edge_tex";
	uniforms[UNIFORM_search_tex].m_name = "search_tex";
	uniforms[UNIFORM_blend_tex].m_name = "blend_tex";

	// Common uniforms
	uniforms[UNIFORM_NUM_WORK_GROUPS] = NGL_shader_uniform( "NumWorkGroups", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_CENTER] = NGL_shader_uniform( "center", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_SCALE] = NGL_shader_uniform( "scale", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_INV_RESOLUTION] = NGL_shader_uniform( "inv_resolution", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_TEXTURE_SAMPLES_INV] = NGL_shader_uniform( "texture_samples_inv", NGL_GROUP_PER_RENDERER_CHANGE );
	uniforms[UNIFORM_LOD_LEVEL] = NGL_shader_uniform( "lod_level", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_INPUT_TEXTURE] = NGL_shader_uniform( "input_texture", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_INPUT_TEXTURE_LOD] = NGL_shader_uniform("input_texture_lod", NGL_GROUP_PER_DRAW, NGL_TEXTURE_SUBRESOURCE);
	uniforms[UNIFORM_TEXTURE_UNIT0] = NGL_shader_uniform( "texture_unit0", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_TEXTURE_UNIT7] = NGL_shader_uniform("texture_unit7", NGL_GROUP_PER_DRAW);
	uniforms[UNIFORM_TEXEL_CENTER] = NGL_shader_uniform( "texel_center", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_SAMPLE_COUNT] = NGL_shader_uniform( "sample_count", NGL_GROUP_PER_DRAW );
	uniforms[UNIFORM_STEP_UV] = NGL_shader_uniform( "step_uv", NGL_GROUP_PER_DRAW );

	// Car Chase
	uniforms[UNIFORM_CAM_NEAR_FAR_PID_VPSCALE].m_name = "cam_near_far_pid_vpscale";
	uniforms[UNIFORM_TESSELLATION_FACTOR].m_name = "tessellation_factor";
	uniforms[UNIFORM_TESSELLATION_MULTIPLIER].m_name = "tessellation_multiplier";

	// ADAS
	uniforms[UNIFORM_VIEW_MATRIX].m_name = "view_matrix";
	uniforms[UNIFORM_Environment_BRDF].m_name = "Environment_BRDF";
	uniforms[UNIFORM_FreiChenK].m_name = "FreiChenK";
}


PipelineCache::PipelineCache()
{
	m_mutex = new Poco::Mutex();
}


PipelineCache::~PipelineCache()
{
	delete m_mutex;
}


bool PipelineCache::Search(const Key &key, NGL_shader_source_descriptor descriptors[NGL_NUM_SHADER_TYPES])
{
	Poco::ScopedLock<Poco::Mutex> lock(*m_mutex);

	std::map<Key, std::vector<NGL_shader_source_descriptor>>::const_iterator it = m_data.find(key);

	if (it != m_data.end())
	{
		for (size_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
		{
			descriptors[i] = it->second[i];
		}
		return true;
	}
	return false;
}


void PipelineCache::Add(const PipelineCache::Key &key, const NGL_shader_source_descriptor descriptors[NGL_NUM_SHADER_TYPES])
{
	Poco::ScopedLock<Poco::Mutex> lock(*m_mutex);

	m_data[key].resize(NGL_NUM_SHADER_TYPES);
	for (size_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		m_data[key][i] = descriptors[i];
	}
}


PipelineCache::Key::Key(const KCL::uint32 sc, const NGL_shader_source_descriptor descriptors[NGL_NUM_SHADER_TYPES])
{
	m_sc = sc;

	m_descriptors.resize(NGL_NUM_SHADER_TYPES);
	for (size_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		m_descriptors[i] = descriptors[i];
	}
}


bool PipelineCache::Key::operator<(const PipelineCache::Key& other) const
{
	if (m_sc != other.m_sc)
	{
		return m_sc < other.m_sc;
	}

	for (size_t i = 0; i < NGL_NUM_SHADER_TYPES; i++)
	{
		if (m_descriptors[i].m_source_data != other.m_descriptors[i].m_source_data)
		{
			return m_descriptors[i].m_source_data < other.m_descriptors[i].m_source_data;
		}
	}

	return false;
}


ShaderCache::ShaderCache()
{
	m_mutex = new Poco::Mutex();
}


ShaderCache::~ShaderCache()
{
	delete m_mutex;
}


bool ShaderCache::Search(NGL_shader_source_descriptor &ssd)
{
	Poco::ScopedLock<Poco::Mutex> lock(*m_mutex);

	std::map<std::string, NGL_shader_source_descriptor>::const_iterator it = m_data.find(ssd.m_source_data);

	if (it != m_data.end())
	{
		ssd = it->second;
		return true;
	}
	return false;
}

void ShaderCache::Add(const NGL_shader_source_descriptor &key, const NGL_shader_source_descriptor &value)
{
	Poco::ScopedLock<Poco::Mutex> lock(*m_mutex);

	m_data[key.m_source_data] = value;
}


