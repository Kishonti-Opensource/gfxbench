/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "kcl_serializable.h"
#include "kcl_io.h"

#include <sstream>
#include <iomanip>

using namespace KCL;

Serializable::Serializable()
{
}


Serializable::~Serializable()
{
}


bool Serializable::SaveParameters(const std::string &filename)
{
	// Serialize the object
	JsonSerializer s(true);
	Serialize(s);

	// Set the output filename
	std::string output_filename = filename.empty() ? GetParameterFilename() : filename;

	// Write the JSON to the file
	KCL::File parameter_file(KCL::File::GetDataRWPath() + output_filename, KCL::Write, KCL::RDir, true);
	if (parameter_file.Opened())
	{
		parameter_file.Write(s.JsonValue.toString());
		parameter_file.Close();
		return true;
	}
	else
	{
		INFO("Serializable::SaveParameters - Error: Can not open parameter file: %s\n", parameter_file.getFilename().c_str());
		parameter_file.Close();
		return false;
	}
}


bool Serializable::LoadParameters(const std::string &filename)
{
	// Set the input filename
	std::string input_filename = filename.empty() ? GetParameterFilename() : filename;

	// Open the JSON file
	KCL::AssetFile parameter_file(input_filename, true);
	if (!parameter_file.Opened())
	{
		parameter_file.Close();

		// Report the error to the object. This way it can create a new file with default values.
		OnParameterFileNotFound(input_filename);

		return false;
	}

	// Read the file as std::string
	std::string json_string = parameter_file.ReadToStdString();

	// Parse the string
	JsonSerializer s(false);
	ng::Result status;
	s.JsonValue.fromString(json_string.c_str(), status);

	if (status.ok())
	{
		// Serialize the object
		Serialize(s);
	}
	else
	{
		// JSON serialization error
		std::stringstream sstream;
		sstream << "Serializable::LoadParameters - Error: Can not parse: " << input_filename << " " << status.what();
		std::string error_msg = sstream.str();

		INFO("%s", error_msg.c_str());
		INFO("%s", json_string.c_str());

		throw KCL::IOException(error_msg);
	}

	return true;
}


void Serializable::OnParameterFileNotFound(const std::string &filename)
{
}


void Serializable::SerializeHexaColor(JsonSerializer &s, const char *name, KCL::Vector4D &color)
{
	KCL::Vector3D color3(color.x, color.y, color.z);
	SerializeHexaColor(s, name, color3);
	color.set(color3, 1.0f);
}


void Serializable::SerializeHexaColor(JsonSerializer &s, const char *name, KCL::Vector3D &color)
{
	if (!s.IsWriter)
	{
		const ng::JsonValue& json = s.JsonValue[name];

		if (!json.isNull() && json.isString())
		{
			SerializeHexaColor(json.stringD(""), color.x, color.y, color.z);
		}
	}
	else
	{
		std::string hexa_color;
		SerializeHexaColor(color.x, color.y, color.z, hexa_color);
		s.Serialize(name, hexa_color);
	}
}


void Serializable::SerializeHexaColor(const std::string &str, float &r, float &g, float &b)
{
	if (str.length() < 7)
	{
		return;
	}

	std::string hex_color = str.substr(1);

	std::stringstream sstream;
	sstream << hex_color;
	KCL::int32 int_color = 0;
	sstream >> std::hex >> int_color;

	r = ((int_color >> 16) & 0xFF) / 255.0f;
	g = ((int_color >> 8) & 0xFF) / 255.0f;
	b = ((int_color)& 0xFF) / 255.0f;
}


void Serializable::SerializeHexaColor(const float &r, const float &g, const float &b, std::string &str)
{
	KCL::int32 r_int = KCL::int32(r * 255.0);
	KCL::int32 g_int = KCL::int32(g * 255.0);
	KCL::int32 b_int = KCL::int32(b * 255.0);

	KCL::int32 hex = ((r_int & 0xff) << 16) + ((g_int & 0xff) << 8) + (b_int & 0xff);

	std::stringstream hex_color;

	hex_color << '#';

	hex_color << std::uppercase << std::setw(6) << std::hex << std::setfill('0') << hex;

	str = hex_color.str();
}
