/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_analyzer.h"

#include <sstream>
#include <assert.h>


typedef std::pair<uint32_t, std::vector<KSLError>> KSLErrorBlock;


void KSLAnalyzer::ErrorUnexpectedToken()
{
	std::stringstream sstream;
	sstream << "Unexpected token at: " << TN().line << ":" << TN().column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, TN().line, TN().column, sstream.str()));
}


void KSLAnalyzer::ErrorUnknownType(const KSLToken &type_token)
{
	std::stringstream sstream;
	sstream << "unknown type (" << type_token.str_value << ") at " << type_token.line << ":" << type_token.column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, type_token.line, type_token.column, sstream.str()));
}


void KSLAnalyzer::ErrorConstMustBeInitialized(const KSLToken &name_token)
{
	std::stringstream sstream;
	sstream << "const variable must be initialized at (" << name_token.str_value << ") at " << name_token.line << ":" << name_token.column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, name_token.line, name_token.column, sstream.str()));
}


void KSLAnalyzer::ErrorUndeclaredIdentifier(const KSLToken &name_token)
{
	std::stringstream sstream;
	sstream << "undeclared identifier \"" << name_token.str_value << "\" at " << name_token.line << ":" << name_token.column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, name_token.line, name_token.column, sstream.str()));
}


void KSLAnalyzer::ErrorImageAccessMissing(const KSLToken &image_token)
{
	std::stringstream sstream;
	sstream << "image access type must be specified at (" << image_token.str_value << ") at " << image_token.line << ":" << image_token.column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, image_token.line, image_token.column, sstream.str()));
}


void KSLAnalyzer::ErrorUnableToEvalArraySize(const KSLVariableDefinitionsNode* vardef)
{
	std::stringstream sstream;
	KSLVariable &v = ast->variables[vardef->variables[0].variable_id];

	sstream << "unable to eval array size: " << v.orig_name;
	sstream << " defined at " << vardef->start_line << ":" << vardef->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, vardef->start_line, vardef->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorGlobalVariablesNotAllowed(const KSLToken &type_token)
{
	std::stringstream sstream;
	sstream << "only const global variables allowed! " << type_token.line << ":" << type_token.column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, type_token.line, type_token.column, sstream.str()));
}


void KSLAnalyzer::ErrorOnlyOneVariableDeclarationAllowd(const KSLToken &type_token)
{
	std::stringstream sstream;
	sstream << "only one interface variable allowed per variable defintionot! " << type_token.line << ":" << type_token.column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, type_token.line, type_token.column, sstream.str()));
}


void KSLAnalyzer::ErrorUnsizedArrayMustBeInitialized(const KSLToken &name_token)
{
	std::stringstream sstream;
	sstream << "unsized array must be initialized at (" << name_token.str_value << ") at " << name_token.line << ":" << name_token.column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, name_token.line, name_token.column, sstream.str()));
}


void KSLAnalyzer::ErrorImplicitlySizedUbo(const KSLVariableDefinitionsNode* vardef)
{
	std::stringstream sstream;
	sstream << "implicitly sized array in ubo not allowed " << vardef->start_line << ":" << vardef->start_column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, vardef->start_line, vardef->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorUnsupportedUniformType(const KSLVariableDefinitionsNode* vardef)
{
	std::stringstream sstream;
	sstream << "unsupported uniform type (only 1, 2, 4, 16 component allowed) " << vardef->start_line << ":" << vardef->start_column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, vardef->start_line, vardef->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorUnsupportedUniformArrayType(const KSLVariableDefinitionsNode* vardef)
{
	std::stringstream sstream;
	sstream << "unsupported uniform array type (only 4 component allowed) " << vardef->start_line << ":" << vardef->start_column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, vardef->start_line, vardef->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorToManyUniforms(uint64_t bytes)
{
	std::stringstream sstream;
	sstream << "Too many uniforms in stage: " << bytes << " bytes. (" << MAX_PER_STAGE_UNIFORM_IN_BYTES << " bytes allowed).";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, 0, 0, sstream.str()));
}


void KSLAnalyzer::ErrorUnsupportedInOutAttribType(const KSLVariableDefinitionsNode* vardef)
{
	std::stringstream sstream;
	sstream << "unsupported attrib type " << vardef->start_line << ":" << vardef->start_column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, vardef->start_line, vardef->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorFunctionNotFound(KSLFunctionCallExpressionNode* fce)
{
	std::stringstream sstream;
	sstream << "function overload not found: " << fce->name << "(";

	for (size_t i = 0; i < fce->arguments.size(); i++)
	{
		sstream << " <" << ast->GetTypeName(fce->arguments[i]->type) << ">";
		sstream << ((i < fce->arguments.size() - 1)?",":" ");
	}

	sstream << ") at (" << fce->start_line << ":" << fce->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, fce->start_line, fce->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorBinopTypeMismatch(KSLBinaryExpressionNode* be)
{
	std::stringstream sstream;
	sstream << "type mismatch for operation: \"" << KSL::BinopToString(be->operation) << "\" ";
	sstream << "<" << ast->GetTypeName(be->e1->type) << ">, <" << ast->GetTypeName(be->e2->type) << "> ";
	sstream << "at (" << be->start_line << ":" << be->start_column <<")";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, be->start_line, be->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorInvalidSelector(KSLSelectorExpressionNode* se)
{
	std::stringstream sstream;
	sstream << "invalid selector: \"" << se->selector << "\" ";
	sstream << "for type <" << ast->GetTypeName(se->expression->type) << "> ";
	sstream << "at (" << se->start_line << ":" << se->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, se->start_line, se->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorInvalidArrayAccess(KSLSelectorExpressionNode* se)
{
	std::stringstream sstream;
	sstream << "invalid array access: <" << ast->GetTypeName(se->id_expression->type) << "> ";
	sstream << "at (" << se->start_line << ":" << se->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, se->start_line, se->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorBinopAssignLeftOperandNotLValue(KSLBinaryExpressionNode* be)
{
	std::stringstream sstream;
	sstream << "assign operation left operand is not l-value ";
	sstream << "at (" << be->start_line << ":" << be->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, be->start_line, be->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorTernaryExpressionNotBoolConditional(KSLTernaryConditionalExpressionNode *tce)
{
	std::stringstream sstream;
	sstream << "Ternary operation conditinal is not bool: ";
	sstream << "<" << ast->GetTypeName(tce->condition->type) << "> ? <" << ast->GetTypeName(tce->if_expression->type) << "> : <" << ast->GetTypeName(tce->else_expression->type) << "> ";
	sstream << "at (" << tce->start_line << ":" << tce->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, tce->start_line, tce->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorTernaryExpressionBranchTypeMismatch(KSLTernaryConditionalExpressionNode *tce)
{
	std::stringstream sstream;
	sstream << "Ternary operation branch type mismatch: ";
	sstream << "<" << ast->GetTypeName(tce->condition->type) << "> ? <" << ast->GetTypeName(tce->if_expression->type) << "> : <" << ast->GetTypeName(tce->else_expression->type) << "> ";
	sstream << "at (" << tce->start_line << ":" << tce->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, tce->start_line, tce->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorInvalidAttributeQualifier(const KSLVariableDefinitionsNode* vardef, const KSLAttribQualifier qualifer)
{
	std::stringstream sstream;
	sstream << "Invalid attribute qualifier: \"" << ast->GetQualifierStr(qualifer) << "\" at " << vardef->start_line << ":" << vardef->start_column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, vardef->start_line, vardef->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorReadWriteSSBOInFragmentShader(const KSLVariableDefinitionsNode* vardef)
{
	std::stringstream sstream;
	sstream << "Readwrite ssbo in fragment shader not allowed. at " << vardef->start_line << ":" << vardef->start_column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, vardef->start_line, vardef->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorInvalidConstructorParameters(const KSLConstructorExpressionNode* ce)
{
	std::stringstream sstream;
	sstream << "invalid constructor parameters for: " << ast->GetTypeName(ce->constructor_type) << "(";

	for (size_t i = 0; i < ce->initializers.size(); i++)
	{
		sstream << " <" << ast->GetTypeName(ce->initializers[i]->type) << ">";
		sstream << ((i < ce->initializers.size() - 1) ? "," : " ");
	}

	sstream << ") at (" << ce->start_line << ":" << ce->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, ce->start_line, ce->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorInitExpressionTypeMismatch(const KSLVariableDefinitionsNode* vardef, const KSLType init_exp_type)
{
	std::stringstream sstream;
	sstream << "Init expression type mismatch: \"" << ast->GetTypeName(init_exp_type) << "\" expected: \"";
	sstream << ast->GetTypeName(vardef->variable_type) << "\" at " << vardef->start_line << ":" << vardef->start_column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, vardef->start_line, vardef->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorReturnStatetmantTypeMismatch(const KSLReturnStatementNode* rn, const KSLType func_type, const KSLType exp_type)
{
	std::stringstream sstream;
	sstream << "return statement type mismatch: \"" << ast->GetTypeName(exp_type) << "\" expected: \"";
	sstream << ast->GetTypeName(func_type) << "\" at " << rn->start_line << ":" << rn->start_column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, rn->start_line, rn->start_column, sstream.str()));
}


void KSLAnalyzer::WarningBinopPrecisionMismatch(KSLBinaryExpressionNode* be)
{
	std::stringstream sstream;
	sstream << "type precision mismatch for operation (result precision undefined): \"" << KSL::BinopToString(be->operation) << "\" ";
	sstream << "<" << ast->GetTypeName(be->e1->type) << ">, <" << ast->GetTypeName(be->e2->type) << "> ";
	sstream << "at (" << be->start_line << ":" << be->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, m_precision_mismatch_severity, be->start_line, be->start_column, sstream.str()));
}


void KSLAnalyzer::WarningFunctionArgumentPrecisionMismatch(KSLFunctionCallExpressionNode* fce)
{
	std::stringstream sstream;
	sstream << "function overload precision mismatch (result precision undefined): " << fce->name << "(";

	for (size_t i = 0; i < fce->arguments.size(); i++)
	{
		sstream << " <" << ast->GetTypeName(fce->arguments[i]->type) << ">";
		sstream << ((i < fce->arguments.size() - 1) ? "," : " ");
	}

	sstream << ") at (" << fce->start_line << ":" << fce->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, m_precision_mismatch_severity, fce->start_line, fce->start_column, sstream.str()));
}


void KSLAnalyzer::WarningConstructorParametersPrecisionMismatch(KSLConstructorExpressionNode* ce)
{
	std::stringstream sstream;
	sstream << "constructor parameters precision mismatch for: " << ast->GetTypeName(ce->constructor_type) << "(";

	for (size_t i = 0; i < ce->initializers.size(); i++)
	{
		sstream << " <" << ast->GetTypeName(ce->initializers[i]->type) << ">";
		sstream << ((i < ce->initializers.size() - 1) ? "," : " ");
	}

	sstream << ") at (" << ce->start_line << ":" << ce->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, m_precision_mismatch_severity, ce->start_line, ce->start_column, sstream.str()));
}


void KSLAnalyzer::WarningInitExpressionPrecisionMismatch(const KSLVariableDefinitionsNode* vardef, const KSLType init_exp_type)
{
	std::stringstream sstream;
	sstream << "Init expression precision mismatch: \"" << ast->GetTypeName(init_exp_type) << "\" expected: \"";
	sstream << ast->GetTypeName(vardef->variable_type) << "\" at " << vardef->start_line << ":" << vardef->start_column;
	m_errors->push_back(KSLError(KSL_ANALIZER, m_precision_mismatch_severity, vardef->start_line, vardef->start_column, sstream.str()));
}


void KSLAnalyzer::WarningReturnStatetmantPrecisionMismatch(const KSLReturnStatementNode* rn, const KSLType func_type, const KSLType exp_type)
{
	std::stringstream sstream;
	sstream << "return statement precision mismatch: \"" << ast->GetTypeName(exp_type) << "\" expected: \"";
	sstream << ast->GetTypeName(func_type) << "\" at " << rn->start_line << ":" << rn->start_column;
	m_errors->push_back(KSLError(KSL_ANALIZER, m_precision_mismatch_severity, rn->start_line, rn->start_column, sstream.str()));
}


void KSLAnalyzer::WarningUnusedVariable(const KSLVariableDefinitionsNode* vardef, const KSLVariable &v)
{
	std::stringstream sstream;
	sstream << "unused variable: \"" << KSL::StorageQualifierToString(v.storage_type) << ast->GetTypeName(vardef->variable_type) << " " << v.orig_name;
	sstream  << "\" at " << vardef->start_line << ":" << vardef->start_column;
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_WARNING, vardef->start_line, vardef->start_column, sstream.str()));
}


void KSLAnalyzer::WarningIntegerDivision(KSLBinaryExpressionNode* be)
{
	std::stringstream sstream;
	sstream << "integer divides may be slower on some platform, try using uints if possible ";
	sstream << "<" << ast->GetTypeName(be->e1->type) << ">, <" << ast->GetTypeName(be->e2->type) << "> ";
	sstream << "at (" << be->start_line << ":" << be->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_WARNING, be->start_line, be->start_column, sstream.str()));
}


void KSLAnalyzer::ErrorInvalidDynamicArrayAccess(KSLSelectorExpressionNode* se)
{
	std::stringstream sstream;
	sstream << "invalid dynamic access for type: <" << ast->GetTypeName(se->expression->type) << "> ";
	sstream << "at (" << se->start_line << ":" << se->start_column << ")";
	m_errors->push_back(KSLError(KSL_ANALIZER, KSL_ERROR, se->start_line, se->start_column, sstream.str()));
}


//
//	Error message managment
//

// Mark the last block as global
void  KSLAnalyzer::InitErrorBlocks()
{
	m_error_block_ids.push_back((uint32_t)m_error_blocks.size() - 1);
}


// Create a new local block
void KSLAnalyzer::NewErrorBlock()
{
	KSLErrorBlock *new_block = new KSLErrorBlock;
	m_error_blocks.push_back(new_block);
	m_token_run = &new_block->first;
	m_errors = &new_block->second;
}


// Apply the local errors to the last global block
void KSLAnalyzer::AppendErrors()
{
	uint32_t g_id = m_error_block_ids.back();
	std::vector<KSLError> &g_errors = m_error_blocks[g_id]->second;
	g_errors.insert(g_errors.end(), m_errors->begin(), m_errors->end());

	m_error_blocks[g_id]->first = *m_token_run;

	__RemoveLocalErrorBlocks();
}


// Apply the erros from the longest run to the last global block
void KSLAnalyzer::CollectErrors()
{
	size_t max_id = m_error_blocks.size() - 1;
	uint32_t max_length = 0;

	uint32_t g_id = m_error_block_ids.back();
	for (size_t i = m_error_blocks.size() - 1; i > g_id; i--)
	{
		KSLErrorBlock &e = *m_error_blocks[i];
		if (max_length < e.first)
		{
			max_id = i;
			max_length = e.first;
		}
	}

	
	std::vector<KSLError> &g_errors = m_error_blocks[g_id]->second;
	if (m_error_blocks[max_id]->second.size() > 0)
	{
		g_errors.insert(g_errors.end(), m_error_blocks[max_id]->second.begin(), m_error_blocks[max_id]->second.end());
	}
	else
	{
		std::stringstream sstream;
		sstream << "parse error at: " << TN().line << ":" << TN().column << "; maybe compiler error?";
		g_errors.push_back(KSLError(KSL_ANALIZER, KSL_ERROR, TN().line, TN().column, sstream.str()));
	}

	m_error_blocks[g_id]->first = *m_token_run;

	__RemoveLocalErrorBlocks();
}


void KSLAnalyzer::__RemoveLocalErrorBlocks()
{
	uint32_t g_id = m_error_block_ids.back();
	for (size_t i = m_error_blocks.size() - 1; i > g_id; i--)
	{
		delete m_error_blocks[i];
	}
	m_error_blocks.resize(g_id + 1);
	m_error_block_ids.pop_back();

	m_token_run = &m_error_blocks[g_id]->first;
	m_errors = &m_error_blocks[g_id]->second;
}


bool KSLAnalyzer::HasErrors()
{
	for (size_t i = 0; i < m_errors->size(); i++)
	{
		if (m_errors->at(i).severity == KSL_ERROR)
		{
			return true;
		}
	}

	return false;
}

