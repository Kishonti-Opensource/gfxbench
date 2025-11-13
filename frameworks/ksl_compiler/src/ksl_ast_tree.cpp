/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_ast_tree.h"

#include <assert.h>

//
//	KSLFunction
//
KSLFunction::KSLFunction()
{

}

KSLFunction::KSLFunction(const std::string &name, const KSLType rt)
{
	function_name = name;
	return_type = rt;
}

KSLFunction::KSLFunction(const std::string &name, const KSLType rt, const KSLType at)
{
	function_name = name;
	return_type = rt;
	attrib_type.push_back(at);
}

KSLFunction::KSLFunction(const std::string &name, const KSLType rt, const KSLType at1, const KSLType at2)
{
	function_name = name;
	return_type = rt;
	attrib_type.push_back(at1);
	attrib_type.push_back(at2);
}

KSLFunction::KSLFunction(const std::string &name, const KSLType rt, const KSLType at1, const KSLType at2, const KSLType at3)
{
	function_name = name;
	return_type = rt;
	attrib_type.push_back(at1);
	attrib_type.push_back(at2);
	attrib_type.push_back(at3);
}

KSLFunction::KSLFunction(const std::string &name, const KSLType rt, const KSLType at1, const KSLType at2, const KSLType at3, const KSLType at4)
{
	function_name = name;
	return_type = rt;
	attrib_type.push_back(at1);
	attrib_type.push_back(at2);
	attrib_type.push_back(at3);
	attrib_type.push_back(at4);
}


//
//	KSLASTNode
//
KSLASTNode::KSLASTNode()
	: node_type(KSL_NODE_INVALID)
	, start_line(0)
	, end_line(0)
	, start_column(0)
{
}

KSLASTNode::~KSLASTNode()
{
}


//
//	KSLExpressionNode
//
KSLExpressionNode::KSLExpressionNode()
{
	precedence = KSL_PRECEDENCE_INVALID;
	l_value = false;
	constant_value = false;
}


//
//	KSLLiteralExpressionNode
//
KSLLiteralExpressionNode::KSLLiteralExpressionNode()
{
	node_type = KSL_NODE_LITERAL_EXPRESSION;
	precedence = KSL_PRECEDENCE_PARENTHESIS;
}


//
//	KSLVariableExpressionNode
//
KSLVariableExpressionNode::KSLVariableExpressionNode()
{
	node_type = KSL_NODE_VARIABLE_EXPRESSION;
	precedence = KSL_PRECEDENCE_PARENTHESIS;
	variable_id = 0;
}


//
//	KSLConstructorExpressionNode
//
KSLConstructorExpressionNode::KSLConstructorExpressionNode()
{
	node_type = KSL_NODE_CONSTRUCTOR_EXPRESSION;
	precedence = KSL_PRECEDENCE_PARENTHESIS;
}

KSLConstructorExpressionNode::~KSLConstructorExpressionNode()
{
	for (size_t i = 0; i < initializers.size(); i++)
	{
		delete initializers[i];
	}
}


//
//	KSLSelectorExpressionNode
//
KSLSelectorExpressionNode::KSLSelectorExpressionNode()
{
	node_type = KSL_NODE_INVALID;
	expression = NULL;
	id_expression = NULL;
	precedence = KSL_PRECEDENCE_SUFFIX;
}

KSLSelectorExpressionNode::~KSLSelectorExpressionNode()
{
	delete expression;
	delete id_expression;
}


//
//	KSLParenthesisExpressionNode
//
KSLParenthesisExpressionNode::KSLParenthesisExpressionNode()
{
	node_type = KSL_NODE_PARENTHESIS_EXPRESSION;
	precedence = KSL_PRECEDENCE_PARENTHESIS;
	expression = NULL;
}

KSLParenthesisExpressionNode::~KSLParenthesisExpressionNode()
{
	delete expression;
}


//
//	KSLUnaryExpressionNode
//
KSLUnaryExpressionNode::KSLUnaryExpressionNode()
{
	node_type = KSL_NODE_UNARY_OP_EXPRESSION;
	operation = KSL_NUM_UNOP;
	expression = NULL;
	precedence = KSL_PRECEDENCE_INVALID;
}

KSLUnaryExpressionNode::~KSLUnaryExpressionNode()
{
	delete expression;
}

//
//	KSLBinaryExpressionNode
//
KSLBinaryExpressionNode::KSLBinaryExpressionNode()
{
	node_type = KSL_NODE_BINOP_EXPRESSION;
	operation = KSL_NUM_BINOP;
	e1 = NULL;
	e2 = NULL;
}

KSLBinaryExpressionNode::~KSLBinaryExpressionNode()
{
	delete e1;
	delete e2;
}


//
//	KSLFunctionCallExpressionNode
//
KSLFunctionCallExpressionNode::KSLFunctionCallExpressionNode()
{
	node_type = KSL_NODE_FUNCTION_CALL_EXPRESSION;
	precedence = KSL_PRECEDENCE_PARENTHESIS;
}

KSLFunctionCallExpressionNode::~KSLFunctionCallExpressionNode()
{
	for (size_t i = 0; i < arguments.size(); i++)
	{
		delete arguments[i];
	}
}


//
//	KSLTernaryConditionalExpressionNode
//
KSLTernaryConditionalExpressionNode::KSLTernaryConditionalExpressionNode()
{
	node_type = KSL_NODE_TERNARY_CONDITIONAL_EXPRESSION;
	precedence = KSL_PRECEDENCE_TERNARY_CONDITIONAL;
	condition = NULL;
	if_expression = NULL;
	else_expression = NULL;
}

KSLTernaryConditionalExpressionNode::~KSLTernaryConditionalExpressionNode()
{
	delete condition;
	delete if_expression;
	delete else_expression;
}


//
//	KSLExpressionOrEmptyStatementNode
//
KSLExpressionOrEmptyStatementNode::KSLExpressionOrEmptyStatementNode()
	: expression(NULL)
{
	node_type = KSL_NODE_INVALID;
}

KSLExpressionOrEmptyStatementNode::~KSLExpressionOrEmptyStatementNode()
{
	delete expression;
}


//
//	KSLBlockStatementNode
//
KSLBlockStatementNode::KSLBlockStatementNode()
{
	node_type = KSL_NODE_BLOCK_STATEMENT;
}

KSLBlockStatementNode::~KSLBlockStatementNode()
{
	for (size_t i = 0; i < nodes.size(); i++)
	{
		delete nodes[i];
	}
}


//
//	KSLIfStatementNode
//
KSLIfStatementNode::KSLIfStatementNode()
	: condition(NULL)
	, if_statement(NULL)
	, else_statement(NULL)
{
	node_type = KSL_NODE_IF_STATEMENT;
}

KSLIfStatementNode::~KSLIfStatementNode()
{
	delete condition;
	delete if_statement;
	delete else_statement;
}


//
//	KSLForStatementNode
//
KSLForStatementNode::KSLForStatementNode()
	: init_node(NULL)
	, conditional_statement(NULL)
	, step_statement(NULL)
	, body(NULL)
	, is_loop(false)
{
	node_type = KSL_NODE_FOR_STATEMENT;
}

KSLForStatementNode::~KSLForStatementNode()
{
	delete init_node;
	delete conditional_statement;
	delete step_statement;
	delete body;
}


//
//	KSLControlStatementNode
//
KSLControlStatementNode::KSLControlStatementNode()
{
	node_type = KSL_NODE_INVALID;
}


//
//	KSLReturnStatementNode
//
KSLReturnStatementNode::KSLReturnStatementNode()
{
	node_type = KSL_NODE_RETURN_STATEMENT;
	expression = NULL;
}

KSLReturnStatementNode::~KSLReturnStatementNode()
{
	delete expression;
}


//
//	KSLVariableDefinitionsNode
//
KSLVariableDefinitionsNode::KSLVariableDefinitionsNode()
{
	node_type = KSL_NODE_VARIABLE_DEFINITIONS;
	storage_type = KSL_STORAGE_DEFAULT;
}

KSLVariableDefinitionsNode::~KSLVariableDefinitionsNode()
{
	for (size_t i = 0; i < variables.size(); i++)
	{
		delete variables[i].size_expression;
		for (size_t j = 0; j < variables[i].init_expressions.size(); j++)
		{
			delete variables[i].init_expressions[j];
		}
	}
}


//
//	KSLFunctionNode
//
KSLFunctionNode::KSLFunctionNode()
{
	node_type = KSL_NODE_FUNCTION;
	body = NULL;
}

KSLFunctionNode::~KSLFunctionNode()
{
	delete body;

	for (size_t i = 0; i < attribs.size(); i++)
	{
		delete attribs[i].size_expression;
	}
}


//
//	KSLImageDefinitionNode
//
KSLImageDefinitionNode::KSLImageDefinitionNode()
{
	node_type = KSL_NODE_IMAGE_DEFINITION;
}


//
//	KSLNumThreadNode
//
KSLNumThreadNode::KSLNumThreadNode()
{
	node_type = KSL_NODE_NUMTHREAD;
	x = y = z = 0;
}


//
//	KSLStructDefintionNode
//
KSLStructDefinitionNode::KSLStructDefinitionNode()
{
	node_type = KSL_NODE_STRUCT_DEFINITION;
}

KSLStructDefinitionNode::~KSLStructDefinitionNode()
{
	for (size_t i = 0; i < members.size(); i++)
	{
		delete members[i];
	}
}


//
//	KSLProgramNode
//
KSLProgramNode::KSLProgramNode()
{
	node_type = KSL_NODE_PROGRAM;
}


KSLProgramNode::~KSLProgramNode()
{
	for (size_t i = 0; i < nodes.size(); i++)
	{
		delete nodes[i];
	}
	nodes.clear();
}


//
//	KSLProgramAST
//
KSLProgramAST::KSLProgramAST()
{
	root = NULL;
	variables.clear();
	shader_type = KSL_UINT32_MAX;
	m_main_function_node_id = KSL_UINT32_MAX;
	force_highp = false;
	has_discard = false;
}


KSLProgramAST::~KSLProgramAST()
{
	delete root;
}


void KSLProgramAST::CollectSamplerDefinitions(std::vector<KSLVariableDefinitionsNode*> &samplers, bool remove)
{
	CollectVariableDefinitions(samplers, remove, KSL_STORAGE_UNIFORM, KSL_TYPECLASS_SAMPLER);
}


void KSLProgramAST::CollectBufferDefinitions(std::vector<KSLVariableDefinitionsNode*> &buffers, bool remove)
{
	CollectVariableDefinitions(buffers, remove, KSL_STORAGE_BUFFER, KSL_TYPECLASS_ALL);
}

void KSLProgramAST::CollectSharedDefinitions(std::vector<KSLVariableDefinitionsNode*> &shareds, bool remove)
{
	CollectVariableDefinitions(shareds, remove, KSL_STORAGE_SHARED, KSL_TYPECLASS_ALL);
}


void KSLProgramAST::CollectSubpassDefinitions(std::vector<KSLVariableDefinitionsNode*> &subpasses, bool remove)
{
	CollectVariableDefinitions(subpasses, remove, KSL_STORAGE_UNIFORM, KSL_TYPECLASS_SUBPASS_INPUT);
}


void KSLProgramAST::CollectImageDefinitions(std::vector<KSLImageDefinitionNode*> &images, bool remove)
{
	std::vector<KSLASTNode*> new_nodes;

	for (size_t i = 0; i < root->nodes.size(); i++)
	{
		KSLASTNode *n = root->nodes[i];

		if (n->node_type == KSL_NODE_IMAGE_DEFINITION)
		{
			images.push_back((KSLImageDefinitionNode*)n);
		}
		else
		{
			new_nodes.push_back(n);
		}
	}

	if (remove)
	{
		root->nodes = new_nodes;
	}
}


void KSLProgramAST::CollectVariableDefinitions(std::vector<KSLVariableDefinitionsNode*> &vardef, bool remove, KSLStorageQualifier sq, KSLBaseTypeClass btc)
{
	std::vector<KSLASTNode*> new_nodes;

	for (size_t i = 0; i < root->nodes.size(); i++)
	{
		KSLASTNode *n = root->nodes[i];

		bool collect = false;

		if (n->node_type == KSL_NODE_VARIABLE_DEFINITIONS)
		{
			KSLVariableDefinitionsNode *vd = (KSLVariableDefinitionsNode*)n;

			collect = true;
			collect &= (vd->storage_type == sq);
			collect &= ((vd->variable_type.GetTypeClass() & btc) != 0);
		}

		if (collect)
		{
			vardef.push_back((KSLVariableDefinitionsNode*)n);
		}
		else
		{
			new_nodes.push_back(n);
		}
	}

	if (remove)
	{
		root->nodes = new_nodes;
	}
}


uint32_t KSLProgramAST::GetMainFunctionNodeId()
{
	if (m_main_function_node_id != KSL_UINT32_MAX) return m_main_function_node_id;

	for (size_t i = 0; i < root->nodes.size(); i++)
	{
		KSLASTNode *n = root->nodes[i];

		if (n->node_type == KSL_NODE_FUNCTION)
		{
			KSLFunctionNode* fn = (KSLFunctionNode*)n;
			KSLFunction &f = functions[fn->function_id];

			if (f.function_name == "main")
			{
				m_main_function_node_id = (uint32_t)i;
				break;
			}
		}
	}

	return m_main_function_node_id;
}


std::string KSLProgramAST::GetTypeName(KSLType type)
{
	if (type.id < KSL_NUM_INBUILT_TYPES)
	{
		return type.ToString();
	}
	else // get user type name
	{
		return user_types[type.id - KSL_NUM_INBUILT_TYPES]->name;
	}

	assert(0);
	return "error_type";
}


std::vector<KSLASTNode*> KSL::GetChildren(KSLASTNode* n)
{
	std::vector<KSLASTNode*> children;
	KSL::GetChildren(n, children);
	return children;
}


bool KSL::GetChildren(KSLASTNode* n, std::vector<KSLASTNode*> &children)
{
	children.clear();
	switch (n->node_type)
	{

	case KSL_NODE_FUNCTION:
	{
		KSLFunctionNode *fn = (KSLFunctionNode*)n;
		children.push_back(fn->body);
		break;
	}

	case KSL_NODE_BLOCK_STATEMENT:
	{
		KSLBlockStatementNode *bsn = (KSLBlockStatementNode*)n;
		children = bsn->nodes;
		break;
	}

	case KSL_NODE_EXPRESSION_STATEMENT:
	{
		KSLExpressionOrEmptyStatementNode *eesn = (KSLExpressionOrEmptyStatementNode*)n;
		if (eesn->expression != NULL) children.push_back(eesn->expression);
		break;
	}

	case KSL_NODE_FUNCTION_CALL_EXPRESSION:
	{
		KSLFunctionCallExpressionNode* fcen = (KSLFunctionCallExpressionNode*)n;
		for (size_t i = 0; i < fcen->arguments.size(); i++)
		{
			children.push_back(fcen->arguments[i]);
		}
		break;
	}

	case KSL_NODE_BINOP_EXPRESSION:
	{
		KSLBinaryExpressionNode* ben = (KSLBinaryExpressionNode*)n;
		children.push_back(ben->e1);
		children.push_back(ben->e2);
		break;
	}

	case KSL_NODE_PARENTHESIS_EXPRESSION:
	{
		KSLParenthesisExpressionNode* pen = (KSLParenthesisExpressionNode*)n;
		children.push_back(pen->expression);
		break;
	}

	case KSL_NODE_CONSTRUCTOR_EXPRESSION:
	{
		KSLConstructorExpressionNode* cen = (KSLConstructorExpressionNode*)n;
		for (size_t i = 0; i < cen->initializers.size(); i++)
		{
			children.push_back(cen->initializers[i]);
		}
		break;
	}

	case KSL_NODE_RETURN_STATEMENT:
	{
		KSLReturnStatementNode* rsn = (KSLReturnStatementNode*)n;
		if (rsn->expression != NULL) children.push_back(rsn->expression);
		break;
	}

	case KSL_NODE_VARIABLE_DEFINITIONS:
	{
		KSLVariableDefinitionsNode* vardef = (KSLVariableDefinitionsNode*)n;
		for (size_t i = 0; i < vardef->variables.size(); i++)
		{
			KSLASTNode *se = vardef->variables[i].size_expression;
			if (se != NULL) children.push_back(se);

			for (size_t j = 0; j < vardef->variables[i].init_expressions.size(); j++)
			{
				children.push_back(vardef->variables[i].init_expressions[j]);
			}
		}
		break;
	}

	case KSL_NODE_ARRAY_ACCESS_EXPRESSION:
	case KSL_NODE_MEMBER_ACCESS_EXPRESSION:
	{
		KSLSelectorExpressionNode* sen = (KSLSelectorExpressionNode*)n;
		children.push_back(sen->expression);
		if (sen->id_expression != NULL) children.push_back(sen->id_expression);
		break;
	}

	case KSL_NODE_TERNARY_CONDITIONAL_EXPRESSION:
	{
		KSLTernaryConditionalExpressionNode* tcen = (KSLTernaryConditionalExpressionNode*)n;
		children.push_back(tcen->condition);
		children.push_back(tcen->if_expression);
		children.push_back(tcen->else_expression);
		break;
	}

	case KSL_NODE_FOR_STATEMENT:
	{
		KSLForStatementNode* fsn = (KSLForStatementNode*)n;
		if (fsn->init_node != NULL) children.push_back(fsn->init_node);
		if (fsn->conditional_statement != NULL) children.push_back(fsn->conditional_statement);
		if (fsn->step_statement != NULL) children.push_back(fsn->step_statement);
		children.push_back(fsn->body);
		break;
	}

	case KSL_NODE_UNARY_OP_EXPRESSION:
	case KSL_NODE_SUFFIX_EXPRESSION:
	{
		KSLUnaryExpressionNode* uen = (KSLUnaryExpressionNode*)n;
		children.push_back(uen->expression);
		break;
	}

	case KSL_NODE_IF_STATEMENT:
	{
		KSLIfStatementNode* isn = (KSLIfStatementNode*)n;
		children.push_back(isn->condition);
		children.push_back(isn->if_statement);
		if (isn->else_statement != NULL) children.push_back(isn->else_statement);
		break;
	}

	case KSL_NODE_PROGRAM:
	{
		KSLProgramNode* pn = (KSLProgramNode*)n;
		children = pn->nodes;
		break;
	}

	case KSL_NODE_EMPTY_STATEMENT:
	case KSL_NODE_LITERAL_EXPRESSION:
	case KSL_NODE_BREAK_STATEMENT:
	case KSL_NODE_CONTINUE_STATEMENT:
	case KSL_NODE_NUMTHREAD:
	case KSL_NODE_VARIABLE_EXPRESSION:
	case KSL_NODE_DISCARD_STATEMENT:
	case KSL_NODE_IMAGE_DEFINITION:
		break;

	default:
		assert(0);
		return false;
		break;
	}
	return true;
}


bool KSL::Traverse(KSLASTNode* n, KSLNodeVisitor &nv)
{
	bool t = nv.visit(n);

	if (t)
	{
		std::vector<KSLASTNode*> children;
		KSL::GetChildren(n, children);

		for (size_t i = 0; i < children.size(); i++)
		{
			KSL::Traverse(children[i], nv);
		}
	}

	return true;
}


std::string KSL::BinopToString(KSLBinaryOperation binop)
{
	switch (binop)
	{
	case KSL_BINOP_ASSIGN:             return " = ";  
	case KSL_BINOP_SUB:                return "-";    
	case KSL_BINOP_ADD:                return "+";    
	case KSL_BINOP_MUL:                return "*";    
	case KSL_BINOP_DIV:                return "/";    
	case KSL_BINOP_MOD:                return "%";    
	case KSL_BINOP_ADD_ASSIGN:         return " += "; 
	case KSL_BINOP_SUB_ASSIGN:         return " -= "; 
	case KSL_BINOP_MUL_ASSIGN:         return " *= "; 
	case KSL_BINOP_DIV_ASSIGN:         return " /= "; 
	case KSL_BINOP_LESS:               return " < ";  
	case KSL_BINOP_GREATER:            return " > ";  
	case KSL_BINOP_LESSEQUAL:          return " <= "; 
	case KSL_BINOP_GREATEREQUAL:       return " >= "; 
	case KSL_BINOP_LAZY_AND:           return " && "; 
	case KSL_BINOP_LAZY_OR:            return " || "; 
	case KSL_BINOP_SHIFT_LEFT:         return " << "; 
	case KSL_BINOP_SHIFT_RIGHT:        return " >> "; 
	case KSL_BINOP_EQUAL:              return " == "; 
	case KSL_BINOP_NOTEQUAL:           return " != "; 
	case KSL_BINOP_BITWISE_AND:        return " & ";  
	case KSL_BINOP_BITWISE_OR:         return " | ";  
	case KSL_BINOP_SHIFT_LEFT_ASSIGN:  return " <<= ";
	case KSL_BINOP_SHIFT_RIGHT_ASSIGN: return " >>= ";
	default:
		break;
	}

	assert(0);
	return " error_binop ";
}


bool KSL::BinopIsAssing(KSLBinaryOperation binop)
{
	switch (binop)
	{
	case KSL_BINOP_ASSIGN:            
	case KSL_BINOP_ADD_ASSIGN:        
	case KSL_BINOP_SUB_ASSIGN:        
	case KSL_BINOP_MUL_ASSIGN:        
	case KSL_BINOP_DIV_ASSIGN:        
	case KSL_BINOP_SHIFT_LEFT_ASSIGN: 
	case KSL_BINOP_SHIFT_RIGHT_ASSIGN:
		return true;

	case KSL_BINOP_SUB:         
	case KSL_BINOP_ADD:         
	case KSL_BINOP_MUL:         
	case KSL_BINOP_DIV:         
	case KSL_BINOP_MOD:         
	case KSL_BINOP_LESS:        
	case KSL_BINOP_GREATER:     
	case KSL_BINOP_LESSEQUAL:   
	case KSL_BINOP_GREATEREQUAL:
	case KSL_BINOP_LAZY_AND:    
	case KSL_BINOP_LAZY_OR:     
	case KSL_BINOP_SHIFT_LEFT:  
	case KSL_BINOP_SHIFT_RIGHT: 
	case KSL_BINOP_EQUAL:       
	case KSL_BINOP_NOTEQUAL:    
	case KSL_BINOP_BITWISE_AND: 
	case KSL_BINOP_BITWISE_OR:
		return false;

	default:
		break;
	}

	assert(0);
	return true;
}


std::string KSLProgramAST::GetQualifierStr(KSLAttribQualifier qualifier)
{
	switch (qualifier)
	{
	case KSL_ATTRIB_QUALIFIER_COLOR: return "color";
	case KSL_ATTRIB_QUALIFIER_DEPTH: return "depth";
	case KSL_ATTRIB_QUALIFIER_SSBO: return "ssbo";
	case KSL_ATTRIB_QUALIFIER_READONLY: return "readonly";
	case KSL_ATTRIB_QUALIFIER_WRITEONLY: return "writeonly";

	case KSL_ATTRIB_QUALIFIER_RGBA8: return "rgba8";
	case KSL_ATTRIB_QUALIFIER_RGBA16F: return "rgba16f";

	default:
		break;
	}

	assert(0);
	return "error_qualifier";
}


std::string KSL::StorageQualifierToString(KSLStorageQualifier qualifier)
{
	switch (qualifier)
	{
	case KSL_STORAGE_IN:      return "in ";
	case KSL_STORAGE_OUT:     return "out ";
	case KSL_STORAGE_UNIFORM: return "uniform ";
	case KSL_STORAGE_CONST:   return "const ";
	case KSL_STORAGE_BUFFER:  return "buffer ";
	case KSL_STORAGE_SHARED:  return "shared ";
	case KSL_STORAGE_DEFAULT: return "";
	default: assert(0); return "error_qualifer ";
	}
}


