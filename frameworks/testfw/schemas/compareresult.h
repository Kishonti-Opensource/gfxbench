/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMPARERESULT_H
#define COMPARERESULT_H

#include "serializable.h"

#include <string>



namespace tfw
{

class CompareResult: public Serializable
{
public:
    CompareResult(): score_(-1), fps_(-1) {};
    
    void fromJsonValue(const ng::JsonValue &jsonValue);
    ng::JsonValue toJsonValue() const;
    
    std::string variant() const;
    void setVariant(const std::string &variant);
    
    std::string testBase() const;
    void setTestBase(const std::string &testBase);
    
    std::string subType() const;
    void setSubType(const std::string &subType);
    
    std::string os() const;
    void setOs(const std::string &os);
    
    std::string api() const;
    void setApi(const std::string &api);
    
    std::string testFamily() const;
    void setTestFamily(const std::string &testFamily);

    std::string deviceName() const;
    void setDeviceName(const std::string &deviceName);

    std::string deviceImage() const;
    void setDeviceImage(const std::string &deviceImage);
    
    std::string vendor() const;
    void setVendor(const std::string &vendor);
    
    std::string gpu() const;
    void setGpu(const std::string &gpu);
    
    std::string architecture() const;
    void setArchitecture(const std::string &architecture);

    double score() const;
    void setScore(double score);

    double fps() const;
    void setFps(double fps);
    
    std::string resultId() const;
    std::string testId() const;
    std::string compareId() const;
    
private:
    std::string variant_;
    std::string testBase_;
    std::string subType_;
    std::string os_;
    std::string api_;
    std::string testFamily_;
    std::string deviceName_;
    std::string deviceImage_;
    std::string vendor_;
    std::string gpu_;
    std::string architecture_;
    double score_;
    double fps_;
};

}

#endif
