/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef AKBTYPES_H
#define AKBTYPES_H

#include <kcl_base.h>
#include <kcl_math3d.h>
#include "../gfxbench/global_test_environment.h"

class MetalGraphicsContext ;


#define DEBUG_RENEW_BUFFER( COMMAND_BUFFER ) \
{ \
	NSUInteger status = [COMMAND_BUFFER status]; \
	if ( (status == MTLCommandBufferStatusCommitted) || (status == MTLCommandBufferStatusCompleted) ) \
	{ \
		COMMAND_BUFFER = [[COMMAND_BUFFER commandQueue] commandBuffer]; \
		COMMAND_BUFFER.label = @"__DEBUG_COMMAND_BUFFER__"; \
	} \
}

#define DEBUG_COMMIT_BUFFER( COMMAND_BUFFER ) \
{ \
	NSString *command_buffer_label = @"__DEBUG_COMMAND_BUFFER__"; \
	if ( [COMMAND_BUFFER.label isEqualToString:command_buffer_label] ) \
	{ \
		[COMMAND_BUFFER commit]; \
	} \
}


namespace MetalRender
{
	void InitMetalGraphicsContext(GraphicsContext *ctx, const std::string device_id);
	
	void Initialize(const GlobalTestEnvironment* const gte);
	void Release();
    void Finish();
    
    const char* GetDeviceName() ;
    bool isASTCSupported();
    
    MetalGraphicsContext* GetContext() ;

    const uint8_t METAL_MAX_FRAME_LAG = 3;

	enum ShaderType
	{
		kShaderTypeSingleRGBA8Default = 0,
        kShaderTypeSingleBGRA8,
        kShaderTypeSingleBGR565,
        kShaderTypeSingleRGB10A2,
		kShaderTypeSingleRG8,
		kShaderTypeSingleR8,
        SINGLE_SHADER_TYPE_COUNT,
		kShaderTypeTransformFeedback,
        kShaderTypeNoColorAttachment,
        kShaderTypeManhattanGBuffer,
		kShaderTypeCarChaseShadow,
        kShaderTypeCarChaseGBuffer,
        kShaderTypeUnknown
	};
}

#endif // AKBTYPES_H

