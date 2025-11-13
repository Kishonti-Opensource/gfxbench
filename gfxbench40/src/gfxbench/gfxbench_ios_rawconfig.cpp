/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if __APPLE__
#include <TargetConditionals.h>
#endif

#include <fstream>
#include <sstream>

#include "ng/json.h"
#include "descriptors.h"


void ParseIOSOverrideRawconfig(const std::string &datarw_, tfw::Descriptor &desc)
{
#if __APPLE__
#if TARGET_OS_IPHONE
    std::string raw_config_json_file_name = datarw_ + "raw_config.json";
    std::ifstream raw_config_file(raw_config_json_file_name);
    
    if (raw_config_file.is_open())
    {
        std::string raw_config_json_str = "";
        
        {
            std::stringstream buffer;
            buffer << raw_config_file.rdbuf();
            raw_config_json_str = buffer.str();
            raw_config_json_str += "\n";
        }
            
        ng::Result res;
        ng::JsonValue raw_config_json;
        
        raw_config_json.fromString(raw_config_json_str.c_str(), res);
        
        if (res.ok())
        {
            printf("iOS rawconfig override found.\n");
            ng::JsonValue::object_iterator it = raw_config_json.beginObject();
            
            for ( ; it != raw_config_json.endObject(); it++)
            {
                desc.setRawConfig<ng::JsonValue>(it->first.c_str(), *it->second);
            }
        }
        else
        {
            printf("iOS rawconfig override found, but corrupt!\n");
        }
    }
#endif
#endif
}

