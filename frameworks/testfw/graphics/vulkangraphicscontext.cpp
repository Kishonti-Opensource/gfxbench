/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "vulkangraphicscontext.h"
#include "ng/log.h"
#include "ng/macro_utils.h"


VulkanGraphicsContext::VulkanGraphicsContext()
{
}


VulkanGraphicsContext::~VulkanGraphicsContext()
{

}


bool VulkanGraphicsContext::init()
{
	return true;
}


void VulkanGraphicsContext::destroy()
{

}


bool VulkanGraphicsContext::isValid()
{
	return true;
}


bool VulkanGraphicsContext::makeCurrent()
{
	return true;
}


bool VulkanGraphicsContext::detachThread()
{
	return true;
}


bool VulkanGraphicsContext::swapBuffers()
{
	return true;
}


GraphicsContext::GraphicsType VulkanGraphicsContext::type()
{
	return GraphicsContext::VULKAN;	
}


int VulkanGraphicsContext::versionMajor()
{
	return 1;
}


int VulkanGraphicsContext::versionMinor()
{
	return 0;
}


bool VulkanGraphicsContext::hasFlag(int flag)
{
	return true;
}
