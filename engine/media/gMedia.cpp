///
/// @file gMedia.cpp
/// @copyright Copyright (C) 2024, Nitra Oyun Yazılım Ltd. Sti. (Gamelab Istanbul)
///

#include "gMedia.h"

#include "gMediaUtils.h"

gMedia::gMedia(gMediaType t_type) : type(t_type) {
}

gMedia::~gMedia() {
}

bool gMedia::isLoaded() {
	return iscreated;
}

bool gMedia::isPlaying() {
	return isplaying;
}

void gMedia::setPaused(bool t_isPaused) {
	ispaused = t_isPaused;
}

bool gMedia::isPaused() {
	return ispaused;
}

float gMedia::getDuration() {
	return duration;
}

void gMedia::setPosition(float t_position) {
	gMediaUtils::seek(t_position, this);
}

float gMedia::getPosition() {
	return currentposition;
}

void gMedia::setLoopType(int t_loopType) {
	looptype = t_loopType;
}

int gMedia::getLoopType() {
	return looptype;
}

void gMedia::setVolume(float t_volume) {
	audiovolume = t_volume;
}

float gMedia::getVolume() {
	return audiovolume;
}
