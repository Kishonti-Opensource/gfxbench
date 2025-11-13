/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_tools.h"

#include <sstream>
#include <algorithm>

using namespace GFXB;


size_t Tools::SplitString(const std::string &str, const char c, std::vector<std::string> &result)
{
	result.clear();

	std::stringstream sstream(str);
	std::string substring;
	while (std::getline(sstream, substring, c))
	{
		result.push_back(substring);
	}

	return result.size();
}


void Tools::SplitPath(const std::string &full_path, std::string *path, std::string *file, std::string *ext)
{
	std::string path_;
	std::string file_ext_;
	std::string file_;
	std::string ext_;

	std::string front;
	std::string back;

	SplitString(full_path, '/', front, back);
	if (back.size())
	{
		path_ = front;
		file_ext_ = back;
	}
	else
	{
		file_ext_ = front;
	}


	SplitString(file_ext_, '.', file_, ext_);

	if (path)
	{
		*path = path_;
	}
	if (file)
	{
		*file = file_;
	}
	if (ext)
	{
		*ext = ext_;
	}
}


void Tools::SplitString(const std::string &str, const char c, std::string &front, std::string &back)
{
	front.clear();
	back.clear();

	if (str.empty())
	{
		return;
	}

	const size_t pos = str.find_last_of(c);
	if (pos == std::string::npos)
	{
		front = str;
		return;
	}

	const int front_length = int(pos);
	const int back_length = int(str.size()) - int(pos) - 1;

	if (front_length > 0)
	{
		front = str.substr(0, pos);
	}

	if (back_length > 0)
	{
		back = str.substr(pos + 1, str.size() - 1);
	}
}
