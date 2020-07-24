#include "CFrustum.h"

void CFrustum::OnResize()
{
	D3DApp::OnResize();

	// ���� ����� �����쿡 �̸� �ݿ��Ѵ�.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

// ���콺 �Է��� ������ ��ǥ�谡 ���ŵǰ� �غ���.
void CFrustum::Update(const GameTimer& gt)
{
	// ���� ��ǥ�� ��ī��Ʈ ��ǥ�� ��ȯ����
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float y = mRadius * cosf(mPhi);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);

	// �þ� ����� �����Ѵ�.
	XMVECTOR position = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();				// direction
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// �� ��� ����
	XMMATRIX view = XMMatrixLookAtLH(position, target, up);
	XMStoreFloat4x4(&mView, view);	// ��� ��Ŀ� �����Ѵ�.

	// ����, ����, WVP�� ���� �����Ѵ�.
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX projection = XMLoadFloat4x4(&mProj);
	XMMATRIX WVP = world * view * projection;

	// �ֽ��� MVP ��ķ� ��� ���۸� �����Ѵ�.
	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(WVP));
	mObjectCB->CopyData(0, objConstants);
}

void CFrustum::Draw(const GameTimer& gt)
{
	// ��� ��Ͽ� ���õ� �޸��� ��Ȱ���� ���� ��� �Ҵ��ڸ� �缳���Ѵ�.
	// �缳���� GPU�� ���� ��� ��ϵ��� ��� ó���� �ڿ� �Ͼ��.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// ��� ����� ExecuteCommandList�� ���ؼ� ��� ��⿭�� �߰��ߴٸ� ��� �����
	// �缳���ϴ� �͵� �����ϴ�. ��� ����� �缳���ߴٸ� �޸𸮰� ��Ȱ��ȴ�.
	// �� ����!!!! : ���� ���⼭ ��ɾ� �Ҵ��ڿ� ���������� ���� ������Ʈ�� ���� ��Ű�� ������ �ƹ��͵� �� ���!!!
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

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
	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// ������ ���̺��� ���������ο� ���´�.
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	// �̹� �׸��� ȣ�⿡ ����� CBV�� �������̴�.
	CD3DX12_GPU_DESCRIPTOR_HANDLE cbv(mCbvHeap->GetGPUDescriptorHandleForHeapStart());

	// �ڽ� ������Ʈ�� ���� ���� ����ü�� ���۸� �����Ѵ�.
	mCommandList->IASetVertexBuffers(0, 1, &mFrustum->VertexBufferView());
	mCommandList->IASetIndexBuffer(&mFrustum->IndexBufferView());
	mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// �׷��Ƚ� ��Ʈ ������ ���̺��� �����Ѵ�.
	mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

	// �ε��� ī��Ʈ��ŭ �׸���.
	mCommandList->DrawIndexedInstanced(
		mFrustum->DrawArgs["Frustum"].IndexCount,
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

void CFrustum::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void CFrustum::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

// ���콺 �����ӿ� ���� ��ü�� ���ϰ� �����. 
void CFrustum::OnMouseMove(WPARAM btnState, int x, int y)
{
	// ���� ���� ���콺 ��ư�� �ƴ϶��
	if ((btnState & MK_LBUTTON) != 0)
	{
		// ���콺 �� �ȼ� �̵��� 4���� 1���� ������Ų��.
		float dx = XMConvertToRadians(0.25f * static_cast<float> (x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float> (y - mLastMousePos.y));

		// �Է¿� ������ ������ �����ؼ� ī�޶� ���ڸ� �߽����� �����ϰ� �Ѵ�.
		mTheta += dx;
		mPhi += dy;

		// mPhi�� ������ �����Ѵ�
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	// ���� ���� ���콺���
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// ���콺 �� �ȼ� �̵��� 0.005���� ������Ų��.
		float dx = 0.005f * static_cast<float> (x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float> (y - mLastMousePos.y);

		// �Է¿� �����ؼ� ī�޶� �������� �����Ѵ�.
		mRadius += dx - dy;

		// �������� �����Ѵ�.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.f);
	}

	// ���콺 ��ġ�� �����Ѵ�.
	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void CFrustum::BuildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesk;	// ��� ���� ������ ����
	cbvHeapDesk.NumDescriptors = 1;
	// SRV : ���̴� �ڿ� ��
	// UAV : �������� ���� ��
	cbvHeapDesk.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesk.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesk.NodeMask = 0;

	// ��� ���� �����ڸ� ��� ������ �����Ѵ�.
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesk, IID_PPV_ARGS(&mCbvHeap)));
}

void CFrustum::BuildConstantBuffers()
{
	// ��ü n���� ��� �ڷḦ ���� ��� ���� (���⼭�� 1���� �㵵�� �ϰڴ�.)
	mObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);

	// ��� ������ ����Ʈ ����� �����´�.
	// CalcConstantBufferByteSize�� ���� ����� �ڵ����� �����ִ� ������
	// �ּ� �ϵ���� �Ҵ� ũ��(256����Ʈ)�� ����� �ǰ� �ϴ� ����� �����Ѵ�.
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	// ���� ��ü�� ���� �ּ�(0��° ��� ������ �ּ�)
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mObjectCB->Resource()->GetGPUVirtualAddress();

	// ���ۿ� ��� i��° ��� ������ ������ (���⼭�� 0)
	int FrustumCBufIndex = 0;
	cbAddress += FrustumCBufIndex * objCBByteSize;

	// ��� ���� �� �����ڸ� �����Ѵ�.
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

	// ����̽��� ������ ������ �������� ��� ���� �並 �����Ѵ�.
	md3dDevice->CreateConstantBufferView(&cbvDesc, mCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

// ��Ʈ ����� ������ ���̺��� �����Ѵ�.
void CFrustum::BuildRootSignature()
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
	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));

	// ��Ʈ ������ ���� �ڿ����� ���� �ʰ� �ڿ����� ���Ǹ� �Ѵ�!
}

// ���̴��� �������ϰ� ������ �� ���̾ƿ��� ��� �Է��� �� ���ΰ��� ���� �޼����̴�.
void CFrustum::BuildShadersAndInputLayout()
{
	// �ϴ� ����� OK�� �ʱ�ȭ�Ѵ�.
	HRESULT hr = S_OK;

	// ���̴��� �������Ѵ�.
	// ���� ���� �ð��� �ƴ϶� �̸� ����� �� �������ο��� ���������� �������� �������� �������� �Ѵٸ� 
	// cso ���Ϸ� ����ȴ�. (�� ���� ����� å�� �� ��)
	mvsByteCode = d3dUtil::CompileShader(L"MyShader.hlsl", nullptr, "vs_main", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"MyShader.hlsl", nullptr, "ps_main", "ps_5_0");

	// ���̾ƿ��� �����Ѵ�.
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void CFrustum::BuildFrustumGeometry()
{
	// �̸� ������ �������� ����
	std::array<Vertex, 5> vertices =
	{
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::LightGoldenrodYellow) }),
		Vertex({ XMFLOAT3(+0.0f, +1.0f, +0.0f), XMFLOAT4(Colors::Red) })
	};

	// �ε��� �迭�� �����Ѵ�.
	std::array<std::uint16_t, 18> indices =
	{
		0, 2, 1,
		1, 2, 3,
		4, 0, 1,
		4, 1, 3,
		4, 3, 2,
		4, 2, 0
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	// ���� ����ü�� �����Ѵ�.
	mFrustum = std::make_unique<MeshGeometry>();
	mFrustum->Name = "FrustumGeo";

	// ��κ�(This interface is used to return data of arbitrary length, MSDN)�� �����Ѵ�. 
	// �� vbByteSize ������ VertexBufferCPU�� �����Ͱ� ��������.
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mFrustum->VertexBufferCPU));
	CopyMemory(mFrustum->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	// �� ibByteSize ������ IndexBufferCPU�� �����Ͱ� ��������.
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mFrustum->IndexBufferCPU));
	CopyMemory(mFrustum->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	// ���� ���۸� �����Ѵ�.
	mFrustum->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, mFrustum->VertexBufferUploader);

	// ���� ���۸� �����Ѵ�.
	mFrustum->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mFrustum->IndexBufferUploader);

	// ���� ���� ������ ü���.
	mFrustum->VertexByteStride = sizeof(Vertex);		// ������ �����Ѵ�.
	mFrustum->VertexBufferByteSize = vbByteSize;		// ���� ���� ����Ʈ�� ����� �־��ش�.
	// ���� ���� ������ ü���.
	mFrustum->IndexFormat = DXGI_FORMAT_R16_UINT;	// ���� ������ �����Ѵ�.
	mFrustum->IndexBufferByteSize = ibByteSize;		// ���� ���� ����Ʈ�� ����� �־��ش�.

	// ���� ���� ����ü�� �����Ѵ�.
	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mFrustum->DrawArgs["Frustum"] = submesh;
}

// Pipeline State Object
// ���������� ���¸� ����Ѵ�.
// �� �� ������ȭ�� ���´� ������ ���������ο��� ���α׷����� �Ұ����ϰ� ������ ������ �κ� �� �ϳ��̴�.
void CFrustum::BuildPSO()
{
	// �� �κ��� ���α׷����� �ƴ϶� ������ ������ �κ��̴�.
	// �� ���������� �󿡼� ���α׷����� �Ұ��� �κ� �� �ϳ���� ���̴�.
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

CFrustum::CFrustum(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	mMainWndCaption = L"My First D3D Application";
}

CFrustum::~CFrustum()
{
}

bool CFrustum::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// �ϴ� ��� �κ��� �����Ѵ�.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// ���带 ���� ������ �Ѵ�.
	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();
	BuildShadersAndInputLayout();
	BuildFrustumGeometry();
	BuildPSO();

	// �ʱ�ȭ ��� �����Ѵ�
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}