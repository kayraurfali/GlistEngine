///
/// @file gSound.h
/// @copyright Copyright (C) 2024, Nitra Oyun Yazılım Ltd. Sti. (Gamelab Istanbul)
///

#include "gSound.h"

#include "gMedia.h"
#include "gMediaUtils.h"

/*
 * This routine will be called by the PortAudio engine when audio is needed.
 * It may called at interrupt level on some machines so don't do anything
 * that could mess up the system like calling malloc() or free().
*/

static int PaCallback(void const* inputBuffer, void* outputBuffer,
                      unsigned long framesPerBuffer,
                      PaStreamCallbackTimeInfo const* timeInfo,
                      PaStreamCallbackFlags statusFlags,
                      void* userData)
{
    /* Cast data passed through stream to our structure. */
    gSound::paTestData *data = (gSound::paTestData*)userData;
    float* out = (float*)outputBuffer;
    unsigned int i;
    (void) inputBuffer; /* Prevent unused variable warning. */

    for( i=0; i<framesPerBuffer; i++ )
    {
        *out = data->left_phase;    ++out;
        *out = data->right_phase; ++out;
        /* Generate simple sawtooth phaser that ranges between -1.0 and 1.0. */
        data->left_phase += 0.01f;
        /* When signal reaches top, drop back down. */
        if( data->left_phase >= 1.0f ) data->left_phase -= 2.0f;
        /* higher pitch so we can distinguish left and right. */
        data->right_phase += 0.03f;
        if( data->right_phase >= 1.0f ) data->right_phase -= 2.0f;
    }
    return 0;
}

gSound::gSound() : gMedia{gMediaType::MEDIATYPE_SOUND}{
    initialize();
}

gSound::~gSound() {
    shutdown();
}

bool gSound::load(const std::string& t_fullPath) {
    //gMediaUtils::load(t_fullPath, this);

    //PaError err;
    //gLogi("Skibidi Toilet");

    //err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, 44100, 256, PaCallback, &test_data);
}

bool gSound::loadSound(const std::string& t_soundPath) {
	return load(gGetSoundsDir() + t_soundPath);
}

bool gSound::loadRaw(uint8_t const* t_rawBytes) {
	return false;
}

void gSound::play() {
    Pa_StartStream( stream );
    Pa_Sleep(4*1000);
    Pa_StopStream( stream );
}

void gSound::stop() {
    Pa_StopStream( stream );
}

void gSound::close() {
    Pa_CloseStream( stream );
}

void gSound::initialize() {
    PaError pa_result = Pa_Initialize();
    if (pa_result != PaErrorCode::paNoError)
    {
        gLoge("gSound::initialize") << "Failed to initialize PortAudio: " << Pa_GetErrorText(pa_result);
        shutdown();
        return;
    }
}

void gSound::shutdown() {
    PaError pa_result = Pa_Terminate();
    if (pa_result != PaErrorCode::paNoError)
    {
        gLoge("gSound::initialize") << "Failed to terminate PortAudio: " << Pa_GetErrorText(pa_result);
    }
}
