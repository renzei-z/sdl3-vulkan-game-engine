struct VSOutput {
    float4 Pos : SV_POSITION;
    float3 Color : COLOR;
};

static const float2 positions[3] = {
    float2(-0.5, -0.5),
    float2(0.5, -0.5),
    float2(0.0, 0.25)
};

static const float3 colors[3] = {
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0)
};

// Vertex Shader
VSOutput MainVS(uint VertexIndex : SV_VertexID) {
    VSOutput output;
    output.Pos = float4(positions[VertexIndex], 0.0, 1.0);
    output.Color = colors[VertexIndex];
    return output;
}

// Fragment/Pixel Shader
float4 MainPS(VSOutput input) : SV_TARGET {
    return float4(input.Color, 1.0);
}