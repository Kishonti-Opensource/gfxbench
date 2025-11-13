/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLWIDGET_H_
#define GLWIDGET_H_

#include <QGLWidget>
#include <QDebug>

class GLWidget : public QGLWidget
{
	Q_OBJECT
public:
	GLWidget(const QGLFormat &format, QWidget *parent = 0)
		: QGLWidget(format, parent)
	{
		inited_ = false;
	}
	
	virtual ~GLWidget()
	{
		qDebug() << "~GLWidget()";
	}
signals:
	void glInitialized();

protected:
	virtual void resizeEvent(QResizeEvent*)
	{
	}
	virtual void paintEvent(QPaintEvent*)
	{
		if(!inited_)
		{
			initializeGL();
			inited_ = true;
		}
	}
	virtual void initializeGL()
	{
		emit glInitialized();
	}
	
	bool inited_;
};


#endif  // GLWIDGET_H_
