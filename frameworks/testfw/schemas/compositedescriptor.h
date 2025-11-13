/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMPOSITEDESCRIPTOR_H_
#define COMPOSITEDESCRIPTOR_H_

#include "serializable.h"
#include "descriptors.h"
#include <vector>
#include <string>
#include <stdint.h>

namespace tfw
{

class CompositeDescriptor : public Serializable
{
public:
    const std::vector<Descriptor> &descriptors() const;
    std::string testId() const;

private:
    virtual ng::JsonValue toJsonValue() const;
    virtual void fromJsonValue(const ng::JsonValue& value);
    std::vector<Descriptor> descriptors_;
    std::string testId_;
};

}

#endif
