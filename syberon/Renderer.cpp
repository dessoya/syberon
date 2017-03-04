#include "Renderer.h"
#include "PeriodCorrector.h"
#include "Logger.h"
#include "boost\lexical_cast.hpp"
#include "Image.h"
#include "Utils.h"

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DEFINE_GUID(IID_IDirectDraw7, 0x15e65ec0, 0x3b9c, 0x11d2, 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b);

char *DDErrorString(HRESULT hr)
{
	switch (hr)
	{
	case DDERR_ALREADYINITIALIZED:           return "DDERR_ALREADYINITIALIZED";
	case DDERR_CANNOTATTACHSURFACE:          return "DDERR_CANNOTATTACHSURFACE";
	case DDERR_CANNOTDETACHSURFACE:          return "DDERR_CANNOTDETACHSURFACE";
	case DDERR_CURRENTLYNOTAVAIL:            return "DDERR_CURRENTLYNOTAVAIL";
	case DDERR_EXCEPTION:                    return "DDERR_EXCEPTION";
	case DDERR_GENERIC:                      return "DDERR_GENERIC";
	case DDERR_HEIGHTALIGN:                  return "DDERR_HEIGHTALIGN";
	case DDERR_INCOMPATIBLEPRIMARY:          return "DDERR_INCOMPATIBLEPRIMARY";
	case DDERR_INVALIDCAPS:                  return "DDERR_INVALIDCAPS";
	case DDERR_INVALIDCLIPLIST:              return "DDERR_INVALIDCLIPLIST";
	case DDERR_INVALIDMODE:                  return "DDERR_INVALIDMODE";
	case DDERR_INVALIDOBJECT:                return "DDERR_INVALIDOBJECT";
	case DDERR_INVALIDPARAMS:                return "DDERR_INVALIDPARAMS";
	case DDERR_INVALIDPIXELFORMAT:           return "DDERR_INVALIDPIXELFORMAT";
	case DDERR_INVALIDRECT:                  return "DDERR_INVALIDRECT";
	case DDERR_LOCKEDSURFACES:               return "DDERR_LOCKEDSURFACES";
	case DDERR_NO3D:                         return "DDERR_NO3D";
	case DDERR_NOALPHAHW:                    return "DDERR_NOALPHAHW";
	case DDERR_NOCLIPLIST:                   return "DDERR_NOCLIPLIST";
	case DDERR_NOCOLORCONVHW:                return "DDERR_NOCOLORCONVHW";
	case DDERR_NOCOOPERATIVELEVELSET:        return "DDERR_NOCOOPERATIVELEVELSET";
	case DDERR_NOCOLORKEY:                   return "DDERR_NOCOLORKEY";
	case DDERR_NOCOLORKEYHW:                 return "DDERR_NOCOLORKEYHW";
	case DDERR_NODIRECTDRAWSUPPORT:          return "DDERR_NODIRECTDRAWSUPPORT";
	case DDERR_NOEXCLUSIVEMODE:              return "DDERR_NOEXCLUSIVEMODE";
	case DDERR_NOFLIPHW:                     return "DDERR_NOFLIPHW";
	case DDERR_NOGDI:                        return "DDERR_NOGDI";
	case DDERR_NOMIRRORHW:                   return "DDERR_NOMIRRORHW";
	case DDERR_NOTFOUND:                     return "DDERR_NOTFOUND";
	case DDERR_NOOVERLAYHW:                  return "DDERR_NOOVERLAYHW";
	case DDERR_NORASTEROPHW:                 return "DDERR_NORASTEROPHW";
	case DDERR_NOROTATIONHW:                 return "DDERR_NOROTATIONHW";
	case DDERR_NOSTRETCHHW:                  return "DDERR_NOSTRETCHHW";
	case DDERR_NOT4BITCOLOR:                 return "DDERR_NOT4BITCOLOR";
	case DDERR_NOT4BITCOLORINDEX:            return "DDERR_NOT4BITCOLORINDEX";
	case DDERR_NOT8BITCOLOR:                 return "DDERR_NOT8BITCOLOR";
	case DDERR_NOTEXTUREHW:                  return "DDERR_NOTEXTUREHW";
	case DDERR_NOVSYNCHW:                    return "DDERR_NOVSYNCHW";
	case DDERR_NOZBUFFERHW:                  return "DDERR_NOZBUFFERHW";
	case DDERR_NOZOVERLAYHW:                 return "DDERR_NOZOVERLAYHW";
	case DDERR_OUTOFCAPS:                    return "DDERR_OUTOFCAPS";
	case DDERR_OUTOFMEMORY:                  return "DDERR_OUTOFMEMORY";
	case DDERR_OUTOFVIDEOMEMORY:             return "DDERR_OUTOFVIDEOMEMORY";
	case DDERR_OVERLAYCANTCLIP:              return "DDERR_OVERLAYCANTCLIP";
	case DDERR_OVERLAYCOLORKEYONLYONEACTIVE: return "DDERR_OVERLAYCOLORKEYONLYONEACTIVE";
	case DDERR_PALETTEBUSY:                  return "DDERR_PALETTEBUSY";
	case DDERR_COLORKEYNOTSET:               return "DDERR_COLORKEYNOTSET";
	case DDERR_SURFACEALREADYATTACHED:       return "DDERR_SURFACEALREADYATTACHED";
	case DDERR_SURFACEALREADYDEPENDENT:      return "DDERR_SURFACEALREADYDEPENDENT";
	case DDERR_SURFACEBUSY:                  return "DDERR_SURFACEBUSY";
	case DDERR_CANTLOCKSURFACE:              return "DDERR_CANTLOCKSURFACE";
	case DDERR_SURFACEISOBSCURED:            return "DDERR_SURFACEISOBSCURED";
	case DDERR_SURFACELOST:                  return "DDERR_SURFACELOST";
	case DDERR_SURFACENOTATTACHED:           return "DDERR_SURFACENOTATTACHED";
	case DDERR_TOOBIGHEIGHT:                 return "DDERR_TOOBIGHEIGHT";
	case DDERR_TOOBIGSIZE:                   return "DDERR_TOOBIGSIZE";
	case DDERR_TOOBIGWIDTH:                  return "DDERR_TOOBIGWIDTH";
	case DDERR_UNSUPPORTED:                  return "DDERR_UNSUPPORTED";
	case DDERR_UNSUPPORTEDFORMAT:            return "DDERR_UNSUPPORTEDFORMAT";
	case DDERR_UNSUPPORTEDMASK:              return "DDERR_UNSUPPORTEDMASK";
	case DDERR_VERTICALBLANKINPROGRESS:      return "DDERR_VERTICALBLANKINPROGRESS";
	case DDERR_WASSTILLDRAWING:              return "DDERR_WASSTILLDRAWING";
	case DDERR_XALIGN:                       return "DDERR_XALIGN";
	case DDERR_INVALIDDIRECTDRAWGUID:        return "DDERR_INVALIDDIRECTDRAWGUID";
	case DDERR_DIRECTDRAWALREADYCREATED:     return "DDERR_DIRECTDRAWALREADYCREATED";
	case DDERR_NODIRECTDRAWHW:               return "DDERR_NODIRECTDRAWHW";
	case DDERR_PRIMARYSURFACEALREADYEXISTS:  return "DDERR_PRIMARYSURFACEALREADYEXISTS";
	case DDERR_NOEMULATION:                  return "DDERR_NOEMULATION";
	case DDERR_REGIONTOOSMALL:               return "DDERR_REGIONTOOSMALL";
	case DDERR_CLIPPERISUSINGHWND:           return "DDERR_CLIPPERISUSINGHWND";
	case DDERR_NOCLIPPERATTACHED:            return "DDERR_NOCLIPPERATTACHED";
	case DDERR_NOHWND:                       return "DDERR_NOHWND";
	case DDERR_HWNDSUBCLASSED:               return "DDERR_HWNDSUBCLASSED";
	case DDERR_HWNDALREADYSET:               return "DDERR_HWNDALREADYSET";
	case DDERR_NOPALETTEATTACHED:            return "DDERR_NOPALETTEATTACHED";
	case DDERR_NOPALETTEHW:                  return "DDERR_NOPALETTEHW";
	case DDERR_BLTFASTCANTCLIP:              return "DDERR_BLTFASTCANTCLIP";
	case DDERR_NOBLTHW:                      return "DDERR_NOBLTHW";
	case DDERR_NODDROPSHW:                   return "DDERR_NODDROPSHW";
	case DDERR_OVERLAYNOTVISIBLE:            return "DDERR_OVERLAYNOTVISIBLE";
	case DDERR_NOOVERLAYDEST:                return "DDERR_NOOVERLAYDEST";
	case DDERR_INVALIDPOSITION:              return "DDERR_INVALIDPOSITION";
	case DDERR_NOTAOVERLAYSURFACE:           return "DDERR_NOTAOVERLAYSURFACE";
	case DDERR_EXCLUSIVEMODEALREADYSET:      return "DDERR_EXCLUSIVEMODEALREADYSET";
	case DDERR_NOTFLIPPABLE:                 return "DDERR_NOTFLIPPABLE";
	case DDERR_CANTDUPLICATE:                return "DDERR_CANTDUPLICATE";
	case DDERR_NOTLOCKED:                    return "DDERR_NOTLOCKED";
	case DDERR_CANTCREATEDC:                 return "DDERR_CANTCREATEDC";
	case DDERR_NODC:                         return "DDERR_NODC";
	case DDERR_WRONGMODE:                    return "DDERR_WRONGMODE";
	case DDERR_IMPLICITLYCREATED:            return "DDERR_IMPLICITLYCREATED";
	case DDERR_NOTPALETTIZED:                return "DDERR_NOTPALETTIZED";
	case DDERR_UNSUPPORTEDMODE:              return "DDERR_UNSUPPORTEDMODE";
	case DDERR_NOMIPMAPHW:                   return "DDERR_NOMIPMAPHW";
	case DDERR_INVALIDSURFACETYPE:           return "DDERR_INVALIDSURFACETYPE";
	case DDERR_DCALREADYCREATED:             return "DDERR_DCALREADYCREATED";
	case DDERR_CANTPAGELOCK:                 return "DDERR_CANTPAGELOCK";
	case DDERR_CANTPAGEUNLOCK:               return "DDERR_CANTPAGEUNLOCK";
	case DDERR_NOTPAGELOCKED:                return "DDERR_NOTPAGELOCKED";
	case DDERR_NOTINITIALIZED:               return "DDERR_NOTINITIALIZED";
	}
	return "Unknown Error";
}


DWORD WINAPI MyThreadFunction(LPVOID lpParam) {
	Renderer *renderer = (Renderer *)lpParam;
	return renderer->threadProc();
}

void Renderer::saveWindowedState() {

	_windowStyle = GetWindowLong(_hwnd, GWL_STYLE);
	_windowPos = { sizeof(_windowPos) };
	GetWindowPlacement(_hwnd, &_windowPos);

	//	SetWindowLong(_hwnd, GWL_STYLE, _windowStyle & ~WS_OVERLAPPEDWINDOW);

	memset(&_monitorModeInfo, 0, sizeof(_monitorModeInfo));
	_monitorModeInfo.dwSize = sizeof(_monitorModeInfo);
	HRESULT hr = _dd->GetDisplayMode(&_monitorModeInfo);
	if (FAILED(hr)) {
		lprint(std::string("Error: GetDisplayMode ") + DDErrorString(hr));
		return;
	}

}

void Renderer::releaseAllSurfaces() {

	lprint("Renderer::releaseAllSurfaces");

	if (_dds_backFullScreen) {
		_dds_backFullScreen->Release();
		_dds_backFullScreen = NULL;
	}

	if (_ddc_Clipper) {
		_ddc_Clipper->Release();
		_ddc_Clipper = NULL;
	}

	if (_dds_Primary) {
		_dds_Primary->Release();
		_dds_Primary = NULL;
	}

	for (int i = 0; i < _backSurfaceCount; i++) {
		if (_dds_Back[i]) {
			_dds_Back[i]->Release();
			_dds_Back[i] = NULL;
		}
	}
}

void Renderer::switchToFullscreen() {

	DDSURFACEDESC2 _curmonitorModeInfo;
	memset(&_curmonitorModeInfo, 0, sizeof(_curmonitorModeInfo));
	_curmonitorModeInfo.dwSize = sizeof(_curmonitorModeInfo);
	HRESULT hr = _dd->GetDisplayMode(&_curmonitorModeInfo);
	if (FAILED(hr)) {
		lprint(std::string("Error: GetDisplayMode ") + DDErrorString(hr));
	}


	if (_fullscreenW == _curmonitorModeInfo.dwWidth && _fullscreenH == _curmonitorModeInfo.dwHeight) {

		_useBackBuffer = false;
		createSurfaces();

	}
	else {

		_useBackBuffer = true;

		// lprint(std::string("_thread_enableFullScreen SetDisplayMode"));
		hr = _dd->SetDisplayMode(_fullscreenW, _fullscreenH, 32, 0, 0);
		if (FAILED(hr)) {
			lprint(std::string("Error: SetDisplayMode ") + DDErrorString(hr));
		}


		lprint("Renderer::switchToFullscreen createSurface");

		DDSURFACEDESC2 ddsd;
		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_SYSTEMMEMORY;
		// ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.ddsCaps.dwCaps2 = DDCAPS2_FLIPNOVSYNC;
		ddsd.dwBackBufferCount = 1;

		hr = _dd->CreateSurface(&ddsd, &_dds_Primary, NULL);
		if (FAILED(hr)) {
			lprint(std::string("Error: CreateSurface ") + DDErrorString(hr));
		}

		DDSCAPS2 surfcaps;
		memset(&surfcaps, 0, sizeof(surfcaps));
		surfcaps.dwCaps = DDSCAPS_BACKBUFFER;
		// lprint(std::string("_thread_enableFullScreen GetAttachedSurface"));
		hr = _dds_Primary->GetAttachedSurface(&surfcaps, &_dds_backFullScreen);
		if (hr != DD_OK) {
			_dds_backFullScreen = NULL;
			lprint(std::string("Error: GetAttachedSurface ") + DDErrorString(hr));
		}

	}
	PostMessage(_hwnd, 0x400, 0, 0);
}


void Renderer::enableFullScreen(int w, int h) {

	boost::unique_lock<boost::mutex> scoped_lock(_threadMutex);

	HRESULT hr;

	// lprint(std::string("_thread_enableFullScreen ") + inttostr(w) + "x" + inttostr(h));


	if (!_fullscreen) {
		saveWindowedState();
	}

	_fullscreen = true;

	SetWindowLong(_hwnd, GWL_STYLE, _windowStyle & ~WS_OVERLAPPEDWINDOW);

	hr = _dd->SetCooperativeLevel(_hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_MULTITHREADED);
	if (FAILED(hr)) {
		lprint("Error: SetCooperativeLevel");
		return;
	}

	releaseAllSurfaces();

	// release surfaces
	_width = _fullscreenW = w;
	_height = _fullscreenH = h;

	switchToFullscreen();

	// lprint(std::string("_thread_enableFullScreen done"));
}

void Renderer::disableFullScreen() {

	// lprint(std::string("disableFullScreen 1"));

	boost::unique_lock<boost::mutex> scoped_lock(_threadMutex);

	HRESULT hr;

	_fullscreen = false;

	releaseAllSurfaces();

	DDSURFACEDESC2 _curmonitorModeInfo;
	memset(&_curmonitorModeInfo, 0, sizeof(_curmonitorModeInfo));
	_curmonitorModeInfo.dwSize = sizeof(_curmonitorModeInfo);
	hr = _dd->GetDisplayMode(&_curmonitorModeInfo);
	if (FAILED(hr)) {
		lprint(std::string("Error: GetDisplayMode ") + DDErrorString(hr));
	}


	if (_fullscreenW == _curmonitorModeInfo.dwWidth && _fullscreenH == _curmonitorModeInfo.dwHeight) {

	}
	else {

		hr = _dd->SetDisplayMode(_monitorModeInfo.dwWidth, _monitorModeInfo.dwHeight, _monitorModeInfo.ddpfPixelFormat.dwRGBBitCount, 0, 0);
		if (FAILED(hr)) {
			lprint(std::string("Error: SetDisplayMode ") + DDErrorString(hr));
			// continue;
		}
	}

	hr = _dd->SetCooperativeLevel(_hwnd, DDSCL_NORMAL | DDSCL_MULTITHREADED);

	SetWindowLong(_hwnd, GWL_STYLE, _windowStyle| WS_OVERLAPPEDWINDOW);
	SetWindowPlacement(_hwnd, &_windowPos);
	SetWindowPos(_hwnd, NULL,
		_windowPos.rcNormalPosition.left, _windowPos.rcNormalPosition.top, _windowPos.rcNormalPosition.right, _windowPos.rcNormalPosition.bottom,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

	_width = _windowPos.rcNormalPosition.right - _windowPos.rcNormalPosition.left;
	_height = _windowPos.rcNormalPosition.bottom - _windowPos.rcNormalPosition.top;
	createSurfaces();

	PostMessage(_hwnd, 0x400, 0, 0);
}

/*

HRESULT WINAPI EnumModes_(_In_ LPDDSURFACEDESC2  d, _In_ LPVOID lpContext) {

	lprint(std::string("EnumModes_ ") + inttostr(d->dwWidth) + " " + inttostr(d->dwHeight) + " " + inttostr(d->ddpfPixelFormat.dwRGBBitCount) + " " + inttostr(d->dwRefreshRate));

	return DDENUMRET_OK;
}

*/


Renderer::Renderer(HWND hwnd, bool runInFullscreen, int mw, int mh) : _hwnd(hwnd) {
	
	_arrowNotSet = true;
	_drawMachine = new DrawMachine();

	_fullscreen = false;
	_useBackBuffer = false;
	_hided = false;

	HRESULT hr;

	_hwnd = hwnd;
	_thread_resize = false;

	_width = 1;
	_height = 1;

	hr = DirectDrawCreateEx(NULL, (VOID**)&_dd, IID_IDirectDraw7, NULL);
	if (FAILED(hr)) {
		this->_state = DirectDrawCreateFailed;
		return;
	}

	Image::init(_dd);

	_backSurfaceCount = 1;
	_backSurfaceIndex = 0;

	_dds_backFullScreen = NULL;
	_dds_Primary = NULL;
	_ddc_Clipper = NULL;
	_dds_Back = new LPDIRECTDRAWSURFACE7[_backSurfaceCount];
	for (int i = 0; i < _backSurfaceCount; i++) {
		_dds_Back[i] = NULL;
	}



	// check for full screen or windowed
	saveWindowedState();

	if (runInFullscreen) {

		_fullscreenW = mw;
		_fullscreenH = mh;

		SetWindowLong(_hwnd, GWL_STYLE, _windowStyle & ~WS_OVERLAPPEDWINDOW);

		hr = _dd->SetCooperativeLevel(_hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_MULTITHREADED);
		if (FAILED(hr)) {
			lprint("Error: SetCooperativeLevel");
			return;
		}

		switchToFullscreen();
	}
	else {

		hr = this->_dd->SetCooperativeLevel(hwnd, DDSCL_NORMAL | DDSCL_MULTITHREADED);
		if (FAILED(hr))
			return;

		if (!this->createSurfaces()) {
			return;
		}

	}
	
	for (int l = 0; l < sizeof(RendererLayer); l++) {
		this->_layers[l] = new RObjectSet();
	}

	// _dd->EnumDisplayModes(DDEDM_REFRESHRATES, NULL, NULL, EnumModes_);
	

	HANDLE ht = CreateThread(NULL, 0, MyThreadFunction, this, 0, NULL);

	this->_state = OK;
}

bool Renderer::createBackSurface() {
	HRESULT hr;

	DDSURFACEDESC2 ddsd;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
	ddsd.dwWidth = this->_width;
	ddsd.dwHeight = this->_height;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	// ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

	// this->_dds_Back = new LPDIRECTDRAWSURFACE7[this->_backSurfaceCount];
	this->_backSurfaceIndex = 0;

	for (int i = 0; i < this->_backSurfaceCount; i++) {

		hr = this->_dd->CreateSurface(&ddsd, &this->_dds_Back[i], NULL);
		if (FAILED(hr)) {
			this->_state = CreateBackSurfaceFailed;
			lprint(std::string("error createBackSurface ") + DDErrorString(hr));
			return false;
		}

	}

	return true;
}

bool Renderer::createSurfaces() {

	HRESULT hr;

	// --------------------------------------------------------------
	lprint("Renderer::createSurfaces");

	DDSURFACEDESC2 ddsd;

	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	ddsd.dwFlags = DDSD_CAPS;
	// ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_SYSTEMMEMORY;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	hr = this->_dd->CreateSurface(&ddsd, &this->_dds_Primary, NULL);
	if (FAILED(hr)) {
		this->_state = CreatePrimarySurfaceFailed;
		lprint(std::string("error createSurfaces ") + DDErrorString(hr));
		return false;
	}

	// --------------------------------------------------------------

	if (!this->createBackSurface()) {
		return false;
	}

	// --------------------------------------------------------------

	hr = this->_dd->CreateClipper(0, &this->_ddc_Clipper, NULL);
	if (FAILED(hr)) {
		lprint(std::string("error CreateClipper ") + DDErrorString(hr));
		return false;
	}

	hr = this->_ddc_Clipper->SetHWnd(0, _hwnd);
	if (FAILED(hr)) {
		lprint(std::string("error SetHWnd ") + DDErrorString(hr));
		return false;
	}

	hr = this->_dds_Primary->SetClipper(this->_ddc_Clipper);
	if (FAILED(hr)) {
		lprint(std::string("error SetClipper ") + DDErrorString(hr));
		return false;
	}

	// --------------------------------------------------------------

	hr = this->_dds_Back[0]->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if (FAILED(hr)) {
		lprint(std::string("error Lock ") + DDErrorString(hr));
		return false;
	}

	this->_bpp = ddsd.ddpfPixelFormat.dwRGBBitCount;

	hr = this->_dds_Back[0]->Unlock(NULL);
	if (FAILED(hr)) {
		lprint(std::string("error Unlock ") + DDErrorString(hr));
		return false;
	}


	return true;
}

int Renderer::getFPS() { return _fps;  }
void Renderer::setFPS(int fps) {
	pc->setPeriodsPerSecond(fps);
}

DWORD Renderer::threadProc() {

	Logger::setThreadName("Renderer");
	lprint("Renderer::threadProc");
	pc = new PeriodCorrector(80);
	HRESULT hr;


	while (true) {

		{
			pc->startPeriod();

			if (!_hided) {

				boost::unique_lock<boost::mutex> scoped_lock(_threadMutex);

				if (this->_thread_resize && !_fullscreen) {

					// lprint("wm_size " + boost::lexical_cast<std::string>(_new_width) + " " + boost::lexical_cast<std::string>(_new_height));


					for (int i = 0; i < this->_backSurfaceCount; i++) {
						this->_dds_Back[i]->Release();
					}

					this->_width = this->_new_width;
					this->_height = this->_new_height;
					this->createBackSurface();

					this->_thread_resize = false;
				}

				// swprintf(wString, L"delta %d, st %d, msperframe %d, framems %d, drawms %d, sleep %d, bsum %d, cnt %d, fps %d", (int)delta, st, (int)msperframe, (int)framems, (int)drawms, a, (int)bsum, cnt, (int)fps);
				/*
				swprintf(this->_wString, L"nResults %d, fps %d, frameMS %d, curFrameMS %d, msWithSleep %d, sleepMS %d, delta %d",
					nResults,
					(int)pc->periodsPerSecond,
					(int)pc->msPerPeriod,
					(int)pc->msCurrentPeriod,
					(int)pc->msWithSleep,
					pc->sleepMS,
					(int)pc->delta
				);
				*/

				_fps = (int)pc->periodsPerSecond;
				_frameTime = (int)pc->msCurrentPeriod;

				this->checkSurfaces();
				this->draw();
				this->flip();
			}
		}

		pc->endPeriod();

	}

	return 0;
}

void Renderer::add(RObject *object, int layer) {

	// boost::unique_lock<boost::mutex> scoped_lock(_robjectMutex);

	auto list = _layers[layer];
	object->_index = list->_botIndex;
	list->_botIndex++;

	list->_set.insert(object);
}

void Renderer::del(RObject *object, int layer) {

	// boost::unique_lock<boost::mutex> scoped_lock(_robjectMutex);

	lprint(std::string("delete object id:") + inttostr(object->_id));
	auto list = _layers[layer];

	typedef _RObjectSet::nth_index<1>::type listType;
	listType::iterator it = list->_set.get<1>().find(object->_id);
	if (it != list->_set.get<1>().end()) {
		list->_set.get<1>().erase(it);
	}
	else {
		lprint(std::string("error object id:") + inttostr(object->_id) + " not found");
	}
}


void Renderer::checkSurfaces() {

	if (_fullscreen && _useBackBuffer) {
		if (_dds_Primary->IsLost() == DDERR_SURFACELOST)
			_dds_Primary->Restore();
		
		if (_dds_backFullScreen->IsLost() == DDERR_SURFACELOST)
			_dds_backFullScreen->Restore();
		
	}
	else {
		if (_dds_Primary->IsLost() == DDERR_SURFACELOST)
			_dds_Primary->Restore();

		if (_dds_Back[_backSurfaceIndex]->IsLost() == DDERR_SURFACELOST)
			_dds_Back[_backSurfaceIndex]->Restore();
	}
}

using namespace boost::chrono;

void Renderer::lockObjectList() {
	system_clock::time_point time_limit = system_clock::now() + seconds(1);
	if (_robjectMutex.try_lock_until(time_limit)) {

	}
	else {
		lprint("error _robjectMutex.try_lock_until");
	}
}

void Renderer::unlockObjectList() {
	_robjectMutex.unlock();
}


void Renderer::draw() {

	if (_fullscreen && _useBackBuffer) {
		_drawMachine->setDDS(this->_dds_backFullScreen, this->_width, this->_height);
	}
	else {
		_drawMachine->setDDS(this->_dds_Back[this->_backSurfaceIndex], this->_width, this->_height);
	}

	boost::unique_lock<boost::mutex> scoped_lock(_robjectMutex);

	for (int l = 0; l < sizeof(RendererLayer); l++) {
		auto list = this->_layers[l];

		typedef _RObjectSet::nth_index<0>::type listType;
		listType::iterator it = list->_set.get<0>().begin(), ite = list->_set.get<0>().end();
		while (it != ite) {
			RObject *o = *it;
			o->draw(_drawMachine);
			it++;
		}
	}

}

void Renderer::flip() {

	HRESULT hr;

	if (_fullscreen && _useBackBuffer) {
		hr = this->_dds_Primary->Flip(NULL, DDFLIP_NOVSYNC | DDFLIP_DONOTWAIT);
	}
	else {

		RECT    rcSrc;
		RECT    rcDest;
		POINT   p;

		p.x = 0; p.y = 0;
		::ClientToScreen(this->_hwnd, &p);

		SetRect(&rcDest, 0, 0, this->_width, this->_height);
		OffsetRect(&rcDest, p.x, p.y);

		SetRect(&rcSrc, 0, 0, _width, _height);

		if (_width < 2 || _height < 2) {
			return;
		}

		if (rcDest.right < 0 || rcDest.bottom < 0 || rcDest.left > 4096 || rcDest.top > 4096) {
			return;
		}

		// lprint(std::string("flip ") + inttostr(_backSurfaceIndex));

		if (_dds_Primary && _dds_Back[_backSurfaceIndex]) {
			// try {
				hr = _dds_Primary->Blt(&rcDest, _dds_Back[_backSurfaceIndex], &rcSrc, DDBLT_WAIT, NULL);
			// }
			// catch (...) {
				// lprint("flip try");
			// }
		}
		else {
			lprint(std::string("error flip index = ") + inttostr(_backSurfaceIndex));
		}


		_backSurfaceIndex++;
		if (_backSurfaceIndex >= _backSurfaceCount) {
			_backSurfaceIndex = 0;
		}
	}
}

LRESULT Renderer::onWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch (message) {

	case WM_SYSKEYUP:
		// lprint(std::string("WM_KEYUP ") + inttostr(wParam) + " " + inttostr((int)(lParam & (1 << 28))));
		if (wParam == VK_RETURN) {
			if (((GetKeyState(VK_MENU) & 0x8000) || (GetKeyState(VK_RMENU) & 0x8000))) {
				if (_fullscreen) {
					lprint("alt+enter minimize");
					disableFullScreen();
					PostMessage(_hwnd, 0x401, 0, 0);

				}
				else {
					// lprint(std::string("enable fullscreen by alt+tab ") + inttostr(_monitorModeInfo.dwWidth) + "x" + inttostr(_monitorModeInfo.dwHeight));
					lprint("alt+enter expand");
					enableFullScreen(_monitorModeInfo.dwWidth, _monitorModeInfo.dwHeight);
					PostMessage(_hwnd, 0x402, 0, 0);

				}
			}
		}
		break;

	case WM_ACTIVATE:

		lprint(std::string("WM_ACTIVATE ") + inttostr((void *)wParam) + " " + inttostr((void *)lParam) + " " + (_fullscreen ? "true" : "false") + " " + (_hided ? "true" : "false"));

		if (_fullscreen && wParam == 1 && _hided) {
			lprint("show");
			_hided = false;
			boost::unique_lock<boost::mutex> scoped_lock(_threadMutex);

			SetWindowLong(_hwnd, GWL_STYLE, _windowStyle & ~WS_OVERLAPPEDWINDOW);

			HRESULT hr = _dd->SetCooperativeLevel(_hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_MULTITHREADED);
			if (FAILED(hr)) {
				lprint("Error: SetCooperativeLevel");
				return 0;
			}


			releaseAllSurfaces();

			switchToFullscreen();
		}

		if (_fullscreen && wParam == 0 && !_hided) {

			lprint("hide");

			_hided = true;

			boost::unique_lock<boost::mutex> scoped_lock(_threadMutex);

			releaseAllSurfaces();

			HRESULT hr;

			DDSURFACEDESC2 _curmonitorModeInfo;
			memset(&_curmonitorModeInfo, 0, sizeof(_curmonitorModeInfo));
			_curmonitorModeInfo.dwSize = sizeof(_curmonitorModeInfo);
			hr = _dd->GetDisplayMode(&_curmonitorModeInfo);
			if (FAILED(hr)) {
				lprint(std::string("Error: GetDisplayMode ") + DDErrorString(hr));
			}


			if (_fullscreenW != _curmonitorModeInfo.dwWidth || _fullscreenH != _curmonitorModeInfo.dwHeight) {

				// Set 640x480x256 full-screen mode
				hr = _dd->SetDisplayMode(_monitorModeInfo.dwWidth, _monitorModeInfo.dwHeight, _monitorModeInfo.ddpfPixelFormat.dwRGBBitCount, 0, 0);
				if (FAILED(hr)) {
					lprint(std::string("Error: SetDisplayMode ") + DDErrorString(hr));
					// continue;
				}
			}			

			hr = _dd->SetCooperativeLevel(_hwnd, DDSCL_NORMAL | DDSCL_MULTITHREADED);
			SetWindowLong(_hwnd, GWL_STYLE, _windowStyle | WS_OVERLAPPEDWINDOW);
			ShowWindow(_hwnd, SW_MINIMIZE);
		}
		break;

	case WM_MOUSELEAVE:
		_arrowNotSet = true;

		return 1;

	case WM_SIZE:
		// lprint(std::string("WM_SIZE ") + inttostr((int)wParam))
		if (!_fullscreen) {
			if (_hided) {
				if (wParam == SIZE_RESTORED) {
					boost::unique_lock<boost::mutex> scoped_lock(_threadMutex);
					_hided = false;
					lprint("set hide = false");
				}
			}
			else {
				if (wParam == SIZE_MINIMIZED) {
					boost::unique_lock<boost::mutex> scoped_lock(_threadMutex);
					_hided = true;
					lprint("set hide = true");
				}

			}
		}

		this->WM_Size(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_MOUSEMOVE:
		this->WM_MouseMove(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_PAINT:
		ValidateRect(_hwnd, NULL);
		return 1;

		/*
	case WM_ERASEBKGND:
		return 1;
		*/

	}

	return 0;
}

void Renderer::WM_Size(int width, int height) {

	if (width < 1) width = 1;
	if (height < 1) height = 1;

	_thread_resize = true;

	_new_width = width;
	_new_height = height;

}

void Renderer::WM_MouseMove(int x, int y) {

	if (_arrowNotSet) {
		_arrowNotSet = false;
		TRACKMOUSEEVENT tme = { sizeof(tme) };
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = _hwnd;
		TrackMouseEvent(&tme);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}

}
