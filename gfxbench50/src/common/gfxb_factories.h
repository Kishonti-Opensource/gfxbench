/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB5_FACTORIES_H
#define GFXB5_FACTORIES_H

#include "kcl_texture.h"
#include "kcl_mesh.h"
#include "kcl_factory_base.h"
#include "gfxb_material.h"
#include "gfxb_particlesystem.h"

namespace KCL
{
	class SceneHandler;
	class Image;
}

namespace GFXB
{
	class MeshFactory : public KCL::MeshFactory
	{
	public:
		virtual KCL::Mesh *Create(const std::string &name, KCL::Node *parent, KCL::Object *owner);
	};


	class TextureFactory : public KCL::TextureFactory
	{
	public:
		TextureFactory();

		KCL::Texture *CreateTexture(const KCL::Image* img, bool releaseUponCommit = false);
		KCL::uint32 CreateCubeMap();

		void SetAnisotropyValue(KCL::uint32 value);

		void SetTextureSizeLimit(KCL::uint32 limit);
		void SetMipLevelLimit(KCL::uint32 limit);
		KCL::uint32 GetMipLevelLimit() const;

	private:
		KCL::uint32 m_anisotropy_value;
		KCL::uint32 m_mip_level_limit;
	};


	class Mesh3Factory : public KCL::Mesh3Factory
	{
	public:
		KCL::Mesh3 *Create(const char *name);
	};

	class MeshFilter;
	class LightFactory : public KCL::FactoryBase
	{
	public:
		LightFactory(SceneBase *scene);
		virtual KCL::Object *Create(const std::string &name, KCL::Node *parent, KCL::Object *owner);
		SceneBase *GetScene();
	private:
		SceneBase *m_scene;
	};


	class ParticleSystemFactory : public KCL::FactoryBase
	{
	public:
		ParticleSystemFactory(GFXB::ParticleSystemManager *manager);
		virtual KCL::Object *Create(const std::string &name, KCL::Node *parent, KCL::Object *owner);
	private:
		GFXB::ParticleSystemManager *m_manager;
	};


	class MaterialFactory : public KCL::FactoryBase
	{
	public:
		virtual ~MaterialFactory();
		virtual KCL::Material *Create(const std::string& material_name, KCL::Node *parent, KCL::Object *owner) override;
	};

}

#endif