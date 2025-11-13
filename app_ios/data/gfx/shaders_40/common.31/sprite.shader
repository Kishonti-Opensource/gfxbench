/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform vec2 TextureSize;
uniform vec2 ViewportSize;
uniform sampler2D texture_unit0;

#ifndef INSTANCING
uniform mat4 Transform;
uniform vec4 Color;
uniform vec4 SourceRect;
#endif

#ifdef TYPE_vertex

#ifndef INSTANCING
layout(location = 0) in vec4 PosUV;
#else
layout(location = 0) in vec4 PosUV;
layout(location = 1) in vec4 Transform0;
layout(location = 2) in vec4 Transform1;
layout(location = 3) in vec4 Transform2;
layout(location = 4) in vec4 Transform3;
layout(location = 5) in vec4 Color;
layout(location = 6) in vec4 SourceRect;
#endif

out vec2 outTexCoord;
out vec4 outColor;

void SpriteVSCommon(vec2 position, 
                        vec2 texCoord, 
                        mat4 transform, 
                        vec4 color, 
                        vec4 sourceRect)
{
    // Scale the quad so that it's texture-sized    
    vec4 positionSS = vec4(position * sourceRect.zw, 0.0, 1.0);
    
    // Apply transforms in screen space
    positionSS = transform * positionSS;

    // Scale by the viewport size, flip Y, then rescale to device coordinates
    vec4 positionDS = positionSS;
    positionDS.xy /= ViewportSize;
    positionDS = positionDS * 2.0 - 1.0;
    positionDS.y *= -1.0;

    // Figure out the texture coordinates
    outTexCoord = texCoord;
    outTexCoord.xy *= sourceRect.zw / TextureSize;
    outTexCoord.xy += sourceRect.xy / TextureSize;
	outTexCoord.y *= -1.0;

    gl_Position = positionDS;

    outColor = color;
}

void main()
{
#ifdef INSTANCING
	mat4 Transform = mat4(Transform0, Transform1, Transform2, Transform3);
#endif
    SpriteVSCommon(PosUV.xy, PosUV.zw, /*transpose(*/Transform, Color, SourceRect); 
}

#endif

#ifdef TYPE_fragment

in vec2 outTexCoord;
in vec4 outColor;

out vec4 fragColor;

void main()
{
    vec4 texColor = texture(texture_unit0, outTexCoord);    
    texColor = texColor * outColor;    
    //texColor.rgb *= texColor.a;
    fragColor = texColor;
	
	//fragColor = vec4(1.0,0.0,0.0,1.0);
}

#endif