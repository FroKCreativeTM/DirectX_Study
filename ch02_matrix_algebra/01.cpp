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
		std::cout << "�� CPU�� �������� �ʴ� CPU�Դϴ�." << std::endl;
		return 0;
	}

	// �ܼ� ��� ����
	XMMATRIX mat1 = XMMatrixSet(
		3.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 4.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 2.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	std::cout << mat1 << std::endl;

	// ������� I�� �����ش�.
	XMMATRIX mat2 = XMMatrixIdentity();
	std::cout << mat2 << std::endl;

	// mat1�� mat2�� ��� ���� �����ش�.
	XMMATRIX mat3 = XMMatrixMultiply(mat1, mat2);
	std::cout << mat3 << std::endl;

	// ��ġ����� �����ش�.
	XMMATRIX mat4 = XMMatrixTranspose(mat1);
	std::cout << mat4 << std::endl;

	// ��Ľ��� �����ش�.
	XMVECTOR vecDetMat1 = XMMatrixDeterminant(mat1);
	std::cout << vecDetMat1 << std::endl;

	// ������� �����ش�.
	XMMATRIX mat5 = XMMatrixInverse(&vecDetMat1, mat1);
	std::cout << mat5 << std::endl;

	XMMATRIX mat6 = mat1 * mat5;
	std::cout << mat6 << std::endl;

	return 0;
}