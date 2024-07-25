///
/// @file gMedia.h
/// @copyright Copyright (C) 2024, Nitra Oyun Yazılım Ltd. Sti. (Gamelab Istanbul)
///

#ifndef MEDIA_G_MEDIA_H_
#define MEDIA_G_MEDIA_H_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libswscale/swscale.h>
}

#include <array>
#include <cstdint>
#include <memory>
#include <utility>
#include <string>

#include "gObject.h"

enum class gMediaFrameType : uint32_t {
    FRAMETYPE_VIDEO
  , FRAMETYPE_AUDIO
  , FRAMETYPE_NONE
};

enum class gMediaType : uint32_t {
    MEDIATYPE_VIDEO
  , MEDIATYPE_SOUND
};

class gMedia : public gObject {
public:
    using gPixelArray = std::unique_ptr<uint8_t[]>;

    static constexpr int LOOPTYPE_DEFAULT = 0;
    static constexpr int LOOPTYPE_NONE    = 1;
    static constexpr int LOOPTYPE_NORMAL  = 2;

    static constexpr int STREAM_VIDEO    = 0;
    static constexpr int STREAM_AUDIO    = 1;
    static constexpr int STREAM_SUBTITLE = 2;

    gMedia(gMediaType t_type);
    virtual ~gMedia();

    //virtual bool load(const std::string& t_fulPath) = 0;
    virtual void play() = 0;
    virtual void stop() = 0;
    virtual void close() = 0;

    bool isLoaded();
    bool isPlaying();
    void setPaused(bool t_isPaused);
    bool isPaused();

    float getDuration();

    void  setPosition(float t_position);
    float getPosition();

    void  setLoopType(int t_loopType);
    int   getLoopType();

    void  setVolume(float t_volume);
    float getVolume();

protected:
    friend class gMediaUtils;

    gMediaType type;

    bool iscreated{false};
    bool readytoplay{false};
    bool isplaying{false};
    bool ispaused{false};
    bool isfinished{false};

    float currentposition{0.0f};
    float duration{0.0f};

    int32_t looptype{};

    AVFormatContext* formatcontext{};
    AVPacket*        currentpacket{};
    AVRational       timebase{};

    // Video, Audio, Subtitle
    std::array<int, 3> streamindices;

    //	Video
    AVCodecParameters* videocodecparameters{};
    AVCodecContext*    videocodeccontext{};
    AVCodec const*     videocodec{};
    AVFrame*           videoframe{};
    SwsContext*        swscontext{};
    int32_t            width{};
    int32_t            height{};
    int64_t            framecount{};
    int64_t            framesprocessed{};
    int32_t            numpixelcomponents;
    int64_t            avgfps{};
    gPixelArray        videoframepixeldata{};

    //	Audio
    AVCodecParameters* audiocodecparameters{};
    AVCodecContext*    audiocodeccontext{};
    AVCodec const*     audiocodec{};
    AVFrame*           audioframe{};
    int32_t            audiochannelsnum{};
    int32_t            audiosamplerate{};
    int32_t            audiovolume{};

    gMediaFrameType lastreceivedframetype;
};

#endif // MEDIA_G_MEDIA_H_
