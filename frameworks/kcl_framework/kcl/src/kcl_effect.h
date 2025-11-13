/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_EFFECT_H
#define KCL_EFFECT_H

#include <kcl_node.h>

namespace KCL
{
    class Effect : public Node
    {
    public:
        Effect(const std::string& name, Node *parent, Object *owner);
        virtual ~Effect();
    };
}

#endif
