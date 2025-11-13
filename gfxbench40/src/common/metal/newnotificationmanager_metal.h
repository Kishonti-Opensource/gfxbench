/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "newnotificationmanager.h"
#include "platform.h"

#include "mtl_quadBuffer.h"
#include "mtl_shader_helper.h"
#include "mtl_dynamic_data_buffer.h"

namespace GLB {

class NewNotificationManagerMetal: public NewNotificationManager
{
	public:
		NewNotificationManagerMetal();
		virtual ~NewNotificationManagerMetal();
		virtual void ShowLogo(bool stretch, bool blend);
	protected:
		virtual KCL::Texture *CreateTexture(const KCL::Image* img, bool releaseUponCommit = false);
		
	private:

        MTLRenderPassDescriptor * getRenderPassDescriptor(id<MTLTexture> texture) ;
    
        MetalRender::QuadBuffer* m_portrait_full_screen_quad ;
        MetalRender::QuadBuffer* m_landscape_full_screen_quad ;
        id <MTLCommandQueue> m_CommandQueue ;
        id <MTLDevice> m_Device ;
    
        id <MTLDepthStencilState> m_DepthState ;
        id <MTLRenderPipelineState> m_NoBlendPipelineState ;
        id <MTLRenderPipelineState> m_BlendPipelineState ;
    
        MetalRender::DynamicDataBufferPool* m_dynamic_data_buffer_pool ;
        MetalRender::DynamicDataBuffer* m_dynamic_data_buffer ;
};

}