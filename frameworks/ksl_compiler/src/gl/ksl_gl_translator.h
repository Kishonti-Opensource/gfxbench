/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_GL_TRANSLATOR__
#define __KSL_GL_TRANSLATOR__

#include "../ksl_translator.h"


enum KSLGLAttribQualifiers
{
	KSL_ATTRIB_QUALIFIER_GL_BINDING = KSL_NUM_ATTRIB_QUALIFIER,
	KSL_ATTRIB_QUALIFIER_GL_STD140,
	KSL_ATTRIB_QUALIFIER_GL_STD430,
	KSL_NUM_GL_ATTRIB_QUALIFIER,
};


class KSLGLTranslator : public KSLTranslator
{
public:
	KSLGLTranslator();
	virtual ~KSLGLTranslator();

	virtual bool Translate();

protected:

	bool SetResourceAttribs();
	bool RenameBufferInnerData();
	virtual bool TranslateSubpassInput();
};


#endif // __KSL_GL_TRANSLATOR__

