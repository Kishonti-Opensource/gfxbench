/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "compositedescriptor.h"


using namespace tfw;


std::string CompositeDescriptor::testId() const
{
    return testId_;
}


const std::vector<Descriptor> &CompositeDescriptor::descriptors() const
{
    return descriptors_;
}


void CompositeDescriptor::fromJsonValue(const ng::JsonValue &jroot)
{
    descriptors_.clear();
    const ng::JsonValue &jtests  = jroot["tests"];
    const ng::JsonValue &jtestId = jroot["test_id"];
    if (jtestId.isString() && jtests.isArray())
    {
        testId_ = jtestId.string();
        descriptors_.resize(jtests.size());
        for(size_t i = 0; i < jtests.size(); ++i)
        {
            descriptors_[i].fromJsonValue(jtests[i]);
        }
    }
    else
    {
        descriptors_.resize(1);
        descriptors_[0].fromJsonValue(jroot);
    }
}


ng::JsonValue CompositeDescriptor::toJsonValue() const
{
    throw std::runtime_error("Unimplemented method");
}
