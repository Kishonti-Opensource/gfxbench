/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef ANDROIDVULKANGRAPHICSCONTEXT_H
#define ANDROIDVULKANGRAPHICSCONTEXT_H

#include "vulkangraphicscontext.h"

struct ANativeWindow;

class AndroidVulkanGraphicsContext : public VulkanGraphicsContext
{
public:
	bool initWindowSurface(ANativeWindow *window);

	ANativeWindow* getWindow()
	{
		return window_;
	}
protected:
	ANativeWindow *window_;
};

#endif //!ANDROIDVULKANGRAPHICSCONTEXT_H
