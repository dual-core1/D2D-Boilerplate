#include "audio_engine.h"

IXAudio2* g_pXAudio = NULL;
IXAudio2MasteringVoice* g_pMasteringVoice = NULL;

bool InitXAudio() {
    HRESULT hr = S_OK;

    hr = XAudio2Create(&g_pXAudio, 0U, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) return false;

    hr = g_pXAudio->CreateMasteringVoice(
        &g_pMasteringVoice,       // pointer to mastering voice
        XAUDIO2_DEFAULT_CHANNELS, // detect system speaker's configuration
        44100,                    // 44.1kHz sample rate
        0,                        // flags
        NULL,                     // select default audio device
        NULL,                     // no effect chain
        AudioCategory_GameMedia   // stream category -- background audio for games.
    );
    if (FAILED(hr)) return false;

    return true;
}

void ShutdownXAudio() {
    if (g_pXAudio) {
        g_pXAudio->Release();
        g_pXAudio = NULL;
    }
    if (g_pMasteringVoice) {
        g_pMasteringVoice->DestroyVoice();
        g_pMasteringVoice = NULL;
    }
}

HRESULT WavFromFileHandle(HANDLE hFile, WAVEFORMATEXTENSIBLE& wavFormat, XAUDIO2_BUFFER& xAudioBuffer, BYTE*& pData) {
    DWORD dwChunkSize;
    DWORD dwChunkOffset;
    DWORD dwFileType;

    HRESULT hr;

    // locate RIFF chunk, check filetype
    hr = FindChunk(hFile, FOURCC__RIFF, dwChunkSize, dwChunkOffset);
    if (hr != S_OK) return E_FAIL;

    hr = ReadChunkData(hFile, &dwFileType, dwChunkSize, dwChunkOffset);
    if (hr != S_OK) return E_FAIL;

    // this is not a wave file
    if (dwFileType != FOURCC_WAVE)
        return E_FAIL;
    
    // locate fmt chunk, copy to wavFormat
    hr = FindChunk(hFile, FOURCC_FMT, dwChunkSize, dwChunkOffset);
    if (hr != S_OK) return E_FAIL;

    hr = ReadChunkData(hFile, &wavFormat, dwChunkSize, dwChunkOffset);
    if (hr != S_OK) return E_FAIL;

    // locate data chunk, read into buffer
    hr = FindChunk(hFile, FOURCC_DATA, dwChunkSize, dwChunkOffset);
    if (hr != S_OK) return E_FAIL;

    pData = new BYTE[dwChunkSize];

    hr = ReadChunkData(hFile, pData, dwChunkSize, dwChunkOffset);
    if (FAILED(hr)) {
        delete[] pData;
        return E_FAIL;
    }

    // populate the xaudio2 buffer
    xAudioBuffer.AudioBytes = dwChunkSize;
    xAudioBuffer.pAudioData = pData;
    xAudioBuffer.Flags = XAUDIO2_END_OF_STREAM;

    return S_OK;
}

HRESULT FindChunk(HANDLE hFile, DWORD dwFourCC, DWORD& dwChunkSize, DWORD& dwChunkPosition) {
    HRESULT hr = S_OK;

    DWORD dwChunkType;
    DWORD dwBytesRead;

    // out values
    DWORD dwSize = 0;       // length of chunk data
    DWORD dwPosition = 0;   // beginning of chunk data

    // go to start of file
    if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        return HRESULT_FROM_WIN32(GetLastError());
    
    // iterate till chunk is found
    while (hr == S_OK) {
        
        // read four-character code
        if (ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwBytesRead, NULL) == 0)
            return HRESULT_FROM_WIN32(GetLastError());
        
        // read chunk size
        if (ReadFile(hFile, &dwSize, sizeof(DWORD), &dwBytesRead, NULL) == 0)
            return HRESULT_FROM_WIN32(GetLastError());
        
        // check chunk type
        switch (dwChunkType) {
        // For RIFF, expose only the 4-byte form type ("WAVE") as the readable payload.
        // The full RIFF size includes the form type plus all subchunks.
        case FOURCC__RIFF:
            dwSize = 4;
            if (SetFilePointer(hFile, 4, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
                return HRESULT_FROM_WIN32(GetLastError());
            break;
        
        // otherwise, move the file pointer to the beginning of the next chunk, accounting for padding
        default:
            if (SetFilePointer(hFile, dwSize + (dwSize & 1), NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
                return HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        // move offset beyond chunk header to start of chunk data
        dwPosition += sizeof(DWORD) * 2;

        // found the right chunk?
        if (dwChunkType == dwFourCC) {
            dwChunkSize = dwSize;
            dwChunkPosition = dwPosition;
            return S_OK;
        }

        // if not, move offset to beginning of next chunk, accounting for padding
        dwPosition += dwSize + (dwSize & 1);
    }

    return hr;
}

HRESULT ReadChunkData(HANDLE hFile, LPVOID lpBuffer, DWORD dwBufferSize, DWORD dwOffset) {
    DWORD dwRead;

    // move the file pointer to the chunk data offset
    if (SetFilePointer(hFile, dwOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        return HRESULT_FROM_WIN32(GetLastError());
    
    // read chunk data
    if (ReadFile(hFile, lpBuffer, dwBufferSize, &dwRead, NULL) == 0)
        return HRESULT_FROM_WIN32(GetLastError());
    
    // check for short read
    if (dwRead != dwBufferSize)
        return E_FAIL;
    
    return S_OK;
}
