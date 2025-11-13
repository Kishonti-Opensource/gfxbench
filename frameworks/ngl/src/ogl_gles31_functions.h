/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef __OGL_GLES31_FUNCIONS_H__
#define __OGL_GLES31_FUNCIONS_H__

#include "oglx/gl.h"

#include <cstdint>

#define GFXB_EMULATOR 0

#if GFXB_EMULATOR
#define GFXB_APIENTRY _stdcall
#elif defined GL_APIENTRY
#define GFXB_APIENTRY GL_APIENTRY
#elif defined _WIN32
#define GFXB_APIENTRY _stdcall
#else
#define GFXB_APIENTRY
#endif


typedef void (GFXB_APIENTRY* PFNGLDISPATCHCOMPUTEPROC) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
extern PFNGLDISPATCHCOMPUTEPROC glDispatchComputeProc;

typedef void (GFXB_APIENTRY* PFNGLDISPATCHCOMPUTEINDIRECTPROC) (GLintptr indirect);
extern PFNGLDISPATCHCOMPUTEINDIRECTPROC glDispatchComputeIndirectProc;

typedef void (GFXB_APIENTRY* PFNGLBINDIMAGETEXTUREPROC) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
extern PFNGLBINDIMAGETEXTUREPROC glBindImageTextureProc;

typedef void (GFXB_APIENTRY* PFNGLDRAWARRAYSINDIRECTPROC) (GLenum mode, const void *indirect);
extern PFNGLDRAWARRAYSINDIRECTPROC glDrawArraysIndirectProc;

typedef void (GFXB_APIENTRY* PFNGLDRAWELEMENTSINDIRECTPROC) (GLenum mode, GLenum type, const GLvoid *indirect);
extern PFNGLDRAWELEMENTSINDIRECTPROC glDrawElementsIndirectProc;

typedef void (GFXB_APIENTRY* PFNGLMEMORYBARRIERPROC) (GLbitfield barriers);
extern PFNGLMEMORYBARRIERPROC glMemoryBarrierProc;

typedef void (GFXB_APIENTRY * PFNGLBLENDFUNCIPROC) (GLuint buf, GLenum src, GLenum dst);
extern PFNGLBLENDFUNCIPROC glBlendFunciProc;

typedef void (GFXB_APIENTRY* PFNGLGETPROGRAMRESOURCEIVPROC) (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum* props, GLsizei bufSize, GLsizei *length, GLint *params);
extern PFNGLGETPROGRAMRESOURCEIVPROC glGetProgramResourceivProc;

typedef void (GFXB_APIENTRY* PFNGLGETPROGRAMINTERFACEIVPROC) (GLuint program, GLenum programInterface, GLenum pname, GLint* params);
extern PFNGLGETPROGRAMINTERFACEIVPROC glGetProgramInterfaceivProc;

typedef void (GFXB_APIENTRY* PFNGLGETPROGRAMRESOURCENAMEPROC) (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei* length, GLchar *name);
extern PFNGLGETPROGRAMRESOURCENAMEPROC glGetProgramResourceNameProc;

// EXT_tessellation_shader
typedef void (GFXB_APIENTRY* PFNGLPATCHPARAMETERIPROC) (GLenum pname, GLint value);
extern PFNGLPATCHPARAMETERIPROC glPatchParameteriProc;

// ARB_timer_query
typedef void(GFXB_APIENTRY* PFNGLGETQUERYOBJECTUI64VPROC) (GLuint id, GLenum pname, uint64_t* params);
extern PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64vProc;

// KHR_debug
typedef void (GFXB_APIENTRY *GLDEBUGPROC)(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);

typedef void(GFXB_APIENTRY* PFNGLDEBUGMESSAGECONTROLPROC) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
extern PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControlProc;

typedef void(GFXB_APIENTRY* PFNGLDEBUGMESSAGEINSERTPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf);
extern PFNGLDEBUGMESSAGEINSERTPROC glDebugMessageInsertProc;

typedef void(GFXB_APIENTRY* PFNGLDEBUGMESSAGECALLBACKPROC) (GLDEBUGPROC callback, const void* userParam);
extern PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallbackProc;

typedef GLuint(GFXB_APIENTRY* PFNGLGETDEBUGMESSAGELOGPROC) (GLuint count, GLsizei bufSize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog);
extern PFNGLGETDEBUGMESSAGELOGPROC glGetDebugMessageLogProc;

typedef void(GFXB_APIENTRY* PFNGLGETPOINTERVPROC) (GLenum pname, void** params);
extern PFNGLGETPOINTERVPROC glGetPointervProc;

typedef void(GFXB_APIENTRY* PFNGLPUSHDEBUGGROUPPROC) (GLenum source, GLuint id, GLsizei length, const GLchar * message);
extern PFNGLPUSHDEBUGGROUPPROC glPushDebugGroupProc;

typedef void(GFXB_APIENTRY* PFNGLPOPDEBUGGROUPPROC) (void);
extern PFNGLPOPDEBUGGROUPPROC glPopDebugGroupProc;

typedef void(GFXB_APIENTRY* PFNGLOBJECTLABELPROC) (GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
extern PFNGLOBJECTLABELPROC glObjectLabelProc;

typedef void(GFXB_APIENTRY* PFNGLGETOBJECTLABELPROC) (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
extern PFNGLGETOBJECTLABELPROC glGetObjectLabelProc;

typedef void(GFXB_APIENTRY* PFNGLOBJECTPTRLABELPROC) (const void* ptr, GLsizei length, const GLchar *label);
extern PFNGLOBJECTPTRLABELPROC glObjectPtrLabelProc;

typedef void(GFXB_APIENTRY* PFNGLGETOBJECTPTRLABELPROC) (const void* ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
extern PFNGLGETOBJECTPTRLABELPROC glGetObjectPtrLabelProc;

#if defined HAVE_GLES3 || defined ENABLE_FRAME_CAPTURE || GL_WRAPPER_ENABLED || defined EMSCRIPTEN || defined NACL

#define GL_READ_ONLY							    0x88B8
#define GL_WRITE_ONLY							    0x88B9
#define GL_READ_WRITE							    0x88BA
#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE           0x8262
#define GL_MAX_COMPUTE_WORK_GROUP_COUNT             0x91BE
#define GL_MAX_COMPUTE_WORK_GROUP_SIZE              0x91BF
#define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS	    0x90EB
#define GL_COMPUTE_SHADER                           0x91B9
#define GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT   0x90DF
#define GL_PRIMITIVES_GENERATED						0x8C87
#define GL_SAMPLES_PASSED							0x8914

#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT	0x00000001
#define GL_ELEMENT_ARRAY_BARRIER_BIT		0x00000002
#define GL_UNIFORM_BARRIER_BIT				0x00000004
#define GL_TEXTURE_FETCH_BARRIER_BIT		0x00000008
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT	0x00000020
#define GL_COMMAND_BARRIER_BIT				0x00000040
#define GL_PIXEL_BUFFER_BARRIER_BIT			0x00000080
#define GL_TEXTURE_UPDATE_BARRIER_BIT		0x00000100
#define GL_BUFFER_UPDATE_BARRIER_BIT		0x00000200
#define GL_FRAMEBUFFER_BARRIER_BIT			0x00000400
#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT	0x00000800
#define GL_ATOMIC_COUNTER_BARRIER_BIT		0x00001000
#define GL_SHADER_STORAGE_BARRIER_BIT		0x00002000
#define GL_ALL_BARRIER_BITS					0xFFFFFFFF

// Program interface query API
#define GL_UNIFORM								0x92E1
#define GL_UNIFORM_BLOCK						0x92E2
#define GL_BUFFER_VARIABLE						0x92E5
#define GL_COMPUTE_WORK_GROUP_SIZE				0x8267
#define GL_SHADER_STORAGE_BLOCK					0x92E6
#define GL_ACTIVE_RESOURCES						0x92F5
#define GL_MAX_NAME_LENGTH						0x92F6
#define GL_MAX_NUM_ACTIVE_VARIABLES				0x92F7
#define GL_BUFFER_DATA_SIZE						0x9303
#define GL_ARRAY_SIZE							0x92FB
#define GL_REFERENCED_BY_VERTEX_SHADER			0x9306
#define GL_REFERENCED_BY_TESS_CONTROL_SHADER	0x9307
#define GL_REFERENCED_BY_TESS_EVALUATION_SHADER 0x9308
#define GL_REFERENCED_BY_GEOMETRY_SHADER		0x9309
#define GL_REFERENCED_BY_FRAGMENT_SHADER		0x930A
#define GL_REFERENCED_BY_COMPUTE_SHADER			0x930B
#define GL_LOCATION								0x930E
#define GL_LOCATION_INDEX						0x930F
#define GL_BUFFER_BINDING						0x9302
#define GL_DRAW_INDIRECT_BUFFER					0x8F3F
#define GL_DISPATCH_INDIRECT_BUFFER				0x90EE
#define GL_SHADER_STORAGE_BUFFER				0x90D2
#define GL_ATOMIC_COUNTER_BUFFER				0x92C0
#define GL_PROGRAM_INPUT						0x92E3
#define GL_PROGRAM_OUTPUT						0x92E4
#define GL_NAME_LENGTH							0x92F9
#define GL_TYPE									0x92FA

#define GL_SAMPLER_2D_MULTISAMPLE 0x9108
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 0x910A

#define GL_IMAGE_2D 0x904D
#define GL_IMAGE_3D 0x904E
#define GL_IMAGE_CUBE 0x9050
#define GL_IMAGE_2D_ARRAY 0x9053
#define GL_INT_IMAGE_2D 0x9058
#define GL_INT_IMAGE_3D 0x9059
#define GL_INT_IMAGE_CUBE 0x905B
#define GL_INT_IMAGE_2D_ARRAY 0x905E
#define GL_UNSIGNED_INT_IMAGE_2D 0x9063
#define GL_UNSIGNED_INT_IMAGE_3D 0x9064
#define GL_UNSIGNED_INT_IMAGE_CUBE 0x9066
#define GL_UNSIGNED_INT_IMAGE_2D_ARRAY 0x9069

#define GL_IMAGE_CUBE_MAP_ARRAY 0x9054
#define GL_INT_IMAGE_CUBE_MAP_ARRAY 0x905F
#define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY 0x906A

#define GL_INT_SAMPLER_2D_MULTISAMPLE 0x9109
#define GL_SAMPLER_CUBE_MAP_ARRAY 0x900C
#define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW 0x900D
#define GL_INT_SAMPLER_CUBE_MAP_ARRAY 0x900E
#define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY 0x900F

// EXT_texture_cube_map_array
#define GL_TEXTURE_CUBE_MAP_ARRAY 0x9009

// EXT_tessellation_shader
#ifndef GL_PATCHES
#define GL_PATCHES					0xE
#endif
#define GL_PATCH_VERTICES			0x8E72
#define GL_TESS_CONTROL_SHADER		0x8E88
#define GL_TESS_EVALUATION_SHADER	0x8E87

// EXT_geometry_shader4
#define GL_GEOMETRY_SHADER			0x8DD9

// ARB_timer_query
#define GL_TIME_ELAPSED				0x88BF

// ARB_pipeline_statistics_query
#define GL_VERTICES_SUBMITTED_ARB					0x82EE
#define GL_PRIMITIVES_SUBMITTED_ARB					0x82EF
#define GL_VERTEX_SHADER_INVOCATIONS_ARB			0x82F0
#define GL_TESS_CONTROL_SHADER_PATCHES_ARB			0x82F1
#define GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB	0x82F2
#define GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB	0x82F3
#define GL_FRAGMENT_SHADER_INVOCATIONS_ARB			0x82F4
#define GL_COMPUTE_SHADER_INVOCATIONS_ARB			0x82F5
#define GL_CLIPPING_INPUT_PRIMITIVES_ARB			0x82F6
#define GL_CLIPPING_OUTPUT_PRIMITIVES_ARB			0x82F7
#define GL_GEOMETRY_SHADER_INVOCATIONS				0x887F

// KHR_debug
#define GL_DEBUG_OUTPUT                        0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS            0x8242
#define GL_MAX_DEBUG_MESSAGE_LENGTH            0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES           0x9144
#define GL_DEBUG_LOGGED_MESSAGES               0x9145
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH    0x8243
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH         0x826C
#define GL_DEBUG_GROUP_STACK_DEPTH             0x826D
#define GL_MAX_LABEL_LENGTH                    0x82E8
#define GL_DEBUG_CALLBACK_FUNCTION             0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM           0x8245
#define GL_DEBUG_SOURCE_API                    0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM          0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER        0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY            0x8249
#define GL_DEBUG_SOURCE_APPLICATION            0x824A
#define GL_DEBUG_SOURCE_OTHER                  0x824B
#define GL_DEBUG_TYPE_ERROR                    0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR      0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR       0x824E
#define GL_DEBUG_TYPE_PORTABILITY              0x824F
#define GL_DEBUG_TYPE_PERFORMANCE              0x8250
#define GL_DEBUG_TYPE_OTHER                    0x8251
#define GL_DEBUG_TYPE_MARKER                   0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP               0x8269
#define GL_DEBUG_TYPE_POP_GROUP                0x826A
#define GL_DEBUG_SEVERITY_HIGH                 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM               0x9147
#define GL_DEBUG_SEVERITY_LOW                  0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION         0x826B
#define GL_STACK_UNDERFLOW                     0x0504
#define GL_STACK_OVERFLOW                      0x0503
#define GL_BUFFER                              0x82E0
#define GL_SHADER                              0x82E1
#define GL_PROGRAM                             0x82E2
#define GL_QUERY                               0x82E3
#define GL_PROGRAM_PIPELINE                    0x82E4
#define GL_SAMPLER                             0x82E6

#define GL_BACK_LEFT                           0x0402

// texture types
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT          0x83F3
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT    0x8C4F

#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR           0x93B2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR   0x93D2

#endif

void getES31ProcAddresses();

#endif  // __OGL_GLES31_FUNCIONS_H__
