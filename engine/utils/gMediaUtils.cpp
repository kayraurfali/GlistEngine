///
/// @file gMediaUtils.cpp
/// @copyright Copyright (C) 2024, Nitra Oyun Yazılım Ltd. Sti. (Gamelab Istanbul)
///

#include "gMediaUtils.h"

extern "C"
{
#include <libavutil/pixdesc.h>
}

bool gMediaUtils::load(std::string const& t_fullPath, gMedia* t_media) {
	t_media->streamindices[gMedia::STREAM_VIDEO] = -1;
	t_media->streamindices[gMedia::STREAM_AUDIO] = -1;
	t_media->streamindices[gMedia::STREAM_SUBTITLE] = -1;

	t_media->formatcontext = avformat_alloc_context();
    if (nullptr == t_media->formatcontext)
	{
		gLoge("gMediaUtils::loadMedia") << "Couldn't create create format context.";
		avformat_free_context(t_media->formatcontext);

		t_media->iscreated = false;
		return false;
	}

	int averror = avformat_open_input(&t_media->formatcontext, t_fullPath.c_str(), nullptr, nullptr);
    if(averror < 0)
	{
		gLoge("gMediaUtils::loadMedia") << "Error executing `avformat_open_input`: " << av_err2str(averror);

		avformat_close_input(&t_media->formatcontext);

		t_media->iscreated = false;
		return false;
	}

	averror = avformat_find_stream_info(t_media->formatcontext, nullptr);
	if (averror < 0) {
        gLoge("gMediaUtils::loadMedia") << "Error executing `avformat_find_stream_info`: " << av_err2str(averror);

		avformat_close_input(&t_media->formatcontext);

		t_media->iscreated = false;
		return false;
    }

	for (int i = 0; i < t_media->formatcontext->nb_streams; ++i) {
		auto* stream = t_media->formatcontext->streams[i];

		auto* codec_params = stream->codecpar;

		if (nullptr == codec_params) {
            gLoge("gMediaUtils::loadMedia") << "Stream " << i << " has null codec parameters.";
            continue;
        }


		if (t_media->streamindices[gMedia::STREAM_VIDEO] == -1 && codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
			t_media->streamindices[gMedia::STREAM_VIDEO] = i;

			t_media->videocodecparameters = codec_params;
			t_media->videocodec = avcodec_find_decoder(codec_params->codec_id);

			gLogd("gMediaUtils::loadMedia") << "Stream " << i << ": codec_id = "
								<< avcodec_get_name(codec_params->codec_id);

			t_media->height = t_media->videocodecparameters->height;
			t_media->width = t_media->videocodecparameters->width;


			t_media->framecount = stream->nb_frames;
			t_media->duration   = stream->duration;
			t_media->timebase   = stream->time_base;

			int64_t duration       = t_media->formatcontext->duration;
			AVRational frame_rate  = av_guess_frame_rate(t_media->formatcontext, stream, nullptr);;
			t_media->avgfps          = frame_rate.num / frame_rate.den;

			continue;
		}

		if(t_media->streamindices[gMedia::STREAM_AUDIO] == -1 && codec_params->codec_type == AVMEDIA_TYPE_AUDIO) {
			t_media->streamindices[gMedia::STREAM_AUDIO] = i;
			gLogd("gMediaUtils::loadMedia")
					<< "Stream " << i << ": codec_id = "
					<< avcodec_get_name(codec_params->codec_id);
			t_media->audiocodecparameters = codec_params;
			t_media->audiocodec = avcodec_find_decoder(codec_params->codec_id);;
			t_media->audiochannelsnum = t_media->audiocodecparameters->ch_layout.nb_channels;
			t_media->audiosamplerate = t_media->audiocodecparameters->sample_rate;
			continue;
		}

	}

	if (t_media->streamindices[gMedia::STREAM_VIDEO] == -1 ) {

		if (t_media->streamindices[gMedia::STREAM_AUDIO] == -1)
		{
			gLoge("gMediaUtils::loadMedia") << "No valid streams from the file could be found.";

			t_media->iscreated = false;
			return false;
		}

		bool result = initializeSound(t_media);
		if (!result)
		{
			return false;
		}
	}
	else
	{
		bool audio_found = t_media->streamindices[gMedia::STREAM_AUDIO] != -1;
		if (!audio_found)
		{
			gLogw("gMediaUtils::loadMedia") << "No AUDIO stream could be found in the file.";
		}

		bool result = initializeVideo(t_media);
		if (audio_found)
		{
			result &= initializeSound(t_media);
		}

		if (!result)
		{
			return false;
		}
	}

	t_media->currentpacket = av_packet_alloc();
	if (nullptr == t_media->currentpacket)
	{
		gLoge("gMediaUtils::loadMedia") << "Could not allocate AVPacket!";
		t_media->iscreated = false;
		return false;
	}

	t_media->iscreated = true;
    return true;
}

bool gMediaUtils::loadVideo(std::string const& t_fileName, gMedia* t_media) {
	return load(gGetVideosDir() + t_fileName, t_media);
}

bool gMediaUtils::loadSound(std::string const& t_fileName, gMedia* t_media) {
	return load(gGetSoundsDir() + t_fileName, t_media);
}

void gMediaUtils::resetMedia(gMedia* t_media) {
	avformat_close_input(&t_media->formatcontext);
    avformat_free_context(t_media->formatcontext);
    av_frame_free(&t_media->videoframe);
    av_frame_free(&t_media->audioframe);
    av_packet_free(&t_media->currentpacket);
    avcodec_free_context(&t_media->videocodeccontext);
    avcodec_free_context(&t_media->audiocodeccontext);
	sws_freeContext(t_media->swscontext);

	t_media->videoframepixeldata.reset();
}

bool gMediaUtils::seek(float t_timeStampInSeconds, gMedia* t_media) {
	auto fmtcontext = t_media->formatcontext;
	auto videostream = t_media->formatcontext->streams[t_media->streamindices[gMedia::STREAM_VIDEO]];
	auto audiostream = t_media->formatcontext->streams[t_media->streamindices[gMedia::STREAM_AUDIO]];

	int64_t timestamptimeshundred = static_cast<int64_t>(t_timeStampInSeconds * 100);

	int64_t seekTarget = av_rescale(timestamptimeshundred, videostream->time_base.den, videostream->time_base.num) / 100;

	//int avresult = avformat_seek_file(fmtcontext, t_media->streamindices[STREAM_VIDEO], seekTarget - videostream->time_base.den, seekTarget, seekTarget, 0);
	int avresult = av_seek_frame(fmtcontext, t_media->streamindices[gMedia::STREAM_VIDEO], seekTarget, 0);
	if(avresult < 0)
	{
		gLoge("gSeekToFrame") << "Error when seeking to time: " << t_timeStampInSeconds << " " << av_err2str(avresult);
	}

	avcodec_flush_buffers(t_media->videocodeccontext);
	if(avresult < 0)
	{
		gLoge("gSeekToFrame") << "Error when flushing codec context buffers " << av_err2str(avresult);
	}
	// g_framesBuffer->popAll();

	av_frame_unref(t_media->videoframe);
	av_frame_unref(t_media->audioframe);
	av_packet_unref(t_media->currentpacket);

	t_media->framesprocessed = 0;
	t_media->readytoplay = false;

	return true;
}

bool gMediaUtils::initializeVideo(gMedia* t_media) {
	// Initialize codec context for the video
    t_media->videocodeccontext = avcodec_alloc_context3(t_media->videocodec);
    if (nullptr == t_media->videocodeccontext)
	{
		gLoge("gMediaUtils::loadMedia")
			<< "Could not create AVCodecContext for the found VIDEO codec :"
			<< avcodec_get_name(t_media->videocodec->id) ;
		t_media->iscreated = false;
		return false;
	}

	// Turn video codec parameters to codec context
	int averror = avcodec_parameters_to_context(t_media->videocodeccontext, t_media->videocodecparameters);
    if (averror < 0)
	{
		gLoge("gMediaUtils::loadMedia")
			<< "Error when executing: avcodec_parameters_to_context"
			<< av_err2str(averror);
		t_media->iscreated = false;
		return false;
	}

    // Initialize the video codec context to use the codec for the video
	averror = avcodec_open2(t_media->videocodeccontext, t_media->videocodec, nullptr);
	if (averror < 0)
	{
		gLoge("gMediaUtils::loadMedia")
			<< "Error executing `avcodec_open2`: "
			<< av_err2str(averror);
		t_media->iscreated = false;
		return false;
	}

	t_media->videoframe = av_frame_alloc();
	if (nullptr == t_media->videoframe)
	{
		gLoge("gMediaUtils::loadMedia") << "Could not allocate the AUDIO frame!";
		t_media->iscreated = false;
		return false;
	}

	auto correctedpixfmt = getCorrectedPixelFormat(t_media->videocodeccontext->pix_fmt);
	t_media->numpixelcomponents = av_pix_fmt_count_planes(correctedpixfmt);
	t_media->swscontext = sws_getContext(
			t_media->width, t_media->height, correctedpixfmt,
			t_media->width, t_media->height, AV_PIX_FMT_RGBA,
			SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
	);

	if (nullptr == t_media->swscontext)
	{
		gLoge("gMediaUtils::loadMedia") << "Couldn't initialize SwsContext";
		t_media->iscreated = false;
		return false;
	}

	return true;
}

bool gMediaUtils::initializeSound(gMedia* t_media) {
	if(t_media->audiochannelsnum != 2) {
 		gLogd("gMediaUtils::loadMedia") << "Only stereo audio found in stream.";
	}

	// Initialize codec context for the audio
    t_media->audiocodeccontext = avcodec_alloc_context3(t_media->audiocodec);
    if (nullptr == t_media->audiocodeccontext)
	{
		gLoge("gMediaUtils::loadMedia")
			<< "Could not create AVCodecContext for the audio.";
		t_media->iscreated = false;
		return false;
	}

	// Turn audio codec parameters to codec context
	int averror = avcodec_parameters_to_context(t_media->audiocodeccontext, t_media->audiocodecparameters);
   	if (averror < 0)
	{
		gLoge("gMediaUtils::loadMedia")
			<< "Error executing `avcodec_parameters_to_context`: "
			<< av_err2str(averror);
		t_media->iscreated = false;
		return false;
	}

	// Initialize the audio codec context to use the codec for the audio
	averror = avcodec_open2(t_media->audiocodeccontext, t_media->audiocodec, nullptr);
	if (averror < 0)
	{
		gLoge("gMediaUtils::loadMedia")
			<< "Error executing `avcodec_open2`: "
			<< av_err2str(averror);
		t_media->iscreated = false;
		return false;
	}

	t_media->audioframe = av_frame_alloc();
	if (nullptr == t_media->audioframe)
	{
		gLoge("gMediaUtils::loadMedia") << "Could not allocate the AUDIO frame!";
		t_media->iscreated = false;
		return false;
	}

	return true;
}

AVPixelFormat gMediaUtils::getCorrectedPixelFormat(AVPixelFormat t_pixelFormat) {
	// Fix swscaler deprecated pixel format warning
    // (YUVJ has been deprecated, change pixel format to regular YUV)
    switch (t_pixelFormat) {
        case AV_PIX_FMT_YUVJ420P: return AV_PIX_FMT_YUV420P;
        case AV_PIX_FMT_YUVJ422P: return AV_PIX_FMT_YUV422P;
        case AV_PIX_FMT_YUVJ444P: return AV_PIX_FMT_YUV444P;
        case AV_PIX_FMT_YUVJ440P: return AV_PIX_FMT_YUV440P;
        default:                  return t_pixelFormat;
    }
}
