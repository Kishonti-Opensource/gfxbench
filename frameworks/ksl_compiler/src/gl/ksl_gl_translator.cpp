/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_gl_translator.h"

#include <assert.h>
#include <string>


const std::string GL_BUFFER_INNER_DATA = "_inner_data_";


KSLGLTranslator::KSLGLTranslator()
{
	ast = NULL;
}


KSLGLTranslator::~KSLGLTranslator()
{

}


bool KSLGLTranslator::Translate()
{
	if (ast == NULL) return false;

	bool s = true;
	
	// The call order may be relevant!
	s &= RenameBufferInnerData();

	s &= TranslateSubpassInput();

	s &= SetResourceAttribs();

	return s;
}


bool KSLGLTranslator::RenameBufferInnerData()
{
	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;

			if (vardef->storage_type == KSL_STORAGE_BUFFER)
			{
				KSLVariable &v = ast->variables[vardef->variables[0].variable_id];
				v.new_name += GL_BUFFER_INNER_DATA;
			}
		}
	}

	return true;
}


bool KSLGLTranslator::TranslateSubpassInput()
{
	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;
			KSLVariable &v = ast->variables[vardef->variables[0].variable_id];
			KSLType &t = v.type;

			if (vardef->storage_type == KSL_STORAGE_UNIFORM)
			{
				if (t.id == KSL_TYPE_SUBPASS_INPUT)
				{
					vardef->int_attribs.erase(KSL_ATTRIB_QUALIFIER_COLOR);
					vardef->int_attribs.erase(KSL_ATTRIB_QUALIFIER_DEPTH);
				}
			}
		}
	}

	return true;
}


bool KSLGLTranslator::SetResourceAttribs()
{
	uint32_t next_buffer_binding = 0;
	uint32_t next_image_binding = 0;
	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vardef = (KSLVariableDefinitionsNode*)n;
			if (vardef->storage_type == KSL_STORAGE_BUFFER)
			{
				vardef->int_attribs[KSL_ATTRIB_QUALIFIER_GL_BINDING] = next_buffer_binding++;
			}
		}
		else if (n->node_type == KSL_NODE_IMAGE_DEFINITION)
		{
			KSLImageDefinitionNode* idn = (KSLImageDefinitionNode*)n;
			idn->int_attribs[KSL_ATTRIB_QUALIFIER_GL_BINDING] = next_image_binding++;
		}
	}

	return true;
}

