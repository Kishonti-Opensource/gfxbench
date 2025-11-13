/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RECTANGLE_H
#define RECTANGLE_H


namespace GLB
{
	struct Rectangle
	{
		Rectangle(float X = 0.0f, float Y = 0.0f, float Width = 0.0f, float Height = 0.0f) : x(X), y(Y), width(Width), height(Height)
		{
		}

		float x;
		float y;
		float width;
		float height;
	};

}

#endif
