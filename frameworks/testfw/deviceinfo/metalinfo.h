/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef METALINFO_H_
#define METALINFO_H_

#include <string>
#include <vector>

namespace tfw
{



class MetalInfo
{
public:
    MetalInfo(): astcSupport(false) {}
    std::string name;
    bool astcSupport;
    std::vector<std::pair<std::string, int> > attributes;
};

template<class F> void applyVisitor(const MetalInfo& info, F visitor) {
    visitor("FEATURE_SET", info.name);
    visitor("ASTC_SUPPORT", info.astcSupport);
    for (auto i = info.attributes.begin(); i != info.attributes.end(); ++i) {
        visitor(i->first, i->second);
    }
}



class MetalInfoCollector
{
public:
    MetalInfoCollector();
    void collect();
    const MetalInfo& metal() const;
    bool isMetalAvailable() const;
    std::string serialize() const;
private:
    bool hasMetal_;
    MetalInfo metal_;
};


    
}

#endif  // METALINFO_H_
