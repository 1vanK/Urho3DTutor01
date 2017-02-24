#include "Uniforms.glsl"
#include "Transform.glsl"
#include "Samplers.glsl"

#ifdef VERTEXCOLOR
    varying vec4 vColor;
#endif

#ifdef DIFFMAP
    varying vec2 vTexCoord;
#endif

void VS()
{
    mat4 modelMatrix = iModelMatrix;
    vec3 worldPos = GetWorldPos(modelMatrix);
    gl_Position = GetClipPos(worldPos);

    #ifdef VERTEXCOLOR
        vColor = iColor;
    #endif

    #ifdef DIFFMAP
        vTexCoord = iTexCoord;
    #endif
}

void PS()
{
    #ifdef VERTEXCOLOR
        vec3 finalColor = vColor.rgb;
    #else
        vec3 finalColor = vec3(1.0);
    #endif

    #ifdef DIFFMAP
        finalColor *= texture2D(sDiffMap, vTexCoord).rgb;
    #endif

    gl_FragColor = vec4(finalColor, 1.0);
}
