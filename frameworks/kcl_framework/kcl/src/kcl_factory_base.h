/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef KCL_FACTORY_BASE
#define KCL_FACTORY_BASE

namespace KCL
{

class FactoryBase
{
public:
	FactoryBase()
	{
	}

	virtual ~FactoryBase()
	{
	}

	virtual KCL::Object* Create(const std::string &name, Node *parent, Object *owner) = 0;
};

}//namespace KCL

#endif
