/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_GENERATOR__
#define __KSL_GENERATOR__

#include "ksl_ast_tree.h"
#include "ksl_common.h"
#include "ksl_analyzer.h"
#include "ksl_translator.h"

#include <sstream>


class KSLGenerator
{
public:
	KSLGenerator();
	virtual ~KSLGenerator();

	KSLProgramAST *ast;

	std::vector<KSLError> m_errors;

	virtual bool Generate();
	std::string GetResult() const;
	void Clear();

	virtual void SetTranslator(KSLTranslator* translator) { }

protected:

	virtual bool VisitVariableDefinitions(KSLVariableDefinitionsNode *vardef);
	virtual bool VisitFunction(KSLFunctionNode* function);
	virtual bool VisitImageDefinition(KSLImageDefinitionNode* idn);
	virtual bool VisitNumThreads(KSLNumThreadNode* ntn);
	virtual bool VisitGlobalSpaceNode(KSLASTNode* n);
	bool VisitStructDefinition(KSLStructDefinitionNode* vdn);
	bool VisitProgram(KSLProgramNode *program);

	// expressions
	virtual bool VisitBinaryExpression(KSLBinaryExpressionNode *be);
	bool VisitUnaryExpression(KSLUnaryExpressionNode *ue);
	bool VisitParenthesisExpression(KSLParenthesisExpressionNode *pe);
	virtual bool VisitVariableExpression(KSLVariableExpressionNode *ve);
	virtual bool VisitLiteralExpression(KSLLiteralExpressionNode *le);
	virtual bool VisitConstructorExpression(KSLConstructorExpressionNode *ce);
	virtual bool VisitSelectorExpression(KSLSelectorExpressionNode *se);
	virtual bool VisitFunctionCallExpression(KSLFunctionCallExpressionNode* fce);
	bool VisitSuffixExpression(KSLUnaryExpressionNode *se);
	bool VisitTernaryConditionalExpression(KSLTernaryConditionalExpressionNode *tce);
	bool VisitExpression(KSLExpressionNode *e);

	// statements
	bool VisitExpressionStatement(KSLExpressionOrEmptyStatementNode* es);
	bool VisitBlockStatement(KSLBlockStatementNode *bs);
	bool VisitIfStatement(KSLIfStatementNode *is);
	virtual bool VisitForStatement(KSLForStatementNode *fs);
	virtual bool VisitControlStatement(KSLControlStatementNode* cs);
	virtual bool VisitReturnStatement(KSLReturnStatementNode* rs);
	virtual bool VisitStatementOrVariableDefinitions(KSLASTNode *n);

	bool PrintVariableAttributes(const std::set<uint32_t> &bool_attribs, const std::map<uint32_t, uint32_t> &int_attribs);
	bool PrintFloatLiteral(KSLLiteralExpressionNode* le, bool force_highp);

	void SyncLine(KSLASTNode* node);
	void NewLine();
	void PrintfExpLeftParent();
	void PrintfExpRightParent();

	virtual std::string TypeToString(KSLType type);
	virtual std::string StorageQualifierToString(KSLStorageQualifier qualifier);
	virtual std::string TranslateAttribQualifer(uint32_t qualifier);

	std::stringstream m_result;

	uint32_t m_line;
	int32_t m_indent;
	bool m_do_syncline;
	bool m_print_literals_from_string;
	bool m_print_expression_parenthesis;
};


// forcehighp => precision = HIGHP
#define CHECK_FORCE_HIGHP(TYPE) assert( \
		!ast->force_highp || \
        TYPE.IsArray() || \
        TYPE.IsBool() || \
        TYPE.id == KSL_TYPE_VOID || \
		(TYPE.precision == KSL_PRECISION_HIGH));


#endif //__KSL_GENERATOR__

