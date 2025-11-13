/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef PLUGIN_H_
#define PLUGIN_H_


#include <QString>
#include <stdint.h>


class PluginManager
{
public:
	PluginManager(const QString &dir);
	int32_t load();
	
private:
	QString dir_;
};

#endif  // PLUGIN_H_
