
#include <metal_graphics>
#include <metal_matrix>
#include <metal_geometric>
#include <metal_math>
#include <metal_texture>
#include <metal_common>

using namespace metal;


struct FragmentUniforms
{
    hfloat    time ;
    hfloat2   aspectRatio ;
    hfloat3   lightDir ;
    hfloat3   eyePosition ;
    hfloat4x4 orientation ;
};

struct AluInOut
{
    hfloat4 m_Position [[position]];
    v_float2 v_texCoord [[user(vtexcoord)]];
};

struct VertexInput
{
	hfloat3 a_Vertex [[attribute(0)]];
	hfloat2 a_TexCoord [[attribute(1)]];
};


#ifdef TYPE_VERTEX
vertex AluInOut AluVertex(VertexInput     in [[ stage_in ]])
{
    AluInOut outVertices;
    
    //outVertices.m_Position = *pMVP * pPosition[vid];
    
    outVertices.m_Position = hfloat4(in.a_Vertex,1.0) ;
    outVertices.v_texCoord = v_float2(_float2(2.0)*_float2(in.a_TexCoord)-_float2(1.0));
    
    return outVertices;
}
#endif


#ifdef TYPE_FRAGMENT
_float rand2D(_float2 co)
{
    return fract(sin(dot(co,_float2(2.9898, 8.233))) * _float(8.5453));
}


_float rand2D_C1(_float2 co)
{
    _float2 flr = floor(co);
    _float2 frc = co - flr;
    _float v0 = rand2D(flr);
    _float v1 = rand2D(flr + _float2(1.0, 0.0));
    _float mix1 = mix(v0, v1, frc.x);
    _float v2 = rand2D(flr + _float2(0.0, 1.0));
    _float v3 = rand2D(flr + _float2(1.0, 1.0));
    _float mix2 = mix(v2, v3, frc.x);
    return mix(mix1, mix2, frc.y);
}


_float3 getSkyColor(_float3 direction, _float3 u_lightDir)
{
    _float cosDir = dot(direction, u_lightDir) * _float(0.5) + _float(0.5);
    _float3 horizon = powr(cosDir, _float(0.3)) * mix(_float3(1.0, 0.75, 0.2), _float3(0.5, 0.9, 1.0), u_lightDir.y);
    _float3 zenit = mix(_float3(0.0, 0.2, 0.4), _float3(0.5, 0.9, 1.0), u_lightDir.y);
    _float3 sky = mix(horizon, zenit, direction.y);
    _float3 direct = _float3(1.0, 1.0, 1.0) * (_float(0.1) * cosDir  + _float(0.5) * powr(cosDir, _float(16.0)) + step(_float(0.999), cosDir));
    _float3 light = sky + direct;
    return light;
}


_float3 getWaterNormal(_float2 coords, _float distance, _float u_time)
{
    _float t = u_time * _float(0.002);
    _float r1 = rand2D_C1(coords * _float2(0.9, 1.8)) * _float(6.28318);
    _float r2 = rand2D_C1(coords * _float2(2.0, 4.0)) * _float(6.28318);
    _float nx = cos(r1 + t) + _float(0.5) * cos(r2 + t * _float(2.0));
    _float ny = sin(r1 + t) + _float(0.5) * sin(r2 + t * _float(2.0)) + _float(0.5) * sin(coords.y + cos(coords.x) - t);
    return normalize(_float3(nx, _float(0.01) + _float(0.1) * powr(distance, _float(1.5)), ny));
}


_float3 getGroundColor(_float2 coords)
{
    _float2 c = step(_float(0.5), fract(coords));
    return mix(_float3(0.2, 0.8, 1.0), _float3(0.1, 0.4, 0.7), abs(c.x - c.y));
}



fragment _float4 AluFragment(AluInOut  inFrag    [[ stage_in ]],
                                     constant FragmentUniforms   *fragmentUniforms [[ buffer(0) ]] )
{   
    _float4 res = _float4(1.0,0.0,0.0,1.0) ;
    
    _float2 u_aspectRatio = _float2(fragmentUniforms->aspectRatio) ;
    _float u_time = fragmentUniforms->time ;
    
    _float3 u_lightDir = _float3(fragmentUniforms->lightDir) ;
    
    _float3   u_eyePosition = _float3( fragmentUniforms->eyePosition ) ;
    _float4x4 u_orientation = _float4x4( fragmentUniforms->orientation ) ;
    
    _float fov = 1.5;
    _float thf = tan(fov * _float(0.5));
    _float3 viewDirection = normalize(_float3( _float(inFrag.v_texCoord.x) * u_aspectRatio.x * thf, _float(inFrag.v_texCoord.y) * u_aspectRatio.y * thf, 1.0));
    viewDirection = (_float4(viewDirection, 1.0) * u_orientation).xyz;
    
    _float targetHeight = (viewDirection.y >= 0.0) ? 5.0 : 0.0;
    _float3 eyeToPlane = viewDirection * ((u_eyePosition.y - targetHeight) / viewDirection.y);
    _float3 intersectPosition = u_eyePosition + eyeToPlane;
    _float3 planeNormal = getWaterNormal(intersectPosition.xz, length(eyeToPlane), u_time);
    
    if (viewDirection.y >= _float(0.0))
    {
        _float3 skyColor = getSkyColor(viewDirection, u_lightDir);
        _float cloud = max(dot(planeNormal, viewDirection), _float(0.3) - _float(0.3) * planeNormal.y);
        res = _float4(mix(skyColor, _float3(1.0, 1.0, 1.0), cloud), 1.0);

    }
    else
    {
        _float3 refractedViewDir = viewDirection - _float(0.25) * planeNormal;
        _float3 waterToGround = refractedViewDir * (_float(1.0) / refractedViewDir.y);
        _float3 groundPosition = intersectPosition + waterToGround;
        _float3 refractedLightDir = normalize(u_lightDir + planeNormal);
        _float3 groundColor = getGroundColor(groundPosition.xz);
        groundColor *= refractedLightDir.y;	// dot(refractedLightDir, (0, 1, 0)
        groundColor = mix(_float3(0.0, 0.2, 0.4), groundColor, abs(dot(planeNormal, viewDirection)));
        
        _float3 reflectedView = max(_float3(-1.0, 0.0, -1.0), reflect(viewDirection, planeNormal));
        _float3 reflectedSky = _float(0.25) * getSkyColor(reflectedView,u_lightDir);
        
        res = _float4(groundColor + reflectedSky, 1.0);
    }
    
    
    return res ;
}
#endif
