#include "Uniforms.hlsl"
#include "Transform.hlsl"

void VS(float4 iPos : POSITION, out float4 oPos : OUTPOSITION)
{
    // Вершинный шейдер полностью соответствует шаблону
    // https://urho3d.github.io/documentation/1.6/_shaders.html
    float4x3 modelMatrix = iModelMatrix;
    float3 worldPos = GetWorldPos(modelMatrix);
    oPos = GetClipPos(worldPos);
}

void PS(out float4 oColor : OUTCOLOR0)
{
    // В пиксельном шейдере просто выводим белый цвет.
    oColor = float4(1.0, 1.0, 1.0, 1.0);
}
