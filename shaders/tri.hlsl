#pragma pack_matrix(column_major)

struct VSInput {
       [[vk::location(0)]] float3 Pos : POSITION;
       [[vk::location(1)]] float3 Color : COLOR;
       [[vk::location(2)]] float2 UV : TEXCOORD0;
};

struct VSOutput {
    float4 Pos : SV_POSITION;
    float3 Color : COLOR;
    float2 UV : TEXCOORD0;
};

// Vertex Shader
VSOutput MainVS(VSInput input) {
    VSOutput output;

    output.Pos = float4(input.Pos, 1.0f);
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

// Fragment Shader
float4 MainFS(VSOutput input) : SV_TARGET {
       // We don't handle UVs yet.
       return float4(input.Color, 1.0);
}