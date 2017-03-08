#include "PeriodCorrector.h"
#include <concrt.h>

PeriodCorrector::PeriodCorrector(int periodPerSecond) {

	QueryPerformanceFrequency(&this->_frequency);
	this->_bufferSize = 100;
	this->_buffer = new double[this->_bufferSize];

	this->setPeriodsPerSecond(periodPerSecond);
}

void PeriodCorrector::setPeriodsPerSecond(int periods) {

	this->periodsPerSecond = 0;
	this->_periodPerSecond = periods;
	this->msPerPeriod = this->_msPerPeriod = (double)1000000 / (double)this->_periodPerSecond;

	this->_bufferPos = 0;
	this->_count = 0;
	this->_bufferSum = 0;
	this->_delta = 0;

	for (int i = 0; i < this->_bufferSize; i++) {
		this->_buffer[i] = 0;
	}
}

void PeriodCorrector::startPeriod() {
	QueryPerformanceCounter(&this->_startPeriod);
}

void PeriodCorrector::afterDraw() {

	LARGE_INTEGER ElapsedMicroseconds, ad;

	QueryPerformanceCounter(&ad);

	ElapsedMicroseconds.QuadPart = ad.QuadPart - _startPeriod.QuadPart;

	double c = (double)ElapsedMicroseconds.QuadPart;

	c *= (double)1000000;
	c /= (double)_frequency.QuadPart;
	msCurrentPeriod = c;
}

void PeriodCorrector::endPeriod() {

	LARGE_INTEGER ElapsedMicroseconds;

	QueryPerformanceCounter(&this->_endPeriod);

	ElapsedMicroseconds.QuadPart = this->_endPeriod.QuadPart - this->_startPeriod.QuadPart;

	double msCurrentPeriod = (double)ElapsedMicroseconds.QuadPart;

	msCurrentPeriod *= (double)1000000;
	msCurrentPeriod /= (double)this->_frequency.QuadPart;

	// this->msCurrentPeriod = msCurrentPeriod;

	int ms = (int)((this->_msPerPeriod - msCurrentPeriod - this->_delta) / 1000);
	this->sleepMS = ms;

	if (ms > 0) {
		SleepEx(ms, FALSE);
		// Concurrency::wait(ms);
	}
	else {
		Sleep(0);
	}

	QueryPerformanceCounter(&this->_afterSleep);

	ElapsedMicroseconds.QuadPart = this->_afterSleep.QuadPart - this->_startPeriod.QuadPart;

	double msWithSleep = (double)ElapsedMicroseconds.QuadPart;
	msWithSleep *= (double)1000000;
	msWithSleep /= (double)this->_frequency.QuadPart;

	this->_bufferSum -= this->_buffer[this->_bufferPos];
	this->_bufferSum += msWithSleep;
	this->_buffer[this->_bufferPos] = msWithSleep;

	this->_bufferPos++;
	if (this->_bufferPos >= this->_bufferSize) {
		this->_bufferPos = 0;
	}

	if (this->_count < this->_bufferSize) {
		this->_count++;
	}

	double period = this->_bufferSum / (double)this->_count;

	if (period > 0) {
		this->periodsPerSecond = 1000000 / period;
	}
	else {
		this->periodsPerSecond = 0;
	}

	this->msWithSleep = msWithSleep;
	this->delta = this->_delta = period - this->_msPerPeriod;
}
