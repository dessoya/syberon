#pragma once

#include "windows.h"

class PeriodCorrector {

private:

	LARGE_INTEGER _frequency, _startPeriod, _endPeriod, _afterSleep;
	int _periodPerSecond;
	double _msPerPeriod, _delta;

	int _bufferSize, _bufferPos, _count;
	double _bufferSum;
	double *_buffer;


public:

	double periodsPerSecond, msCurrentPeriod, msPerPeriod, delta, msWithSleep;
	int sleepMS;

	PeriodCorrector(int periodPerSecond);

	void startPeriod();
	void endPeriod();
	void afterDraw();

	void setPeriodsPerSecond(int periods);
};


class TimeCalc {
public:
	LARGE_INTEGER _startPeriod;
	TimeCalc() { QueryPerformanceCounter(&_startPeriod); }
	long get() { LARGE_INTEGER _endPeriod; QueryPerformanceCounter(&_endPeriod); return _endPeriod.QuadPart - _startPeriod.QuadPart;  };
	double micro(long l) {
		LARGE_INTEGER f;
		QueryPerformanceFrequency(&f);

		double ms = (double)l;
		ms *= (double)1000000;
		ms /= (double)f.QuadPart;

		return ms;
	}
};