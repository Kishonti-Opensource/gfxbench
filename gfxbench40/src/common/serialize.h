/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SERIALIZE_H
#define SERIALIZE_H


#include <string>
#include <vector>


template <typename T>
inline std::string to_string(T value)
{
	std::ostringstream os;
	os << value;
	return os.str();
}

class SerializeEntry
{
public:
	SerializeEntry(const std::string &key, const bool &value)
	{
		m_key = key;
		m_value = to_string(value);
	}
	SerializeEntry(const std::string &key, const std::string &value)
	{
		m_key = key;
		m_value = value;
	}
	SerializeEntry(const std::string &key, const float value)
	{
		m_key = key;
		m_value = to_string(value);
	}

	template<class T1, class T2, class T3, class T4, class T5>
	SerializeEntry(const std::string &key, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
	{
		this->m_key = key;
		this->m_value = to_string(t1);
		this->t2 = to_string(t2);
		this->t3 = to_string(t3);
		this->t4 = to_string(t4);
		this->t5 = to_string(t5);
	}

	std::string key() const
	{
		return m_key;
	}
	std::string value() const
	{
		return m_value;
	}
private://TODO hide member variables
public:
	std::string m_key;
	std::string m_value;//t1
	std::string t2;
	std::string t3;
	std::string t4;
	std::string t5;
};

class Serializable
{
public:
	virtual bool GetObjectData(std::vector<SerializeEntry> &entries) = 0;
};

#endif
