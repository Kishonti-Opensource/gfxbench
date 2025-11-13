/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DESKTOPCORPORATE_H
#define DESKTOPCORPORATE_H

#include "DesktopClient.h"

namespace QtUI
{
	class DesktopCorporate : public DesktopClient
	{
	public:
		DesktopCorporate(int &argc, char **argv) : DesktopClient(argc, argv, false, false, false, false, false, false)
		{
            int major = 0, minor = 0;
#ifdef APP_VERSION_MAJOR
            major = APP_VERSION_MAJOR;
#endif
#ifdef APP_VERSION_MINOR
            minor = APP_VERSION_MINOR;
#endif
            mainWindow->setWindowTitle("CompuBench " + QString::number(major) + "." + QString::number(minor));
		}
	};
}

#endif