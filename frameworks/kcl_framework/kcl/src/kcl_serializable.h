/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_SERIALIZABLE_H
#define KCL_SERIALIZABLE_H

#include "jsonserializer.h"
#include "kcl_math3d.h"

#include <string>

namespace KCL
{
	class Serializable
	{
	public:
		Serializable();
		virtual ~Serializable();

		bool LoadParameters(const std::string &filename = "");
		bool SaveParameters(const std::string &fileName = "");

		virtual void Serialize(JsonSerializer& s) = 0;
		virtual std::string GetParameterFilename() const = 0;

		static void SerializeHexaColor(JsonSerializer &s, const char *name, KCL::Vector3D &color);
		static void SerializeHexaColor(JsonSerializer &s, const char *name, KCL::Vector4D &color);
		static void SerializeHexaColor(const std::string &str, float &r, float &g, float &b);
		static void SerializeHexaColor(const float &r, const float &g, const float &b, std::string &str);


	protected:
		virtual void OnParameterFileNotFound(const std::string &filename);
	};
}

#endif
