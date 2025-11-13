/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
/** \file nulldriver.h
	Interface for our dummy OpenGL implementation.
	Only those GL calls are implemented which were referenced by the engine.
	The actual function definitions are empty or trivial.
	Nulldriver can be used to measure the time used by the engine itself.
	Used only when project configuration is Nulldriver.
*/
#ifndef NULLDRIVER_H
#define NULLDRIVER_H
#if defined OPENGL_IMPLEMENTATION_NULL || defined HAVE_DX
extern "C" {

#ifndef PLATFORM_WINDOWS
#define __w64
#endif

	
#define HAVE_GLES3 1

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef char GLchar;
typedef unsigned char GLubyte;
typedef unsigned int GLbitfield;
typedef unsigned short int GLushort;
typedef float GLclampf;
typedef unsigned char GLboolean;
typedef int GLint;
typedef int GLsizei;
typedef void GLvoid;
typedef float GLfloat;
#ifndef _PTRDIFF_T_DEFINED
#ifdef _WIN64
typedef __int64             ptrdiff_t;
#else
typedef __w64 int            ptrdiff_t;
#endif
#endif
typedef ptrdiff_t GLsizeiptr;
typedef int GLintptr;

extern GLboolean GLEW_VERSION_2_0;
#define GLEW_OK 0
GLenum glewInit ();

#define GL_UNSIGNED_SHORT_5_6_5		0x8363
#define GL_UNSIGNED_SHORT_4_4_4_4	0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1	0x8034
#define GL_ETC1_RGB8_OES			0x8D64

void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data);
void glDeleteTextures(GLsizei n, const GLuint* textures);

#define GL_VIEWPORT 0x0BA2
#define GL_ALPHA 0x1906
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_VERTEX_ARRAY 0x8074
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_QUERY_RESULT_ARB 0x8866
#define GL_QUERY_RESULT_AVAILABLE_ARB 0x8867
#define GL_SAMPLES_PASSED_ARB 0x8914
#define GL_RENDERBUFFER_EXT 0x8D41
#define GL_QUERY_RESULT_ARB 0x8866
#define GL_QUERY_RESULT_AVAILABLE_ARB 0x8867

#define GL_SHADER_SOURCE_LENGTH 0x8B88
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED        0x8622
#define GL_MAX_VERTEX_ATTRIBS               0x8869

#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207

#define GL_DST_COLOR 0x0306
#define GL_SRC_COLOR 0x0300


#define GL_CONSTANT_COLOR                 0x8001


#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_TRIANGLES 0x0004
#define GL_ZERO 0
#define GL_ONE 1
#define GL_SRC_ALPHA 0x0302
#define GL_TRUE 1
#define GL_FALSE 0
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_INT 0x1405
#define GL_UNSIGNED_SHORT 0x1403
#define GL_FLOAT 0x1406
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_OUT_OF_MEMORY 0x0505
#define GL_CW 0x0900
#define GL_CCW 0x0901
#define GL_CULL_FACE 0x0B44
#define GL_DEPTH_TEST 0x0B71
#define GL_BLEND 0x0BE2
#define GL_TEXTURE_2D 0x0DE1
#define GL_MODELVIEW 0x1700
#define GL_PROJECTION 0x1701
#define GL_DEPTH_COMPONENT 0x1902
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_BGR 0x80E0
#define GL_BGRA 0x80E1
#define GL_BGR_EXT 0x80E0
#define GL_BGRA_EXT 0x80E1
#define GL_EXTENSIONS 0x1F03
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_3D                                    0x806F
#define GL_TEXTURE_2D_ARRAY                              0x8C1A
#define GL_TEXTURE_WRAP_R                                0x8072
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_REPEAT 0x2901
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_MIRRORED_REPEAT                0x8370
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 0x851A
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_INVALID_FRAMEBUFFER_OPERATION_EXT 0x0506
#define GL_FRAMEBUFFER_COMPLETE_EXT 0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT 0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT 0x8CDA
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT 0x8CDC
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT 0x8CDD
#define GL_COLOR_ATTACHMENT0_EXT 0x8CE0
#define GL_DEPTH_ATTACHMENT_EXT 0x8D00
#define GL_STENCIL_ATTACHMENT_EXT 0x8D20
#define GL_FRAMEBUFFER_EXT 0x8D40
#define GL_TEXTURE_2D 0x0DE1

/* GetTarget */
#define GL_CURRENT_COLOR                  0x0B00
#define GL_CURRENT_INDEX                  0x0B01
#define GL_CURRENT_NORMAL                 0x0B02
#define GL_CURRENT_TEXTURE_COORDS         0x0B03
#define GL_CURRENT_RASTER_COLOR           0x0B04
#define GL_CURRENT_RASTER_INDEX           0x0B05
#define GL_CURRENT_RASTER_TEXTURE_COORDS  0x0B06
#define GL_CURRENT_RASTER_POSITION        0x0B07
#define GL_CURRENT_RASTER_POSITION_VALID  0x0B08
#define GL_CURRENT_RASTER_DISTANCE        0x0B09
#define GL_POINT_SMOOTH                   0x0B10
#define GL_POINT_SIZE                     0x0B11
#define GL_POINT_SIZE_RANGE               0x0B12
#define GL_POINT_SIZE_GRANULARITY         0x0B13
#define GL_LINE_SMOOTH                    0x0B20
#define GL_LINE_WIDTH                     0x0B21
#define GL_LINE_WIDTH_RANGE               0x0B22
#define GL_LINE_WIDTH_GRANULARITY         0x0B23
#define GL_LINE_STIPPLE                   0x0B24
#define GL_LINE_STIPPLE_PATTERN           0x0B25
#define GL_LINE_STIPPLE_REPEAT            0x0B26
#define GL_LIST_MODE                      0x0B30
#define GL_MAX_LIST_NESTING               0x0B31
#define GL_LIST_BASE                      0x0B32
#define GL_LIST_INDEX                     0x0B33
#define GL_POLYGON_MODE                   0x0B40
#define GL_POLYGON_SMOOTH                 0x0B41
#define GL_POLYGON_STIPPLE                0x0B42
#define GL_EDGE_FLAG                      0x0B43
#define GL_CULL_FACE                      0x0B44
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_LIGHTING                       0x0B50
#define GL_LIGHT_MODEL_LOCAL_VIEWER       0x0B51
#define GL_LIGHT_MODEL_TWO_SIDE           0x0B52
#define GL_LIGHT_MODEL_AMBIENT            0x0B53
#define GL_SHADE_MODEL                    0x0B54
#define GL_COLOR_MATERIAL_FACE            0x0B55
#define GL_COLOR_MATERIAL_PARAMETER       0x0B56
#define GL_COLOR_MATERIAL                 0x0B57
#define GL_FOG                            0x0B60
#define GL_FOG_INDEX                      0x0B61
#define GL_FOG_DENSITY                    0x0B62
#define GL_FOG_START                      0x0B63
#define GL_FOG_END                        0x0B64
#define GL_FOG_MODE                       0x0B65
#define GL_FOG_COLOR                      0x0B66
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_TEST                     0x0B71
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_ACCUM_CLEAR_VALUE              0x0B80
#define GL_STENCIL_TEST                   0x0B90
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_MATRIX_MODE                    0x0BA0
#define GL_NORMALIZE                      0x0BA1
#define GL_VIEWPORT                       0x0BA2
#define GL_MODELVIEW_STACK_DEPTH          0x0BA3
#define GL_PROJECTION_STACK_DEPTH         0x0BA4
#define GL_TEXTURE_STACK_DEPTH            0x0BA5
#define GL_MODELVIEW_MATRIX               0x0BA6
#define GL_PROJECTION_MATRIX              0x0BA7
#define GL_TEXTURE_MATRIX                 0x0BA8
#define GL_ATTRIB_STACK_DEPTH             0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH      0x0BB1
#define GL_ALPHA_TEST                     0x0BC0
#define GL_ALPHA_TEST_FUNC                0x0BC1
#define GL_ALPHA_TEST_REF                 0x0BC2
#define GL_DITHER                         0x0BD0
#define GL_BLEND_DST                      0x0BE0
#define GL_BLEND_SRC                      0x0BE1
#define GL_BLEND                          0x0BE2
#define GL_LOGIC_OP_MODE                  0x0BF0
#define GL_INDEX_LOGIC_OP                 0x0BF1
#define GL_COLOR_LOGIC_OP                 0x0BF2
#define GL_AUX_BUFFERS                    0x0C00
#define GL_DRAW_BUFFER                    0x0C01
#define GL_READ_BUFFER                    0x0C02
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_INDEX_CLEAR_VALUE              0x0C20
#define GL_INDEX_WRITEMASK                0x0C21
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_INDEX_MODE                     0x0C30
#define GL_RGBA_MODE                      0x0C31
#define GL_DOUBLEBUFFER                   0x0C32
#define GL_STEREO                         0x0C33
#define GL_RENDER_MODE                    0x0C40
#define GL_PERSPECTIVE_CORRECTION_HINT    0x0C50
#define GL_POINT_SMOOTH_HINT              0x0C51
#define GL_LINE_SMOOTH_HINT               0x0C52
#define GL_POLYGON_SMOOTH_HINT            0x0C53
#define GL_FOG_HINT                       0x0C54
#define GL_TEXTURE_GEN_S                  0x0C60
#define GL_TEXTURE_GEN_T                  0x0C61
#define GL_TEXTURE_GEN_R                  0x0C62
#define GL_TEXTURE_GEN_Q                  0x0C63
#define GL_PIXEL_MAP_I_TO_I               0x0C70
#define GL_PIXEL_MAP_S_TO_S               0x0C71
#define GL_PIXEL_MAP_I_TO_R               0x0C72
#define GL_PIXEL_MAP_I_TO_G               0x0C73
#define GL_PIXEL_MAP_I_TO_B               0x0C74
#define GL_PIXEL_MAP_I_TO_A               0x0C75
#define GL_PIXEL_MAP_R_TO_R               0x0C76
#define GL_PIXEL_MAP_G_TO_G               0x0C77
#define GL_PIXEL_MAP_B_TO_B               0x0C78
#define GL_PIXEL_MAP_A_TO_A               0x0C79
#define GL_PIXEL_MAP_I_TO_I_SIZE          0x0CB0
#define GL_PIXEL_MAP_S_TO_S_SIZE          0x0CB1
#define GL_PIXEL_MAP_I_TO_R_SIZE          0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE          0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE          0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE          0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE          0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE          0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE          0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE          0x0CB9
#define GL_UNPACK_SWAP_BYTES              0x0CF0
#define GL_UNPACK_LSB_FIRST               0x0CF1
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_UNPACK_SKIP_ROWS               0x0CF3
#define GL_UNPACK_SKIP_PIXELS             0x0CF4
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_SWAP_BYTES                0x0D00
#define GL_PACK_LSB_FIRST                 0x0D01
#define GL_PACK_ROW_LENGTH                0x0D02
#define GL_PACK_SKIP_ROWS                 0x0D03
#define GL_PACK_SKIP_PIXELS               0x0D04
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAP_COLOR                      0x0D10
#define GL_MAP_STENCIL                    0x0D11
#define GL_INDEX_SHIFT                    0x0D12
#define GL_INDEX_OFFSET                   0x0D13
#define GL_RED_SCALE                      0x0D14
#define GL_RED_BIAS                       0x0D15
#define GL_ZOOM_X                         0x0D16
#define GL_ZOOM_Y                         0x0D17
#define GL_GREEN_SCALE                    0x0D18
#define GL_GREEN_BIAS                     0x0D19
#define GL_BLUE_SCALE                     0x0D1A
#define GL_BLUE_BIAS                      0x0D1B
#define GL_ALPHA_SCALE                    0x0D1C
#define GL_ALPHA_BIAS                     0x0D1D
#define GL_DEPTH_SCALE                    0x0D1E
#define GL_DEPTH_BIAS                     0x0D1F
#define GL_MAX_EVAL_ORDER                 0x0D30
#define GL_MAX_LIGHTS                     0x0D31
#define GL_MAX_CLIP_PLANES                0x0D32
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_PIXEL_MAP_TABLE            0x0D34
#define GL_MAX_ATTRIB_STACK_DEPTH         0x0D35
#define GL_MAX_MODELVIEW_STACK_DEPTH      0x0D36
#define GL_MAX_NAME_STACK_DEPTH           0x0D37
#define GL_MAX_PROJECTION_STACK_DEPTH     0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH        0x0D39
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH  0x0D3B
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_INDEX_BITS                     0x0D51
#define GL_RED_BITS                       0x0D52
#define GL_GREEN_BITS                     0x0D53
#define GL_BLUE_BITS                      0x0D54
#define GL_ALPHA_BITS                     0x0D55
#define GL_DEPTH_BITS                     0x0D56
#define GL_STENCIL_BITS                   0x0D57
#define GL_ACCUM_RED_BITS                 0x0D58
#define GL_ACCUM_GREEN_BITS               0x0D59
#define GL_ACCUM_BLUE_BITS                0x0D5A
#define GL_ACCUM_ALPHA_BITS               0x0D5B
#define GL_NAME_STACK_DEPTH               0x0D70
#define GL_AUTO_NORMAL                    0x0D80
#define GL_MAP1_COLOR_4                   0x0D90
#define GL_MAP1_INDEX                     0x0D91
#define GL_MAP1_NORMAL                    0x0D92
#define GL_MAP1_TEXTURE_COORD_1           0x0D93
#define GL_MAP1_TEXTURE_COORD_2           0x0D94
#define GL_MAP1_TEXTURE_COORD_3           0x0D95
#define GL_MAP1_TEXTURE_COORD_4           0x0D96
#define GL_MAP1_VERTEX_3                  0x0D97
#define GL_MAP1_VERTEX_4                  0x0D98
#define GL_MAP2_COLOR_4                   0x0DB0
#define GL_MAP2_INDEX                     0x0DB1
#define GL_MAP2_NORMAL                    0x0DB2
#define GL_MAP2_TEXTURE_COORD_1           0x0DB3
#define GL_MAP2_TEXTURE_COORD_2           0x0DB4
#define GL_MAP2_TEXTURE_COORD_3           0x0DB5
#define GL_MAP2_TEXTURE_COORD_4           0x0DB6
#define GL_MAP2_VERTEX_3                  0x0DB7
#define GL_MAP2_VERTEX_4                  0x0DB8
#define GL_MAP1_GRID_DOMAIN               0x0DD0
#define GL_MAP1_GRID_SEGMENTS             0x0DD1
#define GL_MAP2_GRID_DOMAIN               0x0DD2
#define GL_MAP2_GRID_SEGMENTS             0x0DD3
#define GL_TEXTURE_1D                     0x0DE0
#define GL_TEXTURE_2D                     0x0DE1
#define GL_FEEDBACK_BUFFER_POINTER        0x0DF0
#define GL_FEEDBACK_BUFFER_SIZE           0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE           0x0DF2
#define GL_SELECTION_BUFFER_POINTER       0x0DF3
#define GL_SELECTION_BUFFER_SIZE          0x0DF4



#define GL_COMPARE_REF_TO_TEXTURE                        0x884E
#define GL_TEXTURE_COMPARE_MODE                          0x884C
#define GL_TEXTURE_COMPARE_FUNC                          0x884D


#define GL_DYNAMIC_COPY                                  0x88EA


#define GL_ONE_MINUS_SRC_COLOR            0x0301



#define GL_PIXEL_PACK_BUFFER                             0x88EB
#define GL_PIXEL_UNPACK_BUFFER                           0x88EC
#define GL_STREAM_DRAW                                   0x88E0
#define GL_STATIC_DRAW                                   0x88E4
#define GL_DYNAMIC_DRAW                                  0x88E8
#define GL_RGBA8                                         0x8058


#define GL_FUNC_ADD                                      0x8006


#define GL_MAX_3D_TEXTURE_SIZE                           0x8073
#define GL_MAX_ARRAY_TEXTURE_LAYERS                      0x88FF

#define GL_MIN_PROGRAM_TEXEL_OFFSET                      0x8904
#define GL_MAX_PROGRAM_TEXEL_OFFSET                      0x8905



#define GL_CONSTANT_ALPHA 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA 0x8004

#define GL_SAMPLE_ALPHA_TO_COVERAGE			0x809E
#define GL_SAMPLE_ALPHA_TO_ONE			0x809F
#define GL_SAMPLE_COVERAGE		0x80A0
#define GL_SAMPLE_BUFFERS		0x80A8
#define GL_SAMPLES					0x80A9
#define GL_SAMPLE_COVERAGE_VALUE 0x80AA
#define GL_SAMPLE_COVERAGE_INVERT 0x80AB


/* StringName */
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS	0
#define GL_ALIASED_POINT_SIZE_RANGE 0
#define GL_ALIASED_LINE_WIDTH_RANGE 0

#define GL_VALIDATE_STATUS 0x8B83
#define GL_DEPTH_COMPONENT24 0x81A6


#define GL_INTERLEAVED_ATTRIBS                           0x8C8C


#define GL_NONE                                          0

#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS              0x8B4D

#define GL_MAX_COMBINED_UNIFORM_BLOCKS                   0x8A2E
#define GL_MAX_VERTEX_OUTPUT_COMPONENTS                  0x9122
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS                 0x9125
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS                0x8B4C
#define GL_MAX_VERTEX_UNIFORM_BLOCKS                     0x8A2B
#define GL_MAX_VARYING_VECTORS                           0x8DFC
#define GL_MAX_VERTEX_UNIFORM_VECTORS                    0x8DFB

#define GL_MAX_VARYING_COMPONENTS                        0x8B4B

#define GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS        0x8A31
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS      0x8A33
#define GL_MAX_ELEMENT_INDEX                             0x8D6B
#define GL_MAX_ELEMENTS_VERTICES                         0x80E8
#define GL_MAX_FRAGMENT_UNIFORM_BLOCKS                   0x8A2D
#define GL_MAX_DRAW_BUFFERS                              0x8824
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE                     0x851C

#define GL_MAX_ELEMENTS_INDICES                          0x80E9

#define GL_MAX_UNIFORM_BUFFER_BINDINGS                   0x8A2F
#define GL_MAX_FRAGMENT_INPUT_COMPONENTS                 0x9125
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS                  0x8DFD
#define GL_MAX_UNIFORM_BLOCK_SIZE                        0x8A30
#define GL_MAX_RENDERBUFFER_SIZE                         0x84E8


#define GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS 0x8C8A
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS       0x8C8B
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS      0x8A33
#define GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS      0x8A33
#define GL_MAX_COLOR_ATTACHMENTS                         0x8CDF
#define GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS    0x8C80
#define GL_MAX_TEXTURE_IMAGE_UNITS                       0x8872
#define GL_MAX_SAMPLES                                   0x8D57
#define GL_MAX_SERVER_WAIT_TIMEOUT                       0x9111
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS               0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS                 0x8B4A

void glScissor (GLint x, GLint y, GLsizei width, GLsizei height);

void glBegin (GLenum mode);
void glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void glDisableClientState (GLenum array);
void glEnableClientState (GLenum array);
void glEnd (void);
void glVertex3fv (const GLfloat *v);
void glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void glUniform2f (GLint location, GLfloat x, GLfloat y);
void glUniform4f (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void glBeginQueryARB (GLenum target, GLuint id);
void glEndQueryARB (GLenum target);
void glGenQueriesARB (GLsizei n, GLuint* ids);
void glGetQueryObjectivARB (GLuint id, GLenum pname, GLint* params);
void glGetQueryObjectuivARB (GLuint id, GLenum pname, GLuint* params);
void glUniformMatrix3fvARB (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void glBindRenderbuffer (GLenum target, GLuint renderbuffer);
void glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void glGenRenderbuffers (GLsizei n, GLuint* renderbuffers);
void glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

void glDetachShader (GLuint program, GLuint shader);

void glColor3f (GLfloat red, GLfloat green, GLfloat blue);
GLenum glGetError (void);
const GLubyte *glGetString (GLenum name);
void glLoadMatrixf (const GLfloat *m);
void glMatrixMode (GLenum mode);
void glMultMatrixf (const GLfloat *m);
void glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void glBufferData (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
void glBindFramebuffer (GLenum target, GLuint framebuffer);
GLenum glCheckFramebufferStatus (GLenum target);
void glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void glGenFramebuffers (GLsizei n, GLuint* framebuffers);

#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4

#define GL_FRAMEBUFFER				GL_FRAMEBUFFER_EXT
#define GL_COLOR_ATTACHMENT0		GL_COLOR_ATTACHMENT0_EXT

#define GL_COLOR_ATTACHMENT1                             0x8CE1
#define GL_COLOR_ATTACHMENT2                             0x8CE2
#define GL_COLOR_ATTACHMENT3                             0x8CE3
#define GL_COLOR_ATTACHMENT4                             0x8CE4
#define GL_COLOR_ATTACHMENT5                             0x8CE5
#define GL_COLOR_ATTACHMENT6                             0x8CE6
#define GL_COLOR_ATTACHMENT7                             0x8CE7
#define GL_COLOR_ATTACHMENT8                             0x8CE8
#define GL_COLOR_ATTACHMENT9                             0x8CE9
#define GL_COLOR_ATTACHMENT10                            0x8CEA
#define GL_COLOR_ATTACHMENT11                            0x8CEB
#define GL_COLOR_ATTACHMENT12                            0x8CEC
#define GL_COLOR_ATTACHMENT13                            0x8CED
#define GL_COLOR_ATTACHMENT14                            0x8CEE
#define GL_COLOR_ATTACHMENT15                            0x8CEF

#define GL_CURRENT_QUERY                                 0x8865
#define GL_QUERY_RESULT                                  0x8866
#define GL_QUERY_RESULT_AVAILABLE                        0x8867

#define GL_RENDERBUFFER				GL_RENDERBUFFER_EXT
#define GL_DEPTH_ATTACHMENT			GL_DEPTH_ATTACHMENT_EXT
#define GL_STENCIL_ATTACHMENT		GL_STENCIL_ATTACHMENT_EXT

#define GL_FRAMEBUFFER_COMPLETE							GL_FRAMEBUFFER_COMPLETE_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT			GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT	GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT
#define	GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS			GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
#define	GL_FRAMEBUFFER_INCOMPLETE_FORMATS				GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT
#define	GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER			GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT
#define	GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER			GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT
#define	GL_FRAMEBUFFER_UNSUPPORTED						GL_FRAMEBUFFER_UNSUPPORTED_EXT
#define	GL_INVALID_FRAMEBUFFER_OPERATION				GL_INVALID_FRAMEBUFFER_OPERATION_EXT

#define GL_DEPTH_TEXTURE_MODE_OES GL_DEPTH_TEXTURE_MODE

#define GL_UNSIGNED_INT_24_OES                     0x6005   // TEMP

#define GL_RGB565                                        0x8D62
#define GL_RGB8                                          0x8051

#define GL_RASTERIZER_DISCARD                            0x8C89

#define GL_TRANSFORM_FEEDBACK_BUFFER                     0x8C8E
#define GL_UNIFORM_BUFFER                                0x8A11


void glGenBuffers(GLsizei n, GLuint* buffers);
void glBindBuffer(GLenum target, GLuint buffer);
void glBindBufferBase (GLenum target, GLuint index, GLuint buffer);

void glBindVertexArray (GLuint array);
void glDeleteVertexArrays (GLsizei n, const GLuint* arrays);
void glGenVertexArrays (GLsizei n, GLuint* arrays);

void glDrawArraysInstanced (GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);

GLuint glGetUniformBlockIndex (GLuint program, const GLchar* uniformBlockName);
void   glUniformBlockBinding (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);

void glInvalidateFramebuffer (GLenum target, GLsizei numAttachments, const GLenum* attachments);


void glValidateProgram(GLuint program);

void glDepthFunc (GLenum func);
void glActiveTexture (GLenum texture);
void glAttachShader (GLuint program, GLuint shader);
void glBindAttribLocation (GLuint program, GLuint index, const GLchar* name);
void glBindTexture (GLenum target, GLuint texture);
void glBlendFunc (GLenum sfactor, GLenum dfactor);
void glBlendEquation (GLenum mode);

void glClear (GLbitfield mask);
void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void glCompileShader (GLuint shader);
void glCopyTexImage2D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GLuint glCreateProgram (void);
GLuint glCreateShader (GLenum type);
void glCullFace (GLenum mode);
void glDepthMask (GLboolean flag);

void glDeleteFramebuffers(GLsizei n, const GLuint* textures);
void glDeleteRenderbuffers(GLsizei n, const GLuint* textures);


void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);


void glBeginTransformFeedback (GLenum primitiveMode);
void glEndTransformFeedback (void);
void glTransformFeedbackVaryings (GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode);


void glBindSampler (GLuint unit, GLuint sampler);
void glSamplerParameteri (GLuint sampler, GLenum pname, GLint param);
void glGenSamplers (GLsizei count, GLuint* samplers);
void glDeleteSamplers (GLsizei count, const GLuint* samplers);

		
void glDisable (GLenum cap);
void glDisableVertexAttribArray (GLuint);
void glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void glDepthRangef(GLclampf nearVal,  GLclampf farVal);
void glDrawArrays (GLenum mode, GLint first, GLsizei count);
void glEnable (GLenum cap);
void glEnableVertexAttribArray (GLuint);
void glFrontFace (GLenum mode);
void glGenTextures (GLsizei n, GLuint *textures);
void glGetIntegerv (GLenum pname, GLint *params);
void glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
void glGetProgramiv (GLuint program, GLenum pname, GLint* param);
void glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
void glGetShaderiv (GLuint shader, GLenum pname, GLint* param);
void glLinkProgram (GLuint program);
void glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
void glShaderSource (GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths);
void glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
void glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
void glTexParameterf (GLenum target, GLenum pname, GLfloat param);
void glTexStorage2D (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
void glTexParameteri (GLenum target, GLenum pname, GLint param);
void glUniform1f (GLint location, GLfloat v0);
void glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void glUniform1i (GLint location, GLint v0);
void glUniform2fv (GLint location, GLsizei count, const GLfloat* value);
void glUniform3fv (GLint location, GLsizei count, const GLfloat* value);
void glUniform4i (GLint location, GLint x, GLint y, GLint z, GLint w);
void glUniform4fv (GLint location, GLsizei count, const GLfloat* value);
void glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void glUseProgram (GLuint program);
void glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
void glViewport (GLint x, GLint y, GLsizei width, GLsizei height);
GLint glGetAttribLocation (GLuint program, const GLchar* name);
GLint glGetUniformLocation(GLint programObj, const GLchar* name);
void glGenerateMipmap(GLenum target);

void glGenQueries (GLsizei n, GLuint* ids);
void glDeleteQueries (GLsizei n, const GLuint* ids);
GLboolean glIsQuery (GLuint id);
void glBeginQuery (GLenum target, GLuint id);
void glEndQuery (GLenum target);
void glGetQueryiv (GLenum target, GLenum pname, GLint* params);
void glGetQueryObjectuiv (GLuint id, GLenum pname, GLuint* params);

void glDrawElementsInstanced (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount);

void glVertexAttribDivisor (GLuint index, GLuint divisor);

void glBindVertexArray (GLuint array);

void glDrawBuffers (GLsizei n, const GLenum* bufs);

void glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
GLvoid* glMapBufferRange (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
GLboolean glUnmapBuffer (GLenum target);

void glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
void glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);


#define GL_ANY_SAMPLES_PASSED                            0x8C2F
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE               0x8D6A

#define GL_MAP_READ_BIT                                  0x0001
#define GL_MAP_WRITE_BIT                                 0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT                      0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT                     0x0008

#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_BUFFER_SIZE                    0x8764
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_ONE_MINUS_CONSTANT_COLOR       0x8002

void glDeleteBuffers(GLsizei n, const GLuint* buffers);
void glDeleteProgram (GLuint program);
void glDeleteProgram (GLuint program);
GLboolean glIsProgram (GLuint program);
void glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
void glGetBufferParameteriv (GLenum target, GLenum pname, GLint* params);
void glPixelStorei (GLenum pname, GLint param);
void glPolygonOffset (GLfloat factor, GLfloat units);
void glUniform1iv (GLint location, GLsizei count, const GLint* v);

#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642
#define GL_TEXTURE_BORDER 0x1005
#define GL_TEXTURE_WIDTH 0x1000
#define GL_TEXTURE_HEIGHT 0x1001
#define GL_SAMPLE_BUFFERS 0x80A8
#define GL_SAMPLES 0x80A9
#define GL_SHADER_COMPILER 0x8dFA

void glGetBooleanv(GLenum cap, GLboolean *param);
void glShaderBinary(GLint n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLint length);

GLboolean glIsShader(GLuint shader);

void glGetShaderSource (GLuint shader, GLsizei bufsize, GLsizei* length, char* source);
void glDeleteShader (GLuint shader);
void glGetAttachedShaders (GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders);
void glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params);
void glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
void glFinish();
void glFlush();
GLboolean glIsEnabled (GLenum cap);
void glGetVertexAttribiv (GLuint index, GLenum pname, GLint* params);

}

#endif
#endif
