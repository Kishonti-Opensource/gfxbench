/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GUI_INTERFACE_H
#define GUI_INTERFACE_H

#include <cstdint>
#include <memory>
#include <vector>
#include <string>

class GUIInterface
{
public:
    virtual ~GUIInterface() {}
	virtual void InitializeGUI() = 0;
	virtual void AdjustGUI() = 0;
	virtual void RenderGUI() = 0;
    virtual void UpdateGUI(uint32_t X, uint32_t Y, bool mouseLPressed, bool mouseLClicked, bool mouseRPressed, bool mouseRClicked, const bool *downKeys) = 0;
	virtual void Init() = 0;
protected:
};

#endif
