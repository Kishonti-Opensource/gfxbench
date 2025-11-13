/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef PROPERTY_H
#define PROPERTY_H

#include "serialize.h"
#include <exception>
#include <stdexcept>
#include <fstream>

class PropertyWriter
{
public:

	template <typename T>
	void Serialize(T &obj, std::ofstream &out)
	{
		entries.clear();
		if (obj.GetObjectData(entries))
		{
			if (out.is_open())
			{
				for (auto e = entries.begin(); e != entries.end(); ++e)
				{
					out << e->key();
					out << " ";
					out << e->value();
					out << " ";
					out << e->t2;
					out << " ";
					out << e->t3;
					out << " ";
					out << e->t4;
					out << " ";
					out << e->t5;
					out << std::endl;
				}
			}
		}
	}
protected:
private:
	std::vector<SerializeEntry> entries;
};

class PropertyLoader
{
public:
	template <class T>
	std::vector<SerializeEntry> DeSerialize(T &obj, std::string &fn);
protected:
private:
};


template <class T>
inline std::vector<SerializeEntry> PropertyLoader::DeSerialize(T &obj, std::string &fn)
{
    KCL::AssetFile prop_file(fn);
    if(prop_file.GetLastError())
	{
        INFO("PropertyLoader - ERROR: Can not open: %s", fn.c_str());
		throw std::runtime_error(("ERROR: File not found=" + fn).c_str());
	}

    std::vector<SerializeEntry> entries;
    std::stringstream in_stream(prop_file.GetBuffer());

	int offset = 0;
	std::string line;
	while (std::getline(in_stream, line))
	{
		if (line.length() < 1)//skip empty lines
		{
			continue;
		}
		std::stringstream ss(line);
		std::string name;
		float value;
		float *arr = reinterpret_cast<float*>(&obj);
		float min = 0;
		float max = 1;
		float step = 0;
		int uniform_offset = 0;

		ss >> name;
		ss >> value;
		ss >> min;
		ss >> max;
		ss >> step;
		ss >> uniform_offset;
		if (uniform_offset != -1)
		{
			arr[uniform_offset] = value;
		}
		entries.push_back(SerializeEntry(name, value, min, max, step, uniform_offset));
		continue;
	}		
	
	return entries;
}

#endif
