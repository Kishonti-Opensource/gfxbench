/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_factories.h"
#include "gfxb_material.h"
#include "gfxb_texture.h"
#include "gfxb_mesh.h"
#include "gfxb_mesh_shape.h"
#include "gfxb_light.h"
#include "gfxb_particlesystem.h"
#include "ngl.h"
#include <sstream>

using namespace GFXB;


GFXB::TextureFactory::TextureFactory()
{
	m_anisotropy_value = 1u;
	m_mip_level_limit = 0u;
}


KCL::Texture *TextureFactory::CreateTexture(const KCL::Image* img, bool releaseUponCommit)
{
	Texture *texture = new Texture(this);
	texture->setImage(img);
	texture->SetAnisotropyValue(m_anisotropy_value);
	return texture;
}


KCL::uint32 TextureFactory::CreateCubeMap()
{
	std::string cube_name("cube");

	std::string m_texture_compression_type = "888";
	std::string directory("images_" + m_texture_compression_type + "/prefiltered_cubemaps");
	directory = "";

	std::vector<KCL::Image*> imgs;

	//for (size_t level = 0; level < 8; ++level)
	{
		for (size_t side = 0; side < 6; ++side)
		{
			KCL::Image* img = new KCL::Image;
			std::stringstream name;
			//name << level << "_" << cube_name << "_" << side << ".tga";
			name << cube_name << "_" << side << ".tga";
			bool res = img->load((directory + "/" + name.str()).c_str(), true);

			if (!res)
			{
				printf("file not found: %s\n", name.str().c_str());
			}

			imgs.push_back(img);
		}
	}

	NGL_texture_descriptor texture_layout;
	std::vector<std::vector<uint8_t>> data;

	texture_layout.m_type = NGL_TEXTURE_CUBE;
	texture_layout.m_filter = NGL_LINEAR_MIPMAPPED;
	texture_layout.m_wrap_mode = NGL_REPEAT_ALL;
	//texture_layout.m_format = _gpu_api::_texture_layout::RGB9E5;
	texture_layout.m_format = NGL_R8_G8_B8_A8_UNORM;
	texture_layout.m_is_renderable = false;
	texture_layout.m_size[0] = imgs[0]->getWidth();
	texture_layout.m_size[1] = imgs[0]->getHeight();

	data.resize(imgs.size());
	for (size_t i = 0; i < imgs.size(); ++i)
	{
		//imgs[i]->decodeRGBA8888toRGB9E5();

		uint8_t* from = (uint8_t*)imgs[i]->getData();
		data[i] = std::vector < uint8_t >(from, from + imgs[i]->getSlicePitch());
	}

	KCL::uint32 cube = 0;
	nglGenTexture(cube, texture_layout, &data);

	return cube;
}


void TextureFactory::SetAnisotropyValue(KCL::uint32 value)
{
	m_anisotropy_value = KCL::Max(value, 1u);
}


void TextureFactory::SetTextureSizeLimit(KCL::uint32 limit)
{
	m_mip_level_limit = KCL::uint32(log(limit)/log(2)) + 1u;
}


void TextureFactory::SetMipLevelLimit(KCL::uint32 limit)
{
	m_mip_level_limit = limit;
}


KCL::uint32 TextureFactory::GetMipLevelLimit() const
{
	return m_mip_level_limit;
}



KCL::Mesh3 *Mesh3Factory::Create(const char *name)
{
	return new GFXB::Mesh3(name);
}


LightFactory::LightFactory(SceneBase *scene)
{
	m_scene = scene;
}


KCL::Object *LightFactory::Create(const std::string &name, KCL::Node *parent, KCL::Object *owner)
{
	Light* e = new Light(this, name, parent, owner);
	return e;
}


SceneBase *LightFactory::GetScene()
{
	return m_scene;
}


KCL::Object *ParticleSystemFactory::Create(const std::string &name, KCL::Node *parent, KCL::Object *owner)
{
	return m_manager->CreateParticleEmitter(name, parent, owner);
}


GFXB::ParticleSystemFactory::ParticleSystemFactory(GFXB::ParticleSystemManager *manager)
{
	m_manager = manager;
}


KCL::Material *GFXB::MaterialFactory::Create(const std::string& material_name, KCL::Node *parent, KCL::Object *owner)
{
	return new GFXB::Material(material_name.c_str());
}


MaterialFactory::~MaterialFactory()
{

}


KCL::Mesh *MeshFactory::Create(const std::string &name, KCL::Node *parent, KCL::Object *owner)
{
	return new Mesh(name, parent, owner);
}
