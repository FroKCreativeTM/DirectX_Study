#include <iostream>
#include <Windows.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <d3d12.h>

using namespace DirectX;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

std::ostream& XM_CALLCONV operator<<(std::ostream& os, XMVECTOR v)
{
	XMFLOAT3 dest;
	XMStoreFloat3(&dest, v);

	os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ")";
	return os;
}

int main()
{

}