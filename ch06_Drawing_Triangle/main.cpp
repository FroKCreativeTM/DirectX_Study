#include "MyD3DBoxApp.h"

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
		MyD3DBoxApp app(hInstance);
		if (!app.Initialize())
			return 0;

		return app.Run();
	}
	catch (DxException& err)
	{
		MessageBox(nullptr, err.ToString().c_str(), L"app init error", MB_OK);
		return 0;
	}
}