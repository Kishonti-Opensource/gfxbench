/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ogl_gles30_functions.h"

#include <cassert>
#include <vector>

void GFXB_APIENTRY glGetProgramResourceivLegacy(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei *length, GLint *params)
{
	if (propCount <= 0 || props == nullptr)
		return;

	length = 0;
	GLenum legacy_prop = 0;

	switch (programInterface)
	{
	case GL_UNIFORM:

		for (GLsizei i = 0; i < propCount || i < bufSize; i++, length++)
		{
			if (props[i] == GL_LOCATION)
			{
				GLint uniform_name_returned_length = 0;
				GLint size = 0;
				GLenum type = 0;
				GLint uniform_name_length = 0;
				std::vector<GLchar> uniform_name;
				glGetActiveUniformsiv(program, 1, &index, GL_UNIFORM_NAME_LENGTH, &uniform_name_length);
				uniform_name.resize(uniform_name_length);
				glGetActiveUniform(program, index, uniform_name_length, &uniform_name_returned_length, &size, &type, &uniform_name[0]);
				params[i] = glGetUniformLocation(program, &uniform_name[0]);
			}
			else
			{
				legacy_prop = 0;
				switch (props[i])
				{
				case GL_NAME_LENGTH:	legacy_prop = GL_UNIFORM_NAME_LENGTH; break;
				case GL_TYPE:			legacy_prop = GL_UNIFORM_TYPE; break;
				case GL_ARRAY_SIZE:		legacy_prop = GL_UNIFORM_SIZE; break;
				case GL_OFFSET:			legacy_prop = GL_UNIFORM_OFFSET; break;
				case GL_BLOCK_INDEX:	legacy_prop = GL_UNIFORM_BLOCK_INDEX; break;
				case GL_ARRAY_STRIDE:	legacy_prop = GL_UNIFORM_ARRAY_STRIDE; break;
				case GL_MATRIX_STRIDE:	legacy_prop = GL_UNIFORM_MATRIX_STRIDE; break;
				case GL_IS_ROW_MAJOR:	legacy_prop = GL_UNIFORM_IS_ROW_MAJOR; break;

				default: // property not supported in ES 3.0 or lower
					params[i] = 0;
					break;
				}

				if (legacy_prop != 0)
				{
					glGetActiveUniformsiv(program, 1, &index, legacy_prop, &params[i]);
				}
			}
		}
		break;

	case GL_UNIFORM_BLOCK:

		for (GLsizei i = 0; i < propCount || i < bufSize; i++, length++)
		{
			if (props[i] == GL_ACTIVE_VARIABLES)
			{
				glGetActiveUniformBlockiv(program, index, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, &params[i]);
			}
			else
			{
				legacy_prop = 0;
				switch (props[i])
				{
				case GL_NAME_LENGTH:			legacy_prop = GL_UNIFORM_BLOCK_NAME_LENGTH; break;
				case GL_BUFFER_BINDING:			legacy_prop = GL_UNIFORM_BLOCK_BINDING; break;
				case GL_BUFFER_DATA_SIZE:		legacy_prop = GL_UNIFORM_BLOCK_DATA_SIZE; break;
				case GL_NUM_ACTIVE_VARIABLES:	legacy_prop = GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS; break;
				case GL_REFERENCED_BY_VERTEX_SHADER:	legacy_prop = GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER; break;
				case GL_REFERENCED_BY_FRAGMENT_SHADER:	legacy_prop = GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER; break;

				default: // property not supported in ES 3.0 or lower
					params[i] = 0;
					break;
				}

				if (legacy_prop != 0)
				{
					glGetActiveUniformBlockiv(program, index, legacy_prop, &params[i]);
				}
			}
		}
		break;

	default: // other interfaces are not supported in ES 3.0 or lower
		break;
	}
}

void GFXB_APIENTRY glGetProgramInterfaceivLegacy(GLuint program, GLenum programInterface, GLenum pname, GLint *params)
{
	*params = 0;

	switch (programInterface)
	{
	case GL_UNIFORM:

		switch (pname)
		{
		case GL_ACTIVE_RESOURCES:
			glGetProgramiv(program, GL_ACTIVE_UNIFORMS, params);
			break;

		case GL_MAX_NAME_LENGTH:
			glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, params);
			break;

		default: // other properties are not supported in ES 3.0 or lower
			break;
		}
		break;

	case GL_UNIFORM_BLOCK:

		switch (pname)
		{
		case GL_ACTIVE_RESOURCES:
			glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, params);
			break;

		case GL_MAX_NAME_LENGTH:
			glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, params);
			break;

		default: // other properties are not supported in ES 3.0 or lower
			break;
		}
		break;

	default: // other interfaces are not supported in ES 3.0 or lower
		break;
	}
}

void GFXB_APIENTRY glGetProgramResourceNameLegacy(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)
{
	*length = 0;
	GLint size;
	GLenum type;

	switch (programInterface)
	{
	case GL_UNIFORM:
		glGetActiveUniform(program, index, bufSize, length, &size, &type, name);
		break;

	case GL_UNIFORM_BLOCK:
		glGetActiveUniformBlockName(program, index, bufSize, length, name);
		break;

	default: // other interfaces are not supported in ES 3.0 or lower
		break;
	}
}

void getES30ProcAddresses()
{
	getES31ProcAddresses();

	glGetProgramResourceivProc = glGetProgramResourceivLegacy;
	glGetProgramInterfaceivProc = glGetProgramInterfaceivLegacy;
	glGetProgramResourceNameProc = glGetProgramResourceNameLegacy;
}