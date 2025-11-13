/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_analyzer.h"

#include <assert.h>


bool KSLAnalyzer::ValidateProgram()
{
	bool s = true;
	for (size_t i = 0; i < ast->expressions.size(); i++)
	{
		s &= ValidateExpression(ast->expressions[i]);
	}

	for (size_t i = 0; i < ast->root->nodes.size(); i++)
	{
		KSLASTNode *n = ast->root->nodes[i];
		s &= ValidateNode(n);
	}

	return s;
}


bool KSLAnalyzer::ValidateNode(KSLASTNode *n)
{
	switch (n->node_type)
	{
	case KSL_NODE_VARIABLE_DEFINITIONS: return ValidateVariableDefinitions(dynamic_cast<KSLVariableDefinitionsNode*>(n));
	case KSL_NODE_FUNCTION: return ValidateFunction(dynamic_cast<KSLFunctionNode*>(n));
	case KSL_NODE_EXPRESSION_STATEMENT: return ValidateExpressionStatement(dynamic_cast<KSLExpressionOrEmptyStatementNode*>(n));
	case KSL_NODE_RETURN_STATEMENT: return ValidateReturnStatement(dynamic_cast<KSLReturnStatementNode*>(n));
	case KSL_NODE_IF_STATEMENT: return ValidateIfStatement(dynamic_cast<KSLIfStatementNode*>(n));
	case KSL_NODE_BLOCK_STATEMENT: return ValidateBlockStatement(dynamic_cast<KSLBlockStatementNode*>(n));
	case KSL_NODE_FOR_STATEMENT: return ValidateForStatement(dynamic_cast<KSLForStatementNode*>(n));

	case KSL_NODE_IMAGE_DEFINITION:
	case KSL_NODE_DISCARD_STATEMENT:
	case KSL_NODE_NUMTHREAD:
		return true;

	default:
		break;
	}
	assert(0);
	return true;
}


bool KSLAnalyzer::ValidateVariableDefinitions(KSLVariableDefinitionsNode* vardef)
{
	bool valid = true;

	// check for qualifiers
	{
		std::set<uint32_t> avaliable_int_qualifiers;
		std::set<uint32_t> avaliable_bool_qualifiers;

		if (vardef->storage_type == KSL_STORAGE_BUFFER)
		{
			avaliable_bool_qualifiers.insert(KSL_ATTRIB_QUALIFIER_SSBO);
			avaliable_bool_qualifiers.insert(KSL_ATTRIB_QUALIFIER_READONLY);
		}
		else if (vardef->storage_type == KSL_STORAGE_OUT)
		{
			if (m_shader_type == NGL_FRAGMENT_SHADER)
			{
				avaliable_int_qualifiers.insert(KSL_ATTRIB_QUALIFIER_COLOR);
			}
		}
		else if (vardef->storage_type == KSL_STORAGE_UNIFORM)
		{
			if ((m_shader_type == NGL_FRAGMENT_SHADER) && (vardef->variable_type.IsSubpassInput()))
			{
				avaliable_int_qualifiers.insert(KSL_ATTRIB_QUALIFIER_COLOR);
			}
		}

		for (std::set<uint32_t>::const_iterator it = vardef->bool_attribs.begin(); it != vardef->bool_attribs.end(); it++)
		{
			if (avaliable_bool_qualifiers.find(*it) == avaliable_bool_qualifiers.end())
			{
				ErrorInvalidAttributeQualifier(vardef, (KSLAttribQualifier)*it);
				valid = false;
			}
		}

		for (std::map<uint32_t, uint32_t>::const_iterator it = vardef->int_attribs.begin(); it != vardef->int_attribs.end(); it++)
		{
			if (avaliable_int_qualifiers.find(it->first) == avaliable_int_qualifiers.end())
			{
				ErrorInvalidAttributeQualifier(vardef, (KSLAttribQualifier)it->first);
				valid = false;
			}
		}
	}

	if (vardef->storage_type == KSL_STORAGE_BUFFER)
	{
		KSLVariableDefinitionsNode::InnerNode &dn = vardef->variables[0];
		KSLVariable &v = ast->variables[dn.variable_id];
		bool is_ssbo = (vardef->bool_attribs.find(KSL_ATTRIB_QUALIFIER_SSBO) != vardef->bool_attribs.end());

		// check for implicitly sized ubo
		if (v.type.IsArray() && (dn.size_expression == NULL) && !is_ssbo)
		{
			ErrorImplicitlySizedUbo(vardef);
			valid = false;
		}

		// ssbo in fragment shader must be readonly
		if ((m_shader_type == NGL_FRAGMENT_SHADER) && is_ssbo && (vardef->bool_attribs.find(KSL_ATTRIB_QUALIFIER_READONLY) == vardef->bool_attribs.end()))
		{
			ErrorReadWriteSSBOInFragmentShader(vardef);
			valid = false;
		}
	}

	// check for supported uniforms
	if (vardef->storage_type == KSL_STORAGE_UNIFORM)
	{
		KSLVariableDefinitionsNode::InnerNode &dn = vardef->variables[0];
		KSLVariable &v = ast->variables[dn.variable_id];

		if (v.type.IsBool() || v.type.IsNumeric())
		{
			uint32_t cc = v.type.GetComponentCount();

			bool supported = (cc == 1) || (cc == 2) || (cc == 4) || (cc == 16);
			if (!supported)
			{
				ErrorUnsupportedUniformType(vardef);
				valid = false;
			}
		}
		else if (v.type.IsArray())
		{
			uint32_t cc = v.type.GetBaseType().GetComponentCount();

			bool supported = (cc == 4);
			if (!supported)
			{
				ErrorUnsupportedUniformArrayType(vardef);
				valid = false;
			}
		}
	}

	// check in out attrib type
	if ((vardef->storage_type == KSL_STORAGE_IN) || (vardef->storage_type == KSL_STORAGE_OUT))
	{
		KSLVariableDefinitionsNode::InnerNode &dn = vardef->variables[0];
		KSLVariable &v = ast->variables[dn.variable_id];

		if (v.type.IsMatrix())
		{
			ErrorUnsupportedInOutAttribType(vardef);
			valid = false;
		}
	}


	for (size_t i = 0; i < vardef->variables.size(); i++)
	{
		KSLVariableDefinitionsNode::InnerNode &v = vardef->variables[i];

		for (size_t j = 0; j < v.init_expressions.size(); j++)
		{
			bool s = ValidateExpression(v.init_expressions[j]);
			if (!s) return false;

			KSLType &init_type = v.init_expressions[j]->type;
			if (vardef->variable_type != init_type)
			{
				ErrorInitExpressionTypeMismatch(vardef, init_type);
				return false;
			}

			if (vardef->variable_type.precision != init_type.precision)
			{
				WarningInitExpressionPrecisionMismatch(vardef, init_type);
			}
		}
	}


	return valid;
}


bool KSLAnalyzer::ValidateFunction(KSLFunctionNode* fn)
{
	m_validated_function_node = fn;
	bool s = ValidateBlockStatement(fn->body);
	m_validated_function_node = NULL;
	return s;
}


bool KSLAnalyzer::ValidateBlockStatement(KSLBlockStatementNode* bsn)
{
	bool s = true;
	for (size_t i = 0; i < bsn->nodes.size(); i++)
	{
		s &= ValidateNode(bsn->nodes[i]);
	}
	return s;
}


bool KSLAnalyzer::ValidateExpressionStatement(KSLExpressionOrEmptyStatementNode* eesn)
{
	return (eesn->expression == NULL) ? true : ValidateExpression(eesn->expression);
}


bool KSLAnalyzer::ValidateReturnStatement(KSLReturnStatementNode* rsn)
{
	assert(m_validated_function_node != NULL);

	bool valid = true;
	KSLType exp_type = KSLType(KSL_TYPE_VOID, KSL_PRECISION_NONE);
	KSLType func_type = ast->functions[m_validated_function_node->function_id].return_type;
	
	if (rsn->expression != NULL)
	{
		valid &= ValidateExpression(rsn->expression);
		if (!valid) return false;
		exp_type = rsn->expression->type;
	}
	
	if (func_type != exp_type)
	{
		ErrorReturnStatetmantTypeMismatch(rsn, func_type, exp_type);
		return false;
	}

	if (func_type.precision != exp_type.precision)
	{
		WarningReturnStatetmantPrecisionMismatch(rsn, func_type, exp_type);
	}

	return valid;
}


bool KSLAnalyzer::ValidateIfStatement(KSLIfStatementNode* isn)
{
	bool valid = true;

	valid &= ValidateExpression(isn->condition);
	valid &= ValidateNode(isn->if_statement);

	if (isn->else_statement != NULL)
	{
		valid &= ValidateNode(isn->else_statement);
	}

	// KSL_TODO: condition type validation

	return valid;
}


bool KSLAnalyzer::ValidateForStatement(KSLForStatementNode* fsn)
{
	bool valid = true;

	valid &= ValidateNode(fsn->init_node);
	valid &= ValidateExpression(fsn->conditional_statement);
	valid &= ValidateExpression(fsn->step_statement);
	valid &= ValidateNode(fsn->body);

	// KSL_TODO: additional validations

	return valid;
}


bool KSLAnalyzer::ValidateVariableExpression(KSLVariableExpressionNode* ve)
{
	if (ve->type.id != KSL_TYPE_INVALID) return true;

	KSLVariable v = ast->variables[ve->variable_id];

	if (v.type.id == KSL_TYPE_INVALID)
	{
		assert(0);
		return false;
	}

	ve->type = v.type;
	ve->l_value = !ve->type.IsArray();
	return true;
}


bool KSLAnalyzer::ValidateFunctionCallExpression(KSLFunctionCallExpressionNode* fce)
{
	if (fce->type.id != KSL_TYPE_INVALID) return true;

	for (size_t i = 0; i < fce->arguments.size(); i++)
	{
		if (!ValidateExpression(fce->arguments[i])) return false;
	}

	bool type_valid_found = false;

	for (size_t i = 0; i < ast->functions.size(); i++)
	{
		KSLFunction &f = ast->functions[i];

		if (f.function_name != fce->name) continue;

		// the argument count doesn't match
		if (f.attrib_type.size() != fce->arguments.size()) continue;

		bool valid = true;
		for (size_t j = 0; j < f.attrib_type.size(); j++)
		{
			valid &= fce->arguments[j]->type == f.attrib_type[j];
			if (!valid) break;
		}

		if (!valid)
		{
			continue;
		}

		type_valid_found = true;
		fce->type = f.return_type;

		bool precision_valid_found = true;
		for (size_t j = 0; j < f.attrib_type.size(); j++)
		{
			precision_valid_found &= fce->arguments[j]->type.precision == f.attrib_type[j].precision;
			if (!precision_valid_found) break;
		}

		if (precision_valid_found)
		{
			fce->type = f.return_type;
			return true;
		}
	}

	if (type_valid_found)
	{
		WarningFunctionArgumentPrecisionMismatch(fce);
		fce->type.precision = KSL_PRECISION_NONE;
		return true;
	}
	else
	{
		ErrorFunctionNotFound(fce);
	}

	return false;
}


bool basic_binop_validation_equal_numeric_types(KSLBinaryExpressionNode* be, bool &valid)
{
	if (!be->e1->type.IsNumeric()) return valid;
	if (!be->e2->type.IsNumeric()) return valid;

	if (be->e1->type.id == be->e2->type.id)
	{
		be->type = be->e1->type;
		valid = true;
	}
	return valid;
}


bool basic_binop_validation_scalar_numeric_type(KSLBinaryExpressionNode* be, bool &valid)
{
	if (!be->e1->type.IsNumeric()) return valid;
	if (!be->e2->type.IsNumeric()) return valid;

	if (be->e1->type.GetTypeClass() == be->e2->type.GetTypeClass())
	{
		if (be->e1->type.GetComponentCount() == 1)
		{
			be->type = be->e2->type;
			valid = true;
		}
		if (be->e2->type.GetComponentCount() == 1)
		{
			be->type = be->e1->type;
			valid = true;
		}
	}
	return valid;
}


bool basic_binop_validation_right_operand_scalar_numeric_type(KSLBinaryExpressionNode* be, bool &valid)
{
	if (!be->e1->type.IsNumeric()) return valid;
	if (!be->e2->type.IsNumeric()) return valid;

	if (be->e1->type.GetTypeClass() == be->e2->type.GetTypeClass())
	{
		if (be->e2->type.GetComponentCount() == 1)
		{
			be->type = be->e1->type;
			valid = true;
		}
	}
	return valid;
}


bool KSLAnalyzer::ValidateBinaryExpression(KSLBinaryExpressionNode* be)
{
	if (be->type.id != KSL_TYPE_INVALID) return true;

	if (!ValidateExpression(be->e1)) return false;
	if (!ValidateExpression(be->e2)) return false;

	bool valid = false;
	switch (be->operation)
	{
	
	case KSL_BINOP_ADD:
	case KSL_BINOP_SUB:
	case KSL_BINOP_DIV:
	{
		if (basic_binop_validation_equal_numeric_types(be, valid)) break;
		if (basic_binop_validation_scalar_numeric_type(be, valid)) break;
		break;
	}

	case KSL_BINOP_MUL:
	{
		if (basic_binop_validation_equal_numeric_types(be, valid)) break;
		if (basic_binop_validation_scalar_numeric_type(be, valid)) break;

		KSLBaseType rt = m_multiplication_table[be->e1->type.id][be->e2->type.id];
		if (rt != KSL_TYPE_INVALID)
		{
			be->type.id = rt;
			be->type.precision = KSL_PRECISION_NONE;
			valid = true;
		}
		break;
	}	

	case KSL_BINOP_MOD:
	{
		if (!be->e1->type.IsInteger()) break;
		if (!be->e2->type.IsInteger()) break;

		if (basic_binop_validation_equal_numeric_types(be, valid)) break;
		if (basic_binop_validation_scalar_numeric_type(be, valid)) break;
		break;
	}

	case KSL_BINOP_ADD_ASSIGN:
	case KSL_BINOP_SUB_ASSIGN:
	case KSL_BINOP_DIV_ASSIGN:
	case KSL_BINOP_MUL_ASSIGN:
	{
		if (basic_binop_validation_equal_numeric_types(be, valid)) break;
		if (basic_binop_validation_right_operand_scalar_numeric_type(be, valid)) break;
		break;
	}

	case KSL_BINOP_EQUAL:
	case KSL_BINOP_NOTEQUAL:
	{
		valid = (be->e1->type == be->e2->type);
		if (valid) be->type = KSLType(KSL_TYPE_BOOL, KSL_PRECISION_NONE);
		break;
	}

	case KSL_BINOP_LESS:
	case KSL_BINOP_LESSEQUAL:
	case KSL_BINOP_GREATER:
	case KSL_BINOP_GREATEREQUAL:
	{
		if (!be->e1->type.IsNumeric()) break;
		if (!be->e2->type.IsNumeric()) break;

		valid = true;
		valid &= (be->e1->type.GetComponentCount() == 1) && (be->e2->type.GetComponentCount() == 1);
		valid &= (be->e1->type == be->e2->type);
		if (valid) be->type = KSLType(KSL_TYPE_BOOL, KSL_PRECISION_NONE);
		break;
	}

	case KSL_BINOP_SHIFT_LEFT:
	case KSL_BINOP_SHIFT_RIGHT:
	{
		valid = true;
		valid &= (be->e1->type.id == KSL_TYPE_INT) || (be->e1->type.id == KSL_TYPE_UINT);
		valid &= (be->e2->type.id == KSL_TYPE_INT) || (be->e2->type.id == KSL_TYPE_UINT);
		if (valid) be->type = be->e1->type;
		break;
	}

	case KSL_BINOP_BITWISE_AND:
	case KSL_BINOP_BITWISE_OR:
	{
		valid = true;
		valid &= (be->e1->type.id == KSL_TYPE_INT) || (be->e1->type.id == KSL_TYPE_UINT);
		valid &= be->e1->type.id == be->e2->type.id;
		valid &= be->e1->type.precision == be->e2->type.precision;
		if (valid) be->type = be->e1->type;
		break;
	}

	case KSL_BINOP_LAZY_OR:
	case KSL_BINOP_LAZY_AND:
	{
		if (!be->e1->type.IsBool()) break;
		if (!be->e2->type.IsBool()) break;

		valid = (be->e1->type.GetComponentCount() == 1) && (be->e2->type.GetComponentCount() == 1);
		if (valid) be->type = KSLType(KSL_TYPE_BOOL, KSL_PRECISION_NONE);
		break;
	}

	case KSL_BINOP_ASSIGN:
	{
		valid = true;
		valid &= (be->e1->type.id == be->e2->type.id);
		if (valid) be->type = be->e1->type;
		break;
	}
		
	default:
		assert(0);
		break;
	}

	if (!valid)
	{
		ErrorBinopTypeMismatch(be);
	}

	if (KSL::BinopIsAssing(be->operation) && !be->e1->l_value)
	{
		be->type.id = KSL_TYPE_INVALID;
		ErrorBinopAssignLeftOperandNotLValue(be);
		valid = false;
	}

	if (!valid)
	{
		return false;
	}

	// Check precisions
	assert(be->e1->type.precision != KSL_PRECISION_INVALID);
	assert(be->e2->type.precision != KSL_PRECISION_INVALID);
	
	switch (be->operation)
	{
	case KSL_BINOP_SHIFT_LEFT:
	case KSL_BINOP_SHIFT_LEFT_ASSIGN:
	case KSL_BINOP_SHIFT_RIGHT:
	case KSL_BINOP_SHIFT_RIGHT_ASSIGN:
		break;

	case KSL_BINOP_EQUAL:
	case KSL_BINOP_NOTEQUAL:
	case KSL_BINOP_LESS:
	case KSL_BINOP_LESSEQUAL:
	case KSL_BINOP_GREATER:
	case KSL_BINOP_GREATEREQUAL:
		if (be->e1->type.precision != be->e2->type.precision)
		{
			WarningBinopPrecisionMismatch(be);
		}
		break;

	default:
		{
			if (be->e1->type.precision == be->e2->type.precision)
			{
				be->type.precision = be->e1->type.precision;
			}
			else
			{
				WarningBinopPrecisionMismatch(be);
				be->type.precision = KSL_PRECISION_NONE;
			}
		}
	}

	// check for interger division
	if ((be->operation == KSL_BINOP_DIV) && (be->e1->type.GetTypeClass() == KSL_TYPECLASS_INT) && (be->e2->type.GetTypeClass() == KSL_TYPECLASS_INT))
	{
		WarningIntegerDivision(be);
	}

	be->constant_value = be->e1->constant_value && be->e2->constant_value;

	return valid;
}


bool KSLAnalyzer::ValidateParenthesisExpression(KSLParenthesisExpressionNode* pe)
{
	if (pe->type.id != KSL_TYPE_INVALID) return true;

	bool s = ValidateExpression(pe->expression);
	pe->type = pe->expression->type;
	pe->constant_value = pe->expression->constant_value;
	pe->l_value = pe->expression->l_value;
	return s;
}


bool KSLAnalyzer::ValidateMemberAccessExpression(KSLSelectorExpressionNode* me)
{
	if (me->type.id != KSL_TYPE_INVALID) return true;

	if (!ValidateExpression(me->expression)) return false;

	if (me->expression->type.id < KSL_NUM_INBUILT_TYPES)
	{
		std::string valid_selectors = "";

		// KSL_TODO
		// this allow mixed selectors. for example xyba

		switch (me->expression->type.id)
		{
		case KSL_TYPE_INT2:
		case KSL_TYPE_UINT2:
		case KSL_TYPE_VEC2:
			valid_selectors = "xyrgst";
			break;
		
		case KSL_TYPE_INT3:
		case KSL_TYPE_UINT3:
		case KSL_TYPE_VEC3:
			valid_selectors = "xyzrgbstp";
			break;

		case KSL_TYPE_INT4:
		case KSL_TYPE_UINT4:
		case KSL_TYPE_VEC4:
			valid_selectors = "xyzwrgbastpq";
			break;
		default:
			break;
		}

		bool valid = true;

		if (me->selector.size() <= 4)
		{
			for (size_t i = 0; i < me->selector.size(); i++)
			{
				valid &= (valid_selectors.find(me->selector[i]) != std::string::npos);
			}
		}
		else
		{
			valid = false;
		}

		if (valid)
		{
			me->type = KSLType::Create(me->expression->type.GetTypeClass(), (uint32_t)me->selector.size());
			me->type.precision = me->expression->type.precision;

			// KSL_TODO
			// not all selector l-value
			me->l_value = true;
		}
		else
		{
			ErrorInvalidSelector(me);
		}

		return valid;
	}
	else
	{
		assert(0);
	}
	
	return false;
}


bool KSLAnalyzer::ValidateUnaryOperationExpression(KSLUnaryExpressionNode* ue)
{
	if (ue->type.id != KSL_TYPE_INVALID) return true;

	bool s = ValidateExpression(ue->expression);
	ue->type = ue->expression->type;
	return s;
}


bool KSLAnalyzer::ValidateArrayAccessExpression(KSLSelectorExpressionNode* aae)
{
	if (aae->type.id != KSL_TYPE_INVALID) return true;

	if (!ValidateExpression(aae->expression)) return false;
	if (!ValidateExpression(aae->id_expression)) return false;

	bool valid = true;
	valid &= aae->expression->type.IsArray() || aae->expression->type.IsMatrix() || aae->expression->type.IsVector();
	valid &= (aae->id_expression->type.id == KSL_TYPE_INT) || (aae->id_expression->type.id == KSL_TYPE_UINT);
	if (valid)
	{
		if (aae->expression->type.IsArray())
		{
			aae->type = aae->expression->type.GetBaseType();
			aae->l_value = !aae->type.IsArray();
		}
		else if (aae->expression->type.IsMatrix())
		{
			switch (aae->expression->type.id)
			{
			case KSL_TYPE_MAT4: aae->type = KSLType(KSL_TYPE_VEC4, aae->expression->type.precision); break;
			case KSL_TYPE_MAT3: aae->type = KSLType(KSL_TYPE_VEC3, aae->expression->type.precision); break;
			case KSL_TYPE_MAT2: aae->type = KSLType(KSL_TYPE_VEC2, aae->expression->type.precision); break;
			default:
				assert(0);
				break;
			}
			aae->l_value = false;
		}
		else if (aae->expression->type.IsVector())
		{
			switch (aae->expression->type.id)
			{
			case KSL_TYPE_VEC4:
			case KSL_TYPE_VEC3:
			case KSL_TYPE_VEC2:
				aae->type = KSLType(KSL_TYPE_FLOAT, aae->expression->type.precision);
				break;

			case KSL_TYPE_INT4:
			case KSL_TYPE_INT3:
			case KSL_TYPE_INT2:
				aae->type = KSLType(KSL_TYPE_INT, aae->expression->type.precision);
				break;

			case KSL_TYPE_UINT4:
			case KSL_TYPE_UINT3:
			case KSL_TYPE_UINT2:
				aae->type = KSLType(KSL_TYPE_UINT, aae->expression->type.precision);
				break;

			default:
				assert(0);
				break;
			}
			aae->l_value = aae->expression->l_value;
		}
	}
	else
	{
		ErrorInvalidArrayAccess(aae);
	}

	if (valid)
	{
		const KSLType &et = aae->expression->type;
		bool matrix_or_vector = et.IsVector() || et.IsMatrix();

		if (matrix_or_vector && !aae->id_expression->constant_value)
		{
			ErrorInvalidDynamicArrayAccess(aae);
		}
	}

	return valid;
}


bool KSLAnalyzer::ValidateTernaryConditionalExpression(KSLTernaryConditionalExpressionNode* tce)
{
	if (tce->type.id != KSL_TYPE_INVALID) return true;

	if (!ValidateExpression(tce->condition)) return false;
	if (!ValidateExpression(tce->if_expression)) return false;
	if (!ValidateExpression(tce->else_expression)) return false;

	bool valid = true;
	if (tce->condition->type.id != KSL_TYPE_BOOL)
	{
		ErrorTernaryExpressionNotBoolConditional(tce);
		valid = false;
	}

	if (tce->if_expression->type != tce->else_expression->type)
	{
		ErrorTernaryExpressionBranchTypeMismatch(tce);
		valid = false;
	}
	
	tce->type = tce->if_expression->type;

	return valid;
}


bool KSLAnalyzer::ValidateConstructorExpression(KSLConstructorExpressionNode* ce)
{
	uint32_t component_count = ce->constructor_type.GetComponentCount();

	uint32_t arg_component_count = 0;
	for (size_t i = 0; i < ce->initializers.size(); i++)
	{
		bool s = ValidateExpression(ce->initializers[i]);
		if (!s) return false;

		KSLType &itype = ce->initializers[i]->type;
		
		if (!(itype.IsNumeric() || itype.IsBool()))
		{
			ErrorInvalidConstructorParameters(ce);
			return false;
		}

		arg_component_count += itype.GetComponentCount();
	}

	// general component amount check
	if ((arg_component_count != 1) && (arg_component_count != component_count))
	{
		ErrorInvalidConstructorParameters(ce);
		return false;
	}

	// additional checks for matrices
	if (ce->constructor_type.IsMatrix() && (arg_component_count != 1))
	{
		if ((ce->initializers.size() == 1) && (ce->initializers[0]->type.id != ce->constructor_type.id))
		{
			ErrorInvalidConstructorParameters(ce);
			return false;
		}

		if (ce->initializers.size() != 1)
		{
			uint32_t desired_initalizer_type = KSL_TYPE_INVALID;
			switch (ce->constructor_type.id)
			{
				case KSL_TYPE_MAT2: desired_initalizer_type = KSL_TYPE_VEC2; break;
				case KSL_TYPE_MAT3: desired_initalizer_type = KSL_TYPE_VEC3; break;
				case KSL_TYPE_MAT4: desired_initalizer_type = KSL_TYPE_VEC4; break;
				default: assert(0); break;
			}

			for (size_t i = 0; i < ce->initializers.size(); i++)
			{
				if (ce->initializers[i]->type.id != desired_initalizer_type)
				{
					ErrorInvalidConstructorParameters(ce);
					return false;
				}
			}
		}
	}

	ce->type = ce->constructor_type;
	ce->type.precision = KSL_PRECISION_NONE;

	// precision checking
	if (ce->initializers.size() > 1)
	{
		for (size_t i = 0; i < ce->initializers.size(); i++)
		{
			if (ce->initializers[i]->type.precision != ce->constructor_type.precision)
			{
				WarningConstructorParametersPrecisionMismatch(ce);
				return true;
			}
		}
	}
	
	ce->type.precision = ce->constructor_type.precision;
	return true;
}


bool KSLAnalyzer::ValidateExpression(KSLExpressionNode* e)
{
	if (e->type.id != KSL_TYPE_INVALID) return true;

	switch (e->node_type)
	{
	case KSL_NODE_VARIABLE_EXPRESSION: return ValidateVariableExpression(dynamic_cast<KSLVariableExpressionNode*>(e));
	case KSL_NODE_FUNCTION_CALL_EXPRESSION: return ValidateFunctionCallExpression(dynamic_cast<KSLFunctionCallExpressionNode*>(e));
	case KSL_NODE_BINOP_EXPRESSION: return ValidateBinaryExpression(dynamic_cast<KSLBinaryExpressionNode*>(e));
	case KSL_NODE_PARENTHESIS_EXPRESSION: return ValidateParenthesisExpression(dynamic_cast<KSLParenthesisExpressionNode*>(e));
	case KSL_NODE_MEMBER_ACCESS_EXPRESSION: return ValidateMemberAccessExpression(dynamic_cast<KSLSelectorExpressionNode*>(e));
	case KSL_NODE_ARRAY_ACCESS_EXPRESSION: return ValidateArrayAccessExpression(dynamic_cast<KSLSelectorExpressionNode*>(e));
	case KSL_NODE_TERNARY_CONDITIONAL_EXPRESSION: return ValidateTernaryConditionalExpression(dynamic_cast<KSLTernaryConditionalExpressionNode*>(e));
	case KSL_NODE_CONSTRUCTOR_EXPRESSION: return ValidateConstructorExpression(dynamic_cast<KSLConstructorExpressionNode*>(e));

	case KSL_NODE_SUFFIX_EXPRESSION:
	case KSL_NODE_UNARY_OP_EXPRESSION:
		return ValidateUnaryOperationExpression(dynamic_cast<KSLUnaryExpressionNode*>(e));

	default:
		assert(0);
		break;
	}

	return false;
}


bool KSLAnalyzer::CheckUniformsAmount()
{
	uint64_t uniforms_size = 0;

	for (size_t i = 0; i < ast->uniforms.size(); i++)
	{
		KSLVariableDefinitionsNode *vardef = ast->uniforms[i];
		KSLVariable &v = ast->variables[vardef->variables[0].variable_id];

		if (!(v.type.IsBool() || v.type.IsNumeric())) continue;

		if (v.type.IsArray())
		{
			uniforms_size += vardef->variables[0].array_size * v.type.GetSizeInBytes();
		}
		else
		{
			uniforms_size += v.type.GetSizeInBytes();
		}
	}

	if (uniforms_size > MAX_PER_STAGE_UNIFORM_IN_BYTES)
	{
		ErrorToManyUniforms(uniforms_size);
		return false;
	}

	return true;
}


class KSLAnalyzer::KSLUnusedVariableDetector : public KSLNodeVisitor
{
public:
	KSLUnusedVariableDetector(KSLAnalyzer* analyzer)
		: m_analyzer(analyzer)
	{
	}
private:

	virtual bool visit(KSLASTNode* n)
	{
		switch (n->node_type)
		{
		case KSL_NODE_VARIABLE_DEFINITIONS:
		{
			KSLVariableDefinitionsNode* vardef = dynamic_cast<KSLVariableDefinitionsNode*>(n);
			for (size_t i = 0; i < vardef->variables.size(); i++)
			{
				KSLVariable &v = m_analyzer->ast->variables[vardef->variables[i].variable_id];

				if (v.storage_type == KSL_STORAGE_DEFAULT) continue;
				if (v.storage_type == KSL_STORAGE_CONST) continue;

				if (!v.used)
				{
					m_analyzer->WarningUnusedVariable(vardef, v);
				}
			}

			break;
		}
		}

		return true;
	}

	KSLAnalyzer* m_analyzer;
};



void KSLAnalyzer::DetectUnusedVariables()
{
	KSLUnusedVariableDetector uvd(this);
	KSL::Traverse(ast->root, uvd);
}

