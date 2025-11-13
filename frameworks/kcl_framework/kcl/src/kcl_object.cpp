/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <kcl_object.h>
#include <kcl_math3d.h>
#include <sstream>
#include <iomanip>

KCL::Object::UserParameter::UserParameter()
{
	m_id = 0;
	m_size = 0;
	m_data = 0;
}

KCL::Object::UserParameter::~UserParameter()
{
	delete[] m_data;
	m_data = 0;
}


KCL::Object::Object(const std::string &name, ObjectType type) : m_type(type), m_userId(0), m_name(name), m_userData(0), m_guid(m_name)
{

}


KCL::Object::~Object()
{
	delete[] m_userData;
	m_userData = 0;
}


void KCL::Object::Serialize(JsonSerializer& s)
{
	s.Serialize("name", m_name);
	s.Serialize("type", m_type);
}


std::string KCL::Object::GetParameterFilename() const
{
	return m_name;
}


//TODO m_guid fix if rename the obj it should be generate new guid
void KCL::Object::SetName(const std::string& name)
{
	m_name = name;
}
