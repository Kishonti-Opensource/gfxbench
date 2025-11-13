/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_LIGHTSHAFT_H
#define GFXB_LIGHTSHAFT_H

#include <kcl_serializable.h>

namespace GFXB
{
	class ShadowMap;
	class MeshFilter;

	class Lightshaft : public KCL::Serializable
	{
	public:
		struct Values
		{
			KCL::uint32 shot_index;
			KCL::uint32 m_sample_count;
			float m_intensity;

			Values()
			{
				shot_index = 42;
				m_sample_count = 32;
				m_intensity = 0.0f;
			}
		};

		Lightshaft(const std::string &name);
		virtual ~Lightshaft();

		void Animate(KCL::uint32 shot_index);

		Values &GetValues();
		Values &GetValues(KCL::uint32 shot_index);
		void SetValues(const Values &values);
		void SetValues(const Values &values, KCL::uint32 shot_index);

		void SetShotCount(KCL::uint32 shot_count);

		virtual std::string GetParameterFilename() const;
		virtual void Serialize(JsonSerializer& s);

	private:
		std::string m_name;

		Values m_current_values;
		std::vector<Values> m_values;

		void EnsureValues(KCL::uint32 index);
	};
}

#endif