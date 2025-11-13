/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
// FXAA shader, GLSL code adapted from:
// http://horde3d.org/wiki/index.php5?title=Shading_Technique_-_FXAA
// Whitepaper describing the technique:
// http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf

//#define textureSampler gbuffer_color_texture
//#define texcoordOffset OneOverScreenSize

#ifdef GL_ES
precision mediump float;
precision mediump int;
#endif

uniform sampler2D textureSampler;

// The inverse of the texture dimensions along X and Y
uniform vec2 texcoordOffset;

in vec2 vertTexcoord;
out vec4 FragColor;


void main() {
  // The parameters are hardcoded for now, but could be
  // made into uniforms to control fromt he program.
  float FXAA_SPAN_MAX = 8.0;
  float FXAA_REDUCE_MUL = 1.0/8.0;
  float FXAA_REDUCE_MIN = (1.0/128.0);

  vec3 rgbNW = texture(textureSampler, vertTexcoord.xy + (vec2(-1.0, -1.0) * texcoordOffset)).xyz;
  vec3 rgbNE = texture(textureSampler, vertTexcoord.xy + (vec2(+1.0, -1.0) * texcoordOffset)).xyz;
  vec3 rgbSW = texture(textureSampler, vertTexcoord.xy + (vec2(-1.0, +1.0) * texcoordOffset)).xyz;
  vec3 rgbSE = texture(textureSampler, vertTexcoord.xy + (vec2(+1.0, +1.0) * texcoordOffset)).xyz;
  vec3 rgbM  = texture(textureSampler, vertTexcoord.xy).xyz;
	
  //vec3 luma = vec3(0.299, 0.587, 0.114);
 vec3 luma = vec3(0.2126, 0.7152, 0.0722);
  
  float lumaNW = dot(rgbNW, luma);
  float lumaNE = dot(rgbNE, luma);
  float lumaSW = dot(rgbSW, luma);
  float lumaSE = dot(rgbSE, luma);
  float lumaM  = dot( rgbM, luma);
	
  float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
  float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
	
  vec2 dir;
  dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
  dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
	
  float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
	  
  float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
	
  dir = min(vec2(FXAA_SPAN_MAX,  FXAA_SPAN_MAX), 
        max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) * texcoordOffset;
		
  vec3 rgbA = (1.0/2.0) * (
              texture2D(textureSampler, vertTexcoord.xy + dir * (1.0/3.0 - 0.5)).xyz +
              texture2D(textureSampler, vertTexcoord.xy + dir * (2.0/3.0 - 0.5)).xyz);
  vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
              texture2D(textureSampler, vertTexcoord.xy + dir * (0.0/3.0 - 0.5)).xyz +
              texture2D(textureSampler, vertTexcoord.xy + dir * (3.0/3.0 - 0.5)).xyz);
  float lumaB = dot(rgbB, luma);

  if((lumaB < lumaMin) || (lumaB > lumaMax)){
    FragColor.xyz=rgbA;
  } else {
    FragColor.xyz=rgbB;
  }
	FragColor.a = 1.0;
    //FragColor.xyz=rgbM;
}
