		ZeroMemory(&ddsd2, sizeof(ddsd2));

		ddsd2.dwCaps = DDSCAPS_VIDEOMEMORY;
		DWORD dwTotal, dwFree;
		hr = _dd->GetAvailableVidMem(&ddsd2, &dwTotal, &dwFree);
		lprint(std::string("video mem ") + inttostr(dwTotal) + " " + inttostr(dwFree));
		DWORD modemem = _fullscreenW * _fullscreenH * 4;
		lprint(std::string("per screen ") + inttostr(modemem));
		int cnt = dwFree / modemem;
		cnt--;		
		lprint(std::string("screens ") + inttostr(cnt));
		if (cnt > 20) {
			cnt = 20;
		}

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		// ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		// ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		// ddsd.dwFlags = DDSD_CAPS;
		// ddsd.dwWidth = _fullscreenW;
		// ddsd.dwHeight = _fullscreenH;
		// ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_SYSTEMMEMORY;
		// DDSCAPS_FLIP
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_VIDEOMEMORY;
		// ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
		ddsd.ddsCaps.dwCaps2 = DDCAPS2_FLIPNOVSYNC;
		ddsd.dwBackBufferCount = 3;

		// _dd->GetScanLine()

		hr = _dd->CreateSurface(&ddsd, &_dds_Primary, NULL);
		if (FAILED(hr)) {
			lprint(std::string("Error: CreateSurface ") + DDErrorString(hr));
			// hr = _dd->SetCooperativeLevel(_hwnd, DDSCL_NORMAL | DDSCL_MULTITHREADED);
			hr = _dd->SetCooperativeLevel(_hwnd, DDSCL_NORMAL);
			exit(1);
		}

		auto bb1 = _dds_Primary;
		auto bb2 = _dds_Primary;
		auto bb3 = _dds_Primary;
		auto bb4 = _dds_Primary;

		ZeroMemory(&ddsd2, sizeof(ddsd2));
		ddsd2.dwCaps = DDSCAPS_BACKBUFFER;
		_dds_Primary->GetAttachedSurface(&ddsd2, &bb1);
		_dds_Primary->Flip(0, DDFLIP_NOVSYNC | DDFLIP_DONOTWAIT);
		_dds_Primary->GetAttachedSurface(&ddsd2, &bb2);
		_dds_Primary->Flip(0, DDFLIP_NOVSYNC | DDFLIP_DONOTWAIT);
		_dds_Primary->GetAttachedSurface(&ddsd2, &bb3);
		_dds_Primary->Flip(0, DDFLIP_NOVSYNC | DDFLIP_DONOTWAIT);
		_dds_Primary->GetAttachedSurface(&ddsd2, &bb4);


		ddsd.dwSize = sizeof(ddsd);
		hr = _dds_Primary->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
		if (hr != DD_OK) {
			lprint(std::string("Error _dds_Primary->Lock ") + DDErrorString(hr));
			return;
		}

		auto _surface = (unsigned char *)ddsd.lpSurface;
		lprint(std::string("video 1 0x") + inttostr((void *)_surface));
		/*
		_surface[0] = _surface[1] = _surface[2] = 0;
		_surface[3] = 255;
		*/
		int q = 10 * 4;

		_surface[q++] = 255; // b
		_surface[q++] = 255; // g
		_surface[q++] = 255; // r 
							 /*
		_surface[q++] = 255; // b
		_surface[q++] = 0;	 // g
		_surface[q++] = 255; // r 
		_surface[q++] = 0;

		_surface[q++] = 0;
		_surface[q++] = 255;
		_surface[q++] = 255;
		_surface[q++] = 0;

		_surface[q++] = 255;
		_surface[q++] = 255;
		_surface[q++] = 0;
		_surface[q++] = 0;

		_surface[q++] = 255;
		_surface[q++] = 255;
		_surface[q++] = 255;
		_surface[q++] = 0;
		*/


		q = ddsd.lPitch * 4 + 10 * 4;

		_surface[q++] = 255;
		_surface[q++] = 255;
		_surface[q++] = 255;
		_surface[q++] = 0;


		_dds_Primary->Unlock(NULL);

		hr = _dd->WaitForVerticalBlank(DDWAITVB_BLOCKEND, 0);
		if (hr != DD_OK) {
			lprint(std::string("Error WaitForVerticalBlank DDWAITVB_BLOCKEND ") + DDErrorString(hr))
		}
		DWORD ln;
		int c = 30;
		while (c--) {
			_dd->GetScanLine(&ln);
			lprint(std::string("DDWAITVB_BLOCKEND ") + inttostr(ln));
			Sleep(1);
		}

		hr = _dd->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
		if (hr != DD_OK) {
			lprint(std::string("Error WaitForVerticalBlank DDWAITVB_BLOCKEND ") + DDErrorString(hr))
		}
		c = 30;
		while (c--) {
			_dd->GetScanLine(&ln);
			lprint(std::string("DDWAITVB_BLOCKBEGIN ") + inttostr(ln));
			Sleep(1);
		}

		TimeCalc *t2;

		t2 = new TimeCalc();
		_dds_Primary->Flip(bb3, DDFLIP_NOVSYNC | DDFLIP_DONOTWAIT);
		auto t3 = t2->get();
		long t4 = (long)t2->micro(t3);
		lprint(std::string("bb3 ") + inttostr(t3) + " msec " + inttostr(t4));
		delete t2;

		t2 = new TimeCalc();
		_dds_Primary->Flip(bb1, DDFLIP_NOVSYNC | DDFLIP_DONOTWAIT);
		t3 = t2->get();
		t4 = (long)t2->micro(t3);
		lprint(std::string("bb1 ") + inttostr(t3) + " msec " + inttostr(t4));
		delete t2;

		t2 = new TimeCalc();
		_dds_Primary->Flip(bb4, DDFLIP_NOVSYNC | DDFLIP_DONOTWAIT);
		t3 = t2->get();
		t4 = (long)t2->micro(t3);
		lprint(std::string("bb4 ") + inttostr(t3) + " msec " + inttostr(t4));
		delete t2;

		t2 = new TimeCalc();
		_dds_Primary->Flip(bb2, DDFLIP_NOVSYNC | DDFLIP_DONOTWAIT);
		t3 = t2->get();
		t4 = (long)t2->micro(t3);
		lprint(std::string("bb2 ") + inttostr(t3) + " msec " + inttostr(t4));
		delete t2;

		c = 120;
		while (c--) {
			t2 = new TimeCalc();
			_dds_Primary->Flip(NULL, DDFLIP_NOVSYNC | DDFLIP_DONOTWAIT);			
			auto t3 = t2->get();
			long t4 = (long)t2->micro(t3);
			lprint(std::string("flip ") + inttostr(t3) + " msec " + inttostr(t4));
			delete t2;

			_dd->GetScanLine(&ln);
			lprint(std::string("line ") + inttostr(ln));

			ddsd.dwSize = sizeof(ddsd);
			hr = _dds_Primary->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if (hr != DD_OK) {
				lprint(std::string("Error _dds_Primary->Lock ") + DDErrorString(hr));
				return;
			}

			auto _surface = (unsigned char *)ddsd.lpSurface;
			lprint(std::string("video 0x") + inttostr((void *)_surface));
			_dds_Primary->Unlock(NULL);

			auto bb = _dds_Primary;

			ZeroMemory(&ddsd2, sizeof(ddsd2));
			ddsd2.dwCaps = DDSCAPS_BACKBUFFER;
			_dds_Primary->GetAttachedSurface(&ddsd2, &bb);

			ddsd.dwSize = sizeof(ddsd);
			hr = bb->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
			if (hr != DD_OK) {
				lprint(std::string("Error _dds_Primary->Lock ") + DDErrorString(hr));
				return;
			}

			_surface = (unsigned char *)ddsd.lpSurface;
			lprint(std::string("back 0x") + inttostr((void *)_surface));
			bb->Unlock(NULL);

			_dd->GetScanLine(&ln);
			lprint(std::string("line ") + inttostr(ln));

			Sleep(3);
		}

		_dds_Primary->GetAttachedSurface(&ddsd2, &bb1);
		hr = bb1->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
		if (hr != DD_OK) {
			lprint(std::string("Error _dds_Primary->Lock ") + DDErrorString(hr));
			return;
		}

		_surface = (unsigned char *)ddsd.lpSurface; 
		q = ddsd.lPitch * 20 + 20 * 4;

		_surface[q++] = 255; // b
		_surface[q++] = 255; // g
		_surface[q++] = 255;

		// _surface = (unsigned char *)ddsd.lpSurface;

		t2 = new TimeCalc();
		char *s1 = new char[_fullscreenH * _fullscreenW * 4];
		auto s3 = s1;
		for (int q1 = 0; q1 < _fullscreenH; q1++) {
			memcpy(_surface, s1, _fullscreenW * 4);
			_surface += ddsd.lPitch;
			s1 += _fullscreenW * 4;
		}
		
		t3 = t2->get();
		t4 = (long)t2->micro(t3);
		lprint(std::string("copy v") + inttostr(t3) + " msec " + inttostr(t4));
		delete t2;

		t2 = new TimeCalc();		
		char *s2 = new char[_fullscreenH * _fullscreenW * 4];
		for (int q1 = 0; q1 < _fullscreenH; q1++) {
			memcpy(s2, s3, _fullscreenW * 4);
			s2 += _fullscreenW * 4;
			s3 += _fullscreenW * 4;
		}

		t3 = t2->get();
		t4 = (long)t2->micro(t3);
		lprint(std::string("copy s") + inttostr(t3) + " msec " + inttostr(t4));
		delete t2;

		lprint(std::string("back 0x") + inttostr((void *)_surface));
		bb1->Unlock(NULL);
		_dds_Primary->Flip(NULL, DDFLIP_NOVSYNC | DDFLIP_DONOTWAIT);


		/*
		_dds_Primary->Flip(NULL, DDFLIP_WAIT);

		hr = _dds_Primary->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
		if (hr != DD_OK) {
			lprint(std::string("Error _dds_Primary->Lock ") + DDErrorString(hr));
			return;
		}

		_surface = (unsigned char *)ddsd.lpSurface;
		lprint(std::string("video 2 0x") + inttostr((void *)_surface));
		_surface[0] = _surface[1] = _surface[2] = 0;
		_surface[3] = 255;
		_surface[4] = 255;
		_surface[5] = _surface[6] = 0;
		_surface[7] = 255;
		_dds_Primary->Unlock(NULL);


		_dds_Primary->Flip(NULL, DDFLIP_WAIT);
		*/
		// _dds_Primary->Flip(NULL, DDFLIP_WAIT);

		/*
		auto qq = _dds_Primary;

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_VIDEOMEMORY;

		hr = _dd->CreateSurface(&ddsd, &_dds_Primary, NULL);
		if (FAILED(hr)) {
			lprint(std::string("Error: CreateSurface ") + DDErrorString(hr));
		}
		_dds_Primary->Flip(NULL, DDFLIP_WAIT);
		*/

		_epos = 0;

		memset(&ddsd, 0, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		ddsd.dwWidth = _fullscreenW;
		ddsd.dwHeight = _fullscreenH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

		for (int i = 0; i < _bcount; i++) {
			// _dds_backFullScreen[i] = NULL;
			hr = _dd->CreateSurface(&ddsd, &_dds_backFullScreen[i], NULL);
			if (FAILED(hr)) {
				// this->_state = CreateBackSurfaceFailed;
				lprint(std::string("error createBackSurface ") + DDErrorString(hr));
				// return false;
			}
		}

		/*
		_dds_Primary->EnumAttachedSurfaces(
			this,
			_EnumSurfacesCallback7			
		);

		_epos = 0;
		*/

		// _dds_backFullScreen = NULL;
