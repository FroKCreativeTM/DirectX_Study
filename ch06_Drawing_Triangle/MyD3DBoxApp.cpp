#include "MyD3DBoxApp.h"

void MyD3DBoxApp::OnResize()
{
	D3DApp::OnResize();
}


void MyD3DBoxApp::Update(const GameTimer& gt)
{
}

void MyD3DBoxApp::Draw(const GameTimer& gt)
{
	// ��� ��Ͽ� ���õ� �޸��� ��Ȱ���� ���� ��� �Ҵ��ڸ� �缳���Ѵ�.
	// �缳���� GPU�� ���� ��� ��ϵ��� ��� ó���� �ڿ� �Ͼ��.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// ��� ����� ExecuteCommandList�� ���ؼ� ��� ��⿭�� �߰��ߴٸ� ��� �����
	// �缳���ϴ� �͵� �����ϴ�. ��� ����� �缳���ߴٸ� �޸𸮰� ��Ȱ��ȴ�.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// �ڿ� �뵵�� ���õ� ���� ������ Direct3D�� �����Ѵ�.
	mCommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		)
	);

	// ����Ʈ�� ���� ���簢���� �����Ѵ�.
	// ��� ����� �缳���� ������ �̵鵵 �缳���� �ʿ��ϴ�.
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// �ĸ� ���ۿ� ���� ���۸� �����.
	mCommandList->ClearRenderTargetView(
		CurrentBackBufferView(),
		Colors::Aquamarine,
		0,
		nullptr
	);
	mCommandList->ClearDepthStencilView(
		DepthStencilView(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f,
		0,
		0,
		nullptr
	);

	// ������ ����� ��ϵ� ���� ��� ���۸� �����Ѵ�.
	mCommandList->OMSetRenderTargets(
		1,
		&CurrentBackBufferView(),
		true,
		&DepthStencilView()
	);

	// �ڿ� �뵵�� ���õ� ���� ������ Direct3D�� �˸���.
	mCommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		)
	);

	// ��ɵ��� ����� ��ģ��.
	ThrowIfFailed(mCommandList->Close());

	// ��� ������ ���� ��� ����� ��� ��⿭�� �߰��Ѵ�.
	ID3D12CommandList* cmdsLists[] = {
		mCommandList.Get()
	};
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// �ĸ� ���ۿ� ���� ���۸� ��ȯ�Ѵ�.
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// �� �������� ��ɵ��� ��� ó���Ǳ� ��ٸ���.
	// �̷��� ���� ��ȿ�����̱⿡
	// ������ �����鿡���� ������ �ڵ带 ������ ����ȭ�Ͽ� �����Ӹ��� ��Ⱑ �ʿ���� �����.
	FlushCommandQueue();
}

void MyD3DBoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
}

void MyD3DBoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
}

void MyD3DBoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
}

void MyD3DBoxApp::BuildDescriptorHeaps()
{
}

void MyD3DBoxApp::BuildConstantBuffers()
{
}

void MyD3DBoxApp::BuildRootSignature()
{
}

void MyD3DBoxApp::BuildShadersAndInputLayout()
{
}

void MyD3DBoxApp::BuildBoxGeometry()
{
	// �̸� ������ �������� ����
	Vertex vertices[] =
	{
		{	XMFLOAT3(-1.f, -1.f, -1.f), XMFLOAT4(Colors::White)	},
		{	XMFLOAT3(-1.f, 1.f, -1.f), XMFLOAT4(Colors::Black)	},
		{	XMFLOAT3(1.f, 1.f, -1.f), XMFLOAT4(Colors::Red)		},
		{	XMFLOAT3(-1.f, 1.f, -1.f), XMFLOAT4(Colors::Green)	},
		{	XMFLOAT3(-1.f, -1.f, 1.f), XMFLOAT4(Colors::Blue)	},
		{	XMFLOAT3(-1.f, 1.f, 1.f), XMFLOAT4(Colors::Yellow)	},
		{	XMFLOAT3(1.f, 1.f, 1.f), XMFLOAT4(Colors::Cyan)		},
		{	XMFLOAT3(-1.f, 1.f, -1.f), XMFLOAT4(Colors::Magenta)},
	};

	// ������ �������� ����� �����Ѵ�.
	const UINT64 vbByteSize = 8 * sizeof(Vertex);

	// �Թ�ü�� ���� 8���� �����ϴ� �⺻ �����̴�.
	ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
	vertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices,
		vbByteSize, vertexBufferUploader);

	// �ε��� �迭�� �����Ѵ�.
	std::uint16_t indices[] =
	{
		// �� ��
		0, 1, 2,
		0, 2, 3,

		// �� ��
		4, 6, 5,
		4, 7, 6,

		// ���� ��
		4, 5, 1,
		4, 1, 0,

		// ������ ��
		3, 2, 6,
		3, 6, 7,

		// �� ��
		1, 5, 6,
		1, 6, 2,

		// �Ʒ� ��
		4, 0, 3,
		4, 3, 7
	};

	// �ε��� �迭 ������ ����
	const UINT ibByteSize = 36 * sizeof(std::uint16_t);

	// �ε��� �迭�� GPU�� ������ ���� ���۸� ����
	ComPtr<ID3D12Resource> indexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> indexBufferUploader = nullptr;
	indexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(),
		indices, ibByteSize, indexBufferUploader);

	// �ε��� ���۸� ������ ���ο� ���� ���� ������
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	indexBufferView.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = ibByteSize;
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	// ���� ��� ����Ʈ�� �ε��� ���۸� �����ߴ�.
	mCommandList->IASetIndexBuffer(&indexBufferView);

	// �׸��� ���ؼ� DrawIndexedInstanced �޼��带 ����Ѵ�.
}

void MyD3DBoxApp::BuildPSO()
{
}

MyD3DBoxApp::MyD3DBoxApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	mMainWndCaption = L"My First D3D Application";
}

MyD3DBoxApp::~MyD3DBoxApp()
{
}

bool MyD3DBoxApp::Initialize()
{
	if (D3DApp::Initialize())
		return false;
	return true;
}
