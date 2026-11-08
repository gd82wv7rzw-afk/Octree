
#include <metal_stdlib>

#include "Rendering/Vertices.h"

using namespace metal;
using namespace Rendering;

struct VertexOut
{
    float4 position [[position]];
    float3 normal;
    float3 color;
};
//----------------------------------------------------------------------------------------------------------------------

vertex VertexOut vertexMain
(
    PNVertex in                      [[stage_in]],
    constant SimulationVertex* atoms [[buffer(1)]],
    constant float4x4& pvm           [[buffer(2)]],
    constant float4x4& rot           [[buffer(3)]],
    uint instanceID                  [[instance_id]]
)
{
    VertexOut out;
    const float scale = 2.5e-3;
    const float maxVSquare = 2.5f;

    float3 atomPos  = atoms[instanceID].pos;
    float  vSquare  = dot(atoms[instanceID].vel, atoms[instanceID].vel);

    float3 localPos = in.pos * scale + atomPos;

    // Position: Ensemble-Rotation ist bereits in PVM enthalten
    out.position = pvm * float4(localPos, 1.0f);

    // Normale: Nur Rotation, keine Translation
    out.normal = normalize((rot * float4(in.normale, 0.0f)).xyz);

    // color
    float t = vSquare / maxVSquare; //log(1.0f + vSquare) / log(1.0f + maxVSquare);
    t = clamp(t, 0.0f, 1.0f);
    float3 color;

    if ( t < 0.5)
    {
        color = mix(float3{0.0f, 0.0f, 1.0f}, float3{0.5f, 0.5f, 0.5f}, t * 2);
    }
    else
    {
        color = mix(float3{0.5f, 0.5f, 0.5f}, float3{1.0f, 0.0f, 0.0f}, (t - 0.5f) * 2);
    }

    out.color = color;
    
    return out;
}
//----------------------------------------------------------------------------------------------------------------------

fragment float4 fragmentMain(VertexOut in [[stage_in]])
{
    // 1. Licht- und Materialparameter
    constexpr float3 lightColor       = float3(1.0f, 1.0f, 1.0f);
    constexpr float3 ambientColor     = float3(1.0f, 1.0f, 1.0f);

    constexpr float ambientStrength   = 0.4f;
    constexpr float diffuseStrength   = 1.2f;
    constexpr float specularStrength  = 0.35f;
    constexpr float shininess         = 48.0f;
    constexpr float lightStrength     = 1.0f;

    constexpr float3 lightDirection   = float3(-0.5f, 0.8f, 1.0f);
    constexpr float3 viewDirection    = float3(0.0f, 0.0f, 1.0f);


    // 2. Normalen und Materialfarbe
    float3 N = in.normal;
    float3 L = normalize(lightDirection);
    float3 V = viewDirection;

    // Ausschließlich die Geschwindigkeit bestimmt die Materialfarbe.
    float3 baseColor = in.color;

    // 3. Ambient
    float3 ambient = ambientColor * baseColor * ambientStrength;

    // 4. Diffuse (Lambert)
    float NdotL = max(dot(N, L), 0.0f);

    float3 diffuse = NdotL * lightColor * baseColor * diffuseStrength;

    // 5. Specular (Blinn-Phong)
    float3 H = normalize(L + V);

    float spec = 0.0f;

    if (NdotL > 0.0f)
    {
        spec = pow(max(dot(N, H), 0.0f), shininess);
    }

    float3 specular = spec * specularStrength * lightColor;

    // 6. Final color
    float3 finalColor = (ambient + diffuse + specular) * lightStrength;
    finalColor = clamp(finalColor, 0.0f, 1.0f);

    return float4(finalColor, 1.0f);
}
//----------------------------------------------------------------------------------------------------------------------
