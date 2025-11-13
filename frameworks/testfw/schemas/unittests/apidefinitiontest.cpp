/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "apidefinition.h"

#include "gtest/gtest.h"


    
TEST(ApiDefinitionTest, CompareIncompatible)
{
    tfw::ApiDefinition gl(tfw::ApiDefinition::GL, 4, 0);
    tfw::ApiDefinition dx(tfw::ApiDefinition::DX, 11, 1);
    
    EXPECT_FALSE(gl.isCompatibleWith(dx));
    EXPECT_FALSE(dx.isCompatibleWith(gl));
}



TEST(ApiDefinitionTest, CompareCompatible)
{
    tfw::ApiDefinition best(tfw::ApiDefinition::GL, 4, 1);
    tfw::ApiDefinition middle(tfw::ApiDefinition::GL, 4, 0);
    tfw::ApiDefinition worst(tfw::ApiDefinition::GL, 2, 2);
    
    EXPECT_TRUE(best.isCompatibleWith(middle));
    EXPECT_TRUE(best.isCompatibleWith(worst));
    
    EXPECT_FALSE(middle.isCompatibleWith(best));
    EXPECT_TRUE(middle.isCompatibleWith(worst));
    
    EXPECT_FALSE(worst.isCompatibleWith(best));
    EXPECT_FALSE(worst.isCompatibleWith(middle));
}
