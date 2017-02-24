#include "Uniforms.hlsl"
#include "Transform.hlsl"
#include "Samplers.hlsl"

#line 5

void VS(float4 iPos : POSITION,
    #ifdef VERTEXCOLOR
        float4 iColor : COLOR0,
        out float4 oColor : COLOR0,
    #endif
    #ifdef DIFFMAP
        float2 iTexCoord : TEXCOORD0,
        out float2 oTexCoord : TEXCOORD0,
    #endif
    out float4 oPos : OUTPOSITION)
{
    float4x3 modelMatrix = iModelMatrix;
    float3 worldPos = GetWorldPos(modelMatrix);
    oPos = GetClipPos(worldPos);

    #ifdef VERTEXCOLOR
        oColor = iColor;
    #endif

    #ifdef DIFFMAP
        oTexCoord = iTexCoord;
    #endif
}

void PS(
    #ifdef VERTEXCOLOR
        float4 iColor : COLOR0,
    #endif
    #ifdef DIFFMAP
        float2 iTexCoord : TEXCOORD0,
    #endif
    out float4 oColor : OUTCOLOR0)
{
    #ifdef VERTEXCOLOR
        float3 finalColor = iColor.rgb;
    #else
        float3 finalColor = float3(1.0, 1.0, 1.0);
    #endif

    #ifdef DIFFMAP
        finalColor *= Sample2D(DiffMap, iTexCoord).rgb;
    #endif

    oColor = float4(finalColor, 1.0);
}
