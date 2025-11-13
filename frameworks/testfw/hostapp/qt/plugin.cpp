/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "plugin.h"
#include <QDir>
#include <QLibrary>
#include <QDirIterator>
#include <QCoreApplication>
#include "ng/log.h"

PluginManager::PluginManager(const QString &dir)
	: dir_(dir)
{
}

int32_t PluginManager::load()
{
    bool appDebug = false;
    
#ifndef NDEBUG
    appDebug = true;
#endif
    
	int32_t cnt = 0;
	QDir dir(dir_);
	dir.setFilter(QDir::Files);
	QDirIterator iterator(dir.absolutePath(), QDirIterator::Subdirectories);
	while (iterator.hasNext())
	{
		iterator.next();
		if (!iterator.fileInfo().isDir())
		{
			QFileInfo info = iterator.fileInfo();
			QString filepath = info.absoluteFilePath();
			if (filepath.endsWith(".dll") || filepath.endsWith(".so") || filepath.endsWith(".dylib"))
			{
                bool pluginDebug = info.baseName().endsWith("_d");
                if((appDebug && pluginDebug) || (!appDebug && !pluginDebug))
                {                    
                    QLibrary library(filepath);
                    library.setLoadHints(QLibrary::ExportExternalSymbolsHint);
                    bool loaded = library.load();
                    if (loaded)
                    {
                        NGLOG_INFO("%s sucessfully loaded", filepath.toStdString());
                        ++cnt;
                    }
                    else
                    {
                        NGLOG_WARN("failed to load: %s", filepath.toStdString());
                    }
                }
			}
		}
	}
	return cnt;
}