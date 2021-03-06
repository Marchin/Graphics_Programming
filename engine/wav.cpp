s32
readWAV(WAV* pWAV, const char* pPath) {
    FILE* pFile = fopen(pPath, "rb");
	char buffer[4];
    
    WAV wav = {};
    if (pFile) {
        fseek(pFile, 0, SEEK_END);
        u32 size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
        fread(buffer, sizeof(u8), 4, pFile);
        if (strncmp(buffer, "RIFF", 4) != 0) { return -1; }
        strncpy(wav.riff, buffer, 4);
        fread(buffer, sizeof(u8), 4, pFile);
        memcpy(&wav.overallSize, buffer, 4);
        fread(buffer, sizeof(u8), 4, pFile);
        if (strncmp(buffer, "WAVE", 4) != 0) { return -1; }
        strncpy(wav.wave, buffer, 4);
        fread(buffer, sizeof(u8), 4, pFile);
        strncpy(wav.fmtChunkMarker, buffer, 4);
        fread(buffer, sizeof(u8), 4, pFile);
        memcpy(&wav.lengthOfFmt, buffer, 4);
        fread(buffer, sizeof(u8), 2, pFile);
        memcpy(&wav.formatType, buffer, 2);
        fread(buffer, sizeof(u8), 2, pFile);
        memcpy(&wav.channels, buffer, 2);
        fread(buffer, sizeof(u8), 4, pFile);
        memcpy(&wav.sampleRate, buffer, 4);
        fread(buffer, sizeof(u8), 4, pFile);
        memcpy(&wav.byterate, buffer, 4);
        fread(buffer, sizeof(u8), 2, pFile);
        memcpy(&wav.blockAlign, buffer, 2);
        fread(buffer, sizeof(u8), 2, pFile);
        memcpy(&wav.bitsPerSample, buffer, 2);
        fread(buffer, sizeof(u8), 4, pFile);
        memcpy(&wav.dataChunkHeader, buffer, 4);
        size_t err = fread(buffer, sizeof(u8), 4, pFile);
        memcpy(&wav.dataSize, buffer, 4);
        if (pWAV != 0) {
            *pWAV = wav;
            pWAV->pData = malloc(pWAV->dataSize);
            err = fread(pWAV->pData, sizeof(u8), pWAV->dataSize , pFile);
            fclose(pFile);
        }
    } else {
        perror("Error opening file\n");
        return -1;
    }
    
    return 0;
}

void
freeWAV(MusicData* pMusicData) {
    free(pMusicData->pTBuffer);
    free(pMusicData->musicData);
    free(pMusicData->pFourierData);
}

int
fftCallback(const void* inputBuffer, void* outputBuffer,
            unsigned long framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags,
            void* pUserData) {
    
    MusicData* pMusicData = (MusicData*)pUserData; 
    u32 byteCountToSend = pMusicData->audioBlockAlign*SAMPLE_RATE;
    if (pMusicData->paused) {
        memset(outputBuffer, 0, byteCountToSend);
        memset(pMusicData->pTBuffer, 0, SAMPLE_RATE*sizeof(f32));
        return paContinue;
    }
    pMusicData->changed = true;
    
    s16* pOut = (s16*)outputBuffer;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    
    if (byteCountToSend > pMusicData->musicBytesLeft) {
        byteCountToSend = pMusicData->musicBytesLeft;
    }
    pMusicData->musicBytesLeft -= byteCountToSend;
    
    memcpy(outputBuffer, pMusicData->pPlaying, byteCountToSend);
    
    for (u32 i = 0; i < SAMPLE_RATE; ++i) {
        pMusicData->pTBuffer[i] = (f32)(pOut[2*i] + pOut[2*i + 1]);
    }
    
    // remove_dc
    f32 avg = 0;
    for (u32 i = 0; i < SAMPLE_RATE; ++i) {
        avg += pMusicData->pTBuffer[i];
    } 
    avg /= SAMPLE_RATE;
    for (u32 i = 0; i < SAMPLE_RATE; ++i) {
        pMusicData->pTBuffer[i] -= avg;
    }
    
    pMusicData->changed = true;
    
    if (byteCountToSend < pMusicData->musicBytesLeft) {
        pMusicData->pPlaying += byteCountToSend;
        return paContinue;
    } else {
        return paComplete;
    }
}

void
initMusicData(MusicData* pMusicData, WAV* pWAV) {
    pMusicData->pPlaying = (u8*)pWAV->pData;
    pMusicData->pTBuffer = (f32*)malloc(sizeof(f32)*SAMPLE_RATE);
    pMusicData->audioBlockAlign = pWAV->blockAlign;
    pMusicData->musicBytesLeft = pWAV->dataSize;
    pMusicData->paused = true;
    pMusicData->changed = false;
}