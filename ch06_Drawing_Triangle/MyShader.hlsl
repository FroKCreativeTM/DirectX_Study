cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
	//float4 gPulseColor;
	float gTime;
};

struct VertexIn
{
	float3 PosL  : POSITION;
	float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
	float4 Color : COLOR;
};

VertexOut vs_main(VertexIn vin)
{
	// 출력할 정점
	VertexOut vout;
	
	// vin.PosL.xy += 0.5f * sin(vin.PosL.xy) * sin(3.0f, gTime);
	// vin.PosL.z *= 0.6f + 0.4f * sin(2.0f * gTime);

	// 동차 절단 공간으로 변환한다.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	// 정점 색상을 그대로 픽셀 셰이더에 전달한다.
	vout.Color = vin.Color;

	// 정점을 세계 공간으로 반환
	return vout;
}

float4 ps_main(VertexOut pin) : SV_Target
{
	//const float pi = 3.141592;
	//
	//float s = 0.5f * sin(2 * gTime - 0.25f * gTime) + 0.5f;
	//
	//float4 c = lerp(pin.Color, gPulseColor, s);

	// 각 픽셀마다 색 반환
	return pin.Color;
	//return c;
}
