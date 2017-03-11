#include <windows.h>
#include <eh.h>
#include <Psapi.h>

#include "syberon\Utils.h"

#include "syberon\Logger.h"
#include "syberon\LuaScript.h"

#include "syberon\Files.h"
// #include "steam_api.h"
#include "syberon\Map.h"

const char* opDescription(const ULONG opcode) {
	switch (opcode) {
	case 0: return "read";
	case 1: return "write";
	case 8: return "user-mode data execution prevention (DEP) violation";
	default: return "unknown";
	}
}

const char* seDescription(const int& code) {
	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION:         return "EXCEPTION_ACCESS_VIOLATION";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
	case EXCEPTION_BREAKPOINT:               return "EXCEPTION_BREAKPOINT";
	case EXCEPTION_DATATYPE_MISALIGNMENT:    return "EXCEPTION_DATATYPE_MISALIGNMENT";
	case EXCEPTION_FLT_DENORMAL_OPERAND:     return "EXCEPTION_FLT_DENORMAL_OPERAND";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
	case EXCEPTION_FLT_INEXACT_RESULT:       return "EXCEPTION_FLT_INEXACT_RESULT";
	case EXCEPTION_FLT_INVALID_OPERATION:    return "EXCEPTION_FLT_INVALID_OPERATION";
	case EXCEPTION_FLT_OVERFLOW:             return "EXCEPTION_FLT_OVERFLOW";
	case EXCEPTION_FLT_STACK_CHECK:          return "EXCEPTION_FLT_STACK_CHECK";
	case EXCEPTION_FLT_UNDERFLOW:            return "EXCEPTION_FLT_UNDERFLOW";
	case EXCEPTION_ILLEGAL_INSTRUCTION:      return "EXCEPTION_ILLEGAL_INSTRUCTION";
	case EXCEPTION_IN_PAGE_ERROR:            return "EXCEPTION_IN_PAGE_ERROR";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "EXCEPTION_INT_DIVIDE_BY_ZERO";
	case EXCEPTION_INT_OVERFLOW:             return "EXCEPTION_INT_OVERFLOW";
	case EXCEPTION_INVALID_DISPOSITION:      return "EXCEPTION_INVALID_DISPOSITION";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
	case EXCEPTION_PRIV_INSTRUCTION:         return "EXCEPTION_PRIV_INSTRUCTION";
	case EXCEPTION_SINGLE_STEP:              return "EXCEPTION_SINGLE_STEP";
	case EXCEPTION_STACK_OVERFLOW:           return "EXCEPTION_STACK_OVERFLOW";
	default: return "UNKNOWN EXCEPTION";
	}
}

void MiniDumpFunction(unsigned int nExceptionCode, EXCEPTION_POINTERS *pException) {
	// lprint("Exception:");

	HMODULE hm;
	::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast<LPCTSTR>(pException->ExceptionRecord->ExceptionAddress), &hm);
	MODULEINFO mi;
	::GetModuleInformation(::GetCurrentProcess(), hm, &mi, sizeof(mi));
	char fn[MAX_PATH];
	::GetModuleFileNameExA(::GetCurrentProcess(), hm, fn, MAX_PATH);	

	bool has_exception_code = true;
	std::ostringstream oss;
	/*
	oss << (has_exception_code ? seDescription(nExceptionCode) : "") << " at address 0x" << std::hex << pException->ExceptionRecord->ExceptionAddress << std::dec
		<< " inside " << fn << " loaded at base address 0x" << std::hex << mi.lpBaseOfDll << "\n";
	*/
	oss << (has_exception_code ? seDescription(nExceptionCode) : "") << "\n";

	if (has_exception_code && (
		nExceptionCode == EXCEPTION_ACCESS_VIOLATION ||
		nExceptionCode == EXCEPTION_IN_PAGE_ERROR)) {
		oss << "Invalid operation: " << opDescription(pException->ExceptionRecord->ExceptionInformation[0]) << " at address 0x" << std::hex << pException->ExceptionRecord->ExceptionInformation[1] << std::dec;
	}

	if (has_exception_code && nExceptionCode == EXCEPTION_IN_PAGE_ERROR) {
		oss << "Underlying NTSTATUS code that resulted in the exception " << pException->ExceptionRecord->ExceptionInformation[2];
	}

	lprint(oss.str());
	// exit(1);
}

LuaScript *coreScript = NULL;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	_set_se_translator(MiniDumpFunction);
	try  // this try block allows the SE translator to work
	{

	if (coreScript) {
		if (coreScript->executeObjectMethod("game", "_onWindowMessage", 
			DL->add((_uint)message)->add((_uint)lParam)->add((_uint)LOWORD(lParam))->add((_uint)HIWORD(lParam))->add((_uint)wParam)->add((_int)GET_WHEEL_DELTA_WPARAM(wParam)))) {
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
	case WM_CLOSE:

		if (coreScript) {
			if (coreScript->executeObjectMethod("game", "onQuit")) {
				lprint(coreScript->getError());
			}
		}

		PostQuitMessage(0);
		exit(1);
		// exit(0);

		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
	}
	catch (...)
	{
		return -1;
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {

	_set_se_translator(MiniDumpFunction);
	try  // this try block allows the SE translator to work
	{

	timeBeginPeriod(1);

#ifdef _DEBUG_LOGGER
	Logger::setupLogFilepath(0, "debug.log");
	Logger::setupLogFilepath(1, "error.log");
	Logger::setThreadName("main");
#endif

	_lprint("");

	/*
	// 0x80000000000000
	unsigned long long i1 = 36028797018963840, i2 = ((long long)-512) + 0x80000000000000;
	if (i1 > i2) {
		lprint("ttt");
	}
	*/



	/*
	Files_openPack("data.pack");
	Files_loadPack("lua.pack");
	*/

#ifdef _DEBUG
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
	catch (...)
	{
		return -1;
	}
}