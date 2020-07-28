cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
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
	// ����� ����
	VertexOut vout;

	// ���� ���� �������� ��ȯ�Ѵ�.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	// ���� ������ �״�� �ȼ� ���̴��� �����Ѵ�.
	vout.Color = vin.Color;

	// ������ ���� �������� ��ȯ
	return vout;
}

float4 ps_main(VertexOut pin) : SV_Target
{
	// �� �ȼ����� �� ��ȯ
	return pin.Color;
}