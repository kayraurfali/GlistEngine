///
/// @file gSound.h
/// @copyright Copyright (C) 2024, Nitra Oyun Yazılım Ltd. Sti. (Gamelab Istanbul)
///

#ifndef MEDIA_G_SOUND_H_
#define MEDIA_G_SOUND_H_

extern "C"
{
#	include <portaudio.h>
}
#include <cstddef>

#include "gMedia.h"

class gSound : public gMedia
{
public:

		struct paTestData
	{
		float left_phase{0.f};
		float right_phase{0.f};
	};
    gSound();
	virtual ~gSound();

	bool load(const std::string& t_fullPath);
	bool loadSound(const std::string& t_soundPath);
    bool loadRaw(uint8_t const* t_rawBytes);

	void play() override;
	void stop() override;
	void close() override;
private:
    void initialize();
    void shutdown();

	paTestData test_data{};

	PaStream *stream;
};

#endif // MEDIA_G_SOUND_H_
