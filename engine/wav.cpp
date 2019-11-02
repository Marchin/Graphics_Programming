MusicVisualizerConfig* gpMusicVisualizerConfig;

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
        memcpy(&wav.overall_size, buffer, 4);
        fread(buffer, sizeof(u8), 4, pFile);
        if (strncmp(buffer, "WAVE", 4) != 0) { return -1; }
        strncpy(wav.wave, buffer, 4);
        fread(buffer, sizeof(u8), 4, pFile);
        strncpy(wav.fmt_chunk_marker, buffer, 4);
        fread(buffer, sizeof(u8), 4, pFile);
        memcpy(&wav.length_of_fmt, buffer, 4);
        fread(buffer, sizeof(u8), 2, pFile);
        memcpy(&wav.format_type, buffer, 2);
        fread(buffer, sizeof(u8), 2, pFile);
        memcpy(&wav.channels, buffer, 2);
        fread(buffer, sizeof(u8), 4, pFile);
        memcpy(&wav.sample_rate, buffer, 4);
        fread(buffer, sizeof(u8), 4, pFile);
        memcpy(&wav.byterate, buffer, 4);
        fread(buffer, sizeof(u8), 2, pFile);
        memcpy(&wav.block_align, buffer, 2);
        fread(buffer, sizeof(u8), 2, pFile);
        memcpy(&wav.bits_per_sample, buffer, 2);
        fread(buffer, sizeof(u8), 4, pFile);
        memcpy(&wav.data_chunk_header, buffer, 4);
        size_t err = fread(buffer, sizeof(u8), 4, pFile);
        memcpy(&wav.data_size, buffer, 4);
        if (pWAV != 0) {
            *pWAV = wav;
            pWAV->pData = malloc(pWAV->data_size);
            err = fread(pWAV->pData, sizeof(u8), pWAV->data_size , pFile);
            fclose(pFile);
        }
    } else {
        perror("Error opening file\n");
        return -1;
    }
    
    return 0;
}

global b32 gPaused = true;
global u8* gpPlaying;
global u32 gAudioBlockAlign;
global u32 gMusicBytesLeft;

void
setMusicData(u32 audioBlockAlign, u32 musicSizeInBytes) {
    gAudioBlockAlign = audioBlockAlign;
    gMusicBytesLeft = musicSizeInBytes;
}

void playMusic() {
    gPaused = false;
}

global f32 gVolume = 0.5f;
global f32* gpFFTMod = (f32*)calloc(HALF_SAMPLE_RATE, sizeof(f32));

int
patestCallback(const void* inputBuffer, void* outputBuffer,
               unsigned long framesPerBuffer,
               const PaStreamCallbackTimeInfo* timeInfo,
               PaStreamCallbackFlags statusFlags,
               void* pUserData) {
    /* Cast data passed through stream to our structure. */
    if (gPaused) { return paContinue; }
    
    if (gpPlaying == 0) { gpPlaying = (u8*)pUserData; }
    s16* out = (s16*)outputBuffer;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    u32 byteCountToSend = gAudioBlockAlign*framesPerBuffer;
    
    if (byteCountToSend > gMusicBytesLeft) {
        byteCountToSend = gMusicBytesLeft;
    }
    gMusicBytesLeft -= byteCountToSend;
    
    memcpy(outputBuffer, gpPlaying, byteCountToSend);
    
    if (byteCountToSend < gMusicBytesLeft) {
        gpPlaying += byteCountToSend;
        return paContinue;
    } else {
        return paComplete;
    }
}

kiss_fftr_cfg gMycfg = kiss_fftr_alloc(SAMPLE_RATE, 0, NULL, NULL);
s16* inbuf = (s16*)malloc(sizeof(s16)*2*SAMPLE_RATE);
kiss_fft_scalar* pTBuffer = (kiss_fft_scalar*)malloc(sizeof(kiss_fft_scalar)*SAMPLE_RATE);
kiss_fft_cpx* pFBuffer = (kiss_fft_cpx*)malloc(HALF_SAMPLE_RATE*sizeof(kiss_fft_cpx));

int
fftCallback(const void* inputBuffer, void* outputBuffer,
            unsigned long framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags,
            void* pUserData) {
    /* Cast data passed through stream to our structure. */
    if (gPaused) { return paContinue; }
    
    if (gpPlaying == 0) { gpPlaying = (u8*)pUserData; }
    s16* pOut = (s16*)outputBuffer;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    u32 byteCountToSend = gAudioBlockAlign*SAMPLE_RATE;
    
    if (byteCountToSend > gMusicBytesLeft) {
        byteCountToSend = gMusicBytesLeft;
    }
    gMusicBytesLeft -= byteCountToSend;
    
    memcpy(outputBuffer, gpPlaying, byteCountToSend);
    
    for (u32 i = 0; i < SAMPLE_RATE; ++i) {
        pTBuffer[i] = (f32)(pOut[2*i] + pOut[2*i + 1]);
    }
    
#if 1
    // remove_dc
    float avg = 0;
    for (u32 i = 0; i < SAMPLE_RATE; ++i)  avg += pTBuffer[i];
    avg /= SAMPLE_RATE;
    kiss_fft_scalar scalarAVG = (kiss_fft_scalar)avg;
    for (u32 i = 0; i < SAMPLE_RATE; ++i)  pTBuffer[i] -= scalarAVG;
#endif
    
    s32 a = HALF_SAMPLE_RATE;
    
    kiss_fftr(gMycfg, pTBuffer, pFBuffer);
    
    for (u32 i = 0; i < HALF_SAMPLE_RATE; ++i) {
        gpFFTMod[i] = sqrt(pFBuffer[i].r*pFBuffer[i].r + pFBuffer[i].i*pFBuffer[i].i);
    }
    
    if (byteCountToSend < gMusicBytesLeft) {
        gpPlaying += byteCountToSend;
        return paContinue;
    } else {
        return paComplete;
    }
}

#if 0
int
fftCallback(const void* inputBuffer, void* outputBuffer,
            unsigned long framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags,
            void* pUserData) {
    /* Cast data passed through stream to our structure. */
    if (gPaused) { return paContinue; }
    
    if (gpPlaying == 0) { gpPlaying = (u8*)pUserData; }
    s16* out = (s16*)outputBuffer;
    (void) inputBuffer; /* Prevent unused variable warning. */
    
    u32 byteCountToSend = gAudioBlockAlign*SAMPLE_RATE;
    
    u32 freqCountOutput = HALF_SAMPLE_RATE;
    u32 inputCount = SAMPLE_RATE;
    if (byteCountToSend > gMusicBytesLeft) {
        byteCountToSend = gMusicBytesLeft;
        inputCount = gMusicBytesLeft;
        freqCountOutput = gMusicBytesLeft/2;
#if 0
        free(pINBuffer);
        free(pTBuffer);
        free(pFBuffer);
        gMycfg = kiss_fftr_alloc(gMusicBytesLeft/2, 0, NULL, NULL);
        pINBuffer = (s16*)malloc(sizeof(s16)*gMusicBytesLeft/2);
        pTBuffer = (kiss_fft_scalar*)malloc(sizeof(kiss_fft_scalar)*SAMPLE_RATE);
        pFBuffer = (kiss_fft_cpx*)malloc(HALF_SAMPLE_RATE*sizeof(kiss_fft_cpx));
#endif
    }
    gMusicBytesLeft -= byteCountToSend;
    
    memcpy(outputBuffer, gpPlaying, byteCountToSend/4);
    
    for (u32 i = 0; i < freqCountOutput; ++i) {
        pTBuffer[i] = (f32)(gpPlaying[2*i] + gpPlaying[2*i + 1]);
    }
    
#if 1
    // remove_dc
    float avg = 0;
    for (u32 i = 0; i < inputBuffer; ++i)  avg += pTBuffer[i];
    avg /= SAMPLE_RATE;
    for (u32 i = 0; i < inputBuffer; ++i)  pTBuffer[i] -= (kiss_fft_scalar)avg;
#endif
    
    kiss_fftr(gMycfg, pTBuffer, pFBuffer);
    
    for (u32 i = 0; i < HALF_SAMPLE_RATE; ++i) {
        gpFFTMod[i] = pFBuffer[i].r*pFBuffer[i].r + pFBuffer[i].i*pFBuffer[i].i;
    }
    
    if (byteCountToSend < gMusicBytesLeft) {
        gpPlaying += byteCountToSend;
        return paContinue;
    } else {
        return paComplete;
    }
}

#endif
#if 0
int
sinCallback(const void* inputBuffer, void* outputBuffer,
            unsigned long framesPerBuffer,
            const PaStreamCallbackTimeInfo* timeInfo,
            PaStreamCallbackFlags statusFlags,
            void* pUserData) {
    (u8*)pUserData;
    f32* out = (f32*)outputBuffer;
    (void) inputBuffer; /* Prevent unused variable warning. */
    local_persist f32 wave_period = 440.f/44100.f;
    
    kiss_fft_cpx buffer[SAMPLE_RATE] = {};
    for (u32 i = 0; i < SAMPLE_RATE; ++i) {
        buffer[i].r = sinf(2*PI32*value)*gVolume;
        *out++ = buffer[i].r;
        value += wave_period;
        if (value >= 1.f) {
            value = 1.f;
            wave_period = -wave_period;
        }
    }
    
    kiss_fft_cpx output[SAMPLE_RATE];
    
    kiss_fftr(gMycfg, buffer, output);
    f32 mod[SAMPLE_RATE] = {};
    for (u32 i = 0; i < SAMPLE_RATE; ++i) {
        mod[i] = output[i].r*output[i].r + output[i].i*output[i].i;
    }
    
    return paContinue;
}
#endif

f32*
getFFTModResult() {
    return gpFFTMod;
}

u32
getFFTModSize() {
    return HALF_SAMPLE_RATE;
}

void
setMusicVisualizer(MusicVisualizerConfig* pMusicVisualizerConfig) {
    gpMusicVisualizerConfig = pMusicVisualizerConfig;
}