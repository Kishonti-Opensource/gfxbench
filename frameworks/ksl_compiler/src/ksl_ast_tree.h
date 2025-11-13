/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_AST_TREE__
#define __KSL_AST_TREE__

#include <cstdint>
#include <vector>
#include <set>
#include <map>

#include "ksl_common.h" 


enum KSLStorageQualifier
{
	KSL_STORAGE_INVALID,
	KSL_STORAGE_DEFAULT,
	KSL_STORAGE_IN,
	KSL_STORAGE_OUT,
	KSL_STORAGE_UNIFORM,
	KSL_STORAGE_CONST,
	KSL_STORAGE_BUFFER,
	KSL_STORAGE_SHARED
};


enum KSLASTNodeType
{
	KSL_NODE_INVALID,
	KSL_NODE_ARRAY_ACCESS_EXPRESSION,
	KSL_NODE_BINOP_EXPRESSION,
	KSL_NODE_BLOCK_STATEMENT,
	KSL_NODE_BREAK_STATEMENT,
	KSL_NODE_CONSTRUCTOR_EXPRESSION,
	KSL_NODE_CONTINUE_STATEMENT,
	KSL_NODE_DISCARD_STATEMENT,
	KSL_NODE_EMPTY_STATEMENT,
	KSL_NODE_EXPRESSION_STATEMENT,
	KSL_NODE_FOR_STATEMENT,
	KSL_NODE_FUNCTION,
	KSL_NODE_FUNCTION_CALL_EXPRESSION,
	KSL_NODE_IF_STATEMENT,
	KSL_NODE_IMAGE_DEFINITION,
	KSL_NODE_LITERAL_EXPRESSION,
	KSL_NODE_MEMBER_ACCESS_EXPRESSION,
	KSL_NODE_NUMTHREAD,
	KSL_NODE_PARENTHESIS_EXPRESSION,
	KSL_NODE_PROGRAM,
	KSL_NODE_RETURN_STATEMENT,
	KSL_NODE_SUFFIX_EXPRESSION,
	KSL_NODE_STRUCT_DEFINITION,
	KSL_NODE_TERNARY_CONDITIONAL_EXPRESSION,
	KSL_NODE_VARIABLE_DEFINITIONS,
	KSL_NODE_VARIABLE_EXPRESSION,
	KSL_NODE_UNARY_OP_EXPRESSION,
	KSL_NUM_NODES
};


enum KSLBinaryOperation
{
	KSL_BINOP_ASSIGN,
	KSL_BINOP_ADD,
	KSL_BINOP_SUB,
	KSL_BINOP_MUL,
	KSL_BINOP_DIV,
	KSL_BINOP_MOD,
	KSL_BINOP_ADD_ASSIGN,
	KSL_BINOP_SUB_ASSIGN,
	KSL_BINOP_MUL_ASSIGN,
	KSL_BINOP_DIV_ASSIGN,
	KSL_BINOP_SHIFT_LEFT,
	KSL_BINOP_SHIFT_RIGHT,
	KSL_BINOP_LESS,
	KSL_BINOP_GREATER,
	KSL_BINOP_LESSEQUAL,
	KSL_BINOP_GREATEREQUAL,
	KSL_BINOP_EQUAL,
	KSL_BINOP_NOTEQUAL,
	KSL_BINOP_LAZY_AND,
	KSL_BINOP_LAZY_OR,
	KSL_BINOP_BITWISE_AND,
	KSL_BINOP_BITWISE_OR,
	KSL_BINOP_SHIFT_LEFT_ASSIGN,
	KSL_BINOP_SHIFT_RIGHT_ASSIGN,
	KSL_NUM_BINOP
};


enum KSLAccess
{
	KSL_ACCESS_READ_ONLY,
	KSL_ACCESS_WRITE_ONLY,
	KSL_ACCESS_READ_WRITE
};


enum KSLAttribAccess
{
	KSL_ATTRIB_ACCESS_IN,
	KSL_ATTRIB_ACCESS_OUT,
	KSL_ATTRIB_ACCESS_INOUT
};


enum KSLUnaryOperation
{
	KSL_UNOP_PLUS,
	KSL_UNOP_MINUS,
	KSL_UNOP_INCREMENT,
	KSL_UNOP_DECREMENT,
	KSL_UNOP_LOGICAL_NOT,
	KSL_NUM_UNOP
};


enum KSLAttribQualifier
{
	KSL_ATTRIB_QUALIFIER_INVALID,
	KSL_ATTRIB_QUALIFIER_COLOR,
	KSL_ATTRIB_QUALIFIER_DEPTH,
	KSL_ATTRIB_QUALIFIER_SSBO,
	KSL_ATTRIB_QUALIFIER_READONLY,
	KSL_ATTRIB_QUALIFIER_WRITEONLY,

	KSL_ATTRIB_QUALIFIER_RGBA8,
	KSL_ATTRIB_QUALIFIER_RGBA16F,
	
	KSL_NUM_ATTRIB_QUALIFIER
};


struct KSLVariable
{
	KSLVariable()
	{
		access = KSL_ACCESS_READ_WRITE;
		storage_type = KSL_STORAGE_INVALID;
		used = false;
	}

	KSLVariable(const std::string &name, const KSLType type)
	{
		new_name = orig_name = name;
		this->type = type;
		access = KSL_ACCESS_READ_WRITE;
		storage_type = KSL_STORAGE_DEFAULT;
		used = false;
	}

	KSLType type;
	std::string orig_name;
	std::string new_name;
	KSLAccess access;
	KSLStorageQualifier storage_type;
	bool used;
};


struct KSLFunction
{
	KSLType return_type;
	std::string function_name;
	std::vector<KSLType> attrib_type;
	std::vector<KSLAttribAccess> attrib_access;

	KSLFunction();
	KSLFunction(const std::string &name, const KSLType rt);
	KSLFunction(const std::string &name, const KSLType rt, const KSLType at);
	KSLFunction(const std::string &name, const KSLType rt, const KSLType at1, const KSLType at2);
	KSLFunction(const std::string &name, const KSLType rt, const KSLType at1, const KSLType at2, const KSLType at3);
	KSLFunction(const std::string &name, const KSLType rt, const KSLType at1, const KSLType at2, const KSLType at3, const KSLType at4);
};


// Base Node
struct KSLASTNode
{
	KSLASTNode();
	virtual ~KSLASTNode();

	uint32_t node_type;
	uint32_t start_line;
	uint32_t end_line;
	uint32_t start_column;
};


//
//
//	Expressions
//
//

struct KSLExpressionNode : public KSLASTNode
{
	KSLExpressionNode();

	KSLType type;
	KSLOperationPrecedence precedence;
	bool l_value;
	bool constant_value;
};


struct KSLLiteralExpressionNode : public KSLExpressionNode
{
	KSLLiteralExpressionNode();

	double float_value;
	uint64_t int_value;
	bool bool_value;
	std::string str_value;
};


struct KSLVariableExpressionNode : public KSLExpressionNode
{
	KSLVariableExpressionNode();

	uint32_t variable_id;
};


struct KSLConstructorExpressionNode : public KSLExpressionNode
{
	KSLConstructorExpressionNode();
	~KSLConstructorExpressionNode();

	KSLType constructor_type;
	std::vector<KSLExpressionNode*> initializers;
};


struct KSLSelectorExpressionNode : public KSLExpressionNode
{
	KSLSelectorExpressionNode();
	~KSLSelectorExpressionNode();

	KSLExpressionNode* expression;
	KSLExpressionNode* id_expression;
	std::string selector;
};


struct KSLParenthesisExpressionNode : public KSLExpressionNode
{
	KSLParenthesisExpressionNode();
	~KSLParenthesisExpressionNode();

	KSLExpressionNode* expression;
};


struct KSLUnaryExpressionNode : public KSLExpressionNode
{
	KSLUnaryExpressionNode();
	~KSLUnaryExpressionNode();

	KSLUnaryOperation operation;
	KSLExpressionNode *expression;
};


struct KSLBinaryExpressionNode : public KSLExpressionNode
{
	KSLBinaryExpressionNode();
	~KSLBinaryExpressionNode();

	KSLBinaryOperation operation;

	KSLExpressionNode *e1;
	KSLExpressionNode *e2;
};


struct KSLFunctionCallExpressionNode : public KSLExpressionNode
{
	KSLFunctionCallExpressionNode();
	~KSLFunctionCallExpressionNode();

	uint32_t function_id;
	std::string name;
	std::vector<KSLExpressionNode*> arguments;
};


struct KSLTernaryConditionalExpressionNode : public KSLExpressionNode
{
	KSLTernaryConditionalExpressionNode();
	~KSLTernaryConditionalExpressionNode();

	KSLExpressionNode *condition;
	KSLExpressionNode *if_expression;
	KSLExpressionNode *else_expression;
};


//
//
//	Statements
//
//

struct KSLExpressionOrEmptyStatementNode : public KSLASTNode
{
	KSLExpressionOrEmptyStatementNode();
	~KSLExpressionOrEmptyStatementNode();

	KSLExpressionNode *expression;
};


struct KSLBlockStatementNode : public KSLASTNode
{
	KSLBlockStatementNode();
	~KSLBlockStatementNode();

	std::vector<KSLASTNode*> nodes;
};


struct KSLIfStatementNode : public KSLASTNode
{
	KSLIfStatementNode();
	~KSLIfStatementNode();

	KSLExpressionNode *condition;
	KSLASTNode *if_statement;
	KSLASTNode *else_statement;
};


struct KSLForStatementNode : public KSLASTNode
{
	KSLForStatementNode();
	~KSLForStatementNode();

	KSLASTNode* init_node;
	KSLExpressionNode* conditional_statement;
	KSLExpressionNode* step_statement;
	KSLASTNode* body;
	bool is_loop;
};

struct KSLControlStatementNode : public KSLASTNode
{
	KSLControlStatementNode();
};


struct KSLReturnStatementNode : public KSLASTNode
{
	KSLReturnStatementNode();
	~KSLReturnStatementNode();

	KSLExpressionNode* expression;
};


//
//
//	Other
//
//

struct KSLVariableDefinitionsNode : public KSLASTNode
{
	struct InnerNode
	{
		InnerNode()
			: variable_id(0)
			, size_expression(NULL)
			, array_size(-1)
		{

		}

		uint32_t variable_id;
		KSLExpressionNode* size_expression;
		std::vector<KSLExpressionNode*> init_expressions;
		int64_t array_size;
	};

	KSLVariableDefinitionsNode();
	~KSLVariableDefinitionsNode();

	KSLStorageQualifier storage_type;
	KSLType variable_type;

	std::vector<InnerNode> variables;

	std::set<uint32_t> bool_attribs;
	std::map<uint32_t, uint32_t> int_attribs;
};


struct KSLFunctionNode : public KSLASTNode
{
	struct AttribNode
	{
		uint32_t variable_id;
		KSLExpressionNode* size_expression;
	};

	KSLFunctionNode();
	~KSLFunctionNode();

	uint32_t function_id;
	KSLBlockStatementNode* body;
	std::vector<AttribNode> attribs;
};


struct KSLImageDefinitionNode : public KSLASTNode
{
	KSLImageDefinitionNode();

	uint32_t variable_id;

	std::set<uint32_t> bool_attribs;
	std::map<uint32_t, uint32_t> int_attribs;
};


struct KSLNumThreadNode : public KSLASTNode
{
	KSLNumThreadNode();

	uint32_t x, y, z;
};


struct KSLStructDefinitionNode : public KSLASTNode
{
	KSLStructDefinitionNode();
	~KSLStructDefinitionNode();

	std::string name;
	std::vector<KSLVariableDefinitionsNode*> members;
};


struct KSLProgramNode : public KSLASTNode
{
	KSLProgramNode();
	virtual ~KSLProgramNode();

	std::vector<KSLASTNode*> nodes;
};


class KSLProgramAST
{
public:
	KSLProgramAST();
	~KSLProgramAST();

	void CollectSamplerDefinitions(std::vector<KSLVariableDefinitionsNode*> &samplers, bool remove);
	void CollectBufferDefinitions(std::vector<KSLVariableDefinitionsNode*> &buffers, bool remove);
	void CollectSharedDefinitions(std::vector<KSLVariableDefinitionsNode*> &shareds, bool remove);
	void CollectImageDefinitions(std::vector<KSLImageDefinitionNode*> &images, bool remove);
	void CollectSubpassDefinitions(std::vector<KSLVariableDefinitionsNode*> &subpasses, bool remove);

	void CollectVariableDefinitions(std::vector<KSLVariableDefinitionsNode*> &vardef, bool remove, KSLStorageQualifier sq, KSLBaseTypeClass btc);

	uint32_t GetMainFunctionNodeId();
	std::string GetTypeName(KSLType type);
	static std::string GetQualifierStr(KSLAttribQualifier qualifier);

	KSLProgramNode *root;
	std::vector<KSLVariable> variables;
	std::vector<KSLFunction> functions;
	std::vector<KSLStructDefinitionNode*> user_types;
	std::vector<KSLExpressionNode*> expressions;

	uint32_t shader_type;

	// reflection
	std::vector<KSLVariable> in_attributes;
	std::vector<KSLVariable> out_attributes;
	std::vector<KSLVariableDefinitionsNode*> uniforms;
	std::vector<KSLVariableDefinitionsNode*> buffers;
	std::vector<KSLVariableDefinitionsNode*> readonly_buffers;
	std::vector<KSLImageDefinitionNode*> readonly_images;

	// inbuilts
	uint32_t inbuilt_vertex_position_variable_id;
	uint32_t inbuilt_vertex_id_variable_id;

	uint32_t inbuilt_compute_globalinvocationid_id;
	uint32_t inbuilt_compute_localinvocationindex_id;
	uint32_t inbuilt_compute_localinvocationid_id;
	uint32_t inbuilt_compute_workgroupid_id;

	uint32_t inbuilt_fragment_fragcoord;
	uint32_t inbuilt_fragment_frontfacing;

	bool force_highp;
	bool has_discard;

private:

	uint32_t m_main_function_node_id;
};


class KSLNodeVisitor
{
public:
	virtual bool visit(KSLASTNode* n) = 0;
};


namespace KSL
{
	std::vector<KSLASTNode*> GetChildren(KSLASTNode* n);
	bool GetChildren(KSLASTNode* n, std::vector<KSLASTNode*> &children);

	bool Traverse(KSLASTNode* n, KSLNodeVisitor &nv);

	std::string BinopToString(KSLBinaryOperation binop);
	bool BinopIsAssing(KSLBinaryOperation binop);

	std::string StorageQualifierToString(KSLStorageQualifier qualifier);
}


#endif // __KSL_AST_TREE__

