#include <iostream>
#include <Windows.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3d12.h>

using namespace DirectX;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

std::ostream& XM_CALLCONV operator << (std::ostream& os, DirectX::FXMVECTOR v)
{
	DirectX::XMFLOAT3 dest;
	DirectX::XMStoreFloat3(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ")";
	return os;
}

int main()
{
	std::cout.setf(std::ios_base::boolalpha);

	// SSE2를 지원하는지 (Pentium4, AMD K8 이상) 확인
	if(!DirectX::XMVerifyCPUSupport())
	{
		std::cout << "DirectXMath를 지원하지 않음" << std::endl;
		return 0;
	}

	// 다양한 벡터 선언
	DirectX::XMVECTOR v1 = DirectX::XMVectorZero();
	DirectX::XMVECTOR v2 = DirectX::XMVectorSplatOne();
	DirectX::XMVECTOR v3 = DirectX::XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
	DirectX::XMVECTOR v4 = DirectX::XMVectorReplicate(-2.0f);
	DirectX::XMVECTOR v5 = DirectX::XMVectorSplatZ(v3);

	std::cout << "v1 = " << v1 << std::endl;
	std::cout << "v2 = " << v2 << std::endl;
	std::cout << "v3 = " << v3 << std::endl;
	std::cout << "v4 = " << v4 << std::endl;
	std::cout << "v5 = " << v5 << std::endl;

	// 단순 계산 
	DirectX::XMVECTOR vPlus = v1 + v2;
	DirectX::XMVECTOR vMinus = v3 - v4;
	DirectX::XMVECTOR vMulti = v1 * v5;

	std::cout << "v1 + v2 = " << vPlus << std::endl;
	std::cout << "v3 - v4 = " << vMinus << std::endl;
	std::cout << "v1 * v5 = " << vMulti << std::endl;

	// 길이 구하기와 정규화
	DirectX::XMVECTOR sLength = DirectX::XMVector3Length(v4);
	DirectX::XMVECTOR sNormalize = DirectX::XMVector3Normalize(v4);
	std::cout << "length of v4 = " << sLength << std::endl;
	std::cout << "normalize of v4 = " << sNormalize << std::endl;

	// 내적 외적
	DirectX::XMVECTOR sDot = DirectX::XMVector3Dot(v2, v3);
	DirectX::XMVECTOR vCross = DirectX::XMVector3Cross(v3, v5);
	std::cout << "v2 * v3 = " << sDot << std::endl;
	std::cout << "v3 x v5 = " << vCross << std::endl;

	DirectX::XMVECTOR w = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMVECTOR n = DirectX::XMVectorSet(0.707f, 0.707f, 0.0f, 0.0f);

	// projection(투영)과 perpendicular(수직) 벡터를 구한다.
	DirectX::XMVECTOR projW;
	DirectX::XMVECTOR perpW;
	DirectX::XMVector3ComponentsFromNormal(&projW, &perpW, w, n);
	std::cout << "projection of v2 and v3 = " << projW << std::endl;
	std::cout << "perpendicular of v2 and v3 = " << perpW << std::endl;

	// projv2 + perpv2 == v2?
	bool equal = DirectX::XMVector3Equal(projW + perpW, v2) != 0;
	bool notEqual = DirectX::XMVector3NotEqual(projW + perpW, v2) != 0;
	std::cout << "projv2 + perpv2 == v2? = " << equal << std::endl;
	std::cout << "projv2 + perpv2 != v2? = " << notEqual << std::endl;

	// projv2와 perpv2 사이의 각도는 반드시 90도이다.
	DirectX::XMVECTOR angleVec = DirectX::XMVector3AngleBetweenVectors(projW, perpW);
	float angleRadians = DirectX::XMVectorGetX(angleVec);
	float angleDegrees = DirectX::XMConvertToDegrees(angleRadians);
	std::cout << "angle vector of projv2 and perpv2 = " << angleVec << std::endl;
	std::cout << "angleRadians of projv2 and perpv2 = " << angleRadians << std::endl;
	std::cout << "angleDegrees of projv2 and perpv2 = " << angleDegrees << std::endl;

	return 0;
}