/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SL_PARSER_H
#define SL_PARSER_H

#include "ngl.h"
#include <string>
#include <vector>
#include <set>

#undef VOID

struct _multistring
{
	std::string from_string;
	std::string to_string;

	_multistring() : from_string(""), to_string("")
	{
	}

	_multistring(const std::string &s) : from_string(s), to_string(s)
	{
	}
	_multistring(const std::string &s0, const std::string &s1) : from_string(s0), to_string(s1)
	{
	}
};


struct _parser
{
	enum _reserved_tokens
	{
		TOKEN_BREAK,
		TOKEN_CONST,
		TOKEN_CONTINUE,
		TOKEN_DO,
		TOKEN_ELSE,
		TOKEN_FOR,
		TOKEN_GOTO,
		TOKEN_IF,
		TOKEN_RETURN,
		TOKEN_STRUCT,
		TOKEN_WHILE,
		TOKEN_IN,
		TOKEN_OUT,
		TOKEN_UNIFORM,
		TOKEN_BUFFER,
		TOKEN_IMAGE,
		TOKEN_SHARED,
		TOKEN_GLOBAL,
		TOKEN_METAL_CONSTANT,
		TOKEN_METAL_DEVICE,
		TOKEN_METAL_LEVEL,
        TOKEN_METAL_THREAD,
		NUM_OF_RESERVERED_WORDS
	};
	enum _operators_punctuators
	{
		SEMICOLON,
		LEFT_BRACKET,
		RIGHT_BRACKET,
		LEFT_PARENT,
		RIGHT_PARENT,
		LEFT_BRACE,
		RIGHT_BRACE,
		EQUAL,
		MUL,
		INCR,
		DECR,
		INCR_ASSIGN,
		DECR_ASSIGN,
		MUL_ASSIGN,
		DIV_ASSIGN,
		IS_EQUAL,
		NOT_EQUAL,
		LESS_EQUAL,
		GREATER_EQUAL,
		LESS,
		SHIFT_LEFT,
		SHIFT_RIGHT,
		GREATER,
		DOT,
		COMMA,
		AND,
		OR,
        AMPERSAND,
		NUM_OF_OPERATORS_PUNCTUATORS
	};
	enum _types
	{
		BOOL,
		CHAR,
		UCHAR,
		DOUBLE,
		FLOAT,
		INT,
		UINT,
		SHORT,
		USHORT,
		VOID,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		IVEC2,
		IVEC3,
		IVEC4,
		FLOAT4x4,
		FLOAT3x3,
		SAMPLER2D,
		SAMPLER2DShadow,
		SAMPLER2DArray,
		SAMPLERCUBE,
		NUM_OF_PRIMITIVE_TYPES
	};
	enum _builtin_functions
	{
		MIX,
		TEXTURE,
		TEXTURE_ARRAY,
		TEXTURE_LOD,
        DISCARD,
		NORMALIZE,
		POW,
		FRACT,
		BARRIER,
		MEMORY_BARRIER_SHARED,
        MEMORY_BARRIER,
		HLSL_MUL,
		HLSL_TEXTURE,
        METAL_TEXTURE = HLSL_TEXTURE,
		HLSL_TEXTURE_LOD,
        METAL_DISCARD,
		NUM_OF_BUILTIN_FUNCTIONS
	};

	struct _variable
	{
		std::string m_name;
		int m_type_idx;
		//-1 - not set, 0 - uniform, 1 - in, 2 - out, 3 - sampler, 4 - buffer, 5 - shared, 6 - global, 7 - image
		int m_storage_type;
		int size; // attribute array size; 0: means non array, -1: flexible array
		_variable() : m_storage_type(-1), size(0)
        {
        }
	};

	struct _token
	{
		//-1 - error, 0 - reserverd word, 1 - variable, 2 - number, 3 - macros, 4 - type, 5 - operator/punctuator, 6 - builtin function
		int m_type;
		size_t m_table_idx;

		bool IsType( _reserved_tokens rt)
		{
			if( m_type == 0)
			{
				if( m_table_idx == rt)
				{
					return true;
				}
			}
			return false;
		}
		bool IsType( _operators_punctuators op)
		{
			if( m_type == 5)
			{
				if( m_table_idx == op)
				{
					return true;
				}
			}
			return false;
		}
		bool IsType( _types t)
		{
			if( m_type == 4)
			{
				if( m_table_idx == t)
				{
					return true;
				}
			}
			return false;
		}
		bool IsType( _builtin_functions bf)
		{
			if( m_type == 6)
			{
				if( m_table_idx == bf)
				{
					return true;
				}
			}
			return false;
		}
	};

	struct _statement
	{
		std::vector<_token> m_tokens;
	};

	struct _function
	{
		size_t start_token_idx;
		size_t num_args;
		size_t num_tokens;
	};

	struct _number
	{
		double m_num;
		//0 - double, 1 - int
		uint32_t m_type;
		_number() :m_num(0.0), m_type(0){}
	};

	struct _struct
	{
		size_t name_index;
		std::vector<size_t> type_indices;
		std::vector<size_t> name_indices;
		std::vector<size_t> count; // attribute array size; 0 means non array
	};

	std::string m_text;
	std::vector<_token> m_tokens;
	std::vector<_multistring> m_reserverd_words;
	std::vector<_multistring> m_operators_punctuators;
	std::vector<_multistring> m_types;
	std::vector<_multistring> m_builtin_functions;
	std::vector<_variable> m_variables;
	std::vector<_number> m_numbers;
	std::vector<std::string> m_macros;
	std::vector<_struct> m_structs;
	std::vector<_function*> m_functions;
	_function* m_main_function;

	std::vector<_variable> m_uniforms;
	std::vector<_variable> m_ins;
	std::vector<_variable> m_outs;
	std::vector<_variable> m_samplers;
	std::vector<_variable> m_buffers;
	std::vector<_variable> m_images;
	std::vector<_variable> m_shared;
	std::vector<_variable> m_globals;
	std::set<uint32_t> m_fragdata_indices;
	std::vector<size_t> m_swapped_tokens_first;

	NGL_shader_type m_shader_type;
	NGL_api m_api;
	uint32_t num_binding_points;
	uint32_t num_image_binding_points;
	uint32_t m_workgroup_size[3];

	void SetWorkgroupSize(uint32_t size_x, uint32_t size_y, uint32_t size_z)
	{
		m_workgroup_size[0] = size_x;
		m_workgroup_size[1] = size_y;
		m_workgroup_size[2] = size_z;
	}
	
	void Process();
	void Print( std::string &result);

    void CreateReflection(NGL_shader_source_descriptor &ssd);

	_parser()
	{
		m_workgroup_size[0] = 0;
		m_workgroup_size[1] = 0;
		m_workgroup_size[2] = 0;
		num_image_binding_points = 0;
		num_binding_points = 0;
	}

	~_parser()
	{
		for (size_t i = 0; i < m_functions.size(); ++i)
		{
			delete m_functions[i];
		}
	}

private:
	void Alter();
	void RemoveComments();
	void CreateReservedWord();
	void CreateTypes();
	void CreateOperatorsPunctuators();
	void CreatemBuiltinFunctions();
	void Tokenizer( const std::string &str);
	void Tokenizer3();
	void SearchTypesAndScopes();
    
    void InsertTokens(size_t start_id, std::vector<_token> tokens);
    void InsertGlobalBufferParametersForMetal();

	void BuildStructs();
	void ProcessStruct(size_t start_token_id);
	
	void Analyze();
	void BuildFunctions();
	void PrintTokens( std::vector<_token> &tokens);
	
	void DetectFragDataUsage();

	bool GetVariableSize(size_t start_token_id, _variable &v);
};



#endif
