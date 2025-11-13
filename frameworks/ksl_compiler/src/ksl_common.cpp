/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ksl_common.h"

#include <assert.h>

KSLApiFamily GetAPIFamily(NGL_api api)
{
	switch (api)
	{
	case NGL_OPENGL:
	case NGL_OPENGL_ES:
	case NGL_VULKAN:
		return KSL_API_FAMILY_GL;

	case NGL_DIRECT3D_11:
	case NGL_DIRECT3D_12:
		return KSL_API_FAMILY_D3D;

	case NGL_METAL_IOS:
	case NGL_METAL_MACOS:
		return KSL_API_FAMILY_METAL;

	default:
		assert(0);
		return KSL_API_FAMILY_INVALID;
		break;
	}
}

bool IsGLAPIFamily(NGL_api api)
{
	return GetAPIFamily(api) == KSL_API_FAMILY_GL;
}

bool IsD3DAPIFamily(NGL_api api)
{
	return GetAPIFamily(api) == KSL_API_FAMILY_D3D;
}

bool IsMetalAPIFamily(NGL_api api)
{
	return GetAPIFamily(api) == KSL_API_FAMILY_METAL;
}


bool KSLType::IsSampler() const
{
	return GetTypeClass() == KSL_TYPECLASS_SAMPLER;
}

bool KSLType::IsNumeric() const
{
	return (GetTypeClass() & KSL_TYPECLASS_NUMERIC) != 0;
}

bool KSLType::IsBool() const
{
	return GetTypeClass() == KSL_TYPECLASS_BOOL;
}

bool KSLType::IsSubpassInput() const
{
	return GetTypeClass() == KSL_TYPECLASS_SUBPASS_INPUT;
}

bool KSLType::IsArray() const
{
	return GetTypeClass() == KSL_TYPECLASS_ARRAY;
}


KSLBaseTypeClass KSLType::GetTypeClass() const
{
	if (id >= KSL_NUM_INBUILT_TYPES)
	{
		return KSL_TYPECLASS_USER_DEFINED;
	}

	switch (id)
	{
	case KSL_TYPE_VOID:
		return KSL_TYPECLASS_VOID;

	case KSL_TYPE_FLOAT:

	case KSL_TYPE_VEC4:
	case KSL_TYPE_VEC3:
	case KSL_TYPE_VEC2:

	case KSL_TYPE_MAT4:
	case KSL_TYPE_MAT3:
	case KSL_TYPE_MAT2:
		return KSL_TYPECLASS_FLOAT;

	case KSL_TYPE_INT:
	case KSL_TYPE_INT4:
	case KSL_TYPE_INT3:
	case KSL_TYPE_INT2:
		return KSL_TYPECLASS_INT;

	case KSL_TYPE_UINT:
	case KSL_TYPE_UINT4:
	case KSL_TYPE_UINT3:
	case KSL_TYPE_UINT2:
		return KSL_TYPECLASS_UINT;

	case KSL_TYPE_BOOL:
	case KSL_TYPE_BOOL2:
	case KSL_TYPE_BOOL3:
	case KSL_TYPE_BOOL4:
		return KSL_TYPECLASS_BOOL;

	case KSL_TYPE_SAMPLER_2D:
	case KSL_TYPE_SAMPLER_2D_ARRAY:
	case KSL_TYPE_SAMPLER_CUBE:
	case KSL_TYPE_SAMPLER_CUBE_ARRAY:

	case KSL_TYPE_SAMPLER_2D_SHADOW:
	case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW:
	case KSL_TYPE_SAMPLER_CUBE_SHADOW:
		return KSL_TYPECLASS_SAMPLER;

	case KSL_TYPE_IMAGE2D:
		return KSL_TYPECLASS_IMAGE;

	case KSL_TYPE_SUBPASS_INPUT:
		return KSL_TYPECLASS_SUBPASS_INPUT;
	
	case KSL_TYPE_ARRAY:
		return KSL_TYPECLASS_ARRAY;

	default:
		assert(0);
		break;
	}

	return KSL_TYPECLASS_UNKNOWN;
}


uint32_t KSLType::GetComponentCount() const
{
	switch (id)
	{
	case KSL_TYPE_BOOL:
	case KSL_TYPE_FLOAT:
	case KSL_TYPE_INT:
	case KSL_TYPE_UINT:
		return 1;

	case KSL_TYPE_BOOL2:
	case KSL_TYPE_VEC2:
	case KSL_TYPE_INT2:
	case KSL_TYPE_UINT2:
		return 2;

	case KSL_TYPE_BOOL3:
	case KSL_TYPE_VEC3:
	case KSL_TYPE_INT3:
	case KSL_TYPE_UINT3:
		return 3;

	case KSL_TYPE_BOOL4:
	case KSL_TYPE_VEC4:
	case KSL_TYPE_INT4:
	case KSL_TYPE_UINT4:
	case KSL_TYPE_MAT2:
		return 4;

	case KSL_TYPE_MAT3:
		return 9;

	case KSL_TYPE_MAT4:
		return 16;

	default:
		assert(0);
		break;
	}

	return 0;
}


uint32_t KSLType::GetSizeInBytes() const
{
	if (IsBool())
	{
		switch (id)
		{
		case KSL_TYPE_BOOL:
			return 1;
		case KSL_TYPE_BOOL2:
			return 2;
		case KSL_TYPE_BOOL3:
		case KSL_TYPE_BOOL4:
			return 4;
		}
	}
	else if(IsNumeric())
	{
		if (precision == KSL_PRECISION_HIGH)
		{
			switch (id)
			{
			case KSL_TYPE_FLOAT:
			case KSL_TYPE_INT:
			case KSL_TYPE_UINT:
				return 4;

			case KSL_TYPE_VEC2:
			case KSL_TYPE_INT2:
			case KSL_TYPE_UINT2:
				return 8;

			case KSL_TYPE_VEC3:
			case KSL_TYPE_INT3:
			case KSL_TYPE_UINT3:
			case KSL_TYPE_VEC4:
			case KSL_TYPE_INT4:
			case KSL_TYPE_UINT4:
			case KSL_TYPE_MAT2:
				return 16;

			case KSL_TYPE_MAT3:
				return 48;

			case KSL_TYPE_MAT4:
				return 64;

			default:
				assert(0);
				break;
			}
		}
		else
		{
			// not implemented yet
			assert(0);
		}
	}
	
	// !IsNumeric() && !IsBool()
	assert(0);
	return 0;
}


bool KSLType::IsMatrix() const
{
	switch (id)
	{
	case KSL_TYPE_MAT2:
	case KSL_TYPE_MAT3:
	case KSL_TYPE_MAT4:
		return true;
	}

	return false;
}


bool KSLType::IsInteger() const
{
	switch (id)
	{
	case KSL_TYPE_INT:
	case KSL_TYPE_INT2:
	case KSL_TYPE_INT3:
	case KSL_TYPE_INT4:

	case KSL_TYPE_UINT:
	case KSL_TYPE_UINT2:
	case KSL_TYPE_UINT3:
	case KSL_TYPE_UINT4:
		return true;

	default:
		break;
	}

	return false;
}


bool KSLType::IsVector() const
{
	switch (id)
	{
	// Float vectors
	case KSL_TYPE_VEC4:
	case KSL_TYPE_VEC3:
	case KSL_TYPE_VEC2:

	// Integer vectors
	case KSL_TYPE_INT4:
	case KSL_TYPE_INT3:
	case KSL_TYPE_INT2:

	case KSL_TYPE_UINT4:
	case KSL_TYPE_UINT3:
	case KSL_TYPE_UINT2:

	// Bool vectors
	case KSL_TYPE_BOOL2:
	case KSL_TYPE_BOOL3:
	case KSL_TYPE_BOOL4:
		return true;
	}

	return false;
}


std::string KSLType::ToString() const
{
	if (IsArray())
	{
		return GetBaseType().ToString() + "[]";
	}

	std::string fprefix, iprefix;

	switch (precision)
	{
	case KSL_PRECISION_HIGH:
		fprefix = "float";
		iprefix = "int";
		break;
	case KSL_PRECISION_MEDIUM:
		fprefix = "half";
		iprefix = "short";
		break;
	case KSL_PRECISION_LOW:
		fprefix = "lowp";
		iprefix = "byte";
		break;
	case KSL_PRECISION_NONE:
		fprefix = "float_noprec";
		iprefix = "int_noprec";
		break;
	default:
		assert(0);
		break;
	}

	switch (id)
	{
	case KSL_TYPE_VOID:  return "void";

		// float
	case KSL_TYPE_FLOAT: return fprefix;

		// vectors
	case KSL_TYPE_VEC4:  return fprefix + "4";
	case KSL_TYPE_VEC3:  return fprefix + "3";
	case KSL_TYPE_VEC2:  return fprefix + "2";

		// matrices
	case KSL_TYPE_MAT4:  return fprefix + "4x4";
	case KSL_TYPE_MAT3:  return fprefix + "3x3";
	case KSL_TYPE_MAT2:  return fprefix + "2x2";

	case KSL_TYPE_INT:   return iprefix;
	case KSL_TYPE_UINT:  return "u" + iprefix;

		// Integer vectors
	case KSL_TYPE_INT4:  return iprefix + "4";
	case KSL_TYPE_INT3:  return iprefix + "3";
	case KSL_TYPE_INT2:  return iprefix + "2";

	case KSL_TYPE_UINT4: return "u" + iprefix + "4";
	case KSL_TYPE_UINT3: return "u" + iprefix + "3";
	case KSL_TYPE_UINT2: return "u" + iprefix + "2";

	case KSL_TYPE_BOOL:  return "bool";
	case KSL_TYPE_BOOL2: return "bool2";
	case KSL_TYPE_BOOL3: return "bool3";
	case KSL_TYPE_BOOL4: return "bool4";

		// samplers
	case KSL_TYPE_SAMPLER_2D: return "sampler2D<" + fprefix + ">";
	case KSL_TYPE_SAMPLER_2D_ARRAY: return "sampler2DArray<" + fprefix + ">";
	case KSL_TYPE_SAMPLER_CUBE: return "samplerCube<" + fprefix + ">";
	case KSL_TYPE_SAMPLER_CUBE_ARRAY: return "samplerCubeArray<" + fprefix + ">";

		// shadow samplers
	case KSL_TYPE_SAMPLER_2D_SHADOW: return "sampler2DShadow<" + fprefix + ">";
	case KSL_TYPE_SAMPLER_2D_ARRAY_SHADOW:  return "sampler2DArrayShadow<" + fprefix + ">";
	case KSL_TYPE_SAMPLER_CUBE_SHADOW:  return "samplerCubeShadow<" + fprefix + ">";

	case KSL_TYPE_IMAGE2D: return "image2D<" + fprefix + ">";

		// subpass input
	case KSL_TYPE_SUBPASS_INPUT: return "subpassInput<" + fprefix + ">";

	default:
		break;
	}

	assert(0);
	return "invalid_type";
}


KSLType KSLType::Create(KSLBaseTypeClass base_type, uint32_t component_count)
{
	if (base_type == KSL_TYPECLASS_FLOAT)
	{
		switch (component_count)
		{
			case 1: return KSLType(KSL_TYPE_FLOAT, KSL_PRECISION_INVALID);
			case 2: return KSLType(KSL_TYPE_VEC2, KSL_PRECISION_INVALID);
			case 3: return KSLType(KSL_TYPE_VEC3, KSL_PRECISION_INVALID);
			case 4: return KSLType(KSL_TYPE_VEC4, KSL_PRECISION_INVALID);
			default: break;
		}
	}
	else if (base_type == KSL_TYPECLASS_INT)
	{
		switch (component_count)
		{
			case 1: return KSLType(KSL_TYPE_INT, KSL_PRECISION_INVALID);
			case 2: return KSLType(KSL_TYPE_INT2, KSL_PRECISION_INVALID);
			case 3: return KSLType(KSL_TYPE_INT3, KSL_PRECISION_INVALID);
			case 4: return KSLType(KSL_TYPE_INT4, KSL_PRECISION_INVALID);
			default: break;
		}
	}
	else if (base_type == KSL_TYPECLASS_UINT)
	{
		switch (component_count)
		{
			case 1: return KSLType(KSL_TYPE_UINT, KSL_PRECISION_INVALID);
			case 2: return KSLType(KSL_TYPE_UINT2, KSL_PRECISION_INVALID);
			case 3: return KSLType(KSL_TYPE_UINT3, KSL_PRECISION_INVALID);
			case 4: return KSLType(KSL_TYPE_UINT4, KSL_PRECISION_INVALID);
			default: break;
		}
	}
	
	assert(0);
	return KSLType();
}


KSLType::KSLType(const KSLType &type)
{
	base_type = NULL;
	copy(type);
}


KSLType::~KSLType()
{
	delete base_type;
}


KSLType & KSLType::operator= (const KSLType &type)
{
	copy(type);
	return *this;
}


void KSLType::SetBaseType(const KSLType &type)
{
	delete base_type;
	base_type = new KSLType(type);
}


void KSLType::copy(const KSLType &type)
{
	id = type.id;
	precision = type.precision;
	delete base_type;
	base_type = (type.base_type != NULL) ? new KSLType(*type.base_type) : NULL;
}


bool KSLType::operator==(const KSLType &other) const
{
	if (id != other.id) return false;

	//  KSLTODO: 
	//  precision handling not complete yet
	//if (precision != other.precision) return false;

	if ((base_type == NULL) && (other.base_type == NULL))
	{
		return true;
	}
	else  if ((base_type != NULL) && (other.base_type != NULL))
	{
		return *base_type == *other.base_type;
	}

	return false;
}


std::string GetShaderTypeName(NGL_shader_type st)
{
	switch (st)
	{
	case NGL_VERTEX_SHADER: return "vertex";
	case NGL_FRAGMENT_SHADER: return "fragment";
	case NGL_GEOMETRY_SHADER: return "geometry";
	case NGL_TESS_CONTROL_SHADER: return "tess_control";
	case NGL_TESS_EVALUATION_SHADER: return "tess_eval";
	case NGL_COMPUTE_SHADER: return "compute";
	default:
		break;
	}

	assert(0);
	return "error";
}

