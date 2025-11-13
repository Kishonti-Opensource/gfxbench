/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <istream>
#include <memory>
#include <string>
#include <vector>



namespace tfw
{



class ResourceManager
{
public:
    static ResourceManager& instance();
    ResourceManager();
    ~ResourceManager();
    std::unique_ptr<std::istream> open(const std::string& uri);
    bool isHashingEnabled() const;
    void setHashingEnabled(bool enabled);
    void resetHash();
    std::vector<unsigned char> getSha1() const;
    std::string getSha1String() const;
private:
    class Private;
    std::unique_ptr<Private> d;
    
    ResourceManager(const ResourceManager&); // No copy
    ResourceManager& operator=(const ResourceManager&); // No copy
};



}



#endif // RESOURCEMANAGER_H
