/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "environment.h"

using namespace tfw;

ng::JsonValue Environment::toJsonValue() const
{
	ng::JsonValue jenv;

	jenv["width"] = width();
	jenv["height"] = height();
	jenv["read_path"] = readPath();
	jenv["write_path"] = writePath();

	jenv["graphics"] = graphics().toJsonValue();

	jenv["compute"] = compute().toJsonValue();

	jenv["locale"] = locale();
    jenv["device"] = device();
    jenv["os"] = os();
    jenv["timestamp"] = timestamp();

	return jenv;
}

void Environment::fromJsonValue(const ng::JsonValue& value)
{
	setWidth((int)value["width"].numberD(0));
	setHeight((int)value["height"].numberD(0));
	setReadPath(value["read_path"].stringD(""));
	setWritePath(value["write_path"].stringD(""));

	const ng::JsonValue &jgraphics = value["graphics"];
	if (!jgraphics.isNull())
	{
		graphics_.fromJsonValue(jgraphics);
	}

	const ng::JsonValue &jcompute = value["compute"];
	if (!jcompute.isNull())
	{
		compute_.fromJsonValue(jcompute);
	}

	setLocale(value["locale"].stringD(""));

    setDevice(value["device"].stringD(""));
    setOs(value["os"].stringD(""));
    setTimestamp(value["timestamp"].stringD(""));
}
