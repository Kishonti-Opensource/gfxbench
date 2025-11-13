/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//
//  global_test_enviroment.h
//  GFXBench
//
//  Created by Hapák József on 07/08/14.
//
//

#include "graphics/graphicscontext.h"
#include "test_descriptor.h"

#ifndef GFXBench_global_test_enviroment_h
#define GFXBench_global_test_enviroment_h

class GlobalTestEnvironment
{
public:
    
    GlobalTestEnvironment(GraphicsContext* graphicscontext) ;
    ~GlobalTestEnvironment() ;
    
    GraphicsContext* GetGraphicsContext() const ;
	TestDescriptor*  GetTestDescriptor()  const ;

	void SetTestDescriptor(TestDescriptor *ts) ;

    bool IsGraphicsContextGLorES() const ;
    bool IsGraphicsContextMetal() const ;
	bool IsEngine(std::string engine) const ;

	void SetTestId(const std::string &test_id);
	const std::string &GetTestId() const;

private:
    GraphicsContext *m_graphicscontext ;
	TestDescriptor  *m_ts ;
	std::string m_test_id;
    
};

#endif // GFXBench_global_test_enviroment_h
