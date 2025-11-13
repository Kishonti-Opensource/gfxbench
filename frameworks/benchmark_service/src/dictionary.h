/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "variant.h"

#include <string>
#include <map>



class Language
{
public:
    static int columnCount() { return 1; }
    static const char* columnName(int column) { return "title"; }
    Variant get(int column) const { return languageCode; }
    Language() {}
    Language(const std::string& languageCode): languageCode(languageCode) {}
private:
    std::string languageCode;
};



class Dictionary
{
public:
    Dictionary();
    void initializeFromResource();
    void load(const std::string& localizationDir);
    void setLocale(const std::string& locale);
    const char* getLocalizedString(const char* key) const;
    std::vector<Language> getAvailableLanguages() const;
private:
    std::map<std::string, std::map<std::string, std::string> > mMap;
    std::map<std::string, std::string>* mCurrentMap;
    std::string mCurrentLanguage;
    void addLanguage(const std::string& locale, const std::string& xmlString);
};



#endif
