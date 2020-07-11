#include <iostream>
#include <windows.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3d12.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;

std::ostream& XM_CALLCONV operator<<(std::ostream& os, XMVECTOR v)
{
	XMFLOAT3 dest;
	XMStoreFloat3(&dest, v);

	os << '(' << dest.x << ", " << dest.y << ", " << dest.z << ')';
	os << std::endl;
	return os;
}

std::ostream& XM_CALLCONV operator<<(std::ostream& os, XMMATRIX m)
{
	for (size_t i = 0; i < 4; i++)
	{
		os << XMVectorGetX(m.r[i]) << "\t";
		os << XMVectorGetY(m.r[i]) << "\t";
		os << XMVectorGetZ(m.r[i]) << "\t";
		os << XMVectorGetW(m.r[i]) << "\t";
		os << std::endl;
	}

	return os;
}

int main()
{
	if (!XMVerifyCPUSupport())
	{
		std::cout << "이 CPU는 지원되지 않는 CPU입니다." << std::endl;
		return 0;
	}

	// 단순 행렬 선언
	XMMATRIX mat1 = XMMatrixSet(
		3.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 4.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 2.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	std::cout << mat1 << std::endl;

	// 단위행렬 I를 돌려준다.
	XMMATRIX mat2 = XMMatrixIdentity();
	std::cout << mat2 << std::endl;

	// mat1과 mat2의 행렬 곱을 돌려준다.
	XMMATRIX mat3 = XMMatrixMultiply(mat1, mat2);
	std::cout << mat3 << std::endl;

	// 전치행렬을 돌려준다.
	XMMATRIX mat4 = XMMatrixTranspose(mat1);
	std::cout << mat4 << std::endl;

	// 행렬식을 돌려준다.
	XMVECTOR vecDetMat1 = XMMatrixDeterminant(mat1);
	std::cout << vecDetMat1 << std::endl;

	// 역행렬을 돌려준다.
	XMMATRIX mat5 = XMMatrixInverse(&vecDetMat1, mat1);
	std::cout << mat5 << std::endl;

	XMMATRIX mat6 = mat1 * mat5;
	std::cout << mat6 << std::endl;

	return 0;
}