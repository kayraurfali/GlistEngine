///
/// @file gMediaUtils.h
/// @copyright Copyright (C) 2024, Nitra Oyun Yazılım Ltd. Sti. (Gamelab Istanbul)
///

#include "gObject.h"
#include "gMedia.h"

class gMediaUtils : public gObject
{
public:
    static bool load(std::string const& t_fullPath, gMedia* t_media);
    static bool loadVideo(std::string const& t_filename, gMedia* t_media);
    static bool loadSound(std::string const& t_filename, gMedia* t_media);

    static void resetMedia(gMedia* t_media);

    static bool seek(float t_timeStampInSeconds, gMedia* t_media);
private:
    static bool initializeVideo(gMedia* t_media);
    static bool initializeSound(gMedia* t_media);

    static AVPixelFormat getCorrectedPixelFormat(AVPixelFormat t_pixelFormat);
};