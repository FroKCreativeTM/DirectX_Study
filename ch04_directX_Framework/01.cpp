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
	bool Initialize() override 
	{

	}

	void log()
	{
		UINT i = 0;
		IDXGIAdapter* adapter = nullptr;
		std::vector<IDXGIAdapter*> adapterList;
		while (mdxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);

			std::wstring text = L"***Adapter: ";
			text += desc.Description;
			text += L"\n";

			OutputDebugString(text.c_str());

			adapterList.push_back(adapter);

			++i;
		}

		for (size_t i = 0; i < adapterList.size(); ++i)
		{
			LogAdapterOutputs(adapterList[i]);
			ReleaseCom(adapterList[i]);
		}
	}

private : 
	void OnResize() override {}
	void Update(const GameTimer& gt) override {}
	void Draw(const GameTimer& gt) override
	{

	}
};

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	PSTR strCmdLine,
	int nCmdLine)
{
	MyD3DApp app(hInstance);
	if (app.Initialize())
	{
		app.log();
		return app.Run();
	}

	MessageBox(nullptr, L"Application initializion is failed", L"init fail", MB_OK);
	return 0;
}