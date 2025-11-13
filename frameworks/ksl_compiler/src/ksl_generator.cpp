/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_generator.h"

#include <assert.h>
#include <sstream>
#include <iomanip>

KSLGenerator::KSLGenerator()
	: ast(NULL)
{
	Clear();
}


KSLGenerator::~KSLGenerator()
{
	Clear();
}


bool KSLGenerator::VisitVariableDefinitions(KSLVariableDefinitionsNode *vardef)
{
	SyncLine(vardef);

	m_result << StorageQualifierToString(vardef->storage_type) << TypeToString(vardef->variable_type) << " ";

	for (size_t i = 0; i < vardef->variables.size(); i++)
	{
		KSLVariableDefinitionsNode::InnerNode &dn = vardef->variables[i];
		KSLVariable &v = ast->variables[dn.variable_id];
		m_result << v.new_name;

		if (v.type.IsArray())
		{
			m_result << "[";
			if (dn.size_expression != NULL) VisitExpression(dn.size_expression);
			m_result << "]";
		}

		if (dn.init_expressions.size() != 0)
		{
			m_result << " = ";

			if (v.type.IsArray())
			{
				m_result << "{";

				for (size_t i = 0; i < dn.init_expressions.size(); i++)
				{
					VisitExpression(dn.init_expressions[i]);
					if (i + 1 < dn.init_expressions.size()) m_result << ", ";
				}

				m_result << "}";
			}
			else
			{
				assert(dn.init_expressions.size() == 1);
				VisitExpression(dn.init_expressions[0]);
			}
		}

		if (i+1 < vardef->variables.size()) m_result << ", ";
	}

	PrintVariableAttributes(vardef->bool_attribs, vardef->int_attribs);

	m_result << ";";

	return true;
}

	
bool KSLGenerator::PrintVariableAttributes(const std::set<uint32_t> &bool_attribs, const std::map<uint32_t, uint32_t> &int_attribs)
{
	size_t attrib_count = bool_attribs.size() + int_attribs.size();

	if (attrib_count > 0)
	{
		m_result << " { ";
		size_t i = 0;

		std::set<uint32_t>::const_iterator b_it = bool_attribs.begin();
		for (; b_it != bool_attribs.end(); b_it++)
		{
			m_result << TranslateAttribQualifer(*b_it);
			i++;
			if (i < attrib_count) m_result << ", ";
		}

		std::map<uint32_t, uint32_t>::const_iterator i_it = int_attribs.begin();
		for (; i_it != int_attribs.end(); i_it++)
		{
			m_result << TranslateAttribQualifer(i_it->first) << "(" << i_it->second << ")";
			i++;
			if (i < attrib_count) m_result << ", ";
		}

		m_result << " } ";
	}

	return true;
}


bool KSLGenerator::PrintFloatLiteral(KSLLiteralExpressionNode* le, bool force_highp)
{
	bool is_highp = force_highp || (le->type.precision == KSL_PRECISION_HIGH) || ast->force_highp;

	if (m_print_literals_from_string && (le->str_value.size() > 0))
	{
		// print from string
		if (is_highp && (le->str_value.back() == 'h'))
		{
			m_result << le->str_value.substr(0, le->str_value.length() - 1);
		}
		else
		{
			m_result << le->str_value;
		}
	}
	else
	{
		// printf from value
		m_result << le->float_value;
		if (!is_highp)
		{
			m_result << "h";
		}
	}

	return true;
}


bool KSLGenerator::VisitLiteralExpression(KSLLiteralExpressionNode *le)
{
	CHECK_FORCE_HIGHP(le->type);

	switch (le->type.id)
	{
	case KSL_TYPE_FLOAT:
		PrintFloatLiteral(le, false);
		break;

	case KSL_TYPE_INT:
		if (m_print_literals_from_string && (le->str_value.size() > 0))
			m_result << le->str_value;
		else
			m_result << le->int_value;
		break;

	case KSL_TYPE_UINT:
		if (m_print_literals_from_string && (le->str_value.size() > 0))
			m_result << le->str_value;
		else
			m_result << le->int_value << "u";
	
		break;
	case KSL_TYPE_BOOL:
		m_result << ((le->bool_value) ? "true" : "false");
		break;
	default:
		assert(0);
		return false;
	}
	
	return true;
}


bool KSLGenerator::VisitVariableExpression(KSLVariableExpressionNode *ve)
{
	SyncLine(ve);
	m_result << ast->variables[ ve->variable_id]. new_name;
	return true;
}


bool KSLGenerator::VisitParenthesisExpression(KSLParenthesisExpressionNode *pe)
{
	m_result << "(";
	VisitExpression(pe->expression);
	m_result << ")";
	return true;
}


bool KSLGenerator::VisitConstructorExpression(KSLConstructorExpressionNode *ce)
{
	SyncLine(ce);

	m_result << TypeToString(ce->constructor_type) << "(";

	for (size_t i = 0; i < ce->initializers.size(); i++)
	{
		VisitExpression(ce->initializers[i]);
		if (i + 1 < ce->initializers.size()) m_result << ",";
	}

	m_result << ")";
	return true;
}


bool KSLGenerator::VisitFunctionCallExpression(KSLFunctionCallExpressionNode *fce)
{
	SyncLine(fce);

	m_result << fce->name << "(";

	for (size_t i = 0; i < fce->arguments.size(); i++)
	{
		VisitExpression(fce->arguments[i]);
		if (i + 1 < fce->arguments.size()) m_result << ",";
	}

	m_result << ")";
	return true;
}


bool KSLGenerator::VisitSelectorExpression(KSLSelectorExpressionNode *se)
{
	SyncLine(se);

	PrintfExpLeftParent();
	VisitExpression(se->expression);
	PrintfExpRightParent();

	if (se->node_type == KSL_NODE_ARRAY_ACCESS_EXPRESSION)
	{
		m_result << "[";
		VisitExpression(se->id_expression);
		m_result << "]";
	}
	else
	{
		m_result << "." << se->selector;
	}
	return true;
}


bool KSLGenerator::VisitBinaryExpression(KSLBinaryExpressionNode *be)
{
	SyncLine(be);

	PrintfExpLeftParent();
	VisitExpression(be->e1);
	PrintfExpRightParent();

	m_result << KSL::BinopToString(be->operation);

	PrintfExpLeftParent();
	VisitExpression(be->e2);
	PrintfExpRightParent();

	return true;
}


bool KSLGenerator::VisitUnaryExpression(KSLUnaryExpressionNode *ue)
{
	SyncLine(ue);

	switch (ue->operation)
	{
	case KSL_UNOP_MINUS:       m_result << " -";  break;
	case KSL_UNOP_PLUS:        m_result << " +";  break;
	case KSL_UNOP_INCREMENT:   m_result << " ++"; break;
	case KSL_UNOP_DECREMENT:   m_result << " --"; break;
	case KSL_UNOP_LOGICAL_NOT: m_result << " !";  break;
	default:
		assert(0);
		return false;
		break;
	}

	PrintfExpLeftParent();
	VisitExpression(ue->expression);
	PrintfExpRightParent();

	return true;
}


bool KSLGenerator::VisitSuffixExpression(KSLUnaryExpressionNode *se)
{
	SyncLine(se);

	PrintfExpLeftParent();
	VisitExpression(se->expression);
	PrintfExpRightParent();

	switch (se->operation)
	{
	case KSL_UNOP_INCREMENT: m_result << "++ "; break;
	case KSL_UNOP_DECREMENT: m_result << "-- "; break;
	default:
		assert(0);
		return false;
		break;
	}

	return true;
}


bool KSLGenerator::VisitTernaryConditionalExpression(KSLTernaryConditionalExpressionNode *tce)
{
	SyncLine(tce);

	PrintfExpLeftParent();
	VisitExpression(tce->condition);
	PrintfExpRightParent();

	m_result << " ? ";

	PrintfExpLeftParent();
	VisitExpression(tce->if_expression);
	PrintfExpRightParent();

	m_result << " : ";

	PrintfExpLeftParent();
	VisitExpression(tce->else_expression);
	PrintfExpRightParent();

	return true;
}


bool KSLGenerator::VisitExpression(KSLExpressionNode *e)
{
	SyncLine(e);
	switch (e->node_type)
	{
	case KSL_NODE_LITERAL_EXPRESSION:
		return VisitLiteralExpression(dynamic_cast<KSLLiteralExpressionNode*>(e));
	case KSL_NODE_UNARY_OP_EXPRESSION:
		return VisitUnaryExpression(dynamic_cast<KSLUnaryExpressionNode*>(e));
	case KSL_NODE_BINOP_EXPRESSION:
		return VisitBinaryExpression(dynamic_cast<KSLBinaryExpressionNode*>(e));
	case KSL_NODE_PARENTHESIS_EXPRESSION:
		return VisitParenthesisExpression(dynamic_cast<KSLParenthesisExpressionNode*>(e));
	case KSL_NODE_VARIABLE_EXPRESSION:
		return VisitVariableExpression(dynamic_cast<KSLVariableExpressionNode*>(e));
	case KSL_NODE_CONSTRUCTOR_EXPRESSION:
		return VisitConstructorExpression(dynamic_cast<KSLConstructorExpressionNode*>(e));
	case KSL_NODE_MEMBER_ACCESS_EXPRESSION:
	case KSL_NODE_ARRAY_ACCESS_EXPRESSION:
		return VisitSelectorExpression(dynamic_cast<KSLSelectorExpressionNode*>(e));
	case KSL_NODE_FUNCTION_CALL_EXPRESSION:
		return VisitFunctionCallExpression(dynamic_cast<KSLFunctionCallExpressionNode*>(e));
	case KSL_NODE_SUFFIX_EXPRESSION:
		return VisitSuffixExpression(dynamic_cast<KSLUnaryExpressionNode*>(e));
	case KSL_NODE_TERNARY_CONDITIONAL_EXPRESSION:
		return VisitTernaryConditionalExpression(dynamic_cast<KSLTernaryConditionalExpressionNode*>(e));


	default:
		std::stringstream sstream;
		sstream << "code generation fail; unexpected node; compiler error;" << std::endl;
		m_errors.push_back(KSLError(KSL_GENERATOR, KSL_ERROR, 0, 0, sstream.str()));
		assert(0);
		break;
	}
	return false;
}


bool KSLGenerator::VisitExpressionStatement(KSLExpressionOrEmptyStatementNode* es)
{
	SyncLine(es);
	if (es->node_type == KSL_NODE_EXPRESSION_STATEMENT)
	{
		if (!VisitExpression(es->expression)) return false;
	}
	m_result << ";";
	return true;
}


bool KSLGenerator::VisitIfStatement(KSLIfStatementNode *is)
{
	SyncLine(is);

	m_result << "if (";
	VisitExpression(is->condition);
	m_result << ")";

	if (is->if_statement->node_type != KSL_NODE_BLOCK_STATEMENT) m_indent++;
	
	NewLine();
	VisitStatementOrVariableDefinitions(is->if_statement);

	if (is->if_statement->node_type != KSL_NODE_BLOCK_STATEMENT)
	{
		m_indent--;
		NewLine();
	}

	if (is->else_statement != NULL)
	{
		m_result << "else";

		// just prettyprint
		if (is->else_statement->node_type == KSL_NODE_IF_STATEMENT)
		{
			m_result << " ";
		}
		else if (is->else_statement->node_type == KSL_NODE_BLOCK_STATEMENT)
		{
			NewLine();
		}
		else
		{
			m_indent++;
			NewLine();
		}

		VisitStatementOrVariableDefinitions(is->else_statement);

		// just prettyprint
		if ((is->else_statement->node_type == KSL_NODE_IF_STATEMENT) || (is->else_statement->node_type == KSL_NODE_BLOCK_STATEMENT))
		{
			NewLine();
		}
	}

	return true;
}


bool KSLGenerator::VisitForStatement(KSLForStatementNode *fs)
{
	SyncLine(fs);

	m_result << "for (";
	
	if ( (fs->init_node != NULL) && (fs->init_node->node_type == KSL_NODE_VARIABLE_DEFINITIONS) )
	{
		VisitVariableDefinitions(dynamic_cast<KSLVariableDefinitionsNode*>(fs->init_node));
	}
	else if ((fs->init_node != NULL) && (fs->init_node->node_type == KSL_NODE_EXPRESSION_STATEMENT))
	{
		VisitExpressionStatement(dynamic_cast<KSLExpressionOrEmptyStatementNode*>(fs->init_node));
	}
	else
	{
		m_result << ";";
	}

	m_result << " ";
	if (fs->conditional_statement) VisitExpression(fs->conditional_statement);
	m_result << "; ";
	if (fs->step_statement) VisitExpression(fs->step_statement);
	m_result << ")";
	NewLine();

	VisitStatementOrVariableDefinitions(fs->body);

	return true;
}


bool KSLGenerator::VisitControlStatement(KSLControlStatementNode* cs)
{
	SyncLine(cs);
	switch (cs->node_type)
	{
	case KSL_NODE_BREAK_STATEMENT:    m_result << "break"; break;
	case KSL_NODE_CONTINUE_STATEMENT: m_result << "continue"; break;
	case KSL_NODE_DISCARD_STATEMENT:  m_result << "discard"; break;
	default:
		assert(0);
		break;
	}
	m_result << ";";
	
	return true;
}


bool KSLGenerator::VisitReturnStatement(KSLReturnStatementNode* rs)
{
	SyncLine(rs);
	m_result << "return ";
	if (rs->expression != NULL)
	{
		if (!VisitExpression(rs->expression)) return false;
	}
	m_result << ";";
	return true;
}


bool KSLGenerator::VisitStatementOrVariableDefinitions(KSLASTNode* n)
{
	switch (n->node_type)
	{
	case KSL_NODE_VARIABLE_DEFINITIONS:
		return VisitVariableDefinitions(dynamic_cast<KSLVariableDefinitionsNode*>(n));

	case KSL_NODE_BLOCK_STATEMENT:
		return VisitBlockStatement(dynamic_cast<KSLBlockStatementNode*>(n));

	case KSL_NODE_EMPTY_STATEMENT:
	case KSL_NODE_EXPRESSION_STATEMENT:
		return VisitExpressionStatement(dynamic_cast<KSLExpressionOrEmptyStatementNode*>(n));

	case KSL_NODE_IF_STATEMENT:
		return VisitIfStatement(dynamic_cast<KSLIfStatementNode*>(n));

	case KSL_NODE_FOR_STATEMENT:
		return VisitForStatement(dynamic_cast<KSLForStatementNode*>(n));

	case KSL_NODE_BREAK_STATEMENT:
	case KSL_NODE_CONTINUE_STATEMENT:
	case KSL_NODE_DISCARD_STATEMENT:
		return VisitControlStatement(dynamic_cast<KSLControlStatementNode*>(n));

	case KSL_NODE_RETURN_STATEMENT:
		return VisitReturnStatement(dynamic_cast<KSLReturnStatementNode*>(n));

	default:
		std::stringstream sstream;
		sstream << "code generation fail; unexpected node; compiler error;" << std::endl;
		m_errors.push_back(KSLError(KSL_GENERATOR, KSL_ERROR, 0, 0, sstream.str()));
		assert(0);
		return false;
	}

	return true;
}


bool KSLGenerator::VisitBlockStatement(KSLBlockStatementNode *bs)
{
	SyncLine(bs);

	m_result << "{";
	m_indent++;

	for (size_t i = 0; i < bs->nodes.size(); i++)
	{
		NewLine();

		KSLASTNode *n = bs->nodes[i];

		if (!VisitStatementOrVariableDefinitions(n)) return false;
	}

	m_indent--;
	NewLine();
	m_result << "}";
	NewLine();

	return true;
}


bool KSLGenerator::VisitFunction(KSLFunctionNode* fn)
{
	SyncLine(fn);

	KSLFunction &function = ast->functions[fn->function_id];
	
	m_result << TypeToString(function.return_type) << " " << function.function_name << "(";

	for (size_t i = 0; i < fn->attribs.size(); i++)
	{
		KSLFunctionNode::AttribNode &an = fn->attribs[i];

		KSLVariable &v = ast->variables[an.variable_id];

		switch (function.attrib_access[i])
		{
			case KSL_ATTRIB_ACCESS_IN: break;
			case KSL_ATTRIB_ACCESS_OUT: m_result << "out "; break;
			case KSL_ATTRIB_ACCESS_INOUT: m_result << "inout "; break;
		
			default: assert(0); break;
		}

		if (!v.type.IsArray())
		{
			m_result << TypeToString(v.type) << " ";
			m_result << v.new_name;
		}
		else
		{
			m_result << TypeToString(v.type.GetBaseType()) << " ";
			m_result << v.new_name;

			m_result << "[";
			VisitExpression(an.size_expression);
			m_result << "]";
		}

		if (i + 1 < fn->attribs.size()) m_result << ", ";
	}

	m_result << ")";
	NewLine();

	VisitBlockStatement(fn->body);

	return true;
}


bool KSLGenerator::VisitImageDefinition(KSLImageDefinitionNode* idn)
{
	SyncLine(idn);

	KSLVariable &v = ast->variables[idn->variable_id];

	CHECK_FORCE_HIGHP(v.type);

	m_result << TypeToString(v.type) << " " << v.new_name;

	PrintVariableAttributes(idn->bool_attribs, idn->int_attribs);
	m_result << ";";

	return true;
}


bool KSLGenerator::VisitNumThreads(KSLNumThreadNode* ntn)
{
	SyncLine(ntn);
	m_result << "numthreads(" << ntn->x << ", " << ntn->y << ", " << ntn->z << ");";
	NewLine();
	return true;
}


bool KSLGenerator::VisitStructDefinition(KSLStructDefinitionNode* sdn)
{
	SyncLine(sdn);

	m_result << "struct " << sdn->name; NewLine();
	m_result << "{";
	m_indent++;

	for (size_t i = 0; i < sdn->members.size(); i++)
	{
		NewLine();
		VisitVariableDefinitions(sdn->members[i]);
	}

	m_indent--;
	NewLine();
	m_result << "};"; NewLine();

	return true;
}


bool KSLGenerator::VisitGlobalSpaceNode(KSLASTNode* n)
{
	switch (n->node_type)
	{
	case KSL_NODE_VARIABLE_DEFINITIONS:
		if (!VisitVariableDefinitions(dynamic_cast<KSLVariableDefinitionsNode*>(n)))
			return false;
		break;

	case KSL_NODE_FUNCTION:
		if (!VisitFunction(dynamic_cast<KSLFunctionNode*>(n)))
			return false;
		break;

	case KSL_NODE_IMAGE_DEFINITION:
		if (!VisitImageDefinition(dynamic_cast<KSLImageDefinitionNode*>(n)))
			return false;
		break;

	case KSL_NODE_NUMTHREAD:
		if (!VisitNumThreads(dynamic_cast<KSLNumThreadNode*>(n)))
			return false;
		break;

	case KSL_NODE_STRUCT_DEFINITION:
		if (!VisitStructDefinition(dynamic_cast<KSLStructDefinitionNode*>(n)))
			return false;
		break;

	default:
		std::stringstream sstream;
		sstream << "code generation fail; unexpected token; compiler error;" << std::endl;
		m_errors.push_back(KSLError(KSL_GENERATOR, KSL_ERROR, 0, 0, sstream.str()));
		assert(0);
		return false;
	}
	return true;
}


bool KSLGenerator::VisitProgram(KSLProgramNode *program)
{
	for (size_t i = 0; i < program->nodes.size(); i++)
	{
		KSLASTNode *n = program->nodes[i];
		if (!VisitGlobalSpaceNode(n)) return false;
		NewLine();
	}
	return true;
}


bool KSLGenerator::Generate()
{
	return VisitProgram(ast->root);
}


std::string KSLGenerator::GetResult() const
{
	return m_result.str();
}


std::string KSLGenerator::TypeToString(KSLType type)
{
	CHECK_FORCE_HIGHP(type);

	assert(!type.IsArray());
	return ast->GetTypeName(type);
}


std::string KSLGenerator::StorageQualifierToString(KSLStorageQualifier qualifier)
{
	return KSL::StorageQualifierToString(qualifier);
}


std::string KSLGenerator::TranslateAttribQualifer(uint32_t qualifier)
{
	return ast->GetQualifierStr((KSLAttribQualifier)qualifier);
}


void KSLGenerator::SyncLine(KSLASTNode* node)
{
	if (!m_do_syncline) return;

	while (m_line + 1 < node->start_line)
	{
		NewLine();
	}
}


void KSLGenerator::NewLine()
{
	m_line++;
	m_result << std::endl;

	for (int i = 0; i < m_indent; i++)
		m_result << "   ";
}


void KSLGenerator::PrintfExpLeftParent()
{
	if (m_print_expression_parenthesis) m_result << "(";
}


void KSLGenerator::PrintfExpRightParent()
{
	if (m_print_expression_parenthesis) m_result << ")";
}


void KSLGenerator::Clear()
{
	ast = NULL;
	m_line = 0;
	m_indent = 0;
	m_result.clear();
	m_print_expression_parenthesis = false;
	m_print_literals_from_string = true; // false only for debug reason;
	m_do_syncline = true;
	m_result << std::fixed<< std::setprecision(20);
}

