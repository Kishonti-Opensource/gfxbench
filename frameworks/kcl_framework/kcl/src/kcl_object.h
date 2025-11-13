/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_OBJECT_H
#define KCL_OBJECT_H

#include <kcl_serializable.h>
#include <kcl_base.h>
#include <kcl_os.h>
#include <kcl_io.h>

#include <string>

namespace KCL
{
	enum ObjectType
	{
		UNKNOWN = -1
		,MESH
		,ACTOR
		//SKINNEDMESH,
		//GROUP,
		//CAMERA,
		,LIGHT
		//WORLD,
		//TORCH,
		//GLOW,
		//TEXTURE2D,
		,MATERIAL
		,NODE
		,EFFECT
		,EMITTER1
		,EMITTER2
		,EMITTER4
		,EMITTER5
		,KEYFRAMESEQUENCE
		,ANIMATIONTRACK
		,ROOM
	};


	class Object : public Serializable
	{
	public:
		struct UserParameter
		{
			UserParameter();
			~UserParameter();

			KCL::uint32 m_id;
			KCL::uint32 m_size;
			KCL::uint8 *m_data;
		};

		Object(const std::string &name, ObjectType type);
		virtual ~Object();

		void SetName(const std::string& name);
		virtual void SetGuid(const std::string& guid) { m_guid = guid; }

		virtual void Serialize(JsonSerializer& s);
		virtual std::string GetParameterFilename() const;

		ObjectType m_type;
		KCL::uint32 m_userId;
		std::string m_name;
		UserParameter *m_userData;

		virtual const std::string GetGuid()
		{
			return m_guid;
		}
	protected:
		std::string m_guid;
	};
}

#endif
