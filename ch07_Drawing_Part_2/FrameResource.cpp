#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice, UINT nPassCount, UINT nObjectCount)
{
	// ��� �Ҵ��ڸ� �����Ѵ�.
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(pCmdListAlloc.GetAddressOf())));

	// Pass ��� ���� ����
	pPassCB = std::make_unique<UploadBuffer<PassConstants>>(pDevice, nPassCount, true);

	// ������Ʈ ��� ���� ����
	pObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(pDevice, nObjectCount, true);
}

FrameResource::~FrameResource()
{
}
