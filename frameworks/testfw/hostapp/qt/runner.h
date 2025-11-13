/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RUNNER_H_
#define RUNNER_H_

#include <QThread>
#include <QGLWidget>
#include "testfw.h"
#include "graphics/qgraphicscontext.h"

class Runner : public QThread
{
	Q_OBJECT
public:
	Runner(QGLWidget *parent, tfw::TestBase *test);
	~Runner();
	void run();

signals:
	void finished(tfw::TestBase *test);
	void cancelled(tfw::TestBase *test);
	void failed(tfw::TestBase *test);
	void initialized(tfw::TestBase *test);
	
private:
	QGraphicsContext ctx_;
	tfw::TestBase *test_;
	
};

#endif
