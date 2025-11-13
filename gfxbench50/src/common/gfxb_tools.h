/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_TOOLS_H
#define GFXB_TOOLS_H

#include <ngl.h>

#include <string>
#include <vector>

namespace GFXB
{
	class Tools
	{
	public:
		static size_t SplitString(const std::string &str, const char c, std::vector<std::string> &result);
		static void SplitPath(const std::string &full_path, std::string *path, std::string *file, std::string *ext);

	private:
		static void SplitString(const std::string &str, const char c, std::string &front, std::string &back);
	};
};

#endif