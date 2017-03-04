#include <windows.h>

#include "syberon\Utils.h"

#include "syberon\Logger.h"
#include "syberon\LuaScript.h"

#include "syberon\Files.h"
// #include "steam_api.h"

LuaScript *coreScript = NULL;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	if (coreScript) {
		if (coreScript->executeObjectMethod("game", "_onWindowMessage", 
			DL->add((_uint)message)->add((_uint)lParam)->add((_uint)LOWORD(lParam))->add((_uint)HIWORD(lParam))->add((_uint)wParam))) {
			lprint(coreScript->getError());
		}
		else {
			bool r = coreScript->getReturnValue()->tobool();
			if (r) {
				return 0;
			}
		}
	}

	switch (message) {

	case WM_CHAR:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_ERASEBKGND:
		return TRUE;

		
	case WM_DESTROY:
		PostQuitMessage(0);
		exit(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {

	timeBeginPeriod(1);

#ifdef _DEBUG_LOGGER
	Logger::setupLogFilepath(0, "debug.log");
	Logger::setThreadName("main");
#endif

	lprint("\n\nstarted");

#ifdef _DEBUG
	if (0) {
		Files_openPack("data.pack");
		Files_loadPack("lua.pack");
	}
#else
	Files_openPack("data.pack");
	Files_loadPack("lua.pack");
#endif

	/*
	if (SteamAPI_Init()) {
		lprint("SteamAPI_Init() ok ");
	}
	*/

	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = NULL; // LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"class";
	wcex.hIconSm = NULL;

	RegisterClassExW(&wcex);

	HWND hWnd = CreateWindowW(L"class", L"syberon", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) {
		return FALSE;
	}

	coreScript = new LuaScript();

	if (coreScript->executeFile("start.lua")) {
		lprint(coreScript->getError());
		exit(1);
		return 1;
	}

	if (coreScript->executeObjectMethod("game", "onStart", DL->add(hWnd))) {
		lprint(coreScript->getError());
		exit(1);
		return 1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, hWnd, 0, 0)) {
		// TranslateMessage(&msg);
		DispatchMessage(&msg);		
	}

	return (int)msg.wParam;
}