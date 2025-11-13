/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QWidget>



namespace Ui {
    class LoadingScreen;
}
class ApplicationConfig;



class LoadingScreen: public QWidget
{
    Q_OBJECT
public:
    explicit LoadingScreen(QWidget *parent = 0);
    ~LoadingScreen();
    void setTestItem(const QString& testId, const QString& imagePath);
private:
    Ui::LoadingScreen *ui;
};



#endif // LOADINGSCREEN_H
