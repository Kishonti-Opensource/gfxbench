/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "krl_scene.h"
#include "krl_mesh.h"
#include "krl_material.h"
#if defined HAVE_DX
//#include "d3d11/vbopool.h"
#elif defined USE_ANY_GL
//#include "opengl/vbopool.h"
#elif defined USE_METAL
//#include "metal/vbopool.h"
#endif
#include "ng/stringutil.h"

#define EXPORTED_DATA_PATH	"c:/export/"

using namespace KCL;

KCL::KCL_Status KRL_Scene::reloadLights()
{
	KCL::KCL_Status result = KCL_TESTERROR_NOERROR;

	for(size_t i=0; i<m_actors.size(); ++i)
	{
		Actor* a = m_actors[i];
		for(size_t j=0; j<a->m_lights.size(); ++j)
		{
			Light* l = a->m_lights[j];

			KCL::AssetFile file(std::string("lights/") + l->m_light_name);

			if (file.GetLastError())
			{
				printf("!!!error: no light found - %s\n", file.getFilename().c_str());
				continue;
			}

			while( !file.eof())
			{
				char string0[512];
				char string1[512];
				Vector3D color;
				char buff[4096];
				std::stringstream ss;

				file.Gets(buff, 4096);
				ss << buff;

				ss >> string0;

				if( strcmp( string0, "color") == 0)
				{
					ss >> color.x;
					ss >> color.y;
					ss >> color.z;
					l->m_diffuse_color = color;
				}
				else if( strcmp( string0, "type") == 0)
				{
					ss >> string1;
					if( strcmp( string1, "spot") == 0)
					{
						l->m_light_type = Light::SPOT;
					}
					else if( strcmp( string1, "omni") == 0)
					{
						l->m_light_type = Light::OMNI;
					}
					else if( strcmp( string1, "parallel") == 0)
					{
						l->m_light_type = Light::DIRECTIONAL;
					}
					else
					{
					}
				}
				else if( strcmp( string0, "intensity") == 0)
				{
					ss >> string1;
					l->m_intensity = static_cast<float>(ng::atof( string1));
					if( !l->m_intensity)
					{
						l->m_intensity = (float)atoi( string1);
					}
				}
				else if( strcmp( string0, "intensity_track") == 0)
				{
					ss >> string1;

					KCL::AssetFile animfile(std::string("animations/") + string1);
					if(!animfile.GetLastError())
					{
						_key_node::Read( l->m_intensity_track, animfile);
						animfile.Close();
					}
				}
				else if( strcmp( string0, "radius") == 0)
				{
					ss >> string1;
					l->m_radius = (float)ng::atof( string1);
					if( !l->m_radius)
					{
						l->m_radius = (float)atoi( string1);
					}
				}
				else if( strcmp( string0, "fov") == 0)
				{
					ss >> string1;
					l->m_spotAngle = (float)ng::atof( string1);
				}
				else if( strcmp( string0, "cast_shadow") == 0)
				{
					ss >> string1;
					l->m_is_shadow_caster = atoi(string1) > 0;
				}
				else if( strcmp( string0, "glow") == 0)
				{
					ss >> string1;
					l->m_has_glow = atoi(string1) > 0;
				}
				else if( strcmp( string0, "lens_flare") == 0)
				{
					ss >> string1;
					l->m_has_lensflare = atoi(string1) > 0;
				}
				else if( strcmp( string0, "flicker") == 0)
				{
					ss >> string1;
					l->m_is_flickering = atoi(string1) > 0;
				}

			}

			//undefined radius, use old method
			if(l->m_radius < 0)
			{
				l->m_radius = sqrtf( l->m_intensity * 24.0f);
			}
		}
	}

	return result;
}


KCL::KCL_Status KRL_Scene::reloadEmitters()
{
	KCL::KCL_Status result = KCL_TESTERROR_NOERROR;
	INFO("not implemented");
	exit(0);
	return result;
}

static bool GetLatestDirectory(const char * directory, std::string & result)
{
	std::vector<std::string> dirs;
	if (!KCL::File::ListDir(dirs, KCL::FILETYPE_Directory, directory, "*"))
	{
		return false;
	}

	if (dirs.empty())
	{
		return false;
	}
	long latest = 0;
	for (unsigned int i = 0; i < dirs.size(); i++)
	{
		long dir_value = strtol(dirs[i].c_str(), 0, 0);
		if (dir_value > latest)
		{
			latest = dir_value;
			result = directory + dirs[i] + "/";
		}
	}

	return latest > 0L;
}


static void CopyFilesFromDirectory(const std::string & src, const std::string & dst, const std::string & subdir, std::vector<std::string> & filenames)
{
	filenames.clear();
	std::vector<std::string> files;
	std::string src_dir = src + subdir;
	if (!KCL::File::ListDir(files, KCL::FILETYPE_File, src_dir.c_str(), "*"))
	{
		return;
	}
	char buffer[8096];
	for (unsigned int i = 0; i < files.size(); i++)
	{
		std::string src_path = src + subdir + files[i];
		FILE * src_file = fopen(src_path.c_str(), "rb");
		if (!src_file)
		{
			INFO("KCL Scene - Import: Can not open file: %s", src_path.c_str());
			continue;
		}

		std::string dst_path = dst + subdir + files[i];
		FILE * dst_file = fopen(dst_path.c_str(), "wb");
		if (!dst_file)
		{
			fclose(src_file);

			INFO("KCL Scene - Import: Can not write file: %s", dst_path.c_str());
			continue;
		}

		size_t len = 0;
		while (!feof(src_file))
		{
			len = fread(&buffer, 1, 8096, src_file);
			fwrite(buffer, 1, len, dst_file);
		}

		fclose(src_file);
		fclose(dst_file);

		filenames.push_back(files[i]);
	}
}

void KRL_Scene::SaveFreeCamTransform()
{
	KCL::File file("fps_cam_transform", KCL::Write, RDir);
	if(!file.GetLastError())
	{
        file.Write(&m_camera_position, sizeof( KCL::Vector3D), 1);
        file.Write(&m_camera_ref, sizeof( KCL::Vector3D), 1);
        file.Write(&m_animation_time, sizeof( KCL::uint32), 1);
//        file.Write(&m_camera_orientation, sizeof(KCL::Matrix4x4), 1);
		file.Close(); //dtor would close it anyway
	}
}

KCL::uint32 KRL_Scene::LoadFreeCamTransform()
{
    KCL::uint32 val = 0;

	AssetFile file("fps_cam_transform");
	if(!file.GetLastError())
	{
        file.Read(&m_camera_position, sizeof( KCL::Vector3D), 1);
        file.Read(&m_camera_ref, sizeof( KCL::Vector3D), 1);
        file.Read(&val, sizeof( KCL::uint32), 1);
//        file.Read(&m_camera_orientation, sizeof(KCL::Matrix4x4), 1);
		file.Close(); //dtor would close it anyway
	}
    
    m_free_camera = true;
    
    return val;
}

void KRL_Scene::ReloadExportedData(bool revert)
{
	std::string latest_dir;
	if (!revert)
	{
		if (!GetLatestDirectory(EXPORTED_DATA_PATH, latest_dir))
		{
			return;
		}
		INFO("KCL Scene - Import form directory: %s", latest_dir.c_str());
	}
	else
	{
		latest_dir += EXPORTED_DATA_PATH;
		latest_dir += "0/";
		INFO("KCL Scene - Revert form directory: %s", latest_dir.c_str());
	}

	// Get the destination directory
	std::string dst_dir = KCL::File::GetScenePath();

	// Import meshes
	std::vector<std::string> filenames;
	CopyFilesFromDirectory(latest_dir, dst_dir, "meshes/", filenames);

	std::vector<KCL::Mesh3*> updated_mesh_list;
	ReloadExportedMeshes(filenames, updated_mesh_list);

	// Import textures	
	CopyFilesFromDirectory(latest_dir, dst_dir, ImagesDirectory(), filenames);
	ReloadExportedTextures(filenames);

	// Import textures	
	CopyFilesFromDirectory(latest_dir, dst_dir, "materials/", filenames);
	ReloadExportedMaterials(filenames);
}

void KRL_Scene::ReloadExportedMeshes(const std::vector<std::string> & filenames, std::vector<KCL::Mesh3*> &uploaded_meshes)
{	
	if (filenames.empty())
	{
		return;
	}

	for (unsigned int i = 0; i < filenames.size(); i++)
	{
		std::string path_mesh = filenames[i];
		KCL::Mesh3 *mesh = NULL;
		for (unsigned int j = 0; j < m_meshes.size(); j++)
		{
			if (m_meshes[j]->m_name == filenames[i])
			{
				mesh = m_meshes[j];
				break;
			}
		}
		if (!mesh)
		{
			INFO("Scene does not contain mesh: %s", filenames[i].c_str());
			continue;
		}
		
		KCL::AssetFile mesh_file(path_mesh);
		if(mesh_file.GetLastError())
		{
			INFO("Can not open mesh file: %s", path_mesh.c_str());
			continue;
		}
		INFO("Reload mesh: %s", path_mesh.c_str());
		ReadMeshGeometry(mesh, NULL, mesh_file);
		mesh_file.Close();
		uploaded_meshes.push_back(mesh);
	}
}

void KRL_Scene::ReloadExportedTextures(const std::vector<std::string> & filenames)
{	
	if (filenames.empty())
	{
		return;
	}
	for (unsigned int i = 0; i < filenames.size(); i++)
	{
        std::string path_texture = KCL::File::GetScenePath() + ImagesDirectory() + filenames[i];
		KCL::Texture * texture = NULL;
		for (unsigned int j = 0; j < m_textures.size(); j++)
		{
            std::string texname_noext = m_textures[j]->getName().substr(0, m_textures[j]->getName().size() - 4);
            std::string path_noext = path_texture.substr(0, path_texture.size() - 4);

			if (texname_noext == path_noext)
			{
				texture = m_textures[j];
				break;
			}			
		}

 		if (!texture)
		{
			INFO("Scene does not contain texture: %s", path_texture.c_str());
			continue;
		}

		const char * path_texture_cstr = path_texture.c_str();
		INFO("Reload texture: %s", path_texture_cstr);
		
		// TODO: Refactor KCL::Texture and use reference and not pointers for KCL::Image
		KCL::Image * image = new KCL::Image();
		bool res = false;
		switch(texture->getType())
		{
		case KCL::Texture_2D:
			res = image->load(path_texture_cstr, false);
			break;

		case KCL::Texture_3D:
			res = image->load3D(path_texture_cstr) > 0;
			break;

		case KCL::Texture_Array:
			res = image->loadArray(path_texture_cstr) > 0;
			break;

		case KCL::Texture_Cube:
			res = image->loadCube(path_texture_cstr);
			break;

		default:
			INFO("Not supported texture type: %d", texture->getType());
			delete image;
			continue;
		}	
		
		if (!res)
		{
			INFO("Can not load texture file: %s", path_texture_cstr);
			delete image;
			continue;
		}

		texture->setImage(image);		

		// TODO: Refactor KCL::Texture. Now commit deletes the image
		texture->commit();
	}
}

void KRL_Scene::ReloadExportedMaterials(const std::vector<std::string> & filenames)
{
	if (filenames.empty())
	{
		return;
	}
	for (unsigned int i = 0; i < filenames.size(); i++)
	{	
		if (filenames[i].find(".cfg") != std::string::npos)
		{
			continue;
		}

		KCL::Material * material = NULL;
		for (unsigned int j = 0; j < m_materials.size(); j++)
		{
			std::string compare_name = m_materials[j]->m_name;
			if (compare_name == filenames[i])
			{
				material = m_materials[j];
				break;
			}			
		}

 		if (!material)
		{
			INFO("Scene does not contain material:", filenames[i].c_str());
			continue;
		}
		

		INFO("Reload material: %s", filenames[i].c_str());
		if (!ParseMaterial(material, filenames[i].c_str()))
		{
			INFO("Can not parse material file: %s", filenames[i].c_str());
		}

		KRL::Material * krl_material = dynamic_cast<KRL::Material*>(material);
		if (krl_material)
		{
			krl_material->InitShaders(0, "");
		}
	}
}