/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "runner.h"
#include <QDebug>


Runner::Runner(QGLWidget *parent, tfw::TestBase *test)
	: QThread(parent)
	, ctx_(parent)
	, test_(test)
{
}


Runner::~Runner()
{
	qDebug() << "exiting thread: " << QThread::currentThreadId();
}


void Runner::run()
{
	test_->setGraphicsContext(&ctx_);
	if (test_->init())
	{
		emit initialized(test_);
		test_->run();
		ctx_.widget()->doneCurrent();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
		ctx_.widget()->context()->moveToThread(thread());
#endif
		if (test_->isCancelled())
		{
			emit cancelled(test_);
		}
		else
		{
			emit finished(test_);
		}
	}
	else
	{
		ctx_.widget()->doneCurrent();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
		ctx_.widget()->context()->moveToThread(thread());
#endif
		qDebug() << "Failed to init test: " << test_->name().c_str();
		emit(failed(test_));
	}
}
