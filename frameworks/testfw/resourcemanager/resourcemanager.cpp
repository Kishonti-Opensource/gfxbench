/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "resourcemanager.h"

#include <Poco/DigestStream.h>
#include <Poco/SHA1Engine.h>
#include <Poco/URIStreamOpener.h>



using namespace tfw;



class CustomDigestInputStream: public Poco::DigestInputStream
{
public:
    CustomDigestInputStream(Poco::DigestEngine& digestEngine, std::istream* baseStream):
        Poco::DigestInputStream(digestEngine, *baseStream),
        m_baseStream(baseStream)
    {}
private:
    std::unique_ptr<std::istream> m_baseStream;
};



class ResourceManager::Private
{
public:
    Poco::URIStreamOpener pocoOpener;
    Poco::SHA1Engine digestEngine;
    bool isHashingEnabled;
};



ResourceManager& ResourceManager::instance()
{
    static ResourceManager resourceManager;
    return resourceManager;
}



ResourceManager::ResourceManager():
    d(new Private)
{}



ResourceManager::~ResourceManager()
{}



std::unique_ptr<std::istream> ResourceManager::open(const std::string &uri)
{
    if (d->isHashingEnabled) {
        return std::unique_ptr<std::istream>(
                new CustomDigestInputStream(d->digestEngine, d->pocoOpener.open(uri)));
    } else {
        return std::unique_ptr<std::istream>(d->pocoOpener.open(uri));
    }
}



bool ResourceManager::isHashingEnabled() const
{
    return d->isHashingEnabled;
}



void ResourceManager::setHashingEnabled(bool enabled)
{
    d->isHashingEnabled = enabled;
}



void ResourceManager::resetHash()
{
    d->digestEngine.reset();
}



std::vector<unsigned char> ResourceManager::getSha1() const
{
    return d->digestEngine.digest();
}



std::string ResourceManager::getSha1String() const
{
    return Poco::DigestEngine::digestToHex(getSha1());
}
