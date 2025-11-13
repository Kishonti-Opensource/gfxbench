/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "runner.h"
#include "plugin.h"

#include <QDebug>
#include <QGLContext>

#include "testfw.h"
#include "ng/log.h"
#include "glwidget.h"
#include "schemas/descriptors.h"

std::string testPrefix(const tfw::Descriptor &desc)
{
	if (!desc.dataPrefix().empty())
	{
		return desc.dataPrefix();
	}
	else
	{
		const std::string &test_id = desc.testId();
		size_t pos = test_id.find_first_of("_");
		return test_id.substr(0,pos);
	}
}


MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui_ (new Ui::MainWindow())
	, runner_(0)
	, test_(0)
{
	ui_->setupUi(this);
	const QStringList &args = QApplication::arguments();
	int idx = args.indexOf("--base_path");
	if (idx > 0 && args.size() > idx + 1)
	{
		basePath_ = args.at(idx+1).toStdString();
	}
	else
	{
		basePath_ = QApplication::applicationDirPath().toStdString();
	}
	NGLOG_INFO("base path: %s", basePath_);
	QString pluginDir = QString::fromStdString(basePath_ + "/plugins");
	PluginManager mgr(pluginDir);
	mgr.load();
}


MainWindow::~MainWindow()
{
	if (test_)
	{
		test_->cancel();
		runner_->wait();
		reset();
	}
}

void MainWindow::start()
{
	QGLFormat format;
	GLWidget *gl = new GLWidget(format);
	
	QLayout *contentLayout = ui_->contentWidget->layout();
	QLayoutItem *item = contentLayout->itemAt(0);
	while ( item != NULL )
    {
        delete item->widget();
		item = contentLayout->itemAt(0);
    }
	contentLayout->removeItem(item);
	contentLayout->addWidget(gl);

	connect(gl, SIGNAL(glInitialized()), this, SLOT(onStartTest()));
	//onStartTest();
}


void MainWindow::onStartTest()
{
	using namespace tfw;
	
	QGLWidget *gl = (QGLWidget*) ui_->contentLayout->itemAt(0)->widget();
	
	std::string name = ui_->lineEdit->text().toStdString();
	
	std::string configPath = basePath_ + "/config/" + name + ".json";
	Descriptor d;
	std::string error;
	bool ok = Descriptor::fromJsonFile(configPath, &d, &error);
	if (!ok)
	{
		d.setTestId(name);
		NGLOG_WARN(error);
	}
	Environment &env = d.env();
	env.setWidth(ui_->contentWidget->width());
	env.setHeight(ui_->contentWidget->height());
	std::string prefix = testPrefix(d);
	env.setReadPath(basePath_ + "/data/" + prefix + "/");
	env.setWritePath(basePath_ + "/data/" + prefix + "/");
	std::string config = d.toJsonString();
	NGLOG_DEBUG(config);
	
	TestFactory factory = TestFactory::test_factory(d.testId().c_str());
	if (factory.valid())
	{
		TestBase *test = factory.create_test();
		if (test != NULL)
		{
			test->setConfig(config);
			runner_ = new Runner(gl, test);
			test_ = test;
			connect(runner_, SIGNAL(finished(tfw::TestBase*)), this, SLOT(onTestFinished(tfw::TestBase*)), Qt::QueuedConnection);
			connect(runner_, SIGNAL(cancelled(tfw::TestBase*)), this, SLOT(onTestCancelled(tfw::TestBase*)), Qt::QueuedConnection);
			connect(runner_, SIGNAL(failed(tfw::TestBase*)), this, SLOT(onTestFailed(tfw::TestBase*)), Qt::QueuedConnection);
			connect(runner_, SIGNAL(finished()), runner_, SLOT(deleteLater()));
			ui_->start->setEnabled(false);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
			gl->context()->doneCurrent();
			gl->context()->moveToThread(runner_);
#else
			gl->doneCurrent();
#endif
			runner_->start();
		}
		else
		{
			qDebug() << "Failed to create test: " << name.c_str();
		}
	}
	else
	{
		qDebug() << "Failed to create factory for test: " << name.c_str();
	}
}


void MainWindow::onTestFinished(tfw::TestBase *test)
{
	qDebug() << test->result().c_str();
	ui_->start->setEnabled(true);
	reset();
}


void MainWindow::onTestCancelled(tfw::TestBase *test)
{
	qDebug() << "Test cancelled: " << test->name().c_str();
	ui_->start->setEnabled(true);
	reset();
}


void MainWindow::onTestFailed(tfw::TestBase *test)
{
	ui_->start->setEnabled(true);
	reset();
}


void MainWindow::reset()
{
	if (runner_)
		runner_->wait();
	runner_ = 0;
	delete test_;
	test_ = 0;
}


void MainWindow::cancel()
{
	if (test_)
	{
		test_->cancel();
		runner_->wait();
	}
}
