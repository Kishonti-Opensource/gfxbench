/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "testfw.h"
#include "graphics/metalgraphicscontext.h"
#include "ng/log.h"
#include "ng/require.h"
#include <stdlib.h>
#include "schemas/result.h"

#import <Foundation/Foundation.h>

namespace tfw
{
    
    class Metal : public TestBase
    {
    public:
        Metal() {}
        bool init()
        {
            require(ctx_);
            require(ctx_->makeCurrent());
			NGLOG_INFO("config: %s", config());
            return true;
        }
        
        void run()
        {
			MetalGraphicsContext *ctx = (MetalGraphicsContext*)ctx_;
			while(!isCancelled()) {
				float r = (rand()%32768)/32768.f;
				float g = (rand()%32768)/32768.f;
				float b = (rand()%32768)/32768.f;

				@autoreleasepool {
					id<MTLTexture> backBuffer = ctx->getBackBufferTexture();
					MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
					passDescriptor.colorAttachments[0].texture = backBuffer;
					passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
					passDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
					passDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(r, g, b, 1.0);
					id<MTLCommandQueue> commandQueue = ctx->getMainCommandQueue();
					id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
					id <MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
					[commandEncoder endEncoding];
					[commandBuffer commit];
					ctx->swapBuffers();
				}
			}
		}
		
		std::string result()
		{
			tfw::ResultGroup g;
			Result r;
			r.setTestId(name());
			r.setScore(0);
			g.addResult(r);
			std::string s = g.toJsonString();
			return s;
		}
    };
    
}

CREATE_FACTORY(tfw_metal, tfw::Metal)