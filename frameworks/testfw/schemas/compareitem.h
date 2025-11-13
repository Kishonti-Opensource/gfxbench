/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMPAREITEM_H
#define COMPAREITEM_H

#include "compareresult.h"
#include "resultitem.h"

#include <string>



namespace tfw
{

class CompareItem: public ResultItem
{
public:
    CompareItem() {}
    CompareItem(const ResultItem& resultitem) : ResultItem(resultitem) {};
    
    void fromJsonValue(const ng::JsonValue &jsonValue);
    ng::JsonValue toJsonValue() const;
    
    Result::Status compareStatus() const;
    std::string compareErrorString() const;
    std::string compareDeviceName() const;
    std::string compareDeviceImage() const;
    double compareScore() const;
    double compareFps() const;

    void setCompareResult(const CompareResult &compareResult);
    
private:
    CompareResult compareResult_;
};

}

#endif
