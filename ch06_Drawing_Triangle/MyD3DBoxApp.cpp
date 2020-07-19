#include "MyD3DBoxApp.h"

void MyD3DBoxApp::OnResize()
{
	D3DApp::OnResize();

	// 투영 행렬은 윈도우에 이를 반영한다.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

// 마우스 입력이 있으면 좌표계가 갱신되게 해보자.
void MyD3DBoxApp::Update(const GameTimer& gt)
{
	// 구면 좌표를 데카르트 좌표로 변환하자
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float y = mRadius * cosf(mPhi);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);

	// 시야 행렬을 구축한다.
	XMVECTOR position = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();				// direction
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// 뷰 행렬 생성
	XMMATRIX view = XMMatrixLookAtLH(position, target, up);
	XMStoreFloat4x4(&mView, view);	// 멤버 행렬에 매핑한다.

	// 세계, 투영, WVP를 전부 매핑한다.
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX projection = XMLoadFloat4x4(&mProj);
	XMMATRIX WVP = world * view * projection;

	// 최신의 MVP 행렬로 상수 버퍼를 갱신한다.
	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(WVP));
	mObjectCB->CopyData(0, objConstants);
}

void MyD3DBoxApp::Draw(const GameTimer& gt)
{
	// 명령 기록에 관련된 메모리의 재활용을 위해 명령 할당자를 재설정한다.
	// 재설정은 GPU가 관련 명령 목록들을 모두 처리한 뒤에 일어난다.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// 명령 목록을 ExecuteCommandList를 통해서 명령 대기열에 추가했다면 명령 목록을
	// 재설정하는 것도 가능하다. 명령 목록을 재설정했다면 메모리가 재활용된다.
	// ※ 주의!!!! : 만약 여기서 명령어 할당자에 파이프라인 상태 오브젝트로 리셋 시키지 않으면 아무것도 안 뜬다!!!
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

	// 뷰포트와 가위 직사각형을 설정한다.
	// 명령 목록을 재설정할 때마다 이들도 재설정이 필요하다.
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// 자원 용도에 관련된 상태 전리를 Direct3D에 전달한다.
	mCommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		)
	);

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

	// 서술자 힙을 명령어 리스트에 설정한다.
	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// 서술자 테이블을 파이프라인에 묶는다.
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());
	
	// 이번 그리기 호출에 사용할 CBV의 오프셋이다.
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbv(mCbvHeap->GetGPUDescriptorHandleForHeapStart());

	// 박스 오브젝트의 기하 보조 구조체의 버퍼를 설정한다.
	mCommandList->IASetVertexBuffers(0, 1, &mBoxGeo->VertexBufferView());
	mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 그래픽스 루트 서술자 테이블을 설정한다.
	mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

	// 인덱스 카운트만큼 그린다.
	mCommandList->DrawIndexedInstanced(
		mBoxGeo->DrawArgs["box"].IndexCount,
		1, 0, 0, 0);

	// 자원 용도에 관련된 상태 전이를 Direct3D에 알린다.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

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
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void MyD3DBoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

// 마우스 움직임에 따라서 물체가 변하게 만든다. 
void MyD3DBoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	// 만약 왼쪽 마우스 버튼이 아니라면
	if ((btnState & MK_LBUTTON) != 0)
	{
		// 마우스 한 픽셀 이동을 4분의 1도에 대응시킨다.
		float dx = XMConvertToRadians(0.25f * static_cast<float> (x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float> (y - mLastMousePos.y));

		// 입력에 기초해 각도를 갱신해서 카메라가 상자를 중심으로 공전하게 한다.
		mTheta += dx;
		mPhi += dy;

		// mPhi의 각도를 제한한다
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	// 만약 왼쪽 마우스라면
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// 마우스 한 픽셀 이동을 0.005도에 대응시킨다.
		float dx = 0.005f * static_cast<float> (x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float> (y - mLastMousePos.y);

		// 입력에 기초해서 카메라 반지름을 갱신한다.
		mRadius += dx - dy;

		// 반지름을 제한한다.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.f);
	}

	// 마우스 위치를 갱신한다.
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void MyD3DBoxApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesk;	// 상수 버퍼 서술자 생성
	cbvHeapDesk.NumDescriptors = 1;
	// SRV : 셰이더 자원 뷰
	// UAV : 순서없는 접근 뷰
	cbvHeapDesk.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesk.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesk.NodeMask = 0;

	// 상수 버퍼 서술자를 멤버 변수에 매핑한다.
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesk, IID_PPV_ARGS(&mCbvHeap)));
}

void MyD3DBoxApp::BuildConstantBuffers()
{
	// 물체 n개의 상수 자료를 담을 상수 버퍼 (여기서는 1개만 담도록 하겠다.)
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);

	// 상수 버퍼의 바이트 사이즈를 가져온다.
	// CalcConstantBufferByteSize는 버퍼 사이즈를 자동으로 구해주는 것으로
	// 최소 하드웨어 할당 크기(256바이트)의 배수가 되게 하는 계산을 수행한다.
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	// 버퍼 자체의 시작 주소(0번째 상수 버퍼의 주소)
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();

	// 버퍼에 담긴 i번째 상수 버퍼의 오프셋 (여기서는 0)
	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	// 상수 버퍼 뷰 서술자를 생성한다.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	// 디바이스에 서술자 정보를 바탕으로 상수 버퍼 뷰를 생성한다.
	md3dDevice->CreateConstantBufferView(&cbvDesc, mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

// 루트 서명과 서술자 테이블을 생성한다.
void MyD3DBoxApp::BuildRootSignature()
{
	// 루트 서명은 그리기 호출 전에 응용 프로그램이 반드시 렌더링 파이프라인에 묶어야 할 자원이 무엇이고
	// 그 자원들이 어떤 셰이더 입력 레지스터들에 어떻게 대응되는지 정의하는 것이다.
	// 즉 반드시 그리기 호출에 쓰이는 셰이더들과 호환되어야 한다.
	
	// 이 루트 서명에는 다양한 매개변수가 들어가지만, 여기서는 일단 서술자 테이블이라는
	// 서술자 힙 안에 있는 연속된 서술자들의 구간을 지정하는 것으로 실습하겠다.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// CBV (상수 버퍼 뷰) 하나를 담는 서술자 테이블을 생성한다.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV,	// 루트 서명의 매개변수
		1,		// 테이블의 서술자 갯수
		0);		// 이 루트 매개변수에 묶일 셰이더 인수들의 기준 레지스터 번호

	// 루트 서명을 이미 생성된 서술자 테이블로 초기화 한다.
	slotRootParameter[0].InitAsDescriptorTable(
		1,			// 구간 갯수
		&cbvTable);

	// 루트 서명은 루트 매개변수들의 배열이다.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// 상수 버퍼 하나로 구성된 서술자 구간을 가리키는 슬롯 하나로 이루어진 루트 서명을 생성한다.
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	// 에러 체크
	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	// 루트 서명 생성
	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));

	// 루트 서명은 실제 자원들을 묶지 않고 자원들을 정의만 한다!
}

// 셰이더를 컴파일하고 빌드한 뒤 레이아웃에 어떻게 입력을 줄 것인가에 대한 메서드이다.
void MyD3DBoxApp::BuildShadersAndInputLayout()
{
	// 일단 결과를 OK로 초기화한다.
	HRESULT hr = S_OK;

	// 셰이더를 컴파일한다.
	// 만약 실행 시간이 아니라 미리 빌드된 즉 오프라인에서 개별적으로 컴파일한 오프라인 컴파일을 한다면 
	// cso 파일로 저장된다. (이 빌드 방법은 책을 볼 것)
	mvsByteCode = d3dUtil::CompileShader(L"MyShader.hlsl", nullptr, "vs_main", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"MyShader.hlsl", nullptr, "ps_main", "ps_5_0");

	// 레이아웃을 지정한다.
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void MyD3DBoxApp::BuildBoxGeometry()
{
	// 미리 지정된 정점들의 집합
	std::array<Vertex, 8> vertices =
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	};

	// 인덱스 배열을 생성한다.
	std::array<std::uint16_t, 36> indices =
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

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	mBoxGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);

	mBoxGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

	mBoxGeo->VertexByteStride = sizeof(Vertex);
	mBoxGeo->VertexBufferByteSize = vbByteSize;
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mBoxGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mBoxGeo->DrawArgs["box"] = submesh;
}

// Pipeline State Object
// 파이프라인 상태를 기술한다.
// 이 때 래스터화기 상태는 렌더링 파이프라인에서 프로그래밍은 불가능하고 설정만 가능한 부분 중 하나이다.
void MyD3DBoxApp::BuildPSO()
{
	// 이 부분은 프로그래밍이 아니라 설정만 가능한 부분이다.
	// 즉 파이프라인 상에서 프로그래밍이 불가능 부분 중 하나라는 뜻이다.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	psoDesc.pRootSignature = mRootSignature.Get();
	psoDesc.VS = {
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize()
	};
	psoDesc.PS = {
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
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
	if (!D3DApp::Initialize())
		return false;

	// 일단 모든 부분을 리셋한다.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// 빌드를 위한 설정을 한다.
	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();

	// 초기화 명렁어를 실행한다.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}


/*
MyD3DBoxApp::MyD3DBoxApp(HINSTANCE hInstance)
: D3DApp(hInstance) 
{
}

MyD3DBoxApp::~MyD3DBoxApp()
{
}

bool MyD3DBoxApp::Initialize()
{
    if(!D3DApp::Initialize())
		return false;
		
    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
 
    BuildDescriptorHeaps();
	BuildConstantBuffers();
    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildBoxGeometry();
    BuildPSO();

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

	return true;
}

void MyD3DBoxApp::OnResize()
{
	D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection matrix.
    XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
    XMStoreFloat4x4(&mProj, P);
}

void MyD3DBoxApp::Update(const GameTimer& gt)
{
    // Convert Spherical to Cartesian coordinates.
    float x = mRadius*sinf(mPhi)*cosf(mTheta);
    float z = mRadius*sinf(mPhi)*sinf(mTheta);
    float y = mRadius*cosf(mPhi);

    // Build the view matrix.
    XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
    XMVECTOR target = XMVectorZero();
    XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
    XMStoreFloat4x4(&mView, view);

    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    XMMATRIX worldViewProj = world*view*proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	ObjectConstants objConstants;
    XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
    mObjectCB->CopyData(0, objConstants);
}

void MyD3DBoxApp::Draw(const GameTimer& gt)
{
    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	
    // Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	mCommandList->IASetVertexBuffers(0, 1, &mBoxGeo->VertexBufferView());
	mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());
    mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

    mCommandList->DrawIndexedInstanced(
		mBoxGeo->DrawArgs["box"].IndexCount, 
		1, 0, 0, 0);
	
    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
	ThrowIfFailed(mCommandList->Close());
 
    // Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	
	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	FlushCommandQueue();
}

void MyD3DBoxApp::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void MyD3DBoxApp::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void MyD3DBoxApp::OnMouseMove(WPARAM btnState, int x, int y)
{
    if((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

        // Update angles based on input to orbit camera around box.
        mTheta += dx;
        mPhi += dy;

        // Restrict the angle mPhi.
        mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
    }
    else if((btnState & MK_RBUTTON) != 0)
    {
        // Make each pixel correspond to 0.005 unit in the scene.
        float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
        float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

        // Update the camera radius based on input.
        mRadius += dx - dy;

        // Restrict the radius.
        mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}

void MyD3DBoxApp::BuildDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
    cbvHeapDesc.NumDescriptors = 1;
    cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
    ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
        IID_PPV_ARGS(&mCbvHeap)));
}

void MyD3DBoxApp::BuildConstantBuffers()
{
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);

	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();
    // Offset to the ith object constant buffer in the buffer.
    int boxCBufIndex = 0;
	cbAddress += boxCBufIndex*objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	md3dDevice->CreateConstantBufferView(
		&cbvDesc,
		mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void MyD3DBoxApp::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr, 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void MyD3DBoxApp::BuildShadersAndInputLayout()
{
    HRESULT hr = S_OK;
    
	mvsByteCode = d3dUtil::CompileShader(L"MyShader.hlsl", nullptr, "vs_main", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"MyShader.hlsl", nullptr, "ps_main", "ps_5_0");

    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };
}

void MyD3DBoxApp::BuildBoxGeometry()
{
    std::array<Vertex, 8> vertices =
    {
        Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
    };

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	mBoxGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);

	mBoxGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

	mBoxGeo->VertexByteStride = sizeof(Vertex);
	mBoxGeo->VertexBufferByteSize = vbByteSize;
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	mBoxGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mBoxGeo->DrawArgs["box"] = submesh;
}

void MyD3DBoxApp::BuildPSO()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
    ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
    psoDesc.pRootSignature = mRootSignature.Get();
    psoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()), 
		mvsByteCode->GetBufferSize() 
	};
    psoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()), 
		mpsByteCode->GetBufferSize() 
	};
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = mBackBufferFormat;
    psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
    psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
    psoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}
*/