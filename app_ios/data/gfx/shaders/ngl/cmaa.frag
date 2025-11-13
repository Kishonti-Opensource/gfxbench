/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
precision highp float;
precision highp int;
                             
#define SETTINGS_ALLOW_SHORT_Zs 1                                                                                                                                                               
#define EDGE_DETECT_THRESHOLD   10000.0                                                                                                                                                           
                                                                                                                                                                                                
#define saturate(x)     clamp((x), 0.0, 1.0)                                                                                                                                                    

uniform float g_Depth;  
uniform vec2   OneOverScreenSize;                                                                                                                                                                    
#ifndef EDGE_DETECT_THRESHOLD                                                                                                                                                                   
uniform float  ColorThreshold;                                                                                                                                                                     
#endif                                                                                                                                                                                          
             
#ifdef SUPPORTS_USAMPLER2D                                                                                                                                                 
#define USAMPLER usampler2D
#define UVEC4    uvec4
#define LOAD_UINT(arg) arg
#define STORE_UVEC4(arg) arg
#else
#define USAMPLER sampler2D
#define UVEC4    vec4
#define LOAD_UINT(arg) uint(arg * 255.0f)
#define STORE_UVEC4(arg) vec4(float(arg.x) / 255.0f, float(arg.y) / 255.0f, float(arg.z) / 255.0f, float(arg.w) / 255.0f)
#endif

// bind to texture stage 0/1
highp uniform sampler2D                   g_screenTexture;                                                                                                                        
highp uniform sampler2D                   g_src0TextureFlt;                                                                                                                       
highp uniform USAMPLER                    g_src0Texture4Uint;                                                                                                                     

// bind to image stage 0/1/2
highp layout(EDGE_READ_FORMAT)		restrict writeonly uniform image2D  g_resultTexture;                                                                                                                        
highp layout(rgba8)				restrict writeonly uniform image2D  g_resultTextureFlt4Slot1;                                                                                                               
highp layout(EDGE_READ_FORMAT)		restrict writeonly uniform image2D  g_resultTextureSlot2;                                                                                                                   
                                                                                                                                                                                                
// Constants                                                                                                                                                                                    
const vec4 c_lumWeights = vec4( 0.2126f, 0.7152f, 0.0722f, 0.0000f );                                                                                                                               
                                                                                                                                                                                                
#ifdef EDGE_DETECT_THRESHOLD                                                                                                                                                                    
const float c_ColorThreshold = 1.0f / EDGE_DETECT_THRESHOLD;                                                                                                                                    
#endif                                                                                                                                                                                          
                                                                                                                                                                                                
// Must be even number; Will work with ~16 pretty good too for additional performance, or with ~64 for highest quality.                                                                         
const int c_maxLineLength = 64;                                                                                                                                                                 
                                                                                                                                                                                                
const vec4 c_edgeDebugColours[5] = vec4[5]( vec4( 0.5, 0.5, 0.5, .4 ), vec4( 1, 0.1, 1.0, 0.8 ), vec4( 0.9, 0, 0, 0.8 ), vec4( 0, 0.9, 0, .8 ), vec4( 0, 0, 0.9, 0.8 ) );                       
                                                                                                                                                                                                
// this isn't needed if colour UAV is _SRGB but that doesn't work everywhere                                                                                                                    
#ifdef IN_GAMMA_CORRECT_MODE                                                                                                                                                                       
/////////////////////////////////////////////////////////////////////////////////////////                                                                                                       
//                                                                                                                                                                                              
// SRGB Helper Functions taken from D3DX_DXGIFormatConvert.inl                                                                                                                                  
float D3DX_FLOAT_to_SRGB(float val)                                                                                                                                                             
{                                                                                                                                                                                               
    if( val < 0.0031308f )                                                                                                                                                                      
        val *= 12.92f;                                                                                                                                                                          
    else                                                                                                                                                                                        
    {                                                                                                                                                                                           
        val = 1.055f * pow(val,1.0f/2.4f) - 0.055f;                                                                                                                                             
    }                                                                                                                                                                                           
    return val;                                                                                                                                                                                 
}                                                                                                                                                                                               
//                                                                                                                                                                                              
vec3 D3DX_FLOAT3_to_SRGB(vec3 val)                                                                                                                                                              
{                                                                                                                                                                                               
    vec3 outVal;                                                                                                                                                                                
    outVal.x = D3DX_FLOAT_to_SRGB( val.x );                                                                                                                                                     
    outVal.y = D3DX_FLOAT_to_SRGB( val.y );                                                                                                                                                     
    outVal.z = D3DX_FLOAT_to_SRGB( val.z );                                                                                                                                                     
    return outVal;                                                                                                                                                                              
}                                                                                                                                                                                               
//                                                                                                                                                                                              
/////////////////////////////////////////////////////////////////////////////////////////                                                                                                       
#endif // IN_GAMMA_CORRECT_MODE                                                                                                                                                                 
                                                                                                                                                                                                
// how .rgba channels from the edge texture maps to pixel edges:                                                                                                                                
//                                                                                                                                                                                              
//                   A - 0x08                                                                                                                                                                   
//              ||                                                                                                                                                                     
//              |         |                                                                                                                                                                     
//     0x04 - B |  pixel  | R - 0x01                                                                                                                                                            
//              |         |                                                                                                                                                                     
//              |_________|                                                                                                                                                                     
//                   G - 0x02                                                                                                                                                                   
//                                                                                                                                                                                              
// (A - there's an edge between us and a pixel above us)                                                                                                                                        
// (R - there's an edge between us and a pixel to the right)                                                                                                                                    
// (G - there's an edge between us and a pixel at the bottom)                                                                                                                                   
// (B - there's an edge between us and a pixel to the left)                                                                                                                                     
                                                                                                                                                                                                
// Expecting values of 1 and 0 only!                                                                                                                                                            
uint PackEdge( uvec4 edges )                                                                                                                                                                    
{                                                                                                                                                                                               
   return (edges.x << 0u) | (edges.y << 1u) | (edges.z << 2u) | (edges.w << 3u);                                                                                                                
}                                                                                                                                                                                               
                                                                                                                                                                                                
uvec4 UnpackEdge( uint value )                                                                                                                                                                  
{                                                                                                                                                                                               
   uvec4 ret;                                                                                                                                                                                   
   ret.x = (value & 0x01u) != 0u ? 1u : 0u;                                                                                                                                                     
   ret.y = (value & 0x02u) != 0u ? 1u : 0u;                                                                                                                                                     
   ret.z = (value & 0x04u) != 0u ? 1u : 0u;                                                                                                                                                     
   ret.w = (value & 0x08u) != 0u ? 1u : 0u;                                                                                                                                                     
   return ret;                                                                                                                                                                                  
}                                                                                                                                                                                               
                                                                                                                                                                                                
uint PackZ( const uvec2 screenPos, const bool invertedZShape )                                                                                                                                  
{                                                                                                                                                                                               
   uint retVal = screenPos.x | (screenPos.y << 15u);                                                                                                                                            
   if( invertedZShape )                                                                                                                                                                         
      retVal |= (1u << 30u);                                                                                                                                                                    
   return retVal;                                                                                                                                                                               
}                                                                                                                                                                                               
                                                                                                                                                                                                
void UnpackZ( uint packedZ, out uvec2 screenPos, out bool invertedZShape )                                                                                                                      
{                                                                                                                                                                                               
   screenPos.x = packedZ & 0x7FFFu;                                                                                                                                                             
   screenPos.y = (packedZ>>15u) & 0x7FFFu;                                                                                                                                                      
   invertedZShape = (packedZ>>30u) == 1u;                                                                                                                                                       
}                                                                                                                                                                                               
                                                                                                                                                                                                
uint PackZ( const uvec2 screenPos, const bool invertedZShape, const bool horizontal )                                                                                                           
{                                                                                                                                                                                               
   uint retVal = screenPos.x | (screenPos.y << 15u);                                                                                                                                            
   if( invertedZShape )                                                                                                                                                                         
      retVal |= (1u << 30u);                                                                                                                                                                    
   if( horizontal )                                                                                                                                                                             
      retVal |= (1u << 31u);                                                                                                                                                                    
   return retVal;                                                                                                                                                                               
}                                                                                                                                                                                               
                                                                                                                                                                                                
void UnpackZ( uint packedZ, out uvec2 screenPos, out bool invertedZShape, out bool horizontal )                                                                                                 
{                                                                                                                                                                                               
   screenPos.x    = packedZ & 0x7FFFu;                                                                                                                                                          
   screenPos.y    = (packedZ>>15u) & 0x7FFFu;                                                                                                                                                   
   invertedZShape = (packedZ & (1u << 30u)) != 0u;                                                                                                                                              
   horizontal     = (packedZ & (1u << 31u)) != 0u;                                                                                                                                              
}                                                                                                                                                                                               
                                                                                                                                                                                                
vec4 PackBlurAAInfo( ivec2 pixelPos, uint shapeType )                                                                                                                                           
{                                                                                                                                                                                               
    uint packedEdges = uint(texelFetch(g_src0TextureFlt, pixelPos, 0).r * 255.5);                                                                                                               
                                                                                                                                                                                                
    float retval = float(packedEdges + (shapeType << 4u));                                                                                                                                      
                                                                                                                                                                                                
    return vec4(retval / 255.0);                                                                                                                                                                
}                                                                                                                                                                                               
                                                                                                                                                                                                
void UnpackBlurAAInfo( float packedValue, out uint edges, out uint shapeType )                                                                                                                  
{                                                                                                                                                                                               
    uint packedValueInt = uint(packedValue*255.5);                                                                                                                                              
    edges       = packedValueInt & 0xFu;                                                                                                                                                        
    shapeType   = packedValueInt >> 4u;                                                                                                                                                         
}                                                                                                                                                                                               
                                                                                                                                                                                                
float EdgeDetectColorCalcDiff( vec3 colorA, vec3 colorB )                                                                                                                                       
{                                                                                                                                                                                               
#ifdef IN_BGR_MODE                                                                                                                                                                                 
    vec3 LumWeights   = c_lumWeights.bgr;                                                                                                                                                       
#else                                                                                                                                                                                           
    vec3 LumWeights   = c_lumWeights.rgb;                                                                                                                                                       
#endif                                                                                                                                                                                          
                                                                                                                                                                                                
    return dot( abs( colorA.rgb - colorB.rgb  ), LumWeights );                                                                                                                                  
}                                                                                                                                                                                               
                                                                                                                                                                                                
bool EdgeDetectColor( vec3 colorA, vec3 colorB )                                                                                                                                                
{                                                                                                                                                                                               
#ifdef EDGE_DETECT_THRESHOLD                                                                                                                                                                    
    return EdgeDetectColorCalcDiff( colorA, colorB ) > c_ColorThreshold;                                                                                                                        
#else                                                                                                                                                                                           
     return EdgeDetectColorCalcDiff( colorA, colorB ) > ColorThreshold;                                                                                                             
#endif                                                                                                                                                                                          
}                                                                                                                                                                                               
                                                                                                                                                                                                
void FindLineLength( out int lineLengthLeft, out int lineLengthRight, ivec2 screenPos, const bool horizontal, const bool invertedZShape, const ivec2 stepRight )                                
{                                                                                                                                                                                               
   /////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                                    
   // TODO: there must be a cleaner and faster way to get to these - a precalculated array indexing maybe?                                                                                      
   uint maskLeft, bitsContinueLeft, maskRight, bitsContinueRight;                                                                                                                               
   {                                                                                                                                                                                            
      // Horizontal (vertical is the same, just rotated 90 counter-clockwise)                                                                                                                  
      // Inverted Z case:              // Normal Z case:                                                                                                                                        
      //   __                          // __                                                                                                                                                    
      //  X|                           //  X|                                                                                                                                                   
      //                             //                                                                                                                                                     
      uint maskTraceLeft, maskTraceRight;                                                                                                                                                       
      uint maskStopLeft, maskStopRight;                                                                                                                                                         
      if( horizontal )                                                                                                                                                                          
      {                                                                                                                                                                                         
         if( invertedZShape )                                                                                                                                                                   
         {                                                                                                                                                                                      
            maskTraceLeft    = 0x02u; // tracing bottom edge                                                                                                                                    
            maskTraceRight   = 0x08u; // tracing top edge                                                                                                                                       
         }                                                                                                                                                                                      
         else                                                                                                                                                                                   
         {                                                                                                                                                                                      
            maskTraceLeft    = 0x08u; // tracing top edge                                                                                                                                       
            maskTraceRight   = 0x02u; // tracing bottom edge                                                                                                                                    
         }                                                                                                                                                                                      
         maskStopLeft   = 0x01u; // stop on right edge                                                                                                                                          
         maskStopRight  = 0x04u; // stop on left edge                                                                                                                                           
      }                                                                                                                                                                                         
      else                                                                                                                                                                                      
      {                                                                                                                                                                                         
         if( invertedZShape )                                                                                                                                                                   
         {                                                                                                                                                                                      
            maskTraceLeft    = 0x01u; // tracing right edge                                                                                                                                     
            maskTraceRight   = 0x04u; // tracing left edge                                                                                                                                      
         }                                                                                                                                                                                      
         else                                                                                                                                                                                   
         {                                                                                                                                                                                      
            maskTraceLeft    = 0x04u; // tracing left edge                                                                                                                                      
            maskTraceRight   = 0x01u; // tracing right edge                                                                                                                                     
         }                                                                                                                                                                                      
         maskStopLeft   = 0x08u; // stop on top edge                                                                                                                                            
         maskStopRight  = 0x02u; // stop on bottom edge                                                                                                                                         
      }                                                                                                                                                                                         
                                                                                                                                                                                                
      maskLeft         = maskTraceLeft | maskStopLeft;                                                                                                                                          
      bitsContinueLeft = maskTraceLeft;                                                                                                                                                         
      maskRight        = maskTraceRight | maskStopRight;                                                                                                                                        
      bitsContinueRight= maskTraceRight;                                                                                                                                                        
   }                                                                                                                                                                                            
   /////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                                    
                                                                                                                                                                                                
#ifdef SETTINGS_ALLOW_SHORT_Zs                                                                                                                                                                  
   int i = 1;                                                                                                                                                                                   
#else                                                                                                                                                                                           
   int i = 2; // starting from 2 because we already know it's at least 2...                                                                                                                     
#endif                                                                                                                                                                                          
   for( ; i < c_maxLineLength; i++ )                                                                                                                                                            
   {                                                                                                                                                                                            
      uint edgeLeft  = uint(texelFetch(g_src0TextureFlt, ivec2(screenPos.xy - stepRight * i), 0).r * 255.5);                                                                                    
      uint edgeRight = uint(texelFetch(g_src0TextureFlt, ivec2(screenPos.xy + stepRight * (i+1)), 0).r * 255.5);                                                                                
                                                                                                                                                                                                
      // stop on encountering 'stopping' edge (as defined by masks)                                                                                                                             
      int stopLeft  = (edgeLeft & maskLeft) != bitsContinueLeft ? 1 : 0;                                                                                                                        
      int stopRight = (edgeRight & maskRight) != bitsContinueRight ? 1 : 0;                                                                                                                     
                                                                                                                                                                                                
      if( bool(stopLeft) || bool(stopRight) )                                                                                                                                                   
      {                                                                                                                                                                                         
         lineLengthLeft = 1 + i - stopLeft;                                                                                                                                                     
         lineLengthRight = 1 + i - stopRight;                                                                                                                                                   
         return;                                                                                                                                                                                
      }                                                                                                                                                                                         
   }                                                                                                                                                                                            
   lineLengthLeft = lineLengthRight = i;                                                                                                                                                        
   return;                                                                                                                                                                                      
}                                                                                                                                                                                               
                                                                                                                                                                                                
void ProcessDetectedZ( ivec2 screenPos, bool horizontal, bool invertedZShape )                                                                                                                  
{                                                                                                                                                                                               
   int lineLengthLeft, lineLengthRight;                                                                                                                                                         
                                                                                                                                                                                                
   ivec2 stepRight  = (horizontal)?( ivec2( 1, 0 ) ):( ivec2( 0,  -1 ) );                                                                                                                       
   vec2 blendDir    = (horizontal)?( vec2( 0, -1 ) ):( vec2( -1,  0 ) );                                                                                                                        
                                                                                                                                                                                                
   FindLineLength( lineLengthLeft, lineLengthRight, screenPos, horizontal, invertedZShape, stepRight );                                                                                         
                                                                                                                                                                                                
   vec2 pixelSize = OneOverScreenSize;                                                                                                                                              
                                                                                                                                                                                                
   float leftOdd  = 0.15 * float(lineLengthLeft % 2);                                                                                                                                           
   float rightOdd = 0.15 * float(lineLengthRight % 2);                                                                                                                                          
                                                                                                                                                                                                
   int loopFrom = -int((lineLengthLeft+1)/2)+1;                                                                                                                                                 
   int loopTo   = int((lineLengthRight+1)/2);                                                                                                                                                   
                                                                                                                                                                                                
   float totalLength = float(loopTo - loopFrom) + 1.0 - leftOdd - rightOdd;                                                                                                                     
                                                                                                                                                                                                
   for( int i = loopFrom; i <= loopTo; i++ )                                                                                                                                                    
   {                                                                                                                                                                                            
      ivec2   pixelPos    = screenPos + stepRight * i;                                                                                                                                          
      vec2    pixelPosFlt = vec2( float(pixelPos.x) + 0.5, float(pixelPos.y) + 0.5 );                                                                                                           
                                                                                                                                                                                                
#ifdef DEBUG_OUTPUT_AAINFO                                                                                                                                                                         
      imageStore(g_resultTextureSlot2, pixelPos, PackBlurAAInfo( pixelPos, 1u ));                                                                                                               
#endif                                                                                                                                                                                          
                                                                                                                                                                                                
      float m = (float(i) + 0.5 - leftOdd - float(loopFrom)) / totalLength;                                                                                                                     
      m = saturate( m );                                                                                                                                                                        
      float k = m - ((i > 0) ? 1.0 : 0.0);                                                                                                                                                      
      k = (invertedZShape)?(-k):(k);                                                                                                                                                            
                                                                                                                                                                                                
      vec4 color = textureLod(g_screenTexture, (pixelPosFlt + blendDir * k) * pixelSize, 0.0 );                                                                                                 
                                                                                                                                                                                                
#ifdef IN_GAMMA_CORRECT_MODE                                                                                                                                                                       
      color.rgb = D3DX_FLOAT3_to_SRGB( color.rgb );                                                                                                                                             
#endif
      imageStore(g_resultTextureFlt4Slot1, pixelPos, color);                                                                                                                                    
   }                                                                                                                                                                                            
}                                                                                                                                                                                               
                                                                                                                                                                                                
vec4 CalcDbgDisplayColor( const vec4 blurMap )                                                                                                                                                  
{                                                                                                                                                                                               
   vec3 pixelC = vec3( 0.0, 0.0, 0.0 );                                                                                                                                                         
   vec3 pixelL = vec3( 0.0, 0.0, 1.0 );                                                                                                                                                         
   vec3 pixelT = vec3( 1.0, 0.0, 0.0 );                                                                                                                                                         
   vec3 pixelR = vec3( 0.0, 1.0, 0.0 );                                                                                                                                                         
   vec3 pixelB = vec3( 0.8, 0.8, 0.0 );                                                                                                                                                         
                                                                                                                                                                                                
   const float centerWeight      = 1.0;                                                                                                                                                         
   float fromBelowWeight   = (1.0 / (1.0 - blurMap.x)) - 1.0;                                                                                                                                   
   float fromAboveWeight   = (1.0 / (1.0 - blurMap.y)) - 1.0;                                                                                                                                   
   float fromRightWeight   = (1.0 / (1.0 - blurMap.z)) - 1.0;                                                                                                                                   
   float fromLeftWeight    = (1.0 / (1.0 - blurMap.w)) - 1.0;                                                                                                                                   
                                                                                                                                                                                                
   float weightSum = centerWeight + dot( vec4( fromBelowWeight, fromAboveWeight, fromRightWeight, fromLeftWeight ), vec4( 1, 1, 1, 1 ) );                                                       
                                                                                                                                                                                                
   vec4 pixel;                                                                                                                                                                                  
                                                                                                                                                                                                
   pixel.rgb = pixelC.rgb + fromAboveWeight * pixelT + fromBelowWeight * pixelB +                                                                                                               
      fromLeftWeight * pixelL + fromRightWeight * pixelR;                                                                                                                                       
   pixel.rgb /= weightSum;                                                                                                                                                                      
                                                                                                                                                                                                
   pixel.a = dot( pixel.rgb, vec3( 1, 1, 1 ) ) * 100.0;                                                                                                                                         
                                                                                                                                                                                                
   return saturate( pixel );                                                                                                                                                                    
}                                                                                                                                                                                               
                                                                                                                                                                                                
#ifdef DETECT_EDGES1                                                                                                                                                                            
layout(location = 0) out UVEC4 outEdges;                                                                                                                                                                                                                                                                                                               
void DetectEdges1()                                                                                                                                                                             
{    
	uvec4 outputEdges;                                                                                                                                                                                           
    ivec2 screenPosI = ivec2( gl_FragCoord.xy ) * ivec2( 2, 2 );   
       
    // .rgb contains colour, .a contains flag whether to output it to working colour texture                                                                                                    
    vec4 pixel00   = texelFetch(g_screenTexture,       screenPosI.xy, 0                );                                                                                                       
    vec4 pixel10   = texelFetchOffset(g_screenTexture, screenPosI.xy, 0, ivec2(  1, 0 ));                                                                                                       
    vec4 pixel20   = texelFetchOffset(g_screenTexture, screenPosI.xy, 0, ivec2(  2, 0 ));                                                                                                       
    vec4 pixel01   = texelFetchOffset(g_screenTexture, screenPosI.xy, 0, ivec2(  0, 1 ));                                                                                                       
    vec4 pixel11   = texelFetchOffset(g_screenTexture, screenPosI.xy, 0, ivec2(  1, 1 ));                                                                                                       
    vec4 pixel21   = texelFetchOffset(g_screenTexture, screenPosI.xy, 0, ivec2(  2, 1 ));                                                                                                       
    vec4 pixel02   = texelFetchOffset(g_screenTexture, screenPosI.xy, 0, ivec2(  0, 2 ));                                                                                                       
    vec4 pixel12   = texelFetchOffset(g_screenTexture, screenPosI.xy, 0, ivec2(  1, 2 ));    

    float storeFlagPixel00 = 0.0;                                                                                                                                                               
    float storeFlagPixel10 = 0.0;                                                                                                                                                               
    float storeFlagPixel20 = 0.0;                                                                                                                                                               
    float storeFlagPixel01 = 0.0;                                                                                                                                                               
    float storeFlagPixel11 = 0.0;                                                                                                                                                               
    float storeFlagPixel21 = 0.0;                                                                                                                                                               
    float storeFlagPixel02 = 0.0;                                                                                                                                                               
    float storeFlagPixel12 = 0.0;                                                                                                                                                               
                          
    vec2 et;                                                                                                                                                                                    
                                                                                                                                                                                                
#ifdef EDGE_DETECT_THRESHOLD                                                                                                                                                                    
    float threshold = c_ColorThreshold;                                                                                                                                                         
#else                                                                                                                                                                                           
    float threshold = ColorThreshold;                                                                                                                                               
#endif                                                                                                                                                                                          
                                                                                                                                                                                                
    {                                                                                                                                                                                           
        et.x = EdgeDetectColorCalcDiff( pixel00.rgb, pixel10.rgb );                                                                                                                             
        et.y = EdgeDetectColorCalcDiff( pixel00.rgb, pixel01.rgb );                                                                                                                             
        et = saturate( et - threshold );                                                                                                                                                        
        ivec2 eti = ivec2( et * 15.0 + 0.99 );                                                                                                                                                  
        outputEdges.x = uint(eti.x | (eti.y << 4));                                                                                                                                                
                                                                                                                                                                                                
        storeFlagPixel00 += et.x;                                                                                                                                                               
        storeFlagPixel00 += et.y;                                                                                                                                                               
        storeFlagPixel10 += et.x;                                                                                                                                                               
        storeFlagPixel01 += et.y;                                                                                                                                                               
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    {                                                                                                                                                                                           
        et.x = EdgeDetectColorCalcDiff( pixel10.rgb, pixel20.rgb );                                                                                                                             
        et.y = EdgeDetectColorCalcDiff( pixel10.rgb, pixel11.rgb );                                                                                                                             
        et = saturate( et - threshold );                                                                                                                                                        
        ivec2 eti = ivec2( et * 15.0 + 0.99 );                                                                                                                                                  
        outputEdges.y = uint(eti.x | (eti.y << 4));                                                                                                                                                
                                                                                                                                                                                                
        storeFlagPixel10 += et.x;                                                                                                                                                               
        storeFlagPixel10 += et.y;                                                                                                                                                               
        storeFlagPixel20 += et.x;                                                                                                                                                               
        storeFlagPixel11 += et.y;                                                                                                                                                               
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    {                                                                                                                                                                                           
        et.x = EdgeDetectColorCalcDiff( pixel01.rgb, pixel11.rgb );                                                                                                                             
        et.y = EdgeDetectColorCalcDiff( pixel01.rgb, pixel02.rgb );                                                                                                                             
        et = saturate( et - threshold );                                                                                                                                                        
        ivec2 eti = ivec2( et * 15.0 + 0.99 );                                                                                                                                                  
        outputEdges.z = uint(eti.x | (eti.y << 4));                                                                                                                                                
                                                                                                                                                                                                
        storeFlagPixel01 += et.x;                                                                                                                                                               
        storeFlagPixel01 += et.y;                                                                                                                                                               
        storeFlagPixel11 += et.x;                                                                                                                                                               
        storeFlagPixel02 += et.y;                                                                                                                                                               
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    {                                                                                                                                                                                           
        et.x = EdgeDetectColorCalcDiff( pixel11.rgb, pixel21.rgb );                                                                                                                             
        et.y = EdgeDetectColorCalcDiff( pixel11.rgb, pixel12.rgb );                                                                                                                             
        et = saturate( et - threshold );                                                                                                                                                        
        ivec2 eti = ivec2( et * 15.0 + 0.99 );                                                                                                                                                  
        outputEdges.w = uint(eti.x | (eti.y << 4));                                                                                                                                                
                                                                                                                                                                                                
        storeFlagPixel11 += et.x;                                                                                                                                                               
        storeFlagPixel11 += et.y;                                                                                                                                                               
        storeFlagPixel21 += et.x;                                                                                                                                                               
        storeFlagPixel12 += et.y;                                                                                                                                                               
    }  
                                                                                                                                                                                                
    gl_FragDepth = any(bvec4(outputEdges)) ? 1.0 : 0.0;
	                                                                                                                                                                                
    if( gl_FragDepth != 0.0 )                                                                                                                                                                   
    {                                                                                                                                                                                           
        if( storeFlagPixel00 != 0.0 )                                                                                                                                                           
		    imageStore(g_resultTextureFlt4Slot1, screenPosI.xy + ivec2( 0, 0 ), pixel00);
        if( storeFlagPixel10 != 0.0 )                                                                                                                                                           
		    imageStore(g_resultTextureFlt4Slot1, screenPosI.xy + ivec2( 1, 0 ), pixel10);                                                                                                       
        if( storeFlagPixel20 != 0.0 )                                                                                                                                                           
            imageStore(g_resultTextureFlt4Slot1, screenPosI.xy + ivec2( 2, 0 ), pixel20);                                                                                                       
        if( storeFlagPixel01 != 0.0 )                                                                                                                                                           
            imageStore(g_resultTextureFlt4Slot1, screenPosI.xy + ivec2( 0, 1 ), pixel01);                                                                                                       
        if( storeFlagPixel02 != 0.0 )                                                                                                                                                           
            imageStore(g_resultTextureFlt4Slot1, screenPosI.xy + ivec2( 0, 2 ), pixel02);                                                                                                       
        if( storeFlagPixel11 != 0.0 )                                                                                                                                                           
            imageStore(g_resultTextureFlt4Slot1, screenPosI.xy + ivec2( 1, 1 ), pixel11);                                                                                                       
        if( storeFlagPixel21 != 0.0 )                                                                                                                                                           
            imageStore(g_resultTextureFlt4Slot1, screenPosI.xy + ivec2( 2, 1 ), pixel21);                                                                                                       
        if( storeFlagPixel12 != 0.0 )                                                                                                                                                           
            imageStore(g_resultTextureFlt4Slot1, screenPosI.xy + ivec2( 1, 2 ), pixel12);                                                                                                       
    }
	outEdges = STORE_UVEC4(outputEdges);
}                                                                                                                                                                                               
#endif // DETECT_EDGES1                                                                                                                                                                         
                                                                                                                                                                                                
vec2 UnpackThresholds( uint val )                                                                                                                                                               
{                                                                                                                                                                                               
    return vec2( val & 0x0Fu, val >> 4u ) / 15.0f;                                                                                                                                              
}                                                                                                                                                                                               
                                                                                                                                                                                                
uint PruneNonDominantEdges( vec4 edges[3] )                                                                                                                                                     
{                                                                                                                                                                                               
    vec4 maxE4    = vec4( 0.0, 0.0, 0.0, 0.0 );                                                                                                                                                 
                                                                                                                                                                                                
    float avg = 0.0;                                                                                                                                                                            
                                                                                                                                                                                                
    for( int i = 0; i < 3; i++ )                                                                                                                                                                
    {                                                                                                                                                                                           
        maxE4 = max( maxE4, edges[i] );                                                                                                                                                         
                                                                                                                                                                                                
        avg = dot( edges[i], vec4( 1, 1, 1, 1 ) / ( 3.0 * 4.0 ) );                                                                                                                              
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    vec2 maxE2    = max( maxE4.xy, maxE4.zw );                                                                                                                                                  
    float maxE      = max( maxE2.x, maxE2.y );                                                                                                                                                  
                                                                                                                                                                                                
    float threshold = avg * 0.65 + maxE * 0.35;                                                                                                                                                 
                                                                                                                                                                                                
//    threshold = 0.0001; // this disables non-dominant edge pruning!                                                                                                                           
                                                                                                                                                                                                
    uint cx = edges[0].x >= threshold ? 1u : 0u;                                                                                                                                                
    uint cy = edges[0].y >= threshold ? 1u : 0u;                                                                                                                                                
    return PackEdge( uvec4( cx, cy, 0, 0 ) );                                                                                                                                                   
}                                                                                                                                                                                               
                                                                                                                                                                                                
void CollectEdges( int offX, int offY, out vec4 edges[3], const uint packedVals[6][6] )                                                                                                         
{                                                                                                                                                                                               
    vec2 pixelP0P0 = UnpackThresholds( packedVals[offX][offY] );                                                                                                                                
    vec2 pixelP1P0 = UnpackThresholds( packedVals[offX+1][offY] );                                                                                                                              
    vec2 pixelP0P1 = UnpackThresholds( packedVals[offX][offY+1] );                                                                                                                              
    vec2 pixelM1P0 = UnpackThresholds( packedVals[offX-1][offY] );                                                                                                                              
    vec2 pixelP0M1 = UnpackThresholds( packedVals[offX][offY-1] );                                                                                                                              
    vec2 pixelP1M1 = UnpackThresholds( packedVals[offX+1][offY-1] );                                                                                                                            
    vec2 pixelM1P1 = UnpackThresholds( packedVals[offX-1][offY+1] );                                                                                                                            
                                                                                                                                                                                                
    edges[ 0].x = pixelP0P0.x;                                                                                                                                                                  
    edges[ 0].y = pixelP0P0.y;                                                                                                                                                                  
    edges[ 0].z = pixelP1P0.x;                                                                                                                                                                  
    edges[ 0].w = pixelP1P0.y;                                                                                                                                                                  
    edges[ 1].x = pixelP0P1.x;                                                                                                                                                                  
    edges[ 1].y = pixelP0P1.y;                                                                                                                                                                  
    edges[ 1].z = pixelM1P0.x;                                                                                                                                                                  
    edges[ 1].w = pixelM1P0.y;                                                                                                                                                                  
    edges[ 2].x = pixelP0M1.x;                                                                                                                                                                  
    edges[ 2].y = pixelP0M1.y;                                                                                                                                                                  
    edges[ 2].z = pixelP1M1.y;                                                                                                                                                                  
    edges[ 2].w = pixelM1P1.x;                                                                                                                                                                  
}                                                                                                                                                                                               
                                                                                                                                                                                                
#ifdef DETECT_EDGES2                                                                                                                                                                            
layout (early_fragment_tests) in;                                                                                                                                                               
void DetectEdges2()                                                                                                                                                                             
{                                                                                                                                                                                               
    ivec2 screenPosI = ivec2(gl_FragCoord.xy);                                                                                                                                                  
	                                                                                                                                                                                            
    // source : edge differences from previous pass                                                                                                                                             
    uint packedVals[6][6];                                                                                                                                                                      
                                                                                                                                                                                                
    // center pixel (our output)                                                                                                                                                                
    UVEC4 packedQ4 = texelFetch(g_src0Texture4Uint, screenPosI.xy, 0);
    packedVals[2][2] = LOAD_UINT(packedQ4.x);                                                                                                                                                               
    packedVals[3][2] = LOAD_UINT(packedQ4.y);                                                                                                                                                               
    packedVals[2][3] = LOAD_UINT(packedQ4.z);                                                                                                                                                             
    packedVals[3][3] = LOAD_UINT(packedQ4.w); 
	                                                                                                                                                            
    vec4 edges[3];                                                                                                                                                                              
    if( bool(packedVals[2][2]) || bool(packedVals[3][2]) )                                                                                                                                      
    {                                                                                                                                                                                           
        UVEC4 packedQ1 = texelFetchOffset(g_src0Texture4Uint, screenPosI.xy, 0, ivec2(0, -1));                                                                                                  
        packedVals[2][0] = LOAD_UINT(packedQ1.x);                                                                                                                                                          
        packedVals[3][0] = LOAD_UINT(packedQ1.y);                                                                                                                                                         
        packedVals[2][1] = LOAD_UINT(packedQ1.z);                                                                                                                                                          
        packedVals[3][1] = LOAD_UINT(packedQ1.w);                                                                                                                                                          
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    if( bool(packedVals[2][2]) || bool(packedVals[2][3]) )                                                                                                                                      
    {                                                                                                                                                                                           
        UVEC4 packedQ3 = texelFetchOffset(g_src0Texture4Uint, screenPosI.xy, 0, ivec2(-1, 0));                                                                                                  
        packedVals[0][2] = LOAD_UINT(packedQ3.x);                                                                                                                                                          
        packedVals[1][2] = LOAD_UINT(packedQ3.y);                                                                                                                                                          
        packedVals[0][3] = LOAD_UINT(packedQ3.z);                                                                                                                                                          
        packedVals[1][3] = LOAD_UINT(packedQ3.w);                                                                                                                                                          
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    if( bool(packedVals[2][2]) )                                                                                                                                                                
    {                                                                                                                                                                                           
        CollectEdges( 2, 2, edges, packedVals );                                                                                                                                                
        uint pe = PruneNonDominantEdges( edges );                                                                                                                                               
        if( pe != 0u )                                                                                                                                                                          
        {
		    imageStore(g_resultTexture, 2 * screenPosI.xy + ivec2( 0, 0 ), vec4(float(0x80u | pe) / 255.0, 0, 0, 0));                                                                           
		}
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    if( bool(packedVals[3][2]) || bool(packedVals[3][3]) )                                                                                                                                      
    {                                                                                                                                                                                           
        UVEC4 packedQ5 = texelFetchOffset(g_src0Texture4Uint, screenPosI.xy, 0, ivec2(1, 0));                                                                                                   
        packedVals[4][2] = LOAD_UINT(packedQ5.x);                                                                                                                                                          
        packedVals[5][2] = LOAD_UINT(packedQ5.y);                                                                                                                                                          
        packedVals[4][3] = LOAD_UINT(packedQ5.z);                                                                                                                                                          
        packedVals[5][3] = LOAD_UINT(packedQ5.w);                                                                                                                                                          
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    if( bool(packedVals[3][2]) )                                                                                                                                                                
    {                                                                                                                                                                                           
        UVEC4 packedQ2 = texelFetchOffset(g_src0Texture4Uint, screenPosI.xy, 0, ivec2(1, -1));                                                                                                  
        packedVals[4][0] = LOAD_UINT(packedQ2.x);                                                                                                                                                          
        packedVals[5][0] = LOAD_UINT(packedQ2.y);                                                                                                                                                          
        packedVals[4][1] = LOAD_UINT(packedQ2.z);                                                                                                                                                          
        packedVals[5][1] = LOAD_UINT(packedQ2.w);                                                                                                                                                          
                                                                                                                                                                                                
        CollectEdges( 3, 2, edges, packedVals );                                                                                                                                                
        uint pe = PruneNonDominantEdges( edges );                                                                                                                                               
        if( pe != 0u )                                                                                                                                                                          
        {
		    imageStore(g_resultTexture, 2 * screenPosI.xy + ivec2( 1, 0 ), vec4(float(0x80u | pe) / 255.0, 0, 0, 0));                                                                           
		}
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    if( bool(packedVals[2][3]) || bool(packedVals[3][3]) )                                                                                                                                      
    {                                                                                                                                                                                           
        UVEC4 packedQ7 = texelFetchOffset(g_src0Texture4Uint, screenPosI.xy, 0, ivec2(0, 1));                                                                                                   
        packedVals[2][4] = LOAD_UINT(packedQ7.x);                                                                                                                                                          
        packedVals[3][4] = LOAD_UINT(packedQ7.y);                                                                                                                                                          
        packedVals[2][5] = LOAD_UINT(packedQ7.z);                                                                                                                                                         
        packedVals[3][5] = LOAD_UINT(packedQ7.w);                                                                                                                                                          
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    if( bool(packedVals[2][3]) )                                                                                                                                                                
    {                                                                                                                                                                                           
        UVEC4 packedQ6 = texelFetchOffset(g_src0Texture4Uint, screenPosI.xy, 0, ivec2(-1, -1));                                                                                                 
        packedVals[0][4] = LOAD_UINT(packedQ6.x);                                                                                                                                                          
        packedVals[1][4] = LOAD_UINT(packedQ6.y);                                                                                                                                                          
        packedVals[0][5] = LOAD_UINT(packedQ6.z);                                                                                                                                                          
        packedVals[1][5] = LOAD_UINT(packedQ6.w);                                                                                                                                                          
                                                                                                                                                                                                
        CollectEdges( 2, 3, edges, packedVals );                                                                                                                                                
        uint pe = PruneNonDominantEdges( edges );                                                                                                                                               
        if( pe != 0u )                                                                                                                                                                          
        {
		    imageStore(g_resultTexture, 2 * screenPosI.xy + ivec2( 0, 1 ), vec4(float(0x80u | pe) / 255.0, 0, 0, 0));                                                                           
		}
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    if( bool(packedVals[3][3]) )                                                                                                                                                                
    {                                                                                                                                                                                           
        CollectEdges( 3, 3, edges, packedVals );                                                                                                                                                
        uint pe = PruneNonDominantEdges( edges );                                                                                                                                               
        if( pe != 0u )                                                                                                                                                                          
        {
		    imageStore(g_resultTexture, 2 * screenPosI.xy + ivec2( 1, 1 ), vec4(float(0x80u | pe) / 255.0, 0, 0, 0));                                                                           
		}
    } 
}                                                                                                                                                                                               
#endif // DETECT_EDGES2                                                                                                                                                                         
                                                                                                                                                                                                
#ifdef COMBINE_EDGES                                                                                                                                                                            
void CombineEdges()                                                                                                                                                                             
{                                                                                                                                                                                               
    ivec3 screenPosIBase = ivec3( ivec2(gl_FragCoord.xy) * 2, 0 );                                                                                                                              
    vec3 screenPosBase = vec3(screenPosIBase);                                                                                                                                                  
    uint packedEdgesArray[3][3];                                                                                                                                                                
                                                                                                                                                                                                
    // use only if it has the 'prev frame' flag: do [sample * 255.0 - 127.5] -> if it has the last bit flag (128), it's going to stay above 0  
    uvec4 sampA = uvec4(textureGatherOffset(g_src0TextureFlt, screenPosBase.xy * OneOverScreenSize, ivec2( 1, 0 )) * 255.0 - 127.5);                                                
    uvec4 sampB = uvec4(textureGatherOffset(g_src0TextureFlt, screenPosBase.xy * OneOverScreenSize, ivec2( 0, 1 )) * 255.0 - 127.5);                                                
    uint  sampC = uint(texelFetchOffset(g_src0TextureFlt, screenPosIBase.xy, 0, ivec2(1, 1)).r * 255.0 - 127.5);                                                                                
                                                                                                                                                                                     
    packedEdgesArray[0][0] = 0u;                                                                                                                                                                
    packedEdgesArray[1][0] = sampA.w;                                                                                                                                                           
    packedEdgesArray[2][0] = sampA.z;                                                                                                                                                           
    packedEdgesArray[1][1] = sampA.x;                                                                                                                                                           
    packedEdgesArray[2][1] = sampA.y;                                                                                                                                                           
    packedEdgesArray[0][1] = sampB.w;                                                                                                                                                           
    packedEdgesArray[0][2] = sampB.x;                                                                                                                                                           
    packedEdgesArray[1][2] = sampB.y;                                                                                                                                                          
    packedEdgesArray[2][2] = sampC;                                                                                                                                                             
                                                                                                                                                                                                
    uvec4 pixelsC = uvec4( packedEdgesArray[1+0][1+0], packedEdgesArray[1+1][1+0], packedEdgesArray[1+0][1+1], packedEdgesArray[1+1][1+1] );                                                    
    uvec4 pixelsL = uvec4( packedEdgesArray[0+0][1+0], packedEdgesArray[0+1][1+0], packedEdgesArray[0+0][1+1], packedEdgesArray[0+1][1+1] );                                                    
    uvec4 pixelsU = uvec4( packedEdgesArray[1+0][0+0], packedEdgesArray[1+1][0+0], packedEdgesArray[1+0][0+1], packedEdgesArray[1+1][0+1] );                                                    
                                                                                                                                                                                                
    uvec4 outEdge4 = pixelsC | ((pixelsL & 0x01u) << 2u) | ((pixelsU & 0x02u) << 2u);                                                                                                           
    vec4 outEdge4Flt = vec4(outEdge4) / 255.0;  
                                                                                                                                                                                             
    imageStore(g_resultTextureSlot2, screenPosIBase.xy + ivec2( 0, 0 ), outEdge4Flt.xxxx);                                                                                                      
    imageStore(g_resultTextureSlot2, screenPosIBase.xy + ivec2( 1, 0 ), outEdge4Flt.yyyy);                                                                                                      
    imageStore(g_resultTextureSlot2, screenPosIBase.xy + ivec2( 0, 1 ), outEdge4Flt.zzzz);                                                                                                      
    imageStore(g_resultTextureSlot2, screenPosIBase.xy + ivec2( 1, 1 ), outEdge4Flt.wwww);                                                                                                      
                                                                                                                                                                                              
    uvec4 numberOfEdges4 = uvec4(bitCount( outEdge4 ));                                                                                                                                         
                                                                                                                                                                                                
    gl_FragDepth = any(greaterThan(numberOfEdges4, uvec4(1))) ? 1.0 : 0.0;                                                                                                                      
}                                                                                                                                                                                               
#endif // COMBINE_EDGES                                                                                                                                                                         
                                                                                                                                                                                                
#ifdef BLUR_EDGES                                                                                                                                                                               
layout (early_fragment_tests) in;                                                                                                                                                               
void BlurEdges()                                                                                                                                                                                
{                                                                                                                                                                                               
    int _i;                                                                                                                                                                                     
                                                                                                                                                                                                
    ivec3 screenPosIBase = ivec3( ivec2(gl_FragCoord.xy) * 2, 0 );                                                                                                                              
    vec3 screenPosBase   = vec3(screenPosIBase);                                                                                                                                                
    uint forFollowUpCount = 0u;                                                                                                                                                                 
    ivec4 forFollowUpCoords[4];                                                                                                                                                                 
                                                                                                                                                                                                
    uint packedEdgesArray[4][4];                                                                                                                                                                
                                                                                                                                                                                                
    uvec4 sampA = uvec4(textureGatherOffset(g_src0TextureFlt, screenPosBase.xy * OneOverScreenSize, ivec2( 0, 0 ) ) * 255.5);                                                       
    uvec4 sampB = uvec4(textureGatherOffset(g_src0TextureFlt, screenPosBase.xy * OneOverScreenSize, ivec2( 2, 0 ) ) * 255.5);                                                       
    uvec4 sampC = uvec4(textureGatherOffset(g_src0TextureFlt, screenPosBase.xy * OneOverScreenSize, ivec2( 0, 2 ) ) * 255.5);                                                       
    uvec4 sampD = uvec4(textureGatherOffset(g_src0TextureFlt, screenPosBase.xy * OneOverScreenSize, ivec2( 2, 2 ) ) * 255.5);                                                       
                                                                                                                                                                                                   
    packedEdgesArray[0][0] = sampA.w;                                                                                                                                                           
    packedEdgesArray[1][0] = sampA.z;                                                                                                                                                           
    packedEdgesArray[0][1] = sampA.x;                                                                                                                                                           
    packedEdgesArray[1][1] = sampA.y;                                                                                                                                                           
    packedEdgesArray[2][0] = sampB.w;                                                                                                                                                           
    packedEdgesArray[3][0] = sampB.z;                                                                                                                                                           
    packedEdgesArray[2][1] = sampB.x;                                                                                                                                                           
    packedEdgesArray[3][1] = sampB.y;                                                                                                                                                           
    packedEdgesArray[0][2] = sampC.w;                                                                                                                                                           
    packedEdgesArray[1][2] = sampC.z;                                                                                                                                                           
    packedEdgesArray[0][3] = sampC.x;                                                                                                                                                           
    packedEdgesArray[1][3] = sampC.y;                                                                                                                                                           
    packedEdgesArray[2][2] = sampD.w;                                                                                                                                                           
    packedEdgesArray[3][2] = sampD.z;                                                                                                                                                           
    packedEdgesArray[2][3] = sampD.x;                                                                                                                                                           
    packedEdgesArray[3][3] = sampD.y;                                                                                                                                                           
                                                                                                                                                                                                
    for( _i = 0; _i < 4; _i++ )                                                                                                                                                                 
    {                                                                                                                                                                                           
        int _x = _i%2;                                                                                                                                                                          
        int _y = _i/2;                                                                                                                                                                          
                                                                                                                                                                                                
        ivec3 screenPosI = screenPosIBase + ivec3( _x, _y, 0 );                                                                                                                                 
                                                                                                                                                                                                
        uint packedEdgesC = packedEdgesArray[1+_x][1+_y];                                                                                                                                       
                                                                                                                                                                                                
        uvec4 edges     = UnpackEdge( packedEdgesC );                                                                                                                                           
        vec4 edgesFlt   = vec4(edges);                                                                                                                                                          
                                                                                                                                                                                                
        float numberOfEdges = dot( edgesFlt, vec4( 1, 1, 1, 1 ) );    
        if( numberOfEdges < 2.0 )                                                                                                                                                               
            continue;                                                                                                                                                                           
                                                                                                                                                                                                
        float fromRight   = edgesFlt.r;                                                                                                                                                         
        float fromBelow   = edgesFlt.g;                                                                                                                                                         
        float fromLeft    = edgesFlt.b;                                                                                                                                                         
        float fromAbove   = edgesFlt.a;                                                                                                                                                         
                                                                                                                                                                                                
        vec4 xFroms = vec4( fromBelow, fromAbove, fromRight, fromLeft );                                                                                                                        
                                                                                                                                                                                                
        float blurCoeff = 0.0;                                                                                                                                                                  
                                                                                                                                                                                                
        // These are additional blurs that complement the main line-based blurring;                                                                                                             
        // Unlike line-based, these do not necessarily preserve the total amount of screen colour as they will                                                                                  
        // take neighbouring pixel colours and apply them to the one currently processed.                                                                                                       
                                                                                                                                                                                                
        // 1.) L-like shape.                                                                                                                                                                    
        // For this shape, the total amount of screen colour will be preserved when this is a part                                                                                              
        // of a (zigzag) diagonal line as the corners from the other side will do the same and                                                                                                  
        // take some of the current pixel's colour in return.                                                                                                                                   
        // However, in the case when this is an actual corner, the pixel's colour will be partially                                                                                             
        // overwritten by it's 2 neighbours.                                                                                                                                                    
        // if( numberOfEdges > 1.0 )                                                                                                                                                            
        {                                                                                                                                                                                       
                                                                                                                                                                                                
            // with value of 0.15, the pixel will retain approx 77% of its colour and the remaining 23% will                                                                                    
            // come from its 2 neighbours (which are likely to be blurred too in the opposite direction)                                                                                        
            blurCoeff = 0.08;                                                                                                                                                                   
                                                                                                                                                                                                
            // Only do blending if it's L shape - if we're between two parallel edges, don't do anything                                                                                        
            blurCoeff *= (1.0 - fromBelow * fromAbove) * (1.0 - fromRight * fromLeft);                                                                                                          
        }                                                                                                                                                                                       
                                                                                                                                                                                                
        // 2.) U-like shape (surrounded with edges from 3 sides)                                                                                                                                
        if( numberOfEdges > 2.0 )                                                                                                                                                               
        {                                                                                                                                                                                       
            // with value of 0.13, the pixel will retain approx 72% of its colour and the remaining 28% will                                                                                    
            // be picked from its 3 neighbours (which are unlikely to be blurred too but could be)                                                                                              
            blurCoeff = 0.11;                                                                                                                                                                   
        }                                                                                                                                                                                       
                                                                                                                                                                                                
        // 3.) Completely surrounded with edges from all 4 sides                                                                                                                                
        if( numberOfEdges > 3.0 )                                                                                                                                                               
        {                                                                                                                                                                                       
            // with value of 0.07, the pixel will retain 78% of its colour and the remaining 22% will                                                                                           
            // come from its 4 neighbours (which are unlikely to be blurred)                                                                                                                    
            blurCoeff = 0.05;                                                                                                                                                                   
        }                                                                                                                                                                                       
                                                                                                                                                                                                
        if( blurCoeff == 0.0 )                                                                                                                                                                  
        {                                                                                                                                                                                       
            // this avoids Z search below as well but that's ok because a Z shape will also always have                                                                                         
            // some blurCoeff                                                                                                                                                                   
            continue;                                                                                                                                                                           
        }
		
                                                                                                                                                                                                
        vec4 blurMap = xFroms * blurCoeff;                                                                                                                                                      
                                                                                                                                                                                                
        vec4 pixelC = texelFetch(g_screenTexture, screenPosI.xy, 0); 
                                                                                                                                                                                                
        const float centerWeight = 1.0;                                                                                                                                                         
        float fromBelowWeight = blurMap.x; // (1.0 / (1.0 - blurMap.x)) - 1.0; // this would be the proper math for blending if we were handling                                                
        float fromAboveWeight = blurMap.y; // (1.0 / (1.0 - blurMap.y)) - 1.0; // lines (Zs) and mini kernel smoothing here, but since we're doing                                              
        float fromRightWeight = blurMap.z; // (1.0 / (1.0 - blurMap.z)) - 1.0; // lines separately, no need to complicate, just tweak the settings.                                             
        float fromLeftWeight  = blurMap.w; // (1.0 / (1.0 - blurMap.w)) - 1.0;                                                                                                                  
                                                                                                                                                                                                
        float fourWeightSum   = dot( blurMap, vec4( 1, 1, 1, 1 ) );                                                                                                                             
        float allWeightSum    = centerWeight + fourWeightSum;                                                                                                                                   
                                                                                                                                                                                                
        vec4 color = vec4( 0, 0, 0, 0 );                                                                                                                                                        
        if( fromLeftWeight > 0.0 )                                                                                                                                                              
        {                                                                                                                                                                                       
            vec3 pixelL = texelFetchOffset(g_screenTexture, screenPosI.xy, 0, ivec2(-1, 0)).rgb;   
            color.rgb += fromLeftWeight * pixelL;                                                                                                                                               
        }                                                                                                                                                                                       
        if( fromAboveWeight > 0.0 )                                                                                                                                                             
        {                                                                                                                                                                                       
            vec3 pixelT = texelFetchOffset(g_screenTexture, screenPosI.xy, 0, ivec2(0, -1)).rgb;                                                                                                
            color.rgb += fromAboveWeight * pixelT;                                                                                                                                              
        }                                                                                                                                                                                       
        if( fromRightWeight > 0.0 )                                                                                                                                                             
        {                                                                                                                                                                                       
            vec3 pixelR = texelFetchOffset(g_screenTexture, screenPosI.xy, 0, ivec2(1, 0)).rgb;                                                                                                 
            color.rgb += fromRightWeight * pixelR;                                                                                                                                              
        }                                                                                                                                                                                       
        if( fromBelowWeight > 0.0 )                                                                                                                                                             
        {                                                                                                                                                                                       
            vec3 pixelB = texelFetchOffset(g_screenTexture, screenPosI.xy, 0, ivec2(0, 1)).rgb;                                                                                                 
            color.rgb += fromBelowWeight * pixelB;                                                                                                                                              
        }                                                                                                                                                                                       
                                                                                                                                                                                                
        color /= fourWeightSum + 0.0001;                                                                                                                                                        
        color.a = 1.0 - centerWeight / allWeightSum;                                                                                                                                            
                                                                                                                                                                                                
        color.rgb = mix( pixelC.rgb, color.rgb, color.a ).rgb;                                                                                                                                  
#ifdef IN_GAMMA_CORRECT_MODE                                                                                                                                                                       
        color.rgb = D3DX_FLOAT3_to_SRGB( color.rgb );                                                                                                                                           
#endif                                                                                                                                                                                          
                                                                                                                                                                                                
#ifdef DEBUG_OUTPUT_AAINFO                                                                                                                                                                         
        imageStore(g_resultTextureSlot2, screenPosI.xy, PackBlurAAInfo( screenPosI.xy, uint(numberOfEdges) ));                                                                                  
#endif  
        imageStore(g_resultTextureFlt4Slot1, screenPosI.xy, vec4( color.rgb, pixelC.a ) );                                                                                                      
                                                                                                                                                                                                
        if( numberOfEdges == 2.0 )                                                                                                                                                              
        {                                                                                                                                                                                       
                                                                                                                                                                                                
            uint packedEdgesL    = packedEdgesArray[0+_x][1+_y];                                                                                                                                
            uint packedEdgesT    = packedEdgesArray[1+_x][0+_y];                                                                                                                                
            uint packedEdgesR    = packedEdgesArray[2+_x][1+_y];                                                                                                                                
            uint packedEdgesB    = packedEdgesArray[1+_x][2+_y];                                                                                                                                
                                                                                                                                                                                                
            bool isHorizontalA = ( ( packedEdgesC ) == (0x01u | 0x02u) ) && ( (packedEdgesR & (0x01u | 0x08u) ) == (0x08u) );                                                                   
            bool isHorizontalB = ( ( packedEdgesC ) == (0x01u | 0x08u) ) && ( (packedEdgesR & (0x01u | 0x02u) ) == (0x02u) );                                                                   
                                                                                                                                                                                                
            bool isHCandidate = isHorizontalA || isHorizontalB;                                                                                                                                 
                                                                                                                                                                                                
            bool isVerticalA = ( ( packedEdgesC ) == (0x08u | 0x01u) ) && ( (packedEdgesT & (0x08u | 0x04u) ) == (0x04u) );                                                                     
            bool isVerticalB = ( ( packedEdgesC ) == (0x08u | 0x04u) ) && ( (packedEdgesT & (0x08u | 0x01u) ) == (0x01u) );                                                                     
            bool isVCandidate = isVerticalA || isVerticalB;                                                                                                                                     
                                                                                                                                                                                                
            bool isCandidate = isHCandidate || isVCandidate;                                                                                                                                    
                                                                                                                                                                                                
            if( !isCandidate )                                                                                                                                                                  
                continue;                                                                                                                                                                       
                                                                                                                                                                                                
            bool horizontal = isHCandidate;                                                                                                                                                     
                                                                                                                                                                                                
            // what if both are candidates? do additional pruning (still not 100% but gets rid of worst case errors)                                                                            
            if( isHCandidate && isVCandidate )                                                                                                                                                  
                horizontal = ( isHorizontalA && ( ( packedEdgesL & 0x02u ) == 0x02u ) ) || ( isHorizontalB && ( ( packedEdgesL & 0x08u ) == 0x08u ) );                                          
                                                                                                                                                                                                
            ivec2 offsetC;                                                                                                                                                                      
            uint packedEdgesM1P0;                                                                                                                                                               
            uint packedEdgesP1P0;                                                                                                                                                               
            if( horizontal )                                                                                                                                                                    
            {                                                                                                                                                                                   
                packedEdgesM1P0 = packedEdgesL;                                                                                                                                                 
                packedEdgesP1P0 = packedEdgesR;                                                                                                                                                 
                offsetC = ivec2( 2,  0 );                                                                                                                                                       
            }                                                                                                                                                                                   
            else                                                                                                                                                                                
            {                                                                                                                                                                                   
                packedEdgesM1P0 = packedEdgesB;                                                                                                                                                 
                packedEdgesP1P0 = packedEdgesT;                                                                                                                                                 
                offsetC = ivec2( 0, -2 );                                                                                                                                                       
            }                                                                                                                                                                                   
                                                                                                                                                                                                
            uvec4 edgesM1P0    = UnpackEdge( packedEdgesM1P0 );                                                                                                                                 
            uvec4 edgesP1P0    = UnpackEdge( packedEdgesP1P0 );                                                                                                                                 
            uvec4 edgesP2P0    = UnpackEdge( uint(texelFetch(g_src0TextureFlt, screenPosI.xy + offsetC, 0).r * 255.5) );                                                                        
                                                                                                                                                                                                
            uvec4 arg0;                                                                                                                                                                         
            uvec4 arg1;                                                                                                                                                                         
            uvec4 arg2;                                                                                                                                                                         
            uvec4 arg3;                                                                                                                                                                         
            bool arg4;                                                                                                                                                                          
                                                                                                                                                                                                
            if( horizontal )                                                                                                                                                                    
            {                                                                                                                                                                                   
                arg0 = uvec4(edges);                                                                                                                                                            
                arg1 = edgesM1P0;                                                                                                                                                               
                arg2 = edgesP1P0;                                                                                                                                                               
                arg3 = edgesP2P0;                                                                                                                                                               
                arg4 = true;                                                                                                                                                                    
            }                                                                                                                                                                                   
            else                                                                                                                                                                                
            {                                                                                                                                                                                   
                // Reuse the same code for vertical (used for horizontal above), but rotate input data 90 counter-clockwise, so that:                                                          
                // left     becomes     bottom                                                                                                                                                  
                // top      becomes     left                                                                                                                                                    
                // right    becomes     top                                                                                                                                                     
                // bottom   becomes     right                                                                                                                                                   
                                                                                                                                                                                                
                // we also have to rotate edges, thus .argb                                                                                                                                     
                arg0 = uvec4(edges.argb);                                                                                                                                                       
                arg1 = edgesM1P0.argb;                                                                                                                                                          
                arg2 = edgesP1P0.argb;                                                                                                                                                          
                arg3 = edgesP2P0.argb;                                                                                                                                                          
                arg4 = false;                                                                                                                                                                   
            }                                                                                                                                                                                   
                                                                                                                                                                                                
            {                                                                                                                                                                                   
                ivec2 screenPos  = screenPosI.xy;                                                                                                                                               
                uvec4 _edges     = arg0;                                                                                                                                                        
                uvec4 _edgesM1P0 = arg1;                                                                                                                                                        
                uvec4 _edgesP1P0 = arg2;                                                                                                                                                        
                uvec4 _edgesP2P0 = arg3;                                                                                                                                                        
                bool horizontal = arg4;                                                                                                                                                         
                // Inverted Z case:                                                                                                                                                             
                //   __                                                                                                                                                                         
                //  X|                                                                                                                                                                          
                //                                                                                                                                                                            
                bool isInvertedZ = false;                                                                                                                                                       
                bool isNormalZ = false;                                                                                                                                                         
                {                                                                                                                                                                               
#ifndef SETTINGS_ALLOW_SHORT_Zs                                                                                                                                                                 
                    // (1u-_edges.a) constraint can be removed; it was added for some rare cases                                                                                                
                    uint isZShape       = _edges.r * _edges.g * _edgesM1P0.g * _edgesP1P0.a * _edgesP2P0.a * (1u-_edges.b) * (1u-_edgesP1P0.r) * (1u-_edges.a) * (1u-_edgesP1P0.g);             
#else                                                                                                                                                                                           
                    uint isZShape       = _edges.r * _edges.g *                _edgesP1P0.a *                (1u-_edges.b) * (1u-_edgesP1P0.r) * (1u-_edges.a) * (1u-_edgesP1P0.g);             
                    isZShape           *= ( _edgesM1P0.g + _edgesP2P0.a ); // and at least one of these need to be there                                                                        
#endif                                                                                                                                                                                          
                    if( isZShape > 0u )                                                                                                                                                         
                    {                                                                                                                                                                           
                        isInvertedZ = true;                                                                                                                                                     
                    }                                                                                                                                                                           
                }                                                                                                                                                                               
                                                                                                                                                                                                
                // Normal Z case:                                                                                                                                                               
                // __                                                                                                                                                                           
                //  X|                                                                                                                                                                          
                //                                                                                                                                                                            
                {                                                                                                                                                                               
#ifndef SETTINGS_ALLOW_SHORT_Zs                                                                                                                                                                 
                    uint isZShape     = _edges.r * _edges.a * _edgesM1P0.a * _edgesP1P0.g * _edgesP2P0.g * (1u-_edges.b) * (1u-_edgesP1P0.r) * (1u-_edges.g) * (1u-_edgesP1P0.a);               
#else                                                                                                                                                                                           
                    uint isZShape     = _edges.r * _edges.a *                _edgesP1P0.g                * (1u-_edges.b) * (1u-_edgesP1P0.r) * (1u-_edges.g) * (1u-_edgesP1P0.a);               
                    isZShape         *= ( _edgesM1P0.a + _edgesP2P0.g ); // and at least one of these need to be there                                                                          
#endif                                                                                                                                                                                          
                                                                                                                                                                                                
                    if( isZShape > 0u )                                                                                                                                                         
                    {                                                                                                                                                                           
                        isNormalZ = true;                                                                                                                                                       
                    }                                                                                                                                                                           
                }                                                                                                                                                                               
                                                                                                                                                                                                
                bool isZ = isInvertedZ || isNormalZ;                                                                                                                                            
                if( isZ )                                                                                                                                                                       
                {                                                                                                                                                                               
                    forFollowUpCoords[forFollowUpCount++] = ivec4( screenPosI.xy, horizontal, isInvertedZ );                                                                                    
                }                                                                                                                                                                               
            }                                                                                                                                                                                   
        }                                                                                                                                                                                       
    }                                                                                                                                                                                           
                                                                                                                                                                                                
    // This code below is the only potential bug with this algorithm : it HAS to be executed after the simple shapes above. It used to be executed as a separate compute                        
    // shader (by storing the packed 'forFollowUpCoords' in an append buffer and consuming it later) but the whole thing (append/consume buffers, using CS) appears to                          
    // be too inefficient on most hardware.                                                                                                                                                     
    // However, it seems to execute fairly efficiently here and without any issues, although there is no 100% guarantee that this code below will execute across all pixels                     
    // (it has a c_maxLineLength wide kernel) after other shaders processing same pixels have done solving simple shapes. It appears to work regardless, across all                             
    // hardware; pixels with 1-edge or two opposing edges are ignored by simple shapes anyway and other shapes stop the long line algorithm from executing; the only danger                     
    // appears to be simple shape L's colliding with Z shapes from neighbouring pixels but I couldn't reproduce any problems on any hardware.
    for( uint _i = 0u; _i < forFollowUpCount; _i++ )                                                                                                                                            
    {                                                                                                                                                                                           
        ivec4 data = forFollowUpCoords[_i];                                                                                                                                                     
        ProcessDetectedZ( data.xy, bool(data.z), bool(data.w) );                                                                                                                                
    }                                                                                                                                                                                           
}                                                                                                                                                                                               
#endif // BLUR_EDGES                                                                                                                                                                            
                                                                                                                                                                                                
#ifdef DISPLAY_EDGES                                                                                                                                                                            
layout(location = 0) out vec4 color;                                                                                                                                                            
layout(location = 1) out vec4 hasEdges;                                                                                                                                                         
void DisplayEdges()                                                                                                                                                                             
{                                                                                                                                                                                               
    ivec2 screenPosI = ivec2( gl_FragCoord.xy );                                                                                                                                                
                                                                                                                                                                                                
    uint packedEdges, shapeType;                                                                                                                                                                
    UnpackBlurAAInfo( texelFetch(g_src0TextureFlt, screenPosI, 0).r, packedEdges, shapeType );                                                                                                  
                                                                                                                                                                                                
    vec4 edges = vec4(UnpackEdge( packedEdges ));                                                                                                                                               
    if( any( greaterThan( edges.xyzw, vec4(0) ) ) )                                                                                                                                             
    {                                                                                                                                                                                           
#ifdef IN_BGR_MODE                                                                                                                                                                                 
        color    = c_edgeDebugColours[shapeType].bgra;                                                                                                                                          
#else                                                                                                                                                                                           
        color    = c_edgeDebugColours[shapeType];                                                                                                                                               
#endif                                                                                                                                                                                          
        hasEdges = vec4(1.0);                                                                                                                                                                   
    }                                                                                                                                                                                           
    else                                                                                                                                                                                        
    {                                                                                                                                                                                           
        color    = vec4(0);                                                                                                                                                                     
        hasEdges = vec4(0.0);                                                                                                                                                                   
    }                                                                                                                                                                                           
}                                                                                                                                                                                               
#endif // DISPLAY_EDGES                                                                                                                                                                         
                                                                                                                                                                                                
void main()                                                                                                                                                                                     
{     
#ifdef DETECT_EDGES1                                                                                                                                                                            
    DetectEdges1();
#endif                                                                                                                                                                             
#if defined DETECT_EDGES2                                                                                                                                                                     
    DetectEdges2();                                                                                                                                                                             
#endif                                                                                                                                                                             
#if defined COMBINE_EDGES                                                                                                                                                                     
    CombineEdges();                                                                                                                                                                             
#endif                                                                                                                                                                             
#if defined BLUR_EDGES                                                                                                                                                                        
    BlurEdges();                                                                                                                                                                                
#endif                                                                                                                                                                             
#if defined DISPLAY_EDGES                                                                                                                                                                     
    DisplayEdges();                                                                                                                                                                             
#endif                                                                                                                                                                                          
}

