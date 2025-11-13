/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <QApplication>
#include "mainwindow.h"
#include "ng/log.h"

int main(int argc, char *argv[])
{
	try
	{
		QApplication app(argc, argv);
		MainWindow window;
		window.show();
		return app.exec();
	}
	catch (std::exception &e)
	{
		NGLOG_FATAL("%s", e.what());
		return 1;
	}
	return 0;
}
