#include "MyD3DApp.h"

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

		if (!app.Initialize())
		{
			return 0;
		} 
		return app.Run();
	}
	catch (const DxException& error)
	{
		MessageBox(nullptr, error.ToString().c_str(), L"DirectX 12 Initialize error", MB_OK);
		return 0;
	}
}