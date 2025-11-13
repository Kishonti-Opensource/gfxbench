/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_ANALYZER__
#define __KSL_ANALYZER__

#include "ksl_ast_tree.h"
#include "ksl_tokenizer.h"

#include "ngl.h"


class KSLTokenIterator
{
public:
	std::vector<KSLToken> *m_tokens;

protected:

	// Token Guard
	class KSLTokenGuard
	{
	public:
		KSLTokenGuard()
			: m_pointer(NULL)
			, m_length(NULL)
		{
		}

		KSLTokenGuard(uint32_t *pointer, uint32_t *length)
		{
			accepted = false;
			m_pointer = pointer;
			start_pointer = *pointer;
			m_length = length;
		}

		bool Accept()
		{
			return accepted = true;
		}

		~KSLTokenGuard()
		{
			*m_length = *m_pointer;
			if (!accepted)
			{
				*m_pointer = start_pointer;
			}
		}

	private:
		uint32_t *m_pointer;
		uint32_t *m_length;
		uint32_t start_pointer;
		bool accepted;
	};


	KSLTokenIterator()
		: m_tokens(NULL)
		, m_pointer(0)
	{ }

	bool Match(KSLTokenType t)
	{
		if (m_tokens->at(m_pointer).type == t)
		{
			m_pointer++;
			return true;
		}
		return false;
	}

	KSLToken &T()
	{
		return m_tokens->at(m_pointer - 1);
	}

	KSLToken &TN()
	{
		return m_tokens->at(m_pointer);
	}

	KSLTokenGuard GetTokenGuard(uint32_t *length)
	{
		return KSLTokenGuard(&m_pointer, length);
	}

private:
	uint32_t m_pointer;
};


class KSLAnalyzer : public KSLTokenIterator
{
public:
	const static uint64_t MAX_PER_STAGE_UNIFORM_IN_BYTES = 4096;

	KSLAnalyzer();
	virtual ~KSLAnalyzer();

	bool Analyze(KSLProgramAST &ast);
	bool CreateReflection(NGL_shader_source_descriptor &ssd);

	void Clear();
	std::vector<KSLError> &GetErrors() const
	{
		return *m_errors;
	}

	NGL_shader_type m_shader_type;
    KSLSeverity m_precision_mismatch_severity;

private:
	bool MatchProgram();
	KSLVariableDefinitionsNode* MatchVariableDefinitions(bool global_space);
	bool MatchVariableAttributes(std::set<uint32_t> &bool_attribs, std::map<uint32_t, uint32_t> &int_attribs);
	bool MatchArrayInitializer(std::vector<KSLExpressionNode*> &init_expressions);
	bool MatchType(KSLType &type, KSLToken &type_token);
	KSLFunctionNode* MatchFunction();
	KSLImageDefinitionNode* MatchImageDefinition();
	KSLNumThreadNode* MatchNumThreads();
	KSLStructDefinitionNode* MatchStructDefinition();
	bool MatchForceHighp();

	// statements
	KSLBlockStatementNode* MatchBlockStatement();
	KSLExpressionOrEmptyStatementNode* MatchExpressionOrEmptyStatement();
	KSLForStatementNode* MatchForStatement();
	KSLIfStatementNode *MatchIfStatement();
	KSLControlStatementNode* MatchControlStatement();
	KSLReturnStatementNode* MatchReturnStatement();
	KSLASTNode* MatchStatement();

	// expressions
	KSLLiteralExpressionNode* MatchLiteralExpression();
	KSLVariableExpressionNode* MatchVariableExpression();
	KSLExpressionNode* MatchBinaryExpression(const KSLOperationPrecedence precedence);
	KSLUnaryExpressionNode* MatchUnaryExpression();
	KSLParenthesisExpressionNode* MatchParenthesisExpression();
	KSLConstructorExpressionNode* MatchConstructorExpression();
	KSLExpressionNode* MatchSuffixExpression();
	KSLFunctionCallExpressionNode* MatchFunctionCallExpression();
	KSLExpressionNode* MatchExpression(KSLOperationPrecedence min_precedence);
	KSLExpressionNode* MatchAssignExpression();
	KSLExpressionNode* MatchTernaryConditionalExpression();

	KSLTokenGuard GetTokenGuard()
	{
		return KSLTokenIterator::GetTokenGuard(m_token_run);
	}

	// validation
	bool ValidateProgram();
	bool ValidateNode(KSLASTNode *n);

	bool ValidateVariableDefinitions(KSLVariableDefinitionsNode* vardef);
	bool ValidateFunction(KSLFunctionNode* fn);
	bool ValidateBlockStatement(KSLBlockStatementNode* bsn);
	bool ValidateExpressionStatement(KSLExpressionOrEmptyStatementNode* eesn);
	bool ValidateReturnStatement(KSLReturnStatementNode* rsn);
	bool ValidateIfStatement(KSLIfStatementNode* isn);
	bool ValidateForStatement(KSLForStatementNode* fsn);

	// expressions
	bool ValidateExpression(KSLExpressionNode* e);
	bool ValidateVariableExpression(KSLVariableExpressionNode* ve);
	bool ValidateFunctionCallExpression(KSLFunctionCallExpressionNode* fce);
	bool ValidateBinaryExpression(KSLBinaryExpressionNode* be);
	bool ValidateParenthesisExpression(KSLParenthesisExpressionNode* pe);
	bool ValidateMemberAccessExpression(KSLSelectorExpressionNode* me);
	bool ValidateUnaryOperationExpression(KSLUnaryExpressionNode* ue);
	bool ValidateArrayAccessExpression(KSLSelectorExpressionNode* me);
	bool ValidateTernaryConditionalExpression(KSLTernaryConditionalExpressionNode* tce);
	bool ValidateConstructorExpression(KSLConstructorExpressionNode* ce);

	class KSLUnusedVariableDetector;
	void DetectUnusedVariables();

	KSLFunctionNode* m_validated_function_node;

	void CheckVariableRedefiniton(std::string variable_name);
	bool CheckUniformsAmount();
	bool GetType(const std::string &str, KSLType &type);
	bool GetAttribQualifier(KSLAttribQualifier &qualifer);
	bool HasFunction(const std::string &name);

	void RegisterBuiltInFunctions();
	void RegisterMultiplications();
	void RegisterBuiltInVariables();
	void RegisterBuiltInTypes();
	void RegisterBuiltInQualifiers();

	void RegisterTextureFunctions();

	void RegisterFunctionWithUniformPrecisions(const std::string &name, const KSLBaseType rt, const KSLBaseType at);
	void RegisterFunctionWithUniformPrecisions(const std::string &name, const KSLBaseType rt, const KSLBaseType at1, const KSLBaseType at2);
	void RegisterFunctionWithUniformPrecisions(const std::string &name, const KSLBaseType rt, const KSLBaseType at1, const KSLBaseType at2, const KSLBaseType at3);

	uint32_t RegisterBuiltInVariable(const KSLVariable &v);

	KSLProgramAST *ast;

	std::map<std::string, KSLType> m_types;
	std::map<std::string, KSLAttribQualifier> m_builtin_qualifiers;
	std::vector<uint32_t> m_active_variable_ids;
	KSLBaseType m_multiplication_table[KSL_NUM_INBUILT_TYPES][KSL_NUM_INBUILT_TYPES];

	class StateGuard;


	// reflection
	void CollectReflectionInfo();
	bool EvalArraySize(KSLExpressionNode* e, int64_t &v);

	// forcehighp
	void ForceHighPrecision();


	// error handling
	void ErrorUnexpectedToken();
	void ErrorUnknownType(const KSLToken &type_token);
	void ErrorConstMustBeInitialized(const KSLToken &name_token);
	void ErrorUndeclaredIdentifier(const KSLToken &name_token);
	void ErrorImageAccessMissing(const KSLToken &image_token);
	void ErrorUnableToEvalArraySize(const KSLVariableDefinitionsNode* vardef);
	void ErrorGlobalVariablesNotAllowed(const KSLToken &type_token);
	void ErrorOnlyOneVariableDeclarationAllowd(const KSLToken &type_token);
	void ErrorUnsizedArrayMustBeInitialized(const KSLToken &name_token);
	void ErrorImplicitlySizedUbo(const KSLVariableDefinitionsNode* vardef);
	void ErrorUnsupportedUniformType(const KSLVariableDefinitionsNode* vardef);
	void ErrorUnsupportedUniformArrayType(const KSLVariableDefinitionsNode* vardef);
	void ErrorUnsupportedInOutAttribType(const KSLVariableDefinitionsNode* vardef);
	void ErrorToManyUniforms(uint64_t bytes);
	void ErrorFunctionNotFound(KSLFunctionCallExpressionNode* fce);
	void ErrorBinopTypeMismatch(KSLBinaryExpressionNode* be);
	void ErrorInvalidSelector(KSLSelectorExpressionNode* se);
	void ErrorInvalidArrayAccess(KSLSelectorExpressionNode* se);
	void ErrorBinopAssignLeftOperandNotLValue(KSLBinaryExpressionNode* be);
	void ErrorTernaryExpressionNotBoolConditional(KSLTernaryConditionalExpressionNode *tce);
	void ErrorTernaryExpressionBranchTypeMismatch(KSLTernaryConditionalExpressionNode *tce);
	void ErrorInvalidAttributeQualifier(const KSLVariableDefinitionsNode* vardef, const KSLAttribQualifier qualifer);
	void ErrorReadWriteSSBOInFragmentShader(const KSLVariableDefinitionsNode* vardef);
	void ErrorInvalidConstructorParameters(const KSLConstructorExpressionNode* ce);
	void ErrorInitExpressionTypeMismatch(const KSLVariableDefinitionsNode* vardef, const KSLType init_exp_type);
	void ErrorReturnStatetmantTypeMismatch(const KSLReturnStatementNode* rn, const KSLType func_type, const KSLType exp_type);
	void ErrorInvalidDynamicArrayAccess(KSLSelectorExpressionNode* se);

	void WarningBinopPrecisionMismatch(KSLBinaryExpressionNode* be);
	void WarningFunctionArgumentPrecisionMismatch(KSLFunctionCallExpressionNode* fce);
	void WarningConstructorParametersPrecisionMismatch(KSLConstructorExpressionNode* ce);
	void WarningInitExpressionPrecisionMismatch(const KSLVariableDefinitionsNode* vardef, const KSLType init_exp_type);
	void WarningReturnStatetmantPrecisionMismatch(const KSLReturnStatementNode* rn, const KSLType func_type, const KSLType exp_type);
	void WarningUnusedVariable(const KSLVariableDefinitionsNode* vardef, const KSLVariable &v);
	void WarningIntegerDivision(KSLBinaryExpressionNode* be);

	bool HasErrors();
	void InitErrorBlocks();
	void NewErrorBlock();
	void AppendErrors();
	void CollectErrors();
	void __RemoveLocalErrorBlocks();

	uint32_t *m_token_run;
	std::vector<KSLError> *m_errors;

	std::vector<std::pair<std::uint32_t, std::vector<KSLError>>*> m_error_blocks;
	std::vector<uint32_t> m_error_block_ids;


	// debug
	bool m_mangling_variable_names;
	std::vector<std::string> m_variable_postfix;
	uint32_t m_block_counter;
};


#endif // __KSL_ANALYZER__

