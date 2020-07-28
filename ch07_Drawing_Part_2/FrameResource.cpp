#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice, UINT nPassCount, UINT nObjectCount)
{
	// 명령 할당자를 생성한다.
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(pCmdListAlloc.GetAddressOf())));

	// Pass 상수 버퍼 생성
	pPassCB = std::make_unique<UploadBuffer<PassConstants>>(pDevice, nPassCount, true);

	// 오브젝트 상수 버퍼 생성
	pObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, nObjectCount, true);
}

FrameResource::~FrameResource()
{
}
