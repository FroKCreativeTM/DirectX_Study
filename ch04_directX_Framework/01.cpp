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