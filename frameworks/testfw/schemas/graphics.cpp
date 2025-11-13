/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "graphics.h"
#include <sstream>

using namespace tfw;

ng::JsonValue Config::toJsonValue() const
{
	ng::JsonValue jconfig;

	jconfig["red_size"] = red();
	jconfig["green_size"] = green();
	jconfig["blue_size"] = blue();
	jconfig["depth_size"] = depth();
	jconfig["alpha_size"] = alpha();
	jconfig["samples"] = samples();
	jconfig["stencil_size"] = stencil();
	jconfig["exact"] = isExact();
	jconfig["vsync"] = isVsync();

	return jconfig;
}

void Config::fromJsonValue(const ng::JsonValue& value)
{
	setRed((int)value["red_size"].numberD(5));
	setGreen((int)value["green_size"].numberD(6));
	setBlue((int)value["blue_size"].numberD(5));
	setDepth((int)value["depth_size"].numberD(16));
	setAlpha((int)value["alpha_size"].numberD(-1));
	setSamples((int)value["samples"].numberD(-1));
	setStencil((int)value["stencil_size"].numberD(-1));
	setExact(value["exact"].booleanD(false));
	setVsync( value["vsync"].booleanD( true ) );
}

ng::JsonValue Graphics::toJsonValue() const
{
	ng::JsonValue jgraphics;

	const std::vector<ApiDefinition> &Versions = versions();

	jgraphics["versions"] = ng::JsonValue();
	ng::JsonValue& jversions = jgraphics["versions"];
	jversions.resize(Versions.size());

	for (size_t i = 0; i < Versions.size(); ++i)
	{
		jversions[i] = Versions[i].toJsonValue();
	}

	jgraphics["config"] = config().toJsonValue();

	jgraphics["fullscreen"] = isFullScreen();
    jgraphics["deviceid"] = deviceid_;
    jgraphics["device_index"] = deviceIndex();

	return jgraphics;
}

void Graphics::fromJsonValue(const ng::JsonValue& value)
{
	if (!value["versions"].isNull() && value["versions"].isArray())
	{
		versions_.resize(value["versions"].size());
		for (size_t i = 0; i < value["versions"].size(); ++i)
		{
			versions_[i].fromJsonValue(value["versions"][i]);
		}
	}
	else {
		versions_.clear();
	}

	if (!value["deviceid"].isNull() && value["deviceid"].isString())
	{
		deviceid_ = value["deviceid"].string();
	}

    if (!value["device_index"].isNull() && value["device_index"].isNumber())
    {
        device_index_ = static_cast<int>(value["device_index"].number());
    }

	const ng::JsonValue &jconfig = value["config"];
	if (!jconfig.isNull())
	{
		config_.fromJsonValue(jconfig);
	}
	setFullScreen(value["fullscreen"].booleanD(false));
}
