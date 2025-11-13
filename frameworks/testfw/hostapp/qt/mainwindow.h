/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>
#include <string>

namespace Ui {
	class MainWindow;
}
class Runner;
namespace tfw {
	class TestBase;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	
public slots:
	void start();
	void cancel();
	
private:
	void reset();

	Ui::MainWindow *ui_;
	Runner *runner_;
	tfw::TestBase *test_;
	std::string basePath_;

private slots:
	void onTestFinished(tfw::TestBase *);
	void onTestCancelled(tfw::TestBase *);
	void onTestFailed(tfw::TestBase *);
	void onStartTest();
};

#endif  // MAINWINDOW_H_
