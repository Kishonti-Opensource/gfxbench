/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GL_DEFINES_H
#define GL_DEFINES_H

#ifndef GL_DRAW_FRAMEBUFFER_BINDING 
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8CA6
#endif

#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif

#ifndef GL_DRAW_FRAMEBUFFER 
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif

#ifndef GL_READ_FRAMEBUFFER_BINDING
#define GL_READ_FRAMEBUFFER_BINDING 0x8CAA
#endif

#ifndef GL_BGR 
#define GL_BGR 0x80E0
#endif

#ifndef GL_TEXTURE_MAX_LEVEL
#define GL_TEXTURE_MAX_LEVEL 0x813D
#endif

#ifndef GL_DEPTH_COMPONENT24
#define GL_DEPTH_COMPONENT24 0x81A6
#endif

#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif

#ifndef GL_COLOR_ATTACHMENT1
#define GL_COLOR_ATTACHMENT1 0x8CE1
#endif

#ifndef GL_COLOR_ATTACHMENT2
#define GL_COLOR_ATTACHMENT2 0x8CE2
#endif

#ifndef GL_COLOR_ATTACHMENT3
#define GL_COLOR_ATTACHMENT3 0x8CE3
#endif

#ifndef GL_RGB16F
#define GL_RGB16F 0x881B
#endif

#ifndef GL_RGBA32F
#define GL_RGBA32F 0x8814
#endif

#ifndef GL_RGB32F
#define GL_RGB32F 0x8815
#endif

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

#if defined EMSCRIPTEN || defined NACL
#define glTexStorage2D( a, b, c, d, e )
#define glSamplerParameteri( a, b, c )
#define glGenSamplers( a, b )
#define glDeleteSamplers( a, b )
#define glBindSampler( a, b )
#define glGetInternalformativ( a, b, c, d, e )
#define glDrawBuffers( a, b )
#define glReadBuffer( a )
#define glDeleteVertexArrays( a, b )
#define glTexStorage3D( a, b, c, d, e, f )
#define glGenVertexArrays( a, b )
#define glBindVertexArray( a )
#define glFramebufferTextureLayer( a, b, c, d, e )
#endif

#ifndef GL_NUM_SAMPLE_COUNTS
#define GL_NUM_SAMPLE_COUNTS 0x9380
#endif

#ifndef GL_COMPARE_REF_TO_TEXTURE
#define GL_COMPARE_REF_TO_TEXTURE 0x884E
#endif

#ifndef glBlitFramebuffer
#define glBlitFramebuffer( a, b, c, d, e, f, g, h, i, j )
#endif

#ifndef glRenderbufferStorageMultisample
#define glRenderbufferStorageMultisample( a, b, c, d, e )
#endif

#ifndef GL_LINE
#define GL_LINE 0x1B01
#endif

#ifndef GL_FILL
#define GL_FILL 0x1B02
#endif

#ifndef GL_POLYGON
#define GL_POLYGON 0x0009
#endif

#ifndef GL_PROGRAM_POINT_SIZE
#define GL_PROGRAM_POINT_SIZE 0x8642
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT 0x8CD
#endif

#ifndef GL_DEPTH_TEXTURE_MODE_OES
#define GL_DEPTH_TEXTURE_MODE_OES GL_DEPTH_TEXTURE_MODE
#endif

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES 0x8D64
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS			GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT
#endif

#ifndef GL_UNSIGNED_INT_24_OES
#define GL_UNSIGNED_INT_24_OES                     0x6005   // TEMP
#endif

#ifndef GL_PATCHES
#define GL_PATCHES 0xE
#endif


#ifndef GL_PATCH_VERTICES
#define GL_PATCH_VERTICES 0x8E72
#endif

#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB 0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_ARB 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION_ARB 0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM_ARB 0x8245
#define GL_DEBUG_SOURCE_API_ARB 0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER_ARB 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY_ARB 0x8249
#define GL_DEBUG_SOURCE_APPLICATION_ARB 0x824A
#define GL_DEBUG_SOURCE_OTHER_ARB 0x824B
#define GL_DEBUG_TYPE_ERROR_ARB 0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB 0x824E
#define GL_DEBUG_TYPE_PORTABILITY_ARB 0x824F
#define GL_DEBUG_TYPE_PERFORMANCE_ARB 0x8250
#define GL_DEBUG_TYPE_OTHER_ARB 0x8251
#define GL_MAX_DEBUG_MESSAGE_LENGTH_ARB 0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES_ARB 0x9144
#define GL_DEBUG_LOGGED_MESSAGES_ARB 0x9145
#define GL_DEBUG_SEVERITY_HIGH_ARB 0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_ARB 0x9147
#define GL_DEBUG_SEVERITY_LOW_ARB 0x9148
#define GL_DEBUG_SEVERITY_NOTIFICATION 0x826B

#ifndef GL_REPEAT
#define GL_REPEAT 0x2901
#endif

#ifndef GL_NEAREST
#define GL_NEAREST 0x2600
#endif

#ifndef GL_LINEAR
#define GL_LINEAR 0x2601
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_TEXTURE_CUBE_MAP_POSITIVE_X
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#endif

#ifndef GL_UNSIGNED_BYTE
#define GL_UNSIGNED_BYTE 0x1401
#endif

#ifndef GL_UNSIGNED_INT
#define GL_UNSIGNED_INT 0x1405
#endif

#ifndef GL_UNSIGNED_SHORT
#define GL_UNSIGNED_SHORT 0x1403
#endif

#ifndef GL_FLOAT
#define GL_FLOAT 0x1406
#endif

#ifndef GL_NEAREST_MIPMAP_NEAREST
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#endif

#ifndef GL_LINEAR_MIPMAP_NEAREST
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#endif

#ifndef GL_NEAREST_MIPMAP_LINEAR
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#endif

#ifndef GL_LINEAR_MIPMAP_LINEAR
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#endif

#if 1 // GLES3+ defines for GLES2 platforms

#ifndef GL_COMPRESSED_RGBA8_ETC2_EAC
#define GL_COMPRESSED_RGBA8_ETC2_EAC 0x9278
#endif

#ifndef GL_SAMPLER_3D
#define GL_SAMPLER_3D                                    0x8B5F
#endif
#ifndef GL_SAMPLER_2D_SHADOW
#define GL_SAMPLER_2D_SHADOW                             0x8B62 
#endif
#ifndef GL_SAMPLER_2D_ARRAY
#define GL_SAMPLER_2D_ARRAY                              0x8DC1
#endif
#ifndef GL_SAMPLER_2D_ARRAY_SHADOW
#define GL_SAMPLER_2D_ARRAY_SHADOW                       0x8DC4
#endif
#ifndef GL_SAMPLER_CUBE_SHADOW
#define GL_SAMPLER_CUBE_SHADOW                           0x8DC5
#endif
#ifndef GL_INT_SAMPLER_2D
#define GL_INT_SAMPLER_2D                                0x8DCA
#endif
#ifndef GL_INT_SAMPLER_3D
#define GL_INT_SAMPLER_3D                                0x8DCB
#endif
#ifndef GL_INT_SAMPLER_CUBE
#define GL_INT_SAMPLER_CUBE                              0x8DCC
#endif
#ifndef GL_INT_SAMPLER_2D_ARRAY
#define GL_INT_SAMPLER_2D_ARRAY                          0x8DCF
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_2D
#define GL_UNSIGNED_INT_SAMPLER_2D                       0x8DD2
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_3D
#define GL_UNSIGNED_INT_SAMPLER_3D                       0x8DD3
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_CUBE
#define GL_UNSIGNED_INT_SAMPLER_CUBE                     0x8DD4
#endif
#ifndef GL_UNSIGNED_INT_SAMPLER_2D_ARRAY
#define GL_UNSIGNED_INT_SAMPLER_2D_ARRAY                 0x8DD7 
#endif
#ifndef GL_TEXTURE_2D_ARRAY
#define GL_TEXTURE_2D_ARRAY                              0x8C1A
#endif
#ifndef GL_RGB10_A2
#define GL_RGB10_A2                                      0x8059
#endif
#endif // GLES3+ defines for GLES2 platforms

#endif

