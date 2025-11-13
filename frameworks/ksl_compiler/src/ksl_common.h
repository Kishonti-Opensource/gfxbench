/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __KSL_COMMON__
#define __KSL_COMMON__

#include <cstdint>
#include <string>
#include "ngl.h"

#define KSL_UINT32_MAX (0xffffffff)
#define KSL_INT32_MAX  (2147483647)


enum KSLBaseTypeClass
{
	KSL_TYPECLASS_UNKNOWN       = 0,
	KSL_TYPECLASS_VOID          = 1 << 0,
	KSL_TYPECLASS_FLOAT         = 1 << 1,
	KSL_TYPECLASS_INT           = 1 << 2,
	KSL_TYPECLASS_UINT          = 1 << 3,
	KSL_TYPECLASS_BOOL          = 1 << 4,
	KSL_TYPECLASS_SAMPLER       = 1 << 5,
	KSL_TYPECLASS_SUBPASS_INPUT = 1 << 6,
	KSL_TYPECLASS_IMAGE         = 1 << 7,
	KSL_TYPECLASS_ARRAY         = 1 << 8,
	KSL_TYPECLASS_USER_DEFINED  = 1 << 9,
	KSL_TYPECLASS_NUMERIC       = KSL_TYPECLASS_FLOAT | KSL_TYPECLASS_INT | KSL_TYPECLASS_UINT,
	KSL_TYPECLASS_ALL           = 0xffffffff
};


enum KSLBaseType
{
	KSL_TYPE_INVALID,

	KSL_TYPE_VOID,

	// Float
	KSL_TYPE_FLOAT,

	// Float vectors
	KSL_TYPE_VEC4,
	KSL_TYPE_VEC3,
	KSL_TYPE_VEC2,

	// Matrices
	KSL_TYPE_MAT4,
	KSL_TYPE_MAT3,
	KSL_TYPE_MAT2,

	// Integers
	KSL_TYPE_INT,
	KSL_TYPE_UINT,

	// Integer vectors
	KSL_TYPE_INT4,
	KSL_TYPE_INT3,
	KSL_TYPE_INT2,

	KSL_TYPE_UINT4,
	KSL_TYPE_UINT3,
	KSL_TYPE_UINT2,

	// Bool
	KSL_TYPE_BOOL,
	KSL_TYPE_BOOL2,
	KSL_TYPE_BOOL3,
	KSL_TYPE_BOOL4,

	// Samplers
	KSL_TYPE_SAMPLER_2D,
	KSL_TYPE_SAMPLER_2D_ARRAY,
	KSL_TYPE_SAMPLER_CUBE,
	KSL_TYPE_SAMPLER_CUBE_ARRAY,

	// shadow sampler
	KSL_TYPE_SAMPLER_2D_SHADOW,
	KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW,
	KSL_TYPE_SAMPLER_CUBE_SHADOW,

	// Images
	KSL_TYPE_IMAGE2D,

	// Subpass input
	KSL_TYPE_SUBPASS_INPUT,

	// Array
	KSL_TYPE_ARRAY,

	KSL_NUM_INBUILT_TYPES
};


enum KSLPrecision
{
	KSL_PRECISION_HIGH,
	KSL_PRECISION_MEDIUM,
	KSL_PRECISION_LOW,
	KSL_PRECISION_NONE,
	KSL_PRECISION_INVALID
};


struct KSLType
{
	KSLType()
		: id(KSL_TYPE_INVALID)
		, precision(KSL_PRECISION_INVALID)
		, base_type(NULL)
	{
	}

	KSLType(uint32_t base_type, KSLPrecision precision)
		: id(base_type)
		, precision(precision)
		, base_type(NULL)
	{
	}

	KSLType(const KSLType &type);
	~KSLType();
	KSLType & operator= (const KSLType &type);

	bool operator==(const KSLType &other) const;
	bool operator!=(const KSLType &other) const { return !(*this == other); }

	KSLBaseTypeClass GetTypeClass() const;
	uint32_t GetComponentCount() const;
	uint32_t GetSizeInBytes() const;
	KSLType & GetBaseType() const { return *base_type; }
	void SetBaseType(const KSLType &type);

	bool IsSampler() const;
	bool IsNumeric() const;
	bool IsBool() const;
	bool IsSubpassInput() const;
	bool IsArray() const;
	bool IsMatrix() const;
	bool IsVector() const;
	bool IsInteger() const;

	std::string ToString() const;

	uint32_t id;
	KSLPrecision precision;

	static KSLType Create(KSLBaseTypeClass base_type, uint32_t component_count);

private:
	void copy(const KSLType &type);

	KSLType* base_type;
};


enum KSLApiFamily
{
	KSL_API_FAMILY_GL,
	KSL_API_FAMILY_D3D,
	KSL_API_FAMILY_METAL,
	KSL_API_FAMILY_INVALID
};


enum KSLOperationPrecedence
{
	KSL_PRECEDENCE_LOWEST,
	KSL_PRECEDENCE_ASSIGN,
	KSL_PRECEDENCE_TERNARY_CONDITIONAL,
	KSL_PRECEDENCE_BINARY_OR,
	KSL_PRECEDENCE_BINARY_AND,
	KSL_PRECEDENCE_BINARY_BITWISE_OR,
	KSL_PRECEDENCE_BINARY_BITWISE_AND,
	KSL_PRECEDENCE_BINARY_EQUAL,
	KSL_PRECEDENCE_BINARY_COMP,
	KSL_PRECEDENCE_BINARY_SHIFT,
	KSL_PRECEDENCE_BINARY_ADD,
	KSL_PRECEDENCE_BINARY_MUL,
	KSL_PRECEDENCE_UNARY,
	KSL_PRECEDENCE_SUFFIX,
	KSL_PRECEDENCE_PARENTHESIS,
	KSL_PRECEDENCE_INVALID
};


enum KSLStage
{
	KSL_PREPROCESSOR,
	KSL_TOKENIZER,
	KSL_ANALIZER,
	KSL_TRANSLATOR,
	KSL_GENERATOR
};


enum KSLSeverity
{
	KSL_WARNING,
	KSL_ERROR
};


struct KSLError
{
	KSLStage stage;
	KSLSeverity severity;

	uint32_t start_line;
	uint32_t end_line;
	uint32_t start_column;
	uint32_t end_column;

	std::string message;

	KSLError(KSLStage stage, KSLSeverity severity, uint32_t line, uint32_t column, const std::string &message)
		: stage(stage)
		, severity(severity)
		, start_line(line)
		, end_line(line)
		, start_column(column)
		, end_column(column)
		, message(message)
	{ }
};


struct KSLErrorByLineCompare
{
	inline bool operator() (const KSLError& err1, const KSLError& err2)
	{
		return (err1.start_line < err2.start_line);
	}
};


KSLApiFamily GetAPIFamily(NGL_api api);
bool IsGLAPIFamily(NGL_api api);
bool IsD3DAPIFamily(NGL_api api);
bool IsMetalAPIFamily(NGL_api api);

std::string GetShaderTypeName(NGL_shader_type st);


#endif // __KSL_COMMON__

