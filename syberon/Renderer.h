#pragma once

#include "windows.h"
#include "ddraw.h"
#include "RObject.h"
#include <boost/thread/mutex.hpp>
#include "PeriodCorrector.h"

enum RendererState {
	OK,
	DirectDrawCreateFailed,
	CreatePrimarySurfaceFailed,
	CreateBackSurfaceFailed
};

enum RendererLayer {
	Map = 0,
	GameObjects,
	Interface
};

class Renderer {

private:
	HWND _hwnd;

	bool _arrowNotSet;
	RObjectSet *_layers[sizeof(RendererLayer)];


	LPDIRECTDRAWSURFACE7 _dds_Primary;


	int _backSurfaceCount, _backSurfaceIndex;
	LPDIRECTDRAWSURFACE7 *_dds_Back;

	LPDIRECTDRAWCLIPPER _ddc_Clipper;

	int _new_width, _new_height;
	bool _thread_resize;

	RendererState _state;
	DrawMachine *_drawMachine;

	int _fps;
	volatile bool _fullscreen, _hided, _useBackBuffer, _inSwitch;

	
	DWORD _windowStyle;
	WINDOWPLACEMENT _windowPos;
	int _fullscreenW, _fullscreenH;



public:

	int _epos, _bcount;
	LPDIRECTDRAWSURFACE7 *_dds_backFullScreen;

	int _width, _height, _bpp, _frameTime;
	
	DDSURFACEDESC2 _monitorModeInfo;
	boost::mutex _robjectMutex, _threadMutex;
	PeriodCorrector *pc;
	LPDIRECTDRAW7 _dd;


	Renderer(HWND hwnd, bool runInFullscreen, int mw, int mh);

	bool createBackSurface();
	bool createSurfaces();
	DWORD threadProc();

	void checkSurfaces();
	void draw();
	void flip();

	LRESULT onWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void WM_Size(int width, int height);
	void WM_MouseMove(int x, int y);

	void add(RObject *object, int layer);
	void del(RObject *object, int layer);


	int getFPS();
	void setFPS(int fps);

	void lockObjectList();
	void unlockObjectList();

	void saveWindowedState();
	void releaseAllSurfaces();
	void switchToFullscreen();

	void enableFullScreen(int w, int h);
	void disableFullScreen();
};