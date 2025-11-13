/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
fragment _float4 shader_main(MBlurInOut  inFrag    [[ stage_in ]] )
{   
    _float4 out_curpos  = inFrag.out_curpos ;
    _float4 out_prevpos = inFrag.out_prevpos ;
    
    _float2 a = (out_curpos.xy / out_curpos.w);
	_float2 b = (out_prevpos.xy / out_prevpos.w);

	_float2 diff = a - b;
	_float l = length( diff);
	
	if( l > _float(0.0) )
	{
		diff.x /= _float(0.0666667);
		diff.y /= _float(0.125);
		diff += _float(0.5);
	}
	else
	{
		diff = _float2( 0.5);
	}

	return _float4( diff, 0.0, 0.0);
}

