#pragma once

#include "../Common/d3dUtil.h"
#include "../Common/MathHelper.h"
#include "../Common/UploadBuffer.h"

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
};

struct PassConstants
{
	DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
	float cbPerObjectPad1 = 0.0f;
	DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
	DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
	float NearZ = 0.0f;
	float FarZ = 0.0f;
	float TotalTime = 0.0f;
	float DeltaTime = 0.0f;
};

class FrameResource
{
public : 
	// 명령 할당자는 GPU가 명령들을 다 처리한 후 재설정 해야한다.
	// 따라서 프레임마다 할당자가 필요하다.
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCmdListAlloc;

	// 상수 버퍼는 그것을 참조하는 명령들을 GPU가 다 처리한 후에
	// 갱신해야 한다. 따라서 프레임마다 상수 버퍼를 새로 만들어야 한다.
	std::unique_ptr<UploadBuffer<PassConstants>> pPassCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> pObjectCB = nullptr;

	// Fence는 현재 울타리 지점까지의 명령들을 표시하는 값이다.
	// 이 값은 GPU가 아직 이 프레임 자원들을 사용하고 있는 지 판정하는 용도로 쓰인다.
	UINT nFence = 0;

public  : 
	FrameResource(ID3D12Device* pDevice, UINT nPassCount, UINT nObjectCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();
};

