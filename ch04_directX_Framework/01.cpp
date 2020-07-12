#include "../Common/d3dApp.h"
#include <DirectXColors.h>

using namespace DirectX;

class MyD3DApp : public D3DApp
{
public : 
	MyD3DApp(HINSTANCE hInstance) : D3DApp(hInstance) 
	{
		mMainWndCaption = L"My First D3D Application";
	}
	~MyD3DApp() {}
	inline bool Initialize() override 
	{
		if (D3DApp::Initialize())
			return false;
		return true;
	}

private : 
	inline void OnResize() override 
	{
		D3DApp::OnResize();
	}
	inline void Update(const GameTimer& gt) override
	{

	}
	inline void Draw(const GameTimer& gt) override
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
};

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PSTR strCmdLine,
	int nCmdLine)
{
#if defined (DEBUG) | defined (_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		MyD3DApp app(hInstance);
		if (app.Initialize())
			return 0;

		return app.Run();
	}
	catch (DxException& err)
	{
		MessageBox(nullptr, err.ToString().c_str(), L"app init error", MB_OK);
		return 0;
	}
}