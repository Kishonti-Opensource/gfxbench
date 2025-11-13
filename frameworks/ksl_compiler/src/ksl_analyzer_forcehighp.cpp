/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_analyzer.h"

#include "ksl_ast_tree.h"

#include <assert.h>


void ForceTypeToHighp(KSLType &type)
{
	if (type.IsArray())
	{
		KSLType t = type.GetBaseType();
		ForceTypeToHighp(t);
		type.SetBaseType(t);
	}
	else
	{
		if (type.GetTypeClass() & (KSL_TYPECLASS_VOID | KSL_TYPECLASS_BOOL | KSL_TYPECLASS_USER_DEFINED)) return;
		type.precision = KSL_PRECISION_HIGH;
	}
}


class KSLForceHighpTransformer2000 : public KSLNodeVisitor
{
	virtual bool visit(KSLASTNode* n)
	{
		switch (n->node_type)
		{
			case KSL_NODE_VARIABLE_DEFINITIONS:
			{
				KSLVariableDefinitionsNode* vardef = dynamic_cast<KSLVariableDefinitionsNode*>(n);
				ForceTypeToHighp(vardef->variable_type);
				break;
			}

			case KSL_NODE_LITERAL_EXPRESSION:
			{
				KSLLiteralExpressionNode* ln = dynamic_cast<KSLLiteralExpressionNode*>(n);
				ForceTypeToHighp(ln->type);
				break;
			}

			case KSL_NODE_CONSTRUCTOR_EXPRESSION:
			{
				KSLConstructorExpressionNode* cn = dynamic_cast<KSLConstructorExpressionNode*>(n);
				ForceTypeToHighp(cn->constructor_type);
				break;
			}
		}

		return true;
	}
};

void KSLAnalyzer::ForceHighPrecision()
{
	// force variables to highp
	for (size_t i = 0; i < ast->variables.size(); i++)
	{
		ForceTypeToHighp(ast->variables[i].type);
	}

	// force functions to highp
	for (size_t i = 0; i < ast->functions.size(); i++)
	{
		KSLFunction &f = ast->functions[i];

		ForceTypeToHighp(f.return_type);

		for (size_t j = 0; j < f.attrib_type.size(); j++)
		{
			ForceTypeToHighp(f.attrib_type[j]);
		}
	}

	KSLForceHighpTransformer2000 fpt;
	KSL::Traverse(ast->root, fpt);
}

