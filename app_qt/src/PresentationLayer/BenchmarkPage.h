/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include "ui_BenchmarkPage.h"

#include "LoadingScreen.h"

#include "graphics.h"
#include "graphics/graphicscontext.h"

#include <QGLFormat>
#include <QScopedPointer>
#include <QWidget>
#include <QTimer>


class BenchmarkService;

namespace QtUI
{
	class BenchmarkPage : public QWidget
	{
		Q_OBJECT
	public:
		BenchmarkPage(QWidget *parent = 0);
        void setBenchmarkService(BenchmarkService *benchmarkService);
        void prepareTest(
                const char* testId,
                const char* loadingImage,
                const tfw::Graphics& graphics,
                QThread* renderThread);
        void showTest();
	protected:
        void showEvent(QShowEvent* event) override;
        void hideEvent(QHideEvent* event) override;
        virtual void leaveEvent(QEvent *event) override;
		void closeEvent(QCloseEvent* event) override;
		void focusOutEvent(QFocusEvent* event) override;
        void changeEvent(QEvent* event) override;
	private slots:
        void onSkipTriggered();
		void onStopTriggered();
        void avoidSleep();
	private:
        void createDxContext(const tfw::Graphics& graphics);
        void createMetalContext(const tfw::Graphics& graphics);
        void createGlContext(const tfw::Graphics& graphics, QThread* renderThread);
		void createVulkanContext(const tfw::Graphics& graphics);

        QGLFormat glFormatFromDescriptor(const tfw::Graphics& graphics);
        QString getButtonCSS(const QString &name);

		Ui::BenchmarkPage ui;
        QWidget *m_renderWidget;
        BenchmarkService *m_benchmarkService;
        QScopedPointer<GraphicsContext> m_context;
		QString m_progressText;

		bool m_ignoreSkipAndStop;
        QTimer m_timer;
	};
}
