/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_D3D_TRANSLATOR__
#define __KSL_D3D_TRANSLATOR__

#include "../ksl_translator.h"


const std::string KSL_D3D_INPUT_LAYOUT_TYPE_NAME = "ksl_d3d_input_layout_type__";
const std::string KSL_D3D_OUTPUT_LAYOUT_TYPE_NAME = "ksl_d3d_output_layout_type__";

const std::string KSL_D3D_INPUT_NAME = "_ksl_d3d_input_";
const std::string KSL_D3D_OUTPUT_NAME = "_ksl_d3d_output_";


class KSLD3DTranslator : public KSLTranslator
{
public:
	KSLD3DTranslator();
	virtual ~KSLD3DTranslator();

	virtual bool Translate();

	bool CollectAttributes();
	bool CreateUniformInterfaceBlock();
	bool CreateMainFunction();
	bool CollectNumThread();

	KSLNumThreadNode* m_num_thread_node;
	bool m_has_input_interface;
	bool m_has_output_interface;
};


#endif // __KSL_D3D_TRANSLATOR__

