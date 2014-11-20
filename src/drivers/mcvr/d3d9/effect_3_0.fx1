//********************************************************************//
//Pixel Shader

//Texture samplers
sampler2D TexY;     //Y component for planar format, YUV component for packed foramt.
sampler2D TexU;     //U component for planar format.
sampler2D TexV;     //V component for planar format.
//1 texel offset
float OffsetX;
float OffsetY;

//********************************//
//Common methods
float4 convert_color_space(float3 yuvColor) : COLOR
{
    float3 matR = float3(1.164,  2.018,    0.0);
    float3 matG = float3(1.164, -0.391, -0.813);
    float3 matB = float3(1.164,    0.0,  1.596);
    float4 color;
    color.r = dot(yuvColor, matR);
    color.g = dot(yuvColor, matG);
    color.b = dot(yuvColor, matB);
    color.a = 1.0f;

    return color;
}

//********************************//
//Planar methods
//Get YUV component from planar format.
float3 get_Planar_YUV(float2 uvCoords)
{
    float3 yuvColor;
    float3 deltaYUV = float3(-16.0/255.0 , -128.0/255.0 , -128.0/255.0);
    yuvColor.x = tex2D(TexY, uvCoords).x;
    yuvColor.y = tex2D(TexU, uvCoords).x;
    yuvColor.z = tex2D(TexV, uvCoords).x;
    yuvColor += deltaYUV;

    return yuvColor;
}

//Planar YUV ---> RGB, without enhancement
float4 PS_Original_Planar(float2 uvCoords : TEXCOORD) : COLOR
{
    float3 yuvColor = get_Planar_YUV(uvCoords);
    return convert_color_space(yuvColor);
}

//********************************//
//UYVY methods
//Get YUV component from UYVY format.
float3 get_UYVY_YUV(float2 uvCoords)
{
    float3 yuvColor;
    float3 deltaYUV = float3(-16.0/255.0 , -128.0/255.0 , -128.0/255.0);
    yuvColor.x = tex2D(TexY, uvCoords).y;
    int evenX = (uvCoords.x * OffsetX) % 2;
    if(0 == evenX) {    //Even number
        yuvColor.y = tex2D(TexY, uvCoords).x;
        yuvColor.z = tex2D(TexY, uvCoords + float2(OffsetX, 0.0f)).x;
    } else {            //Odd number
        yuvColor.y =  tex2D(TexY, uvCoords + float2(-OffsetX, 0.0f)).x;
        yuvColor.y += tex2D(TexY, uvCoords + float2(OffsetX, 0.0f)).x;
        yuvColor.y /= 2.0f;
        yuvColor.z = tex2D(TexY, uvCoords).x;
        yuvColor.z += tex2D(TexY, uvCoords + float2(2.0f*OffsetX, 0.0f)).x;
        yuvColor.z /= 2.0f;
    }
    yuvColor += deltaYUV;

    return yuvColor;
}

//UYVY ---> RGB, without enhancement
float4 PS_Original_UYVY(float2 uvCoords : TEXCOORD) : COLOR
{
    float3 yuvColor = get_UYVY_YUV(uvCoords);
    return convert_color_space(yuvColor);
}

//********************************************************************//
//Effect
technique Tech_Planar
{
    pass P0
    {
        pixelShader = compile ps_3_0 PS_Original_Planar();
    }
}

technique Tech_UYVY
{
    pass P0
    {
        pixelShader = compile ps_3_0 PS_Original_UYVY();
    }
}