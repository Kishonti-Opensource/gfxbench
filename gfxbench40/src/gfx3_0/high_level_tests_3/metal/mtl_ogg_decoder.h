/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_OGGDECODER_H
#define MTL_OGGDECODER_H

#include "ogg_decoder.h"
#include <string>
#include <Metal/Metal.h>
#include <vector>


class MTL_ogg_decoder : public _ogg_decoder
{
public:

	std::string m_filename; //for serialization

	MTL_ogg_decoder(const char *filename);
	virtual ~MTL_ogg_decoder();

    void Set(id<MTLRenderCommandEncoder> renderEncoder, unsigned char slot)
    {
        [renderEncoder setFragmentTexture:m_mtlTexture atIndex:slot];
        [renderEncoder setFragmentSamplerState:m_mtlSampler atIndex:slot];
    }
	bool DecodeDirect(float time);
protected:
    id<MTLTexture> m_mtlTexture;
    id<MTLSamplerState> m_mtlSampler;
    std::vector<unsigned char*> m_layers;
    unsigned int m_level_count ;
    
    void GenerateMipMapLayers() ;

    
    
    id <MTLDevice> m_Device ;
};


#endif // MTL_OGGDECODER_H
