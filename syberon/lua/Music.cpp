#include "windows.h"
#include "..\Logger.h"
#include "..\LuaScript.h"

#include	<dsound.h>						//DirectSound headers
#include    "vorbis/codec.h"				//for OGG vorbis codec
#include    <vorbis/vorbisfile.h>			//for OGG vorbis file information

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")


IDirectSound8* m_DirectSound;
IDirectSoundBuffer* m_primaryBuffer;
IDirectSound3DListener8* m_listener;

void Music_Init(HWND hwnd) {

	HRESULT result;

	result = DirectSoundCreate8(NULL, &m_DirectSound, NULL);
	if (FAILED(result)) {
		return;
	}

	// Set the cooperative level to priority so the format of the primary sound buffer can be modified.
	result = m_DirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
	if (FAILED(result)) {
		return;
	}

	// Setup the primary buffer description.
	DSBUFFERDESC bufferDesc;
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
	bufferDesc.dwBufferBytes = 0;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = NULL;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	// Get control of the primary sound buffer on the default sound device.
	result = m_DirectSound->CreateSoundBuffer(&bufferDesc, &m_primaryBuffer, NULL);
	if (FAILED(result)) {
		return;
	}

	// Setup the format of the primary sound bufffer.
	// In this case it is a .WAV file recorded at 44,100 samples per second in 16-bit stereo (cd audio format).
	WAVEFORMATEX waveFormat;
	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 2;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.cbSize = 0;

	// Set the primary buffer to be the wave format specified.
	result = m_primaryBuffer->SetFormat(&waveFormat);
	if (FAILED(result)) {
		return;
	}

	// Obtain a listener interface.
	result = m_primaryBuffer->QueryInterface(IID_IDirectSound3DListener8, (LPVOID*)&m_listener);
	if (FAILED(result)) {
		return;
	}

	// Set the initial position of the listener to be in the middle of the scene.
	m_listener->SetPosition(0.0f, 0.0f, 0.0f, DS3D_IMMEDIATE);

}

void Music_Load_OGG() {

	// std::string filename = "D:\\steamGames\\steamapps\\common\\Factorio\\data\\base\\sound\\wooden-chest-open.ogg";
	std::string filename = "D:\\steamGames\\steamapps\\common\\Factorio\\data\\base\\sound\\assembling-machine-t2-2.ogg";


	FILE *f;
	fopen_s(&f, filename.c_str(), "rb");

	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);

	OggVorbis_File vorbisFile;

	if ((ov_open(f, &vorbisFile, NULL, 0)) != 0) {

		fclose(f);
		lprint("error 1");

		return;
	}

	int nBytesReaded = 0;
	char *b = NULL;
	int chunk = 1024 * 8;
	int nBitStream = 0, total = 0;

	int nBytesReadThisTime;

	do {

		if (b) {
			char *n = new char[total + chunk];
			memcpy(n, b, total);
			delete b;
			b = n;
		}
		else {
			b = new char[chunk];
		}

		nBytesReadThisTime = ov_read(&vorbisFile, (char*)b + total, chunk, 0, 2, 1, &nBitStream);
		total += nBytesReadThisTime;
	} while (nBytesReadThisTime);

	lprint(std::string("readed ") + inttostr(total));
	ov_clear(&vorbisFile);
	fclose(f);

}

void Music_CreateBuffer(char *b, long bsize) {

	HRESULT result;
	WAVEFORMATEX waveFormat;


	IDirectSoundBuffer8* m_secondaryBuffer1;
	IDirectSound3DBuffer8* m_secondary3DBuffer1;

	waveFormat.nSamplesPerSec = 44100;
	waveFormat.wBitsPerSample = 16;
	waveFormat.nChannels = 1;
	waveFormat.nBlockAlign = (waveFormat.wBitsPerSample / 8) * waveFormat.nChannels;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.cbSize = 0;

	DSBUFFERDESC bufferDesc;
	bufferDesc.dwSize = sizeof(DSBUFFERDESC);
	bufferDesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
	bufferDesc.dwBufferBytes = bsize;
	bufferDesc.dwReserved = 0;
	bufferDesc.lpwfxFormat = &waveFormat;
	bufferDesc.guid3DAlgorithm = GUID_NULL;

	IDirectSoundBuffer* tempBuffer;

	// Create a temporary sound buffer with the specific buffer settings.
	result = m_DirectSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, NULL);
	if (FAILED(result))	{
		return ;
	}

	result = tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&m_secondaryBuffer1);
	if (FAILED(result))
	{
		return;
	}

	// Release the temporary buffer.
	tempBuffer->Release();
	tempBuffer = 0;

	unsigned char* bufferPtr;
	unsigned long bufferSize;

	result = m_secondaryBuffer1->Lock(0, bsize, (void**)&bufferPtr, (DWORD*)&bufferSize, NULL, 0, 0);
	if (FAILED(result))
	{
		return ;
	}

	memcpy(bufferPtr, b, bsize);

	// Unlock the secondary buffer after the data has been written to it.
	result = m_secondaryBuffer1->Unlock((void*)bufferPtr, bufferSize, NULL, 0);
	if (FAILED(result))
	{
		return ;
	}

	result = m_secondaryBuffer1->QueryInterface(IID_IDirectSound3DBuffer8, (void**)&m_secondary3DBuffer1);
	if (FAILED(result))
	{
		return;
	}


}

void Music_Play() {

	IDirectSoundBuffer8* m_secondaryBuffer1;
	IDirectSound3DBuffer8* m_secondary3DBuffer1;

	// HRESULT result;
	float positionX, positionY, positionZ;
	// Setup the position where we want the 3D sound to be located.In this case it will be set over to the left.

	// Set the 3D position of where the sound should be located.
	positionX = -5.0f; // left -value, right +value
	positionY = 0.0f; // forwards, backward
	positionZ = 0.0f; // top bot

	// Set position at the beginning of the sound buffer.
	HRESULT result = m_secondaryBuffer1->SetCurrentPosition(0);
	if (FAILED(result))
	{
		return;
	}

	// Set volume of the buffer to 100%.
	result = m_secondaryBuffer1->SetVolume(DSBVOLUME_MAX);
	if (FAILED(result))
	{
		return ;
	}
	// Now use the position and set the 3D sound position using the 3D interface.The DS3D_IMMEDIATE flag sets the sound position immediately instead of deferring it for later batch processing.

	// Set the 3D position of the sound.
	m_secondary3DBuffer1->SetPosition(positionX, positionY, positionZ, DS3D_IMMEDIATE);

	// Play the contents of the secondary sound buffer.
	lprint("play 1");
	result = m_secondaryBuffer1->Play(0, 0, 0);
	lprint("play 2");
	if (FAILED(result))
	{
		return ;
	}


}

static int luaC_Music_New(lua_State *L) {

	auto filepath = lua_tostring(L, 1);
	/*
	auto music = new sf::Music();


	music->openFromFile(filepath);

	sf::Listener::setPosition(0, 0, 0);
	sf::Listener::setDirection(0, 0, 1);
	
	//music->setRelativeToListener(true);
	//music->setPosition(1000, 0, 500);
	//music->setMinDistance(20);
	// music->setAttenuation(20);
	// music->setVolume(20);
	*/

	auto ud = (UserData *)lua_newuserdata(L, sizeof(UserData));
	ud->type = UDT_Music;
	ud->data = 0;

	return 1;
}

static int luaC_Music_play(lua_State *L) {

	auto ud = (UserData *)lua_touserdata(L, 1);
	/*
	auto music = (sf::Music *)ud->data;
	music->play();
	*/

	return 0;
}

void lm_Music_install(lua_State* _l) {

	lua_register(_l, "C_Music_New", luaC_Music_New);
	lua_register(_l, "C_Music_play", luaC_Music_play);
}

LuaModule *lm_Music = new LuaModule("music", lm_Music_install);