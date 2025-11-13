/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "Dictionary.h"

#include "ng/log.h"

#include <QDir>
#include <QDomDocument>
#include <QLocale>



using namespace QtUI;



Dictionary *Dictionary::instance()
{
    static Dictionary dictionary;
    return &dictionary;
}



void Dictionary::load(const QString& localizationDir)
{
    QFileInfoList fileInfos =
            QDir(localizationDir).entryInfoList(QStringList("strings*.xml"), QDir::Files);
    fileInfos.push_front(QFileInfo(localizationDir + "/strings_en.xml"));

    foreach(QFileInfo fileInfo, fileInfos) {
        QFile file(fileInfo.filePath());
        if (!file.open(QFile::Text | QFile::ReadOnly)) {
            continue;
        }
        QString langCode = fileInfo.fileName().remove("strings_").remove(".xml");
        QString data = QString::fromUtf8(file.readAll());
        addLanguage(langCode, data);
    }
    setLocale(QLocale::system().name());
}



QString Dictionary::getString(const QString &stringId) const
{
    if(m_currentLang.isEmpty()) {
        return stringId;
    }
    if (!m_currentMap->contains(stringId)) {
        return stringId;
    }
    return (*m_currentMap)[stringId];
}



QStringList Dictionary::availableLanguages() const
{
    return m_map.keys();
}



QString Dictionary::currentLanguage() const
{
    return m_currentLang;
}



void Dictionary::addLanguage(const QString &langCode, const QString &xmlString)
{
    QDomDocument doc;
    QString error;
    if(!doc.setContent(xmlString, false, &error)) {
        NGLOG_WARN("Error parsing localization XML: %s", error.toStdString());
        return;
    }

    m_map[langCode] = m_map["en"];
    QDomElement firstElem = doc.documentElement().firstChildElement();
    for(QDomElement elem = firstElem; !elem.isNull(); elem = elem.nextSiblingElement()) {
        QString text = elem.text().replace("%s", "%1").replace("$s", "");
        m_map[langCode][elem.attribute("name")] = text;
    }
}



void Dictionary::setLocale(const QString &locale)
{
    if (m_map.isEmpty()) {
        NGLOG_WARN("Empty dictionary");
        return;
    }

    m_currentLang = "en";
    m_currentMap = &m_map["en"];

    QString lang = locale;
    lang.replace("_", "-");
    foreach(const QString &key, availableLanguages())
    {
        if (key == lang)
        {
            m_currentLang = lang;
            m_currentMap = &m_map[lang];
            break;
        }
        if (key.contains(lang.section("-", 0, 0)))
        {
            m_currentLang = key;
            m_currentMap = &m_map[key];
            break;
        }
    }
    emit languageChanged();
}



Dictionary::Dictionary():
    m_currentMap(0)
{
}



Dictionary::~Dictionary()
{
}

QString QtUI::dict(const QString &stringId, const QStringList &args /*= QStringList()*/)
{
    QString formatted = Dictionary::instance()->getString(stringId);
    for (int i = 0; i < args.count(); i++) {
        if (!formatted.contains('%')) {
            break;
        }
        formatted = formatted.arg(args[i]);
    }

    return formatted;
}
