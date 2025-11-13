/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file bitmask.h
	Class definition of GLB::Bitmask
*/
#ifndef BITMASK_H
#define BITMASK_H

#include "platform.h"
#include <kcl_base.h>


namespace GLB
{

/// \class Bitmask
/// Used for controlling which effects are applied in a render pass.
/// \see GLB::Renderpass
class Bitmask
{
public:
	Bitmask () { m_mask = 0; }
	Bitmask (KCL::uint64 mask) { m_mask = mask; }
	void set (KCL::uint8 index)
	{
		m_mask |= ((KCL::uint64)1) << index;
	}
	void unset (KCL::uint8 index)
	{
	    m_mask &= ((KCL::uint64) 0xffffffffffffffffLL - (((KCL::uint64) 0x1LL)<<index));
	}
	
	bool operator[] (const int index) const
	{
		return (m_mask>>index) & 0x0000000000000001LL;
	}

	bool operator== (const Bitmask &m) const
	{
		return m_mask == m.m_mask;
	}

	Bitmask operator |(const Bitmask &mask) const
	{
		return Bitmask (mask.m_mask | m_mask);
	}

	Bitmask operator &(const Bitmask &mask) const
	{
		return Bitmask (mask.m_mask & m_mask);
	}

private:
public:
	KCL::uint64	m_mask;
};

}

#endif
