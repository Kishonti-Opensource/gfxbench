/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dictionary.h"

#include "getResource.h"

#include "ng/log.h"

#include <Poco/FileStream.h>
#include <Poco/DirectoryIterator.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/NodeFilter.h>
#include <Poco/DOM/NodeList.h>

#include <algorithm>
#include <memory>



Dictionary::Dictionary():
    mCurrentMap(nullptr)
{}



void Dictionary::initializeFromResource()
{
    for (int i = 0; i < getResourceCount(); ++i) {
        std::string resourceName = getResourceName(i);
        Poco::Path path(resourceName);
        if (path.directory(0) != "localization") {
            continue;
        }
        int length;
        const char* data = getResource(resourceName.c_str(), &length);
        std::string language = path.getBaseName().substr(sizeof("string_"));
        addLanguage(language, std::string(data, length));
    }
}



void Dictionary::load(const std::string& localizationDir)
{
    Poco::DirectoryIterator end;
    for (Poco::DirectoryIterator i(localizationDir); i != end; ++i) {
        Poco::FileInputStream stream(i.path().toString());
        std::ostringstream oss;
        oss << stream.rdbuf();
        std::string language = i.path().getBaseName().substr(sizeof("string_"));
        addLanguage(language, oss.str());
    }
}



void Dictionary::addLanguage(const std::string& locale, const std::string& xmlString)
{
    std::map<std::string, std::string>& newMap = mMap[locale];
    newMap = mMap["en"];

    Poco::XML::DOMParser parser;
    Poco::AutoPtr<Poco::XML::Document> document = parser.parseString(xmlString);
    Poco::AutoPtr<Poco::XML::NodeList> nodes = document->documentElement()->getElementsByTagName("string");
    for (unsigned i = 0; i < nodes->length(); ++i) {
        Poco::XML::Element* element = static_cast<Poco::XML::Element*>(nodes->item(i));
        std::string name = element->getAttribute("name");
        std::string value = element->innerText();
        newMap[name] = value;
    }
}



void Dictionary::setLocale(const std::string& locale)
{
    if (mMap.empty()) {
        NGLOG_WARN("Empty dictionary");
        return;
    }

    mCurrentLanguage = "en";
    mCurrentMap = &mMap["en"];

    /* Exact match */
    std::string lang = locale;
    std::replace(lang.begin(), lang.end(), '_', '-');
    auto found = mMap.find(lang);
    if (found != mMap.end()) {
        mCurrentLanguage = lang;
        mCurrentMap = &found->second;
        return;
    }

    /* Language match */
    lang = lang.substr(0, lang.find_first_of('-'));
    for (auto i = mMap.begin(); i != mMap.end(); ++i) {
        const std::string& key = i->first;
        if (key.find(lang) != std::string::npos) {
            mCurrentLanguage = key;
            mCurrentMap = &mMap[key];
            return;
        }
    }
}



const char* Dictionary::getLocalizedString(const char* key) const
{
    if (mCurrentMap == nullptr) {
        return key;
    }
    auto found = mCurrentMap->find(key);
    if (found == mCurrentMap->end()) {
        return key;
    }
    return found->second.c_str();
}



std::vector<Language> Dictionary::getAvailableLanguages() const
{
    std::vector<Language> result;
    for (auto i = mMap.begin(); i != mMap.end(); ++i) {
        result.push_back(i->first);
    }
    return result;
}
