/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

struct LightningRenderOutput
{
	hfloat4 position [[position]];
	v_float4 fs_texcoord;
};


#ifdef TYPE_vertex

struct LightningInput
{
	hfloat4 in_position ;
	hfloat4 in_texcoord ;
};


struct render_buffer_layout
{		
	int count;
	int instance_count;
	int first;
	int base_instance;	
	
#ifdef OSX
	int __padding[60];
#endif

	_float4 positions[6*MAX_LIGHTNING_BUFFER];
}; // render_buffer;

vertex LightningRenderOutput shader_main(device LightningInput* in [[ buffer(0) ]],
#ifdef IOS
										 constant render_buffer_layout* indirect_draw_buffer [[ buffer(1) ]],
#endif
										 uint vid [[vertex_id]])
{
	LightningRenderOutput r ;
				

#ifdef IOS
	if (vid < indirect_draw_buffer->count)
	{
		r.position = in[vid].in_position;		
		r.fs_texcoord = v_float4(in[vid].in_texcoord);
	}
	else
	{
		r.position = hfloat4(0.0,-10000.0,0.0,1.0);		
		r.fs_texcoord = v_float4(0.0);
	}
#else
	r.position = in[vid].in_position;		
	r.fs_texcoord = v_float4(in[vid].in_texcoord);
#endif
	
	return r;
}

#endif


#ifdef TYPE_fragment

fragment _float4 shader_main(LightningRenderOutput in [[ stage_in ]])
{
	_float4 frag_color ;
		
	const _float4 max_color = _float4(0.627, 0.717, 0.878, 1.0); 
	const _float4 min_color = _float4(0.459, 0.509, 0.663, 1.0);     
    
    frag_color = mix(min_color, max_color,  sin( _float(in.fs_texcoord.y) * _float(3.14) ));
    
    _float intensity = in.fs_texcoord.z;	
	if (intensity > 0.9)
	{
		frag_color *= 4.0;
	}	

#ifdef SV_31
	RGBA10A2_ENCODE(frag_color.xyz) ;
#endif

	return frag_color ;
}

#endif

