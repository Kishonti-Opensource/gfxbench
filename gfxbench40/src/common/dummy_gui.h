/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef DUMMY_GUI_H
#define DUMMY_GUI_H

#include "gui_interface.h"

class GLB_Scene4;
class DummyGUI : public GUIInterface
{
public:
	DummyGUI(){}
	virtual void InitializeGUI() {}
	virtual void AdjustGUI() {}
	virtual void RenderGUI() {}
	virtual void UpdateGUI(uint32_t X, uint32_t Y, bool mouseLPressed, bool mouseLClicked, bool mouseRPressed, bool mouseRClicked, const bool *downKeys) {}
	virtual void Init() {}
};


#endif
