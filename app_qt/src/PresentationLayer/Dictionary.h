/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#pragma once

#include <QObject>
#include <QMap>
#include <QStringList>



namespace QtUI {

    class Dictionary : public QObject
    {
        Q_OBJECT

    public:
        static Dictionary *instance();

        void load(const QString& localizationDir);
        QString getString(const QString &stringId) const;
        QString currentLanguage() const;
        QStringList availableLanguages() const;
        void setLocale(const QString &locale);
    signals:
        void languageChanged();
    private:
        QMap<QString, QMap<QString, QString> > m_map;
        QMap<QString, QString> *m_currentMap;
        QString m_currentLang;

        Dictionary();
        ~Dictionary();
        void addLanguage(const QString &langCode, const QString &xmlString);
    };



    QString dict(const QString &stringId, const QStringList &args = QStringList());
} //namespace QtUI
