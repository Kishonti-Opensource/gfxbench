/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_analyzer.h"

#include <assert.h>
#include <sstream>


#define MATCH_OR_FAIL(token) if (!Match(token)) return NULL;


template <typename T>
class TableGuard
{
public:
	TableGuard(std::vector<T> &table)
		: m_table(table)
		, m_accepted(false)
	{
		m_size = table.size();
	}
	~TableGuard()
	{
		if (!m_accepted) m_table.resize(m_size);
	}
	void Accept()
	{
		m_accepted = true;
	}

private:
	std::vector<T> &m_table;
	bool m_accepted;
	size_t m_size;
};


//
//	Guards the analysis state during rule matching
//
class KSLAnalyzer::StateGuard
{
public:
	StateGuard(KSLAnalyzer *a, bool accepted)
		: m_node(NULL)
		, m_accepted(accepted)
		, m_variable_table_guard(a->ast->variables)
		, m_function_table_guard(a->ast->functions)
		, m_expressions_table_guard(a->ast->expressions)
	{
		Init(a);
	}

	StateGuard(KSLAnalyzer *a, KSLASTNode* node)
		: m_node(node)
		, m_accepted(false)
		, m_variable_table_guard(a->ast->variables)
		, m_function_table_guard(a->ast->functions)
		, m_expressions_table_guard(a->ast->expressions)
	{
		Init(a);
	}

	~StateGuard()
	{
		if (m_accepted)
		{
			m_node = NULL;
			m_variable_table_guard.Accept();
			m_function_table_guard.Accept();
			m_expressions_table_guard.Accept();
			m_token_guard.Accept();
		}

		delete m_node;
	}

	void Accept() { m_accepted = true; }
	void Fail() { m_accepted = false; }

private:
	void Init(KSLAnalyzer *a)
	{
		m_token_guard = a->GetTokenGuard();
	}

	KSLTokenIterator::KSLTokenGuard m_token_guard;

	KSLASTNode *m_node;
	bool m_accepted;
	
	TableGuard<KSLVariable> m_variable_table_guard;
	TableGuard<KSLFunction> m_function_table_guard;
	TableGuard<KSLExpressionNode*> m_expressions_table_guard;
};


KSLAnalyzer::KSLAnalyzer()
	: ast(NULL)
{
	Clear();
	RegisterBuiltInTypes();
	RegisterBuiltInQualifiers();
}


KSLAnalyzer::~KSLAnalyzer()
{
	Clear();
}


KSLVariableDefinitionsNode* KSLAnalyzer::MatchVariableDefinitions(bool global_space)
{
	TableGuard<uint32_t> ATG(m_active_variable_ids);
	KSLVariableDefinitionsNode* vardef = new KSLVariableDefinitionsNode();
	
	StateGuard SG(this, vardef);


	//
	//	Setup storage qualifier
	//
	bool is_compute = m_shader_type == NGL_COMPUTE_SHADER;
	bool check_for_qualifiers = false;

	if (global_space && Match(KSL_TOKEN_UNIFORM))
	{
		vardef->storage_type = KSL_STORAGE_UNIFORM;
		check_for_qualifiers = true;
	}
	else if (global_space && !is_compute && Match(KSL_TOKEN_IN))
	{
		vardef->storage_type = KSL_STORAGE_IN;
		check_for_qualifiers = true;
	}
	else if (global_space && !is_compute && Match(KSL_TOKEN_OUT))
	{
		vardef->storage_type = KSL_STORAGE_OUT;
		check_for_qualifiers = true;
	}
	else if (global_space && Match(KSL_TOKEN_BUFFER))
	{
		vardef->storage_type = KSL_STORAGE_BUFFER;
		check_for_qualifiers = true;
	}
	else if (global_space && is_compute && Match(KSL_TOKEN_SHARED))
	{
		vardef->storage_type = KSL_STORAGE_SHARED;
	}
	else if (Match(KSL_TOKEN_CONST))
	{
		vardef->storage_type = KSL_STORAGE_CONST;
	}


	KSLType type;
	KSLToken type_token;
	if (!MatchType(type, type_token)) return NULL;
	vardef->variable_type = type;
	
	//
	//	Match definition instances
	//
	MATCH_OR_FAIL(KSL_TOKEN_IDENTIFIER);

	while (true)
	{
		KSLToken &varname_token = T();
		KSLVariable v;

		vardef->variables.push_back(KSLVariableDefinitionsNode::InnerNode());
		KSLVariableDefinitionsNode::InnerNode &defnode = vardef->variables.back();

		v.orig_name = T().str_value;
		v.type = vardef->variable_type;
		v.storage_type = vardef->storage_type;

		// dot not mangling interface variables
		bool mangling_variable_name = m_mangling_variable_names;
		mangling_variable_name &= !((vardef->storage_type == KSL_STORAGE_IN) || (vardef->storage_type == KSL_STORAGE_OUT)
									|| (vardef->storage_type == KSL_STORAGE_UNIFORM) || (vardef->storage_type == KSL_STORAGE_BUFFER) );
		v.new_name = (mangling_variable_name)?(v.orig_name + m_variable_postfix.back()):v.orig_name;
		if (vardef->storage_type == KSL_STORAGE_CONST)
		{
			v.access = KSL_ACCESS_READ_ONLY;
		}

		// Check variable redeclaration
		CheckVariableRedefiniton(v.orig_name);

		KSLExpressionNode *se = NULL;
		if (Match(KSL_TOKEN_LEFT_BRACKET))
		{
			v.type = KSLType(KSL_TYPE_ARRAY, KSL_PRECISION_NONE);
			v.type.SetBaseType(vardef->variable_type);

			if (!Match(KSL_TOKEN_RIGHT_BRACKET))
			{
				se = MatchExpression(KSL_PRECEDENCE_LOWEST);
				MATCH_OR_FAIL(KSL_TOKEN_RIGHT_BRACKET);
			}
		}
		defnode.size_expression = se;

		m_active_variable_ids.push_back((uint32_t)ast->variables.size());
		defnode.variable_id = (uint32_t)ast->variables.size();
		ast->variables.push_back(v);

		// check for initalization expression
		if (Match(KSL_TOKEN_EQUAL))
		{
			if (!v.type.IsArray())
			{
				KSLExpressionNode *en = MatchExpression(KSL_PRECEDENCE_LOWEST);
				if (en == NULL)
				{
					return NULL;
				}
				defnode.init_expressions.push_back(en);
			}
			else
			{
				if (!MatchArrayInitializer(defnode.init_expressions)) return NULL;
			}
		}
		else
		{
			if (v.access == KSL_ACCESS_READ_ONLY)
			{
				ErrorConstMustBeInitialized(T());
			}

			if (v.type.IsArray() && (defnode.size_expression == NULL) && (v.storage_type != KSL_STORAGE_BUFFER))
			{
				ErrorUnsizedArrayMustBeInitialized(varname_token);
			}
		}

		// jump to next variable definition
		if (Match(KSL_TOKEN_COMMA))
		{
			MATCH_OR_FAIL(KSL_TOKEN_IDENTIFIER);
			continue;
		}
		
		break;
	}

	// parse attribute qualifiers
	if (check_for_qualifiers)
	{
		if (!MatchVariableAttributes(vardef->bool_attribs, vardef->int_attribs)) return NULL;
	}

	MATCH_OR_FAIL(KSL_TOKEN_SEMICOLON);

	//	the parsing valid from here parse valid
	ATG.Accept();
	SG.Accept();

	vardef->start_line = type_token.line;

	bool is_default = vardef->storage_type == KSL_STORAGE_DEFAULT;
	if (global_space && is_default)
	{
		ErrorGlobalVariablesNotAllowed(type_token);
	}

	if (global_space && !is_default && (vardef->variables.size() > 1))
	{
		ErrorOnlyOneVariableDeclarationAllowd(type_token);
	}

	return vardef;
}


bool KSLAnalyzer::MatchType(KSLType &type, KSLToken &type_token)
{
	if (!Match(KSL_TOKEN_IDENTIFIER)) return false;

	type_token = T();
	if (!GetType(type_token.str_value, type))
	{
		ErrorUnknownType(type_token);
		return false;
	}

	bool has_base_type = false;
	switch (type.id)
	{
		// sampler
		case KSL_TYPE_SAMPLER_2D:
		case KSL_TYPE_SAMPLER_2D_ARRAY:
		case KSL_TYPE_SAMPLER_CUBE:
		case KSL_TYPE_SAMPLER_CUBE_ARRAY:

		// shadow sampler
		case KSL_TYPE_SAMPLER_2D_SHADOW:
		case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW:
		case KSL_TYPE_SAMPLER_CUBE_SHADOW:

		case KSL_TYPE_SUBPASS_INPUT:
			has_base_type = true;
			break;

		default: has_base_type = false; break;
	}

	if (has_base_type)
	{
		if (!Match(KSL_TOKEN_LESS)) return false;
		if (!Match(KSL_TOKEN_IDENTIFIER)) return false;

		KSLToken &btt = T();

		if (!Match(KSL_TOKEN_GREATER)) return false;

		KSLType base_type;
		if (!GetType(btt.str_value, base_type))
		{
			ErrorUnknownType(btt);
			return false;
		}

		if (base_type.id != KSL_TYPE_FLOAT) return false;

		type.precision = base_type.precision;
	}

	return true;
}


bool KSLAnalyzer::MatchVariableAttributes(std::set<uint32_t> &bool_attribs, std::map<uint32_t, uint32_t> &int_attribs)
{
	if (Match(KSL_TOKEN_LEFT_BRACE))
	{
		while (true)
		{
			KSLAttribQualifier aq;
			if (!GetAttribQualifier(aq))
			{
				return false;
			}

			// KSL_TODO: validate attribute name

			if (Match(KSL_TOKEN_LEFT_PARENT))
			{
				if (!Match(KSL_TOKEN_SIGNED_INTEGER)) return false;
				int_attribs[aq] = (uint32_t)T().int_value;
				if (!Match(KSL_TOKEN_RIGHT_PARENT)) return false;
			}
			else
			{
				bool_attribs.insert(aq);
			}

			if (Match(KSL_TOKEN_COMMA)) continue;

			break;
		}

		if (!Match(KSL_TOKEN_RIGHT_BRACE))
		{
			ErrorUnexpectedToken();
			return false;
		}
	}

	return true;
}


bool KSLAnalyzer::MatchArrayInitializer(std::vector<KSLExpressionNode*> &init_expressions)
{
	if (!Match(KSL_TOKEN_LEFT_BRACE)) return false;

	while (true)
	{
		KSLExpressionNode* e = MatchExpression(KSL_PRECEDENCE_LOWEST);
		if (e == NULL) return false;

		init_expressions.push_back(e);

		if (!Match(KSL_TOKEN_COMMA)) break;
	}

	if (!Match(KSL_TOKEN_RIGHT_BRACE)) return false;
	return true;
}


KSLLiteralExpressionNode* KSLAnalyzer::MatchLiteralExpression()
{
	KSLLiteralExpressionNode *le = new KSLLiteralExpressionNode();
	StateGuard SG(this, le);

	if (Match(KSL_TOKEN_FLOAT))
	{
		le->type = KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_HIGH);
		le->float_value = T().float_value;
		le->str_value = T().str_value;
	}
	else if (Match(KSL_TOKEN_HALF))
	{
		le->type = KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_MEDIUM);
		le->float_value = T().float_value;
		le->str_value = T().str_value;
	}
	else if (Match(KSL_TOKEN_SIGNED_INTEGER))
	{
		le->type = KSLType(KSL_TYPE_INT, KSL_PRECISION_HIGH);
		le->int_value = T().int_value;
		le->str_value = T().str_value;
	}
	else if (Match(KSL_TOKEN_UNSIGNED_INTEGER))
	{
		le->type = KSLType(KSL_TYPE_UINT, KSL_PRECISION_HIGH);
		le->int_value = T().int_value;
		le->str_value = T().str_value;
	}
	else if (Match(KSL_TOKEN_TRUE))
	{
		le->type = KSLType(KSL_TYPE_BOOL, KSL_PRECISION_NONE);
		le->bool_value = true;
	}
	else if (Match(KSL_TOKEN_FALSE))
	{
		le->type = KSLType(KSL_TYPE_BOOL, KSL_PRECISION_NONE);
		le->bool_value = false;
	}
	else
	{
		return NULL;
	}

	SG.Accept();

	le->constant_value = true;
	le->start_line = T().line;

	return le;
}


KSLVariableExpressionNode* KSLAnalyzer::MatchVariableExpression()
{
	KSLVariableExpressionNode *ve = new KSLVariableExpressionNode();
	StateGuard SG(this, ve);

	MATCH_OR_FAIL(KSL_TOKEN_IDENTIFIER);

	std::vector<uint32_t>::reverse_iterator rit = m_active_variable_ids.rbegin();
	for (; rit != m_active_variable_ids.rend(); rit++)
	{
		if (*rit == KSL_UINT32_MAX) continue; // skip the block flag

		if ( ast->variables[*rit].orig_name == T().str_value)
		{
			break;
		}
	}

	if (rit != m_active_variable_ids.rend())
	{
		KSLToken t = T();
		ve->variable_id = *rit;
		ast->variables[ve->variable_id].used = true;
	}
	else
	{
		ErrorUndeclaredIdentifier(T());
	}

	SG.Accept();

	ve->start_line = T().line;

	return ve;
}


KSLParenthesisExpressionNode* KSLAnalyzer::MatchParenthesisExpression()
{
	KSLParenthesisExpressionNode *pe = new KSLParenthesisExpressionNode;
	StateGuard SG(this, pe);

	MATCH_OR_FAIL(KSL_TOKEN_LEFT_PARENT);

	KSLExpressionNode *e = MatchExpression(KSL_PRECEDENCE_LOWEST);
	if (e == NULL)
	{
		return NULL;
	}
	pe->expression = e;

	MATCH_OR_FAIL(KSL_TOKEN_RIGHT_PARENT);

	SG.Accept();

	return pe;
}


KSLUnaryExpressionNode* KSLAnalyzer::MatchUnaryExpression()
{
	KSLUnaryExpressionNode *ue = new KSLUnaryExpressionNode;
	StateGuard SG(this, ue);

	if      (Match(KSL_TOKEN_MINUS)) ue->operation = KSL_UNOP_MINUS;
	else if (Match(KSL_TOKEN_PLUS))  ue->operation = KSL_UNOP_PLUS;
	else if (Match(KSL_TOKEN_PLUSPLUS)) ue->operation = KSL_UNOP_INCREMENT;
	else if (Match(KSL_TOKEN_MINUSMINUS)) ue->operation = KSL_UNOP_DECREMENT;
	else if (Match(KSL_TOKEN_EXCLAMATION_MARK)) ue->operation = KSL_UNOP_LOGICAL_NOT;
	else    return NULL;

	ue->precedence = KSL_PRECEDENCE_UNARY;

	KSLExpressionNode *e = MatchExpression(KSL_PRECEDENCE_UNARY);
	if (e == NULL)
	{
		return NULL;
	}
	ue->expression = e;
	
	SG.Accept();

	ue->start_line = T().line;

	return ue;
}


KSLExpressionNode* KSLAnalyzer::MatchBinaryExpression(const KSLOperationPrecedence min_precedence)
{
	StateGuard SG(this, false);

	// match first expression
	KSLExpressionNode *e1 = MatchExpression(KSL_PRECEDENCE_UNARY);
	if (e1 == NULL)
	{
		return NULL;
	}

	KSLOperationPrecedence max_precedence = e1->precedence;

	while (true)
	{
		KSLBinaryExpressionNode *be = new KSLBinaryExpressionNode();
		be->e1 = e1;

		assert(e1->precedence != KSL_PRECEDENCE_INVALID);

		int i = min_precedence;
		for (; i <= max_precedence; i++)
		{
			KSLOperationPrecedence precedence = (KSLOperationPrecedence)i;
			be->precedence = precedence;

			if (precedence == KSL_PRECEDENCE_BINARY_ADD)
			{
				if (Match(KSL_TOKEN_MINUS)) { be->operation = KSL_BINOP_SUB; break; }
				else if (Match(KSL_TOKEN_PLUS)) { be->operation = KSL_BINOP_ADD;  break; }
			}
			else if (precedence == KSL_PRECEDENCE_BINARY_SHIFT)
			{
				if (Match(KSL_TOKEN_LESSLESS)) { be->operation = KSL_BINOP_SHIFT_LEFT; break; }
				else if (Match(KSL_TOKEN_GREATGREAT)) { be->operation = KSL_BINOP_SHIFT_RIGHT; break; }
			}
			else if (precedence == KSL_PRECEDENCE_BINARY_EQUAL)
			{
				if (Match(KSL_TOKEN_EQUALEQUAL)) { be->operation = KSL_BINOP_EQUAL; break; }
				else if (Match(KSL_TOKEN_NOTEQUAL)) { be->operation = KSL_BINOP_NOTEQUAL; break; }
			}
			else if (precedence == KSL_PRECEDENCE_BINARY_AND)
			{
				if (Match(KSL_TOKEN_LAZYAND)) { be->operation = KSL_BINOP_LAZY_AND; break; }
			}
			else if (precedence == KSL_PRECEDENCE_BINARY_OR)
			{
				if (Match(KSL_TOKEN_LAZYOR)) { be->operation = KSL_BINOP_LAZY_OR; break; }
			}
			else if (precedence == KSL_PRECEDENCE_BINARY_BITWISE_AND)
			{
				if (Match(KSL_TOKEN_BITWISE_AND)) { be->operation = KSL_BINOP_BITWISE_AND; break; }
			}
			else if (precedence == KSL_PRECEDENCE_BINARY_BITWISE_OR)
			{
				if (Match(KSL_TOKEN_BITWISE_OR)) { be->operation = KSL_BINOP_BITWISE_OR; break; }
			}
			else if (precedence == KSL_PRECEDENCE_BINARY_MUL)
			{
				if (Match(KSL_TOKEN_MUL)) { be->operation = KSL_BINOP_MUL; break; }
				else if (Match(KSL_TOKEN_DIV)) { be->operation = KSL_BINOP_DIV; break; }
				else if (Match(KSL_TOKEN_MOD)) { be->operation = KSL_BINOP_MOD; break; }
			}
			else if (precedence == KSL_PRECEDENCE_BINARY_COMP)
			{
				if (Match(KSL_TOKEN_LESS)) { be->operation = KSL_BINOP_LESS; break; }
				else if (Match(KSL_TOKEN_GREATER)) { be->operation = KSL_BINOP_GREATER; break; }
				else if (Match(KSL_TOKEN_LESSEQUAL)) { be->operation = KSL_BINOP_LESSEQUAL; break; }
				else if (Match(KSL_TOKEN_GREATEREQUAL)) { be->operation = KSL_BINOP_GREATEREQUAL; break; }
			}
		}
		
		if (i > max_precedence)
		{
			// Unable to parse the binop operation
			// However if the first expression is valid we accept and return it.
			if (e1 != NULL)
			{
				SG.Accept();
				be->e1 = NULL;
				delete be;
				return e1;
			}
			else
			{
				delete be;
				return NULL;
			}
		}

		// after the first match, we apply the same precedence only.
		max_precedence = (KSLOperationPrecedence)i;

		be->start_line = T().line;

		// match second expression
		{
			KSLExpressionNode *e2 = MatchExpression((KSLOperationPrecedence)(max_precedence + 1));
			if (e2 == NULL)
			{
				delete be;
				return NULL;
			}
			be->e2 = e2;
		}

		// continue the recursive matching
		e1 = be;
	}

	return NULL;
}


KSLExpressionNode* KSLAnalyzer::MatchAssignExpression()
{
	StateGuard SG(this, false);

	// match first expression
	KSLExpressionNode *e1 = NULL;
	{
		e1 = MatchExpression( KSLOperationPrecedence(KSL_PRECEDENCE_ASSIGN + 1) );
		if (e1 == NULL)
		{
			return NULL;
		}
		
	}

	// e1 can't be an l-value
	if (e1->precedence < KSL_PRECEDENCE_UNARY)
	{
		SG.Accept();
		return e1;
	}

	KSLBinaryOperation binop = KSL_NUM_BINOP;

	if (Match(KSL_TOKEN_EQUAL)) binop = KSL_BINOP_ASSIGN;
	else if (Match(KSL_TOKEN_PLUS_EQUAL)) binop = KSL_BINOP_ADD_ASSIGN;
	else if (Match(KSL_TOKEN_MINUS_EQUAL)) binop = KSL_BINOP_SUB_ASSIGN;
	else if (Match(KSL_TOKEN_MUL_EQUAL)) binop = KSL_BINOP_MUL_ASSIGN;
	else if (Match(KSL_TOKEN_DIV_EQUAL)) binop = KSL_BINOP_DIV_ASSIGN;
	else if (Match(KSL_TOKEN_LESSLESSEQUAL)) binop = KSL_BINOP_SHIFT_LEFT_ASSIGN;
	else if (Match(KSL_TOKEN_GREATGREATEQUAL)) binop = KSL_BINOP_SHIFT_RIGHT_ASSIGN;
	else
	{
		SG.Accept();
		return e1;
	}
		

	// match second expression
	KSLExpressionNode *e2 = NULL;
	{
		e2 = MatchExpression(KSL_PRECEDENCE_LOWEST);
		if (e2 == NULL)
		{
			delete e1;
			return NULL;
		}
	}

	KSLBinaryExpressionNode *be = new KSLBinaryExpressionNode();
	be->operation = binop;
	be->e1 = e1;
	be->e2 = e2;
	be->start_line = T().line;

	SG.Accept();

	return be;
}


KSLConstructorExpressionNode* KSLAnalyzer::MatchConstructorExpression()
{
	KSLConstructorExpressionNode *ce = new KSLConstructorExpressionNode;
	StateGuard SG(this, ce);

	MATCH_OR_FAIL(KSL_TOKEN_IDENTIFIER);
	KSLToken &type_token = T();
	ce->start_line = type_token.line;

	if (!GetType(type_token.str_value, ce->constructor_type))
	{
		ErrorUnknownType(type_token);
		return NULL;
	}

	MATCH_OR_FAIL(KSL_TOKEN_LEFT_PARENT);

	while (true)
	{
		KSLExpressionNode* e = MatchExpression(KSL_PRECEDENCE_LOWEST);
		if (e == NULL) return NULL;

		ce->initializers.push_back(e);

		if (!Match(KSL_TOKEN_COMMA)) break;
	}

	MATCH_OR_FAIL(KSL_TOKEN_RIGHT_PARENT);

	SG.Accept();

	return ce;
}


KSLExpressionNode* KSLAnalyzer::MatchSuffixExpression()
{
	StateGuard SG(this, false);

	KSLExpressionNode *e = MatchExpression(KSL_PRECEDENCE_PARENTHESIS);
	if (e == NULL) return NULL;

	bool ok = true;

	while (ok)
	{
		// match structure element access
		if (Match(KSL_TOKEN_POINT))
		{
			if (!Match(KSL_TOKEN_IDENTIFIER))
			{
				ok = false;
				break;
			}

			KSLSelectorExpressionNode* se = new KSLSelectorExpressionNode();
			se->node_type = KSL_NODE_MEMBER_ACCESS_EXPRESSION;
			se->expression = e;
			se->selector = T().str_value;
			se->start_line = e->start_line;

			e = se;
			continue;
		}

		// match array element access
		else if (Match(KSL_TOKEN_LEFT_BRACKET))
		{
			KSLExpressionNode* ide = MatchExpression(KSL_PRECEDENCE_LOWEST);

			if ((ide == NULL) || !Match(KSL_TOKEN_RIGHT_BRACKET))
			{
				delete ide;
				ok = false;
				break;
			}

			KSLSelectorExpressionNode* se = new KSLSelectorExpressionNode();
			se->node_type = KSL_NODE_ARRAY_ACCESS_EXPRESSION;
			se->expression = e;
			se->id_expression = ide;
			se->start_line = e->start_line;

			e = se;
			continue;
		}

		// match post increment / decrement
		else if (Match(KSL_TOKEN_PLUSPLUS) || Match(KSL_TOKEN_MINUSMINUS))
		{
			KSLUnaryExpressionNode* ue = new KSLUnaryExpressionNode();
			ue->node_type = KSL_NODE_SUFFIX_EXPRESSION;
			ue->expression = e;
			ue->start_line = e->start_line;
			ue->precedence = KSL_PRECEDENCE_UNARY;

			if (T().type == KSL_TOKEN_PLUSPLUS) ue->operation = KSL_UNOP_INCREMENT;
			if (T().type == KSL_TOKEN_MINUSMINUS) ue->operation = KSL_UNOP_DECREMENT;

			e = ue;
			continue;
		}

		ok = false;
		break;
	}

	if (e != NULL)
	{
		SG.Accept();
		return e;
	}
	return NULL;
}


KSLFunctionCallExpressionNode* KSLAnalyzer::MatchFunctionCallExpression()
{
	KSLFunctionCallExpressionNode *fce = new KSLFunctionCallExpressionNode;
	StateGuard SG(this, fce);

	MATCH_OR_FAIL(KSL_TOKEN_IDENTIFIER);
	KSLToken &name_token = T();
	fce->start_line = name_token.line;

	MATCH_OR_FAIL(KSL_TOKEN_LEFT_PARENT);

	if (!Match(KSL_TOKEN_RIGHT_PARENT))
	{
		while (true)
		{
			KSLExpressionNode* e = MatchExpression(KSL_PRECEDENCE_LOWEST);
			if (e == NULL) return NULL;

			fce->arguments.push_back(e);

			if (!Match(KSL_TOKEN_COMMA)) break;
		}

		if (!Match(KSL_TOKEN_RIGHT_PARENT))
		{
			ErrorUnexpectedToken();
			return NULL;
		}
	}

	if (!HasFunction(name_token.str_value))
	{
		ErrorUndeclaredIdentifier(name_token);
		return NULL;
	}

	SG.Accept();

	fce->name = name_token.str_value;

	return fce;
}


KSLExpressionNode* KSLAnalyzer::MatchTernaryConditionalExpression()
{
	KSLTernaryConditionalExpressionNode *tce = new KSLTernaryConditionalExpressionNode;
	StateGuard SG(this, tce);

	// match condition
	KSLExpressionNode* ce = NULL;
	{
		ce = MatchExpression((KSLOperationPrecedence)(KSL_PRECEDENCE_TERNARY_CONDITIONAL + 1));
		if (ce == NULL) return NULL;
		tce->condition = ce;
		tce->start_line = ce->start_line;
	}
	
	// Shortcut return conditional expression if its not a ternary conditional
	if (!Match(KSL_TOKEN_QUESTION_MARK))
	{
		SG.Accept();
		tce->condition = NULL;
		delete tce;
		return ce;
	}

	// match if expression
	{
		KSLExpressionNode* ie = MatchExpression(KSL_PRECEDENCE_ASSIGN);
		if (ie == NULL) return NULL;
		tce->if_expression = ie;
	}

	MATCH_OR_FAIL(KSL_TOKEN_COLON);

	// match else expressin
	{
		KSLExpressionNode* ee = MatchExpression(KSL_PRECEDENCE_ASSIGN);
		if (ee == NULL) return NULL;
		tce->else_expression = ee;
	}

	SG.Accept();
	return tce;
}


KSLExpressionNode* KSLAnalyzer::MatchExpression(KSLOperationPrecedence min_precedence)
{
	StateGuard SG(this,true);
	InitErrorBlocks();

	// Match assign expression, it may return non binary expression with higher precedence
	if (min_precedence <= KSL_PRECEDENCE_ASSIGN)
	{
		NewErrorBlock();
		KSLExpressionNode *ae = MatchAssignExpression();
		if (ae != NULL)
		{
			AppendErrors();
			ast->expressions.push_back(ae);
			return ae;
		}
	}

	
	// Match trenary conditional
	if (min_precedence <= KSL_PRECEDENCE_TERNARY_CONDITIONAL)
	{
		NewErrorBlock();
		KSLExpressionNode *tce = MatchTernaryConditionalExpression();
		if (tce != NULL)
		{
			AppendErrors();
			return tce;
		}
	}


	// Match binary expression, it may return non binary expression with higher precedence
	if (min_precedence <= KSL_PRECEDENCE_BINARY_MUL)
	{
		NewErrorBlock();
		KSLExpressionNode *be = MatchBinaryExpression(min_precedence);
		if (be != NULL)
		{
			AppendErrors();
			ast->expressions.push_back(be);
			return be;
		}
	}


	// Match unary expression
	if (min_precedence <= KSL_PRECEDENCE_UNARY)
	{
		NewErrorBlock();
		KSLUnaryExpressionNode *ue = MatchUnaryExpression();
		if (ue != NULL)
		{
			AppendErrors();
			return ue;
		}
	}


	// Match post decrement, increment expression
	if (min_precedence <= KSL_PRECEDENCE_SUFFIX)
	{
		NewErrorBlock();
		KSLExpressionNode *se = MatchSuffixExpression();
		if (se != NULL)
		{
			AppendErrors();
			return se;
		}
	}


	// Match parenthesis expression
	if (min_precedence <= KSL_PRECEDENCE_PARENTHESIS)
	{
		NewErrorBlock();
		KSLParenthesisExpressionNode *pe = MatchParenthesisExpression();
		if (pe != NULL)
		{
			AppendErrors();
			return pe;
		}
	}

	// Match literal expression
	if (min_precedence <= KSL_PRECEDENCE_PARENTHESIS)
	{
		NewErrorBlock();
		KSLLiteralExpressionNode *le = MatchLiteralExpression();
		if (le != NULL)
		{
			AppendErrors();
			return le;
		}
	}


	// Match constructor expression
	if (min_precedence <= KSL_PRECEDENCE_PARENTHESIS)
	{
		NewErrorBlock();
		KSLConstructorExpressionNode *ce = MatchConstructorExpression();
		if (ce != NULL)
		{
			AppendErrors();
			return ce;
		}
	}


	// Match function call expression
	if (min_precedence <= KSL_PRECEDENCE_PARENTHESIS)
	{
		NewErrorBlock();
		KSLFunctionCallExpressionNode *fce = MatchFunctionCallExpression();
		if (fce != NULL)
		{
			AppendErrors();
			return fce;
		}
	}


	// Match variable expression
	if (min_precedence <= KSL_PRECEDENCE_PARENTHESIS)
	{
		NewErrorBlock();
		KSLVariableExpressionNode *ve = MatchVariableExpression();
		if (ve != NULL)
		{
			AppendErrors();
			return ve;
		}
	}

	CollectErrors();
	SG.Fail();

	return NULL;
}


KSLBlockStatementNode* KSLAnalyzer::MatchBlockStatement()
{
	TableGuard<uint32_t> TG(m_active_variable_ids);
	m_active_variable_ids.push_back(KSL_UINT32_MAX); // <- block start

	// postfixing variables
	TableGuard<std::string> VPG(m_variable_postfix);
	if (m_mangling_variable_names)
	{
		std::stringstream sstream;
		m_block_counter++;
		sstream << "_b" << m_block_counter;
		m_variable_postfix.push_back(sstream.str());
	}

	KSLBlockStatementNode *bs = new KSLBlockStatementNode;
	StateGuard SG(this, bs);

	MATCH_OR_FAIL(KSL_TOKEN_LEFT_BRACE);
	bs->start_line = T().line;

	bool s = true;

	while (s)
	{
		if (Match(KSL_TOKEN_RIGHT_BRACE))
		{
			break;
		}

		InitErrorBlocks();

		// Match variable definition
		{
			NewErrorBlock();
			KSLVariableDefinitionsNode *vdl = MatchVariableDefinitions(false);
			if (vdl)
			{
				AppendErrors();
				bs->nodes.push_back(vdl);
				continue;
			}
		}

		// Match statement
		{
			NewErrorBlock();
			KSLASTNode *s = MatchStatement();
			if (s)
			{
				AppendErrors();
				bs->nodes.push_back(s);
				continue;
			}
		}


		CollectErrors();

		return NULL;
	}

	SG.Accept();

	return bs;
}


KSLExpressionOrEmptyStatementNode* KSLAnalyzer::MatchExpressionOrEmptyStatement()
{
	KSLExpressionOrEmptyStatementNode *es = new KSLExpressionOrEmptyStatementNode;
	StateGuard SG(this, es);

	if (Match(KSL_TOKEN_SEMICOLON))
	{
		es->node_type = KSL_NODE_EMPTY_STATEMENT;
	}
	else
	{
		es->node_type = KSL_NODE_EXPRESSION_STATEMENT;
		KSLExpressionNode *en = MatchExpression(KSL_PRECEDENCE_LOWEST);
		if (en == NULL)
		{
			return NULL;
		}
		es->expression = en;

		MATCH_OR_FAIL(KSL_TOKEN_SEMICOLON);
	}

	SG.Accept();

	return es;
}


KSLIfStatementNode* KSLAnalyzer::MatchIfStatement()
{
	KSLIfStatementNode *is = new KSLIfStatementNode();
	StateGuard SG(this, is);

	MATCH_OR_FAIL(KSL_TOKEN_IF);
	is->start_line = T().line;

	MATCH_OR_FAIL(KSL_TOKEN_LEFT_PARENT);

	// match condition
	{
		KSLExpressionNode *ce = MatchExpression(KSL_PRECEDENCE_LOWEST);
		if (ce == NULL)
		{
			return NULL;
		}
		is->condition = ce;
	}


	MATCH_OR_FAIL(KSL_TOKEN_RIGHT_PARENT);

	// match if statement
	{
		KSLASTNode *ib = MatchStatement();
		if (ib == NULL)
		{
			return NULL;
		}
		is->if_statement = ib;
	}

	if (Match(KSL_TOKEN_ELSE))
	{
		KSLASTNode *eb = MatchStatement();
		if (eb == NULL)
		{
			return NULL;
		}
		is->else_statement = eb;
	}
	
	SG.Accept();

	return is;
}


KSLForStatementNode* KSLAnalyzer::MatchForStatement()
{
	TableGuard<uint32_t> TG(m_active_variable_ids);
	m_active_variable_ids.push_back(KSL_UINT32_MAX); // <- block start

	// postfixing variables
	TableGuard<std::string> VPG(m_variable_postfix);
	if (m_mangling_variable_names)
	{
		std::stringstream sstream;
		m_block_counter++;
		sstream << "_fl" << m_block_counter;
		m_variable_postfix.push_back(sstream.str());
	}

	KSLForStatementNode *fs = new KSLForStatementNode();
	StateGuard SG(this, fs);

	if (Match(KSL_TOKEN_LEFT_BRACKET))
	{
		MATCH_OR_FAIL(KSL_TOKEN_IDENTIFIER);

		if (T().str_value != "loop")
		{
			ErrorUnexpectedToken();
			return NULL;
		}

		fs->is_loop = true;
		MATCH_OR_FAIL(KSL_TOKEN_RIGHT_BRACKET);
	}

	MATCH_OR_FAIL(KSL_TOKEN_FOR);
	fs->start_line = T().line;

	MATCH_OR_FAIL(KSL_TOKEN_LEFT_PARENT);

	// match init statement
	if (!Match(KSL_TOKEN_SEMICOLON))
	{
		InitErrorBlocks();
		
		KSLASTNode* init_node = NULL;

		{
			NewErrorBlock();
			init_node = MatchVariableDefinitions(false);
			if (init_node != NULL) AppendErrors();
		}

		if (init_node == NULL)
		{
			NewErrorBlock();
			init_node = MatchExpressionOrEmptyStatement();
			if (init_node != NULL) AppendErrors();
		}

		if (init_node == NULL)
		{
			CollectErrors();
			return NULL;
		}

		fs->init_node = init_node;
	}


	// match conditional statement
	if (!Match(KSL_TOKEN_SEMICOLON))
	{
		fs->conditional_statement = MatchExpression(KSL_PRECEDENCE_LOWEST);

		if (fs->conditional_statement == NULL) return NULL;

		MATCH_OR_FAIL(KSL_TOKEN_SEMICOLON);
	}


	// match step statement
	if (!Match(KSL_TOKEN_RIGHT_PARENT))
	{
		fs->step_statement = MatchExpression(KSL_PRECEDENCE_LOWEST);

		if (fs->step_statement == NULL) return NULL;

		MATCH_OR_FAIL(KSL_TOKEN_RIGHT_PARENT);
	}

	// match body statement
	{
		fs->body = MatchStatement();
		if (fs->body == NULL) return NULL;
	}

	SG.Accept();

	return fs;
}


KSLControlStatementNode* KSLAnalyzer::MatchControlStatement()
{
	KSLControlStatementNode *cs = new KSLControlStatementNode();
	StateGuard SG(this, cs);

	// KSL_TODO check for loop
	if (Match(KSL_TOKEN_BREAK)) cs->node_type = KSL_NODE_BREAK_STATEMENT;
	else if (Match(KSL_TOKEN_CONTINUE)) cs->node_type = KSL_NODE_CONTINUE_STATEMENT;
	else if (Match(KSL_TOKEN_DISCARD))
	{
		cs->node_type = KSL_NODE_DISCARD_STATEMENT;
		ast->has_discard = true;
	}
	else
	{
		ErrorUnexpectedToken();
		return NULL;
	}

	MATCH_OR_FAIL(KSL_TOKEN_SEMICOLON);

	SG.Accept();
	return cs;
}


KSLReturnStatementNode* KSLAnalyzer::MatchReturnStatement()
{
	KSLReturnStatementNode *rs = new KSLReturnStatementNode();
	StateGuard SG(this, rs);

	MATCH_OR_FAIL(KSL_TOKEN_RETURN);
	rs->start_line = T().line;

	// KSL_TODO: check return value is required

	if (!Match(KSL_TOKEN_SEMICOLON))
	{
		rs->expression = MatchExpression(KSL_PRECEDENCE_LOWEST);

		if (rs->expression == NULL) return NULL;

		MATCH_OR_FAIL(KSL_TOKEN_SEMICOLON);
	}
	
	SG.Accept();
	return rs;
}


KSLASTNode* KSLAnalyzer::MatchStatement()
{
	InitErrorBlocks();
	StateGuard SG(this, true);

	// Match block statement
	{
		NewErrorBlock();
		KSLBlockStatementNode *bs = MatchBlockStatement();
		if (bs != NULL)
		{
			AppendErrors();
			return bs;
		}
	}

	// Match expression statement
	{
		NewErrorBlock();
		KSLExpressionOrEmptyStatementNode *es = MatchExpressionOrEmptyStatement();
		if (es != NULL)
		{
			AppendErrors();
			return es;
		}
	}

	// Match if statement
	{
		NewErrorBlock();
		KSLIfStatementNode *is = MatchIfStatement();
		if (is != NULL)
		{
			AppendErrors();
			return is;
		}
	}

	// Match for statement
	{
		NewErrorBlock();
		KSLForStatementNode *fs = MatchForStatement();
		if (fs != NULL)
		{
			AppendErrors();
			return fs;
		}
	}

	// Match control statement
	{
		NewErrorBlock();
		KSLControlStatementNode *cs = MatchControlStatement();
		if (cs != NULL)
		{
			AppendErrors();
			return cs;
		}
	}


	// Match return statement
	{
		NewErrorBlock();
		KSLReturnStatementNode *rs = MatchReturnStatement();
		if (rs != NULL)
		{
			AppendErrors();
			return rs;
		}
	}

	// while

	CollectErrors();
	SG.Fail();

	return NULL;
}


KSLFunctionNode* KSLAnalyzer::MatchFunction()
{
	TableGuard<uint32_t> TG(m_active_variable_ids);
	m_active_variable_ids.push_back(KSL_UINT32_MAX); // <- block start

	// postfixing variables
	TableGuard<std::string> VPG(m_variable_postfix);
	if (m_mangling_variable_names)
	{
		std::stringstream sstream;
		m_block_counter++;
		sstream << "_fa" << m_block_counter;
		m_variable_postfix.push_back(sstream.str());
	}

	KSLFunction function;
	KSLFunctionNode* function_node = new KSLFunctionNode();
	StateGuard SG(this, function_node);

	MATCH_OR_FAIL(KSL_TOKEN_IDENTIFIER);
	KSLToken &type_token = T();

	MATCH_OR_FAIL(KSL_TOKEN_IDENTIFIER);
	KSLToken &func_name_token = T();

	MATCH_OR_FAIL(KSL_TOKEN_LEFT_PARENT);


	//	Match Function attributes
	while (true)
	{
		KSLAttribAccess access = KSL_ATTRIB_ACCESS_IN;

		if (Match(KSL_TOKEN_IN)) access = KSL_ATTRIB_ACCESS_IN;
		else if (Match(KSL_TOKEN_OUT)) access = KSL_ATTRIB_ACCESS_OUT;
		else if (Match(KSL_TOKEN_INOUT)) access = KSL_ATTRIB_ACCESS_INOUT;

		KSLType type;
		KSLToken type_token;
		if (!MatchType(type, type_token)) break;

		MATCH_OR_FAIL(KSL_TOKEN_IDENTIFIER);

		CheckVariableRedefiniton(T().str_value);

		KSLVariable v;
		v.type = type;
		v.orig_name = T().str_value;

		KSLExpressionNode *se = NULL;
		if (Match(KSL_TOKEN_LEFT_BRACKET))
		{
			v.type = KSLType(KSL_TYPE_ARRAY, KSL_PRECISION_NONE);
			v.type.SetBaseType(type);

			se = MatchExpression(KSL_PRECEDENCE_LOWEST);
			MATCH_OR_FAIL(KSL_TOKEN_RIGHT_BRACKET);
		}

		v.storage_type = KSL_STORAGE_DEFAULT;
		v.new_name = (m_mangling_variable_names)?(v.orig_name + m_variable_postfix.back()):v.orig_name;

		m_active_variable_ids.push_back((uint32_t)ast->variables.size());
		
		function.attrib_type.push_back(v.type);
		function.attrib_access.push_back(access);

		KSLFunctionNode::AttribNode an;
		an.variable_id = (uint32_t)ast->variables.size();
		an.size_expression = se;
		function_node->attribs.push_back(an);
		
		ast->variables.push_back(v);

		if (!Match(KSL_TOKEN_COMMA)) break;
	}

	MATCH_OR_FAIL(KSL_TOKEN_RIGHT_PARENT);

	KSLBlockStatementNode *bs = MatchBlockStatement();
	if (bs == NULL)
	{
		return NULL;
	}
	function_node->body = bs;
		
	SG.Accept();

	KSLType type;
	if (!GetType(type_token.str_value, type))
	{
		ErrorUnknownType(type_token);
	}

	function.return_type = type;
	function.function_name = func_name_token.str_value;

	function_node->function_id = (uint32_t)ast->functions.size();
	function_node->start_line = type_token.line;

	// KSL_TODO: check function redefinition

	ast->functions.push_back(function);

	return function_node;
}


KSLImageDefinitionNode* KSLAnalyzer::MatchImageDefinition()
{
	KSLImageDefinitionNode *idn = new KSLImageDefinitionNode();
	StateGuard SG(this, idn);

	KSLVariable v;

	if (!Match(KSL_TOKEN_IMAGE2D)) return NULL;
	idn->start_line = T().line;

	// Match base type
	{
		if (!Match(KSL_TOKEN_LESS)) return NULL;

		KSLToken type_token;
		if (!MatchType(v.type, type_token)) return NULL;

		if (v.type.id != KSL_TYPE_FLOAT) return NULL;
		if (!Match(KSL_TOKEN_GREATER)) return NULL;
	}
	v.type.id = KSL_TYPE_IMAGE2D;

	MATCH_OR_FAIL(KSL_TOKEN_IDENTIFIER);
	KSLToken &name_token = T();

	if (!MatchVariableAttributes(idn->bool_attribs, idn->int_attribs)) return NULL;

	MATCH_OR_FAIL(KSL_TOKEN_SEMICOLON);

	CheckVariableRedefiniton(name_token.str_value);

	v.orig_name = name_token.str_value;
	// do not mangling interface variables
	v.new_name = v.orig_name;
	v.storage_type = KSL_STORAGE_DEFAULT;

	if (idn->bool_attribs.find(KSL_ATTRIB_QUALIFIER_READONLY) != idn->bool_attribs.end())
	{
		v.access = KSL_ACCESS_READ_ONLY;
	}
	else if (idn->bool_attribs.find(KSL_ATTRIB_QUALIFIER_WRITEONLY) != idn->bool_attribs.end())
	{
		v.access = KSL_ACCESS_WRITE_ONLY;
	}
	else
	{
		ErrorImageAccessMissing(T());
	}

	m_active_variable_ids.push_back((uint32_t)ast->variables.size());
	idn->variable_id = (uint32_t)ast->variables.size();

	ast->variables.push_back(v);	

	SG.Accept();
	return idn;
}


KSLNumThreadNode* KSLAnalyzer::MatchNumThreads()
{
	KSLNumThreadNode *ntn = new KSLNumThreadNode();
	StateGuard SG(this, ntn);

	MATCH_OR_FAIL(KSL_TOKEN_NUMTHREADS);
	ntn->start_line = T().line;

	MATCH_OR_FAIL(KSL_TOKEN_LEFT_PARENT);

	if (!Match(KSL_TOKEN_SIGNED_INTEGER) && !Match(KSL_TOKEN_UNSIGNED_INTEGER)) return NULL;
	ntn->x = (uint32_t)T().int_value;

	MATCH_OR_FAIL(KSL_TOKEN_COMMA);

	if (!Match(KSL_TOKEN_SIGNED_INTEGER) && !Match(KSL_TOKEN_UNSIGNED_INTEGER)) return NULL;
	ntn->y = (uint32_t)T().int_value;

	MATCH_OR_FAIL(KSL_TOKEN_COMMA);

	if (!Match(KSL_TOKEN_SIGNED_INTEGER) && !Match(KSL_TOKEN_UNSIGNED_INTEGER)) return NULL;
	ntn->z = (uint32_t)T().int_value;

	MATCH_OR_FAIL(KSL_TOKEN_RIGHT_PARENT);
	MATCH_OR_FAIL(KSL_TOKEN_SEMICOLON);

	SG.Accept();
	return ntn;
}


bool KSLAnalyzer::MatchForceHighp()
{
	StateGuard SG(this, false);

	if (!Match(KSL_TOKEN_FORCE_HIGHP)) return false;
	if (!Match(KSL_TOKEN_SEMICOLON)) return false;

	SG.Accept();
	ast->force_highp = true;
	return true;
}


KSLStructDefinitionNode* KSLAnalyzer::MatchStructDefinition()
{
	TableGuard<uint32_t> TG(m_active_variable_ids);
	m_active_variable_ids.push_back(KSL_UINT32_MAX); // <- block start

	// postfixing variables
	TableGuard<std::string> VPG(m_variable_postfix);
	if (m_mangling_variable_names) m_variable_postfix.push_back("");

	KSLStructDefinitionNode *sdn = new KSLStructDefinitionNode();
	StateGuard SG(this, sdn);

	MATCH_OR_FAIL(KSL_TOKEN_STRUCT);
	sdn->start_line = T().line;

	MATCH_OR_FAIL(KSL_TOKEN_IDENTIFIER);
	sdn->name = T().str_value;

	MATCH_OR_FAIL(KSL_TOKEN_LEFT_BRACE);

	KSLVariableDefinitionsNode *vdn = MatchVariableDefinitions(false);

	while (vdn != NULL)
	{
		sdn->members.push_back(vdn);

		vdn = MatchVariableDefinitions(false);
	}

	MATCH_OR_FAIL(KSL_TOKEN_RIGHT_BRACE);
	MATCH_OR_FAIL(KSL_TOKEN_SEMICOLON);

	// KSL_TODO: check struct redefinition

	ast->user_types.push_back(sdn);

	SG.Accept();
	return sdn;
}


bool KSLAnalyzer::MatchProgram()
{
	NewErrorBlock();

	ast->root = new KSLProgramNode();
	m_active_variable_ids.push_back(KSL_UINT32_MAX); // <- block start

	// debug
	m_variable_postfix.push_back("_g");
	
	bool s = true;
	
	while (s)
	{
		if (Match(KSL_TOKEN_END_OF_TOKENS))
		{
			break;
		}

		InitErrorBlocks();

		NewErrorBlock();
		KSLVariableDefinitionsNode *vdl = MatchVariableDefinitions(true);
		if (vdl != NULL)
		{
			ast->root->nodes.push_back(vdl);
			AppendErrors();
			continue;
		}

		NewErrorBlock();
		KSLFunctionNode* fn = MatchFunction();
		if (fn != NULL)
		{
			ast->root->nodes.push_back(fn);
			AppendErrors();
			continue;
		}

		NewErrorBlock();
		KSLImageDefinitionNode* idn = MatchImageDefinition();
		if (idn != NULL)
		{
			ast->root->nodes.push_back(idn);
			AppendErrors();
			continue;
		}

		{
			NewErrorBlock();
			KSLStructDefinitionNode* sdn = MatchStructDefinition();
			if (sdn != NULL)
			{
				ast->root->nodes.push_back(sdn);
				AppendErrors();
				continue;
			}
		}


		if (m_shader_type == NGL_COMPUTE_SHADER)
		{
			NewErrorBlock();
			KSLNumThreadNode* ntn = MatchNumThreads();
			if (ntn != NULL)
			{
				ast->root->nodes.push_back(ntn);
				AppendErrors();
				continue;
			}
		}

		{
			NewErrorBlock();
			bool s = MatchForceHighp();
			if (s)
			{
				continue;
			}
		}

		CollectErrors();

		s = false;
	}

	return s;
}


bool KSLAnalyzer::Analyze(KSLProgramAST &ast)
{
	this->ast = &ast;

	RegisterBuiltInFunctions();
	RegisterMultiplications();
	RegisterBuiltInVariables();

	bool s = true;

	if (s) s &= MatchProgram();

	s &= !HasErrors();

	// The call order is relevant!
	if (s && ast.force_highp)
	{
		ForceHighPrecision();
	}

	if (s) s &= ValidateProgram();
	if (s) CollectReflectionInfo();
	if (s) s &= CheckUniformsAmount();
	if (s) DetectUnusedVariables();

	return s;
}


void KSLAnalyzer::CheckVariableRedefiniton(std::string variable_name)
{
	std::vector<uint32_t>::reverse_iterator rit = m_active_variable_ids.rbegin();
	for (; rit != m_active_variable_ids.rend(); rit++)
	{
		if (*rit == KSL_UINT32_MAX) break; // <- block flag reached

		if (ast->variables[*rit].orig_name == variable_name)
		{
			std::stringstream sstream;
			sstream << "variable redefinition \"" << T().str_value << "\" at " << T().line << ":" << T().column;
			m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, T().line, T().column, sstream.str()));
			break;
		}
	}
}


bool KSLAnalyzer::GetType(const std::string &str, KSLType &type)
{
	// check inbuilt types
	std::map<std::string, KSLType>::iterator it = m_types.find(str);
	if (it != m_types.end())
	{
		type = it->second;
		return true;
	}

	// check user types
	for (size_t i = 0; i < ast->user_types.size(); i++)
	{
		if (str == ast->user_types[i]->name)
		{
			type.id = (uint32_t)(KSL_NUM_INBUILT_TYPES + i);
			type.precision = KSL_PRECISION_NONE;
			return true;
		}
	}

	return false;
}


bool KSLAnalyzer::GetAttribQualifier(KSLAttribQualifier &qualifer)
{
	if (Match(KSL_TOKEN_READONLY))
	{
		qualifer = KSL_ATTRIB_QUALIFIER_READONLY;
		return true;
	}
	else if(Match(KSL_TOKEN_WRITEONLY))
	{
		qualifer = KSL_ATTRIB_QUALIFIER_WRITEONLY;
		return true;
	}
	else if (Match(KSL_TOKEN_IDENTIFIER))
	{
		KSLToken &attr_name = T();
		std::map<std::string, KSLAttribQualifier>::iterator it = m_builtin_qualifiers.find(attr_name.str_value);
		if (it != m_builtin_qualifiers.end())
		{
			qualifer = it->second;
			return true;
		}
	}
	return false;
}


bool  KSLAnalyzer::HasFunction(const std::string &name)
{
	for (uint32_t i = 0; i < ast->functions.size(); i++)
	{
		if (name == ast->functions[i].function_name)
		{
			return true;
		}
	}
	return false;
}


void KSLAnalyzer::Clear()
{
	m_types.clear();
	m_active_variable_ids.clear();
	m_tokens = NULL;
	m_shader_type = NGL_NUM_SHADER_TYPES;
	m_validated_function_node = NULL;
    m_precision_mismatch_severity = KSL_WARNING;

	// errors
	m_token_run = NULL;
	m_errors = NULL;

	for (size_t i = 0; i < m_error_blocks.size(); i++)
	{
		delete m_error_blocks[i];
	}
	m_error_blocks.clear();

	// debug
	m_mangling_variable_names = false;
	m_block_counter = 0;
}

