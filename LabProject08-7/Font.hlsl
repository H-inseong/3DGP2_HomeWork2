struct VS_INPUT
{
    float2 Position : POSITION;
    float4 Color : COLOR;
    uint Type : TYPE;
};

struct GS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    uint Type : TYPE;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
    float4 Color : COLOR;
};

Texture2D gtxtFont : register(t0);
SamplerState gSampler : register(s0);

cbuffer cbFontInfo : register(b0)
{
    float4 FontColor;
    float2 ScreenSize;
    float2 TextureSize;
    float Scale;
    float2 StartPos;
};

struct CharInfo
{
    int id;
    int x, y;
    int width, height;
    int xoffset, yoffset;
    int xadvance;
    int page;
    int chnl;
};

StructuredBuffer<CharInfo> gCharInfos : register(t1);


//////////////////////////////////////////////////////////////////////////////////
GS_INPUT VS_Font(VS_INPUT input)
{
    GS_INPUT output;
    output.Position = float4(input.Position, 0.0f, 1.0f);
    output.Color = input.Color;
    output.Type = input.Type;
    return output;
}

float4 ScreenToClip(float2 screenCoord, float2 screenSize)
{
    float x = (screenCoord.x / screenSize.x) * 2.0f - 1.0f;
    float y = 1.0f - (screenCoord.y / screenSize.y) * 2.0f;
    return float4(x, y, 0.0f, 1.0f);
}

[maxvertexcount(4)]
void GS_Font(point GS_INPUT input[1], inout TriangleStream<PS_INPUT> OutStream)
{
    CharInfo charInfo = gCharInfos[input[0].Type];
    float2 size = float2(charInfo.width, charInfo.height) * Scale;
    float2 offset = float2(charInfo.xoffset, charInfo.yoffset) * Scale;
    float2 pos = input[0].Position.xy + offset;
    
    float2 positions[4];
    //좌상단
    positions[0] = pos;
    //우상단
    positions[1] = float2(pos.x + size.x,   pos.y);
    //좌하단
    positions[2] = float2(pos.x,   pos.y + size.y);
    //우하단
    positions[3] = pos + size;
    
    float2 LeftTop = float2(charInfo.x, charInfo.y) / TextureSize;
    float2 RightTop= float2(charInfo.x + charInfo.width, charInfo.y) / TextureSize;
    float2 LeftBottom = float2(charInfo.x, charInfo.y + charInfo.height) / TextureSize;
    float2 RightBottom = float2(charInfo.x + charInfo.width, charInfo.y + charInfo.height) / TextureSize;
    
    PS_INPUT Out;
    Out.Color = FontColor;
    
    
    Out.Position = ScreenToClip(positions[0], ScreenSize);
    Out.UV = LeftTop;
    OutStream.Append(Out);
    
    Out.Position = ScreenToClip(positions[1], ScreenSize);
    Out.UV = RightTop;
    OutStream.Append(Out);
    
    Out.Position = ScreenToClip(positions[2], ScreenSize);
    Out.UV = LeftBottom;
    OutStream.Append(Out);
    
    Out.Position = ScreenToClip(positions[3], ScreenSize);
    Out.UV = RightBottom;
    OutStream.Append(Out);
    
    OutStream.RestartStrip();

}

float4 PS_Font(PS_INPUT input) : SV_TARGET
{
    float4 texColor = gtxtFont.Sample(gSampler, input.UV.xy);
    return texColor * input.Color; // 알파값은 0 아니면 1 0이면 투명 1이면 불투명
}