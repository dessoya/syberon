#include "Utils.h"

wchar_t *makewstr(const char *s) {
	size_t size = strlen(s) + 1;
	wchar_t *wbuf = new wchar_t[size];
	size_t outSize;
	mbstowcs_s(&outSize, wbuf, size, s, size - 1);
	return wbuf;
}

char *tchar2char(wchar_t *t) {
	char *s = new char[wcslen(t) + 1];
	wcstombs(s, t, wcslen(t) + 1);
	return s;
}
