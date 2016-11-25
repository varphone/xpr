//********************************************************************//
//Pixel Shader

//1 texel offset
float OffsetX;
float OffsetY;
float WindowRatio;
float DestRatio;
//Alarm red channnel offset
float AlarmOffset;
//Adjustment factor

//Enchancement factor
float SharpFactor;  //0~1.0
float BrightFactor; //0~1.0
float DefogFactor;  //0~1.0
float2 HistFactor;  //Min pixel, max pixel

//Texture samplers
sampler2D TexY;     //Y component for planar format, YUV component for packed foramt.
sampler2D TexU;     //U component for planar format, or Pixel Map Table.
sampler2D TexV;     //V component for planar format.

//********************************//
//Common methods
float4 convertColorSpace(float3 yuvColor) : COLOR
{
    float3 matR = float3(1.164,  2.018,    0.0);
    float3 matG = float3(1.164, -0.391, -0.813);
    float3 matB = float3(1.164,    0.0,  1.596);
    float4 color = {0.0, 0.0, 0.0, 1.0};
    color.r = dot(yuvColor, matR) + AlarmOffset;
    color.g = dot(yuvColor, matG);
    color.b = dot(yuvColor, matB);
    return color;
}

//********************************//
//Planar methods
//Get YUV component from planar format.

float2 setCoordsRatio(float2 uvCoords)
{
    float2 uv = uvCoords;
    if(0.000001 > abs(WindowRatio-DestRatio)) {
        return uv;
    }
    else if(WindowRatio < DestRatio){
        float offset = (1.0-WindowRatio/DestRatio) / 2.0;
        if(offset<uvCoords.y && uvCoords.y<(1.0-offset)) {
            uv.y = (uvCoords.y-offset) * DestRatio / WindowRatio;
        } else {
            uv.y = -1.0;
        }
    }
    else {
        float offset = (1.0-DestRatio/WindowRatio) / 2.0;
        if(offset<uvCoords.x && uvCoords.x<(1.0-offset)) {
            uv.x = (uvCoords.x-offset) * WindowRatio / DestRatio;
        } else {
            uv.x = -1.0;
        }
    }

    return uv;
}

float3 getPlanarYUV(float2 uvCoords)
{
    float3 yuvColor = {0, 0, 0};
    //-16.0/255.0, -127.5/255.0, -127.5/255.0
    float3 deltaColor = float3(-0.062745, -0.5, -0.5);
    
    yuvColor.x = tex2D(TexY, uvCoords).x;
    yuvColor.y = tex2D(TexU, uvCoords).x;
    yuvColor.z = tex2D(TexV, uvCoords).x;
    yuvColor += deltaColor;    
    return yuvColor;
}

//Planar YUV ---> RGB, without enhancement
float4 PS_Original_Planar(float2 uvCoords : TEXCOORD) : COLOR
{
    float2 fixed_uv;
    fixed_uv.x = uvCoords.x + OffsetX * 0.5;
    fixed_uv.y = uvCoords.y + OffsetY * 0.5;
    float2 ratio_uv = setCoordsRatio(fixed_uv);
    float4 color = {0.0, 0.0, 0.0, 1.0};
    
    if(ratio_uv.x>-0.5 && ratio_uv.y>-0.5) {
        float3 yuvColor = getPlanarYUV(ratio_uv);
        color = convertColorSpace(yuvColor);
    }
    return color;
}

//return original RGBA pixel, fix 0.5 offset.
float4 PS_Original_RGB(float2 uvCoords : TEXCOORD) : COLOR
{
    float2 fixed_uv;
    fixed_uv.x = uvCoords.x + OffsetX * 0.5;
    fixed_uv.y = uvCoords.y + OffsetY * 0.5;
    return tex2D(TexY, fixed_uv);
}

float4 PS_Brighten_Y(float2 uvCoords : TEXCOORD) : COLOR
{
    float2 fixed_uv;
    fixed_uv.x = uvCoords.x + 0.5 * OffsetX;
    fixed_uv.y = uvCoords.y + 0.5 * OffsetY;

    float4 color = tex2D(TexY, fixed_uv);
    color.x += BrightFactor * 0.5 * color.x;
    return color;
}

float4 PS_Sharpen_Y(float2 uvCoords : TEXCOORD) : COLOR
{
    float4 color = {0.0, 0.0, 0.0, 1.0};
    float2 fixed_uv;
    fixed_uv.x = uvCoords.x + 0.5 * OffsetX;
    fixed_uv.y = uvCoords.y + 0.5 * OffsetY;
    //sigma = 1.0
    float3x3 filter = {
        0.058550, 0.096532, 0.058550,
        0.096532, 0.159155, 0.096532,
        0.058550, 0.096532, 0.058550,
    };
    color.x = tex2D(TexY, fixed_uv).x;
    float smoothColor = 0.0;
    float sum = 0.0;
    float2 curUV = { 0.0, 0.0 };
        for (int i = 0; i<3; ++i) {
            for (int j = 0; j<3; ++j) {
                curUV.x = fixed_uv.x + (i - 1.0) * OffsetX;
                curUV.y = fixed_uv.y + (j - 1.0) * OffsetY;
                smoothColor += tex2D(TexY, curUV).x * filter[i][j];
                sum += filter[i][j];
            }
        }
    if (0.000001 > sum) {
        return color;
    }
    smoothColor /= sum;
    color.x += SharpFactor * (color.x - smoothColor);
    
    return color;
}

float4 PS_Stretch_Y(float2 uvCoords : TEXCOORD) : COLOR
{
    float2 fixed_uv;
    fixed_uv.x = uvCoords.x + 0.5 * OffsetX;
    fixed_uv.y = uvCoords.y + 0.5 * OffsetY;
    float4 color = tex2D(TexY, fixed_uv);
    float max_pixel = max(HistFactor.y, 0.9);
    color.x = (color.x - HistFactor.x) * max_pixel / (HistFactor.y - HistFactor.x);
    return color;
}

float4 PS_Equal_Y(float2 uvCoords : TEXCOORD) : COLOR
{
    float2 fixed_uv;
    fixed_uv.x = uvCoords.x + 0.5 * OffsetX;
    fixed_uv.y = uvCoords.y + 0.5 * OffsetY;
    float4 color = tex2D(TexY, fixed_uv);
    //Lookup the pixel map table.
    float2 uv = float2(color.x+0.5/256, 0.5);
    color.x = tex2D(TexU, uv).x;
    return color;
}

//********************************//
//UYVY methods
//Get YUV component from UYVY format.
float3 getUyvyYUV(float2 uvCoords)
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
    float3 yuvColor = getUyvyYUV(uvCoords);
    return convertColorSpace(yuvColor);
}

//********************************************************************//
//technique
technique Tech_Planar
{
    pass P0
    {
        pixelShader = compile ps_2_0 PS_Original_Planar();
    }
    pass P1
    {
        pixelShader = compile ps_2_0 PS_Original_RGB();
    }
    pass P2
    {
        pixelShader = compile ps_2_0 PS_Sharpen_Y();
    }
    pass P3
    {
        pixelShader = compile ps_2_0 PS_Brighten_Y();
    }
    pass P4
    {
        pixelShader = compile ps_2_0 PS_Stretch_Y();
    }
    pass P5
    {
        pixelShader = compile ps_2_0 PS_Equal_Y();
    }
}

technique Tech_UYVY
{
    pass P0
    {
        pixelShader = compile ps_2_0 PS_Original_UYVY();
    }
}