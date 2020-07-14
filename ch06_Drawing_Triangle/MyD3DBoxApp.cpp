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
	// 명령 기록에 관련된 메모리의 재활용을 위해 명령 할당자를 재설정한다.
	// 재설정은 GPU가 관련 명령 목록들을 모두 처리한 뒤에 일어난다.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// 명령 목록을 ExecuteCommandList를 통해서 명령 대기열에 추가했다면 명령 목록을
	// 재설정하는 것도 가능하다. 명령 목록을 재설정했다면 메모리가 재활용된다.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// 자원 용도에 관련된 상태 전리를 Direct3D에 전달한다.
	mCommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		)
	);

	// 뷰포트와 가위 직사각형을 설정한다.
	// 명령 목록을 재설정할 때마다 이들도 재설정이 필요하다.
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// 후면 버퍼와 깊이 버퍼를 지운다.
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

	// 렌더링 결과가 기록될 렌더 대상 버퍼를 지정한다.
	mCommandList->OMSetRenderTargets(
		1,
		&CurrentBackBufferView(),
		true,
		&DepthStencilView()
	);

	// 자원 용도에 관련된 상태 전리를 Direct3D에 알린다.
	mCommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		)
	);

	// 명령들의 기록을 마친다.
	ThrowIfFailed(mCommandList->Close());

	// 명령 실행을 위해 명령 목록을 명령 대기열에 추가한다.
	ID3D12CommandList* cmdsLists[] = {
		mCommandList.Get()
	};
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 후면 버퍼와 전면 버퍼를 교환한다.
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// 이 프레임의 명령들이 모두 처리되길 기다린다.
	// 이러한 대기는 비효율적이기에
	// 이후의 예제들에서는 렌더링 코드를 적절히 조직화하여 프레임마다 대기가 필요없게 만든다.
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
	// 미리 지정된 정점들의 집합
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

	// 지정된 정점들의 사이즈를 저장한다.
	const UINT64 vbByteSize = 8 * sizeof(Vertex);

	// 입방체의 정점 8개를 저장하는 기본 버퍼이다.
	ComPtr<ID3D12Resource> vertexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> vertexBufferUploader = nullptr;
	vertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), vertices,
		vbByteSize, vertexBufferUploader);

	// 인덱스 배열을 생성한다.
	std::uint16_t indices[] =
	{
		// 앞 면
		0, 1, 2,
		0, 2, 3,

		// 뒷 면
		4, 6, 5,
		4, 7, 6,

		// 왼쪽 면
		4, 5, 1,
		4, 1, 0,

		// 오른쪽 면
		3, 2, 6,
		3, 6, 7,

		// 윗 면
		1, 5, 6,
		1, 6, 2,

		// 아랫 면
		4, 0, 3,
		4, 3, 7
	};

	// 인덱스 배열 사이즈 저장
	const UINT ibByteSize = 36 * sizeof(std::uint16_t);

	// 인덱스 배열을 GPU로 보내기 위한 버퍼를 생성
	ComPtr<ID3D12Resource> indexBufferGPU = nullptr;
	ComPtr<ID3D12Resource> indexBufferUploader = nullptr;
	indexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(),
		indices, ibByteSize, indexBufferUploader);

	// 인덱스 버퍼를 파이프 라인에 묶기 위한 서술자
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	indexBufferView.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = ibByteSize;
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	// 이제 명령 리스트에 인덱스 버퍼를 설정했다.
	mCommandList->IASetIndexBuffer(&indexBufferView);

	// 그리기 위해서 DrawIndexedInstanced 메서드를 사용한다.
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
