#include <xaudio2.h>
#include <mmreg.h>

// four-character codes are in reverse order on little-endian systems
#define FOURCC__RIFF 'FFIR'
#define FOURCC_DATA 'atad'
#define FOURCC_FMT  ' tmf'
#define FOURCC_WAVE 'EVAW'

extern IXAudio2* g_pXAudio;
extern IXAudio2MasteringVoice* g_pMasteringVoice;

bool InitXAudio();
void ShutdownXAudio();

HRESULT WavFromFileHandle(HANDLE hFile, WAVEFORMATEXTENSIBLE& wavFormat, XAUDIO2_BUFFER& xAudioBuffer, BYTE*& pData);
HRESULT FindChunk(HANDLE hFile, DWORD dwFourCC, DWORD& dwChunkSize, DWORD& dwChunkPosition);
HRESULT ReadChunkData(HANDLE hFile, LPVOID lpBuffer, DWORD dwBufferSize, DWORD dwOffset);
