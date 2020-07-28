#include "MyD3DApp.h"

void MyD3DApp::OnResize()
{
	D3DApp::OnResize();

	// ���� ����� �����쿡 �̸� �ݿ��Ѵ�.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_Proj, P);
}

// �� ������ �ڿ��� ������Ʈ�Ѵ�.
void MyD3DApp::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);
	UpdateCamera(gt);

	// ��ȯ������ �ڿ� ������ �迭�� ���� ���ҿ� �����Ѵ�.
	m_nCurFrameResourceIdx = (m_nCurFrameResourceIdx + 1) % m_nMaxFrameResourceIdx;
	m_pCurFrameResource = m_vecFrameResources[m_nCurFrameResourceIdx].get();

	// GPU�� ���� ������ �ڿ��� ��ɵ��� �� ó���ߴ��� Ȯ���Ѵ�.
	// ���� �� ó������ �ʾҴٸ� GPU�� �� ��Ÿ�� ���������� ��ɵ��� ó���� ������ ��ٸ���.
	if ((m_pCurFrameResource->nFence != 0) && (mCommandQueue->GetLastCompletedFence() < m_pCurFrameResource->nFence))
	{

	}
}

void MyD3DApp::Draw(const GameTimer& gt)
{// ��� ��Ͽ� ���õ� �޸��� ��Ȱ���� ���� ��� �Ҵ��ڸ� �缳���Ѵ�.
	// �缳���� GPU�� ���� ��� ��ϵ��� ��� ó���� �ڿ� �Ͼ��.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// ��� ����� ExecuteCommandList�� ���ؼ� ��� ��⿭�� �߰��ߴٸ� ��� �����
	// �缳���ϴ� �͵� �����ϴ�. ��� ����� �缳���ߴٸ� �޸𸮰� ��Ȱ��ȴ�.
	// �� ����!!!! : ���� ���⼭ ��ɾ� �Ҵ��ڿ� ���������� ���� ������Ʈ�� ���� ��Ű�� ������ �ƹ��͵� �� ���!!!
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), m_PSO.Get()));

	// ����Ʈ�� ���� ���簢���� �����Ѵ�.
	// ��� ����� �缳���� ������ �̵鵵 �缳���� �ʿ��ϴ�.
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// �ڿ� �뵵�� ���õ� ���� ������ Direct3D�� �����Ѵ�.
	mCommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		)
	);

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

	// ������ ���� ��ɾ� ����Ʈ�� �����Ѵ�.
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_CbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// ������ ���̺��� ���������ο� ���´�.
	mCommandList->SetGraphicsRootSignature(m_RootSignature.Get());

	// �̹� �׸��� ȣ�⿡ ����� CBV�� �������̴�.
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbv(m_CbvHeap->GetGPUDescriptorHandleForHeapStart());

	// �׷��Ƚ� ��Ʈ ������ ���̺��� �����Ѵ�.
	mCommandList->SetGraphicsRootDescriptorTable(0, m_CbvHeap->GetGPUDescriptorHandleForHeapStart());

	// �ε��� ī��Ʈ��ŭ �׸���.
	mCommandList->DrawIndexedInstanced(
		mBoxGeo->DrawArgs["box"].IndexCount,
		1, 0, 0, 0);

	// �ڿ� �뵵�� ���õ� ���� ���̸� Direct3D�� �˸���.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

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

void MyD3DApp::OnMouseDown(WPARAM btnState, int x, int y)
{
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void MyD3DApp::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

// ���콺 �����ӿ� ���� ��ü�� ���ϰ� �����. 
void MyD3DApp::OnMouseMove(WPARAM btnState, int x, int y)
{
	// ���� ���� ���콺 ��ư�� �ƴ϶��
	if ((btnState & MK_LBUTTON) != 0)
	{
		// ���콺 �� �ȼ� �̵��� 4���� 1���� ������Ų��.
		float dx = XMConvertToRadians(0.25f * static_cast<float> (x - m_LastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float> (y - m_LastMousePos.y));

		// �Է¿� ������ ������ �����ؼ� ī�޶� ���ڸ� �߽����� �����ϰ� �Ѵ�.
		m_Theta += dx;
		m_Phi += dy;

		// m_Phi�� ������ �����Ѵ�
		m_Phi = MathHelper::Clamp(m_Phi, 0.1f, MathHelper::Pi - 0.1f);
	}
	// ���� ���� ���콺���
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// ���콺 �� �ȼ� �̵��� 0.005���� ������Ų��.
		float dx = 0.005f * static_cast<float> (x - m_LastMousePos.x);
		float dy = 0.005f * static_cast<float> (y - m_LastMousePos.y);

		// �Է¿� �����ؼ� ī�޶� �������� �����Ѵ�.
		m_Radius += dx - dy;

		// �������� �����Ѵ�.
		m_Radius = MathHelper::Clamp(m_Radius, 3.0f, 15.f);
	}

	// ���콺 ��ġ�� �����Ѵ�.
	m_LastMousePos.x = x;
	m_LastMousePos.y = y;
}

void MyD3DApp::BuildFrameResources()
{
	for (size_t i = 0; i < m_nMaxFrameResourceIdx; i++)
	{
		m_vecFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(), 1, (UINT)m_vecRenderItems.size()));
	}
}

void MyD3DApp::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesk;	// ��� ���� ������ ����
	cbvHeapDesk.NumDescriptors = 1;

	// SRV : ���̴� �ڿ� ��
	// UAV : �������� ���� ��
	cbvHeapDesk.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesk.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesk.NodeMask = 0;

	// ��� ���� �����ڸ� ��� ������ �����Ѵ�.
	ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&cbvHeapDesk, IID_PPV_ARGS(&mCbvHeap)));
}

void MyD3DApp::BuildConstantBuffers()
{
	// ��ü n���� ��� �ڷḦ ���� ��� ���� (���⼭�� 1���� �㵵�� �ϰڴ�.)
	m_ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_d3dDevice.Get(), 1, true);

	// ��� ������ ����Ʈ ����� �����´�.
	// CalcConstantBufferByteSize�� ���� ����� �ڵ����� �����ִ� ������
	// �ּ� �ϵ���� �Ҵ� ũ��(256����Ʈ)�� ����� �ǰ� �ϴ� ����� �����Ѵ�.
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	// ���� ��ü�� ���� �ּ�(0��° ��� ������ �ּ�)
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_ObjectCB->Resource()->GetGPUVirtualAddress();

	// ���ۿ� ��� i��° ��� ������ ������ (���⼭�� 0)
	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	// ��� ���� �� �����ڸ� �����Ѵ�.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	// ����̽��� ������ ������ �������� ��� ���� �並 �����Ѵ�.
	m_d3dDevice->CreateConstantBufferView(&cbvDesc, mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

// ��Ʈ ����� ������ ���̺��� �����Ѵ�.
void MyD3DApp::BuildRootSignature()
{
	// ��Ʈ ������ �׸��� ȣ�� ���� ���� ���α׷��� �ݵ�� ������ ���������ο� ����� �� �ڿ��� �����̰�
	// �� �ڿ����� � ���̴� �Է� �������͵鿡 ��� �����Ǵ��� �����ϴ� ���̴�.
	// �� �ݵ�� �׸��� ȣ�⿡ ���̴� ���̴���� ȣȯ�Ǿ�� �Ѵ�.
	
	// �� ��Ʈ ������ �پ��� �Ű������� ������, ���⼭�� �ϴ� ������ ���̺��̶��
	// ������ �� �ȿ� �ִ� ���ӵ� �����ڵ��� ������ �����ϴ� ������ �ǽ��ϰڴ�.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// CBV (��� ���� ��) �ϳ��� ��� ������ ���̺��� �����Ѵ�.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_CBV,	// ��Ʈ ������ �Ű�����
		1,		// ���̺��� ������ ����
		0);		// �� ��Ʈ �Ű������� ���� ���̴� �μ����� ���� �������� ��ȣ

	// ��Ʈ ������ �̹� ������ ������ ���̺�� �ʱ�ȭ �Ѵ�.
	slotRootParameter[0].InitAsDescriptorTable(
		1,			// ���� ����
		&cbvTable);

	// ��Ʈ ������ ��Ʈ �Ű��������� �迭�̴�.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// ��� ���� �ϳ��� ������ ������ ������ ����Ű�� ���� �ϳ��� �̷���� ��Ʈ ������ �����Ѵ�.
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	// ���� üũ
	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	// ��Ʈ ���� ����
	ThrowIfFailed(m_d3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_RootSignature)));

	// ��Ʈ ������ ���� �ڿ����� ���� �ʰ� �ڿ����� ���Ǹ� �Ѵ�!
}

// ���̴��� �������ϰ� ������ �� ���̾ƿ��� ��� �Է��� �� ���ΰ��� ���� �޼����̴�.
void MyD3DApp::BuildShadersAndInputLayout()
{
	// �ϴ� ����� OK�� �ʱ�ȭ�Ѵ�.
	HRESULT hr = S_OK;

	// ���̴��� �������Ѵ�.
	// ���� ���� �ð��� �ƴ϶� �̸� ����� �� �������ο��� ���������� �������� �������� �������� �Ѵٸ� 
	// cso ���Ϸ� ����ȴ�. (�� ���� ����� å�� �� ��)
	mvsByteCode = d3dUtil::CompileShader(L"MyShader.hlsl", nullptr, "vs_main", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"MyShader.hlsl", nullptr, "ps_main", "ps_5_0");

	// ���̾ƿ��� �����Ѵ�.
	m_InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// m_InputLayout =
	// {
	// 	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	// 	{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	// };
}

void MyD3DApp::BuildBoxGeometry()
{
	// �̸� ������ �������� ����
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

	// �ε��� �迭�� �����Ѵ�.
	std::array<std::uint16_t, 36> indices =
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

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	// ���� ����ü�� �����Ѵ�.
	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";

	// ��κ�(This interface is used to return data of arbitrary length, MSDN)�� �����Ѵ�. 
	// �� vbByteSize ������ VertexBufferCPU�� �����Ͱ� ��������.
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	// �� ibByteSize ������ IndexBufferCPU�� �����Ͱ� ��������.
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	// ���� ���۸� �����Ѵ�.
	mBoxGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);

	// ���� ���۸� �����Ѵ�.
	mBoxGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(m_d3dDevice.Get(),
		m_CommandList.Get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

	// ���� ���� ������ ü���.
	mBoxGeo->VertexByteStride = sizeof(Vertex);		// ������ �����Ѵ�.
	mBoxGeo->VertexBufferByteSize = vbByteSize;		// ���� ���� ����Ʈ�� ����� �־��ش�.
	// ���� ���� ������ ü���.
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;	// ���� ������ �����Ѵ�.
	mBoxGeo->IndexBufferByteSize = ibByteSize;		// ���� ���� ����Ʈ�� ����� �־��ش�.

	// ���� ���� ����ü�� �����Ѵ�.
	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mBoxGeo->DrawArgs["box"] = submesh;
}

// Pipeline State Object
// ���������� ���¸� ����Ѵ�.
// �� �� ������ȭ�� ���´� ������ ���������ο��� ���α׷����� �Ұ����ϰ� ������ ������ �κ� �� �ϳ��̴�.
void MyD3DApp::BuildPSO()
{
	// �� �κ��� ���α׷����� �ƴ϶� ������ ������ �κ��̴�.
	// �� ���������� �󿡼� ���α׷����� �Ұ��� �κ� �� �ϳ���� ���̴�.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_InputLayout.data(), (UINT)m_InputLayout.size() };
	psoDesc.pRootSignature = m_RootSignature.Get();
	psoDesc.VS = {
		reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
		m_vsByteCode->GetBufferSize()
	};
	psoDesc.PS = {
		reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
		m_psByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	// psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME; // ���̾�������
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;

	ThrowIfFailed(m_d3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PSO)));
}

MyD3DApp::MyD3DApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	mMainWndCaption = L"My First D3D Application";
}

MyD3DApp::~MyD3DApp()
{
}

bool MyD3DApp::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// �ϴ� ��� �κ��� �����Ѵ�.
	ThrowIfFailed(m_CommandList->Reset(m_DirectCmdListAlloc.Get(), nullptr));

	// ���带 ���� ������ �Ѵ�.
	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildBoxGeometry();
	BuildPSO();

	// �ʱ�ȭ ��� �����Ѵ�
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}