/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "VideoReader.h"

//#define VIDEO_EXTRALOGGING

#undef min
#include <algorithm>
#include <wx/filename.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/pixdesc.h>
#include <libavutil/hwcontext.h>
#ifdef __WXMSW__
#include <libavutil/hwcontext_dxva2.h>
#include <libavcodec/dxva2.h>
#include <libavutil/imgutils.h>
#endif
}

#ifdef __WXMSW__
#include <d3dcompiler.h>
#endif

#include "SpecialOptions.h"

#include <log4cpp/Category.hh>

#ifdef __WXOSX__
extern void InitVideoToolboxAcceleration();
extern bool SetupVideoToolboxAcceleration(AVCodecContext *s, bool enabled);
extern void CleanupVideoToolbox(AVCodecContext *s, void * cache);
extern bool VideoToolboxScaleImage(AVCodecContext *codecContext, AVFrame *frame, AVFrame *dstFrame, void *& cache);
extern bool IsVideoToolboxAcceleratedFrame(AVFrame *frame);
#else
extern void InitVideoToolboxAcceleration() {}
static inline bool SetupVideoToolboxAcceleration(AVCodecContext *s, bool enabled) { return false; }
static inline void CleanupVideoToolbox(AVCodecContext *s, void * cache) {}
static inline bool VideoToolboxScaleImage(AVCodecContext *codecContext, AVFrame *frame, AVFrame *dstFrame, void *& cache) { return false; }
static inline bool IsVideoToolboxAcceleratedFrame(AVFrame *frame) { return false; }
#endif

static enum AVPixelFormat __hw_pix_fmt = AV_PIX_FMT_NONE;
static enum AVPixelFormat get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts)
{
    const enum AVPixelFormat* p;
    for (p = pix_fmts; *p != -1; p++) {
        if (*p == __hw_pix_fmt)
        {
            return *p;
        }
    }
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.error("Failed to get HW surface format. This is bad - we will have to abandon video read. Suggest you turn off hardware video decoding or force change the device.");
    logger_base.error("   Looking for %s but only found:", av_get_pix_fmt_name(__hw_pix_fmt));
    for (p = pix_fmts; *p != -1; p++) {
        logger_base.error("       %s", av_get_pix_fmt_name(*p));
    }

    return AV_PIX_FMT_NONE;
}

bool VideoReader::HW_ACCELERATION_ENABLED = false;

void VideoReader::SetHardwareAcceleratedVideo(bool accel)
{
#ifdef __LINUX__
    HW_ACCELERATION_ENABLED = false;
#else
    HW_ACCELERATION_ENABLED = accel;
#endif
}


void VideoReader::InitHWAcceleration() {
    InitVideoToolboxAcceleration();
}

VideoReader::VideoReader(const std::string& filename, int maxwidth, int maxheight, bool keepaspectratio, bool usenativeresolution/*false*/,
                         bool wantAlpha, bool bgr, bool wantsHWType)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    _wantsHWType = wantsHWType;
    _maxwidth = maxwidth;
    _maxheight = maxheight;
    _filename = filename;
    _valid = false;
	_lengthMS = 0.0;
	_formatContext = nullptr;
	_codecContext = nullptr;
	_videoStream = nullptr;
	_dstFrame = nullptr;
    _dstFrame2 = nullptr;
    _srcFrame = nullptr;
    _curPos = -1000;
    _wantAlpha = wantAlpha;
    _videoToolboxAccelerated = false;
    if (_wantAlpha) {
        _pixelFmt = bgr ? AVPixelFormat::AV_PIX_FMT_BGRA : AVPixelFormat::AV_PIX_FMT_RGBA;
    } else {
        _pixelFmt = bgr ? AVPixelFormat::AV_PIX_FMT_BGR24 : AVPixelFormat::AV_PIX_FMT_RGB24;
    }
	_atEnd = false;
	_swsCtx = nullptr;
    _dtspersec = 1.0;
    _frames = 0;

    #if LIBAVFORMAT_VERSION_MAJOR < 58
    av_register_all();
    #endif

	int res = avformat_open_input(&_formatContext, filename.c_str(), nullptr, nullptr);
	if (res != 0) {
        logger_base.error("Error opening the file " + filename);
		return;
	}

	if (avformat_find_stream_info(_formatContext, nullptr) < 0) {
        logger_base.error("VideoReader: Error finding the stream info in " + filename);
		return;
	}

	// Find the video stream
	_streamIndex = av_find_best_stream(_formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &_decoder, 0);
	if (_streamIndex < 0) {
        logger_base.error("VideoReader: Could not find any video stream in " + filename);
		return;
	}

	_videoStream = _formatContext->streams[_streamIndex];
    _videoStream->discard = AVDISCARD_NONE;

    _width = _maxwidth;
    _height = _maxheight;

    reopenContext();

    if (_codecContext == nullptr)
    {
        return;
    }

	// at this point it is open and ready
   if ( usenativeresolution )
   {
      _height = _codecContext->height;
      _width = _codecContext->width;
   }
   else
   {
      if ( keepaspectratio )
      {
         if ( _codecContext->width == 0 || _codecContext->height == 0 )
         {
            logger_base.error( "VideoReader: Invalid input reader dimensions (%d,%d) %s", _codecContext->width, _codecContext->height, (const char *)filename.c_str() );
            return;
         }

         // if > 0 then video will be shrunk
         // if < 0 then video will be stretched
         float shrink = std::min( (float)maxwidth / (float)_codecContext->width, (float)maxheight / (float)_codecContext->height );
         _height = (int)( (float)_codecContext->height * shrink );
         _width = (int)( (float)_codecContext->width * shrink );
      }
      else
      {
         _height = maxheight;
         _width = maxwidth;
      }
   }

	// get the video length in MS
	// Use the number of frames as the best possible way to calculate length
	_frames = (long)_videoStream->nb_frames;
    if (_videoStream->time_base.num != 0)
    {
        _dtspersec = (double)_videoStream->time_base.den / (double)_videoStream->time_base.num;
    }
    else
    {
        if (_frames == 0 || _videoStream->avg_frame_rate.den == 0)
        {
            logger_base.warn("VideoReader: dtspersec calc error _videoStream->nb_frames %d and _videoStream->avg_frame_rate.den %d cannot be zero. %s", (int)_videoStream->nb_frames, (int)_videoStream->avg_frame_rate.den, (const char *)filename.c_str());
            logger_base.warn("VideoReader: Video seeking will only work back to the start of the video.");
            _dtspersec = 1.0;
        }
        else
        {
            _dtspersec = (((double)_videoStream->duration * (double)_videoStream->avg_frame_rate.num) / ((double)_frames * (double)_videoStream->avg_frame_rate.den));
        }
    }

    if (_frames > 0)
	{
        if (_videoStream->avg_frame_rate.num != 0)
        {
            _lengthMS = ((double)_frames * (double)_videoStream->avg_frame_rate.den * 1000.0) / (double)_videoStream->avg_frame_rate.num;
        }
        else
        {
            logger_base.info("VideoReader: _videoStream->avg_frame_rate.num = 0");
        }
    }

	// If it does not look right try to base if off the duration
	if (_lengthMS <= 0 || _frames <= 0)
	{
        if (_videoStream->avg_frame_rate.den != 0)
        {
            _lengthMS = (double)_formatContext->duration / 1000.0;
            _frames = (long)(_lengthMS  * (double)_videoStream->avg_frame_rate.num / (double)(_videoStream->avg_frame_rate.den) / 1000.0);
        }
        else
        {
            logger_base.info("VideoReader: _videoStream->avg_frame_rate.den = 0");
        }
    }

	// If it still doesnt look right
	if (_lengthMS <= 0 || _frames <= 0)
	{
        if (_videoStream->avg_frame_rate.den != 0)
        {
            // This seems to work for .asf, .mkv, .flv
            _lengthMS = (double)_formatContext->duration / 1000.0;
            _frames = (long)(_lengthMS  * (double)_videoStream->avg_frame_rate.num / (double)(_videoStream->avg_frame_rate.den) / 1000.0);
        }
    }

	if (_lengthMS <= 0 || _frames <= 0)
	{
		// This is bad ... it still does not look right
        logger_base.warn("Attempts to determine length of video have not been successful. Problems ahead.");
	}

    // Guess the keyframe frequency
    _keyFrameCount = _codecContext->keyint_min;

	_dstFrame = av_frame_alloc();
	_dstFrame->width = _width;
	_dstFrame->height = _height;
	_dstFrame->linesize[0] = _width * GetPixelChannels();
	_dstFrame->data[0] = (uint8_t *)av_malloc(_width * _height * GetPixelChannels() * sizeof(uint8_t));
    _dstFrame->format = _pixelFmt;
    _dstFrame2 = av_frame_alloc();
    _dstFrame2->width = _width;
    _dstFrame2->height = _height;
    _dstFrame2->linesize[0] = _width * GetPixelChannels();
    _dstFrame2->data[0] = (uint8_t *)av_malloc(_width * _height * GetPixelChannels() * sizeof(uint8_t));
    _dstFrame2->format = _pixelFmt;

    _srcFrame = av_frame_alloc();
    _srcFrame2 = av_frame_alloc();

    av_init_packet(&_packet);
	_valid = true;

    logger_base.info("Video loaded: " + filename);
    logger_base.info("      Length MS: %.2f", _lengthMS);
    logger_base.info("      _videoStream->time_base.num: %d", _videoStream->time_base.num);
    logger_base.info("      _videoStream->time_base.den: %d", _videoStream->time_base.den);
    logger_base.info("      _videoStream->avg_frame_rate.num: %d", _videoStream->avg_frame_rate.num);
    logger_base.info("      _videoStream->avg_frame_rate.den: %d", _videoStream->avg_frame_rate.den);
    logger_base.info("      DTS per sec: %f", _dtspersec);
    logger_base.info("      _videoStream->nb_frames: %d", _videoStream->nb_frames);
    logger_base.info("      _frames: %d", _frames);
    logger_base.info("      Frames per second %.2f", (double)_frames * 1000.0 / _lengthMS);
    logger_base.info("      Source size: %dx%d", _codecContext->width, _codecContext->height);
    logger_base.info("      Source coded size: %dx%d", _codecContext->coded_width, _codecContext->coded_height);
    logger_base.info("      Output size: %dx%d", _width, _height);
    logger_base.info("      Guessed key frame frequency: %d", _keyFrameCount);
    if (_wantAlpha)
        logger_base.info("      Alpha: TRUE");
    if (_frames != 0)
    {
        logger_base.info("      Frame ms %f", _lengthMS / (double)_frames);
        _frameMS = _lengthMS / _frames;
        logger_base.info("      Used frame ms %d", _frameMS);
    }
    else
    {
        logger_base.warn("      Frame ms <unknown as _frames is 0>");
        _frameMS = 0;
    }
    
    _firstFramePos = -1;
}

void VideoReader::reopenContext(bool allowHWDecoder) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_codecContext != nullptr) {
        CleanupVideoToolbox(_codecContext, hwDecoderCache);
        hwDecoderCache = nullptr;
        avcodec_close(_codecContext);
        _codecContext = nullptr;
    }

    #if LIBAVFORMAT_VERSION_MAJOR > 57
    enum AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
    if (allowHWDecoder && IsHardwareAcceleratedVideo()) {
#if defined(__WXMSW__)
        std::list<std::string> hwdecoders = { "dxva2" }; //{ "d3d11va", "dxva2", "cuda", "qsv" };
#elif defined(__WXOSX__)
        std::list<std::string> hwdecoders = { "videotoolbox" };
#else
        std::list<std::string> hwdecoders = { "vaapi", "vdpau" };
#endif

        for (const auto& it : hwdecoders) {
            type = av_hwdevice_find_type_by_name(it.c_str());
            if (type != AV_HWDEVICE_TYPE_NONE) {
                break;
            }
        }
        if (type != AV_HWDEVICE_TYPE_NONE) {
            for (int i = 0;; i++) {
                const AVCodecHWConfig* config = avcodec_get_hw_config(_decoder, i);
                if (!config) {
                    type = AV_HWDEVICE_TYPE_NONE;
                    break;
                }
                if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                    config->device_type == type) {
                    __hw_pix_fmt = config->pix_fmt;
                    break;
                }
            }
        }
    }
    #endif

    _codecContext = avcodec_alloc_context3(_decoder);
    if (!_codecContext) {
        logger_base.error("VideoReader: Failed to allocate codec context for %s", _filename.c_str());
        return;
    }

    _codecContext->thread_safe_callbacks = 1;
    _codecContext->thread_type = 0;
    _codecContext->thread_count = 1;
    _codecContext->skip_frame = AVDISCARD_NONE;
    _codecContext->skip_loop_filter = AVDISCARD_NONE;
    _codecContext->skip_idct = AVDISCARD_NONE;

    // Copy codec parameters from input stream to output codec context
    if (avcodec_parameters_to_context(_codecContext, _videoStream->codecpar) < 0) {
        logger_base.error("VideoReader: Failed to copy %s codec parameters to decoder context", _filename.c_str());
        return;
    }

    #if LIBAVFORMAT_VERSION_MAJOR > 57
    _codecContext->hwaccel_context = nullptr;
#ifdef __WXMSW__
    if (_codecContext->codec_id != AV_CODEC_ID_H264 && _codecContext->codec_id != AV_CODEC_ID_WMV3 && _codecContext->codec_id != AV_CODEC_ID_MPEG2VIDEO)
    {
        // dont enable hardware acceleration
        logger_base.debug("Hardware decoding disabled for codec '%s' going with software decoding.", _codecContext->codec->long_name);
    }
    else
#endif
    {
        if (IsHardwareAcceleratedVideo() && type != AV_HWDEVICE_TYPE_NONE)
        {
            const char* opt = nullptr;

#ifdef __WXMSW__
            std::string so;
            so = SpecialOptions::GetOption("dxva2_device", "0").c_str();
            opt = (const char*)so.c_str();

            int priorityFilter = wxAtoi(SpecialOptions::GetOption("dxva2_filter", "-1"));
            if (priorityFilter != -1) _dxva2_filters.push_front((D3DTEXTUREFILTERTYPE)priorityFilter);
#endif
            if (av_hwdevice_ctx_create(&_hw_device_ctx, type, opt, nullptr, 0) < 0)
            {
                logger_base.debug("Failed to create specified HW device.");
                type = AV_HWDEVICE_TYPE_NONE;
            }
            else
            {
                _codecContext->hw_device_ctx = av_buffer_ref(_hw_device_ctx);
                _codecContext->get_format = get_hw_format;
                logger_base.debug("Hardware decoding enabled for codec '%s'", _codecContext->codec->long_name);
            }
        }
        else
        {
            logger_base.debug("Software decoding enabled for codec '%s'", _codecContext->codec->long_name);
        }
    }
    #endif
    _videoToolboxAccelerated = SetupVideoToolboxAcceleration(_codecContext, HW_ACCELERATION_ENABLED && allowHWDecoder);

    //  Init the decoders, with or without reference counting
    AVDictionary *opts = nullptr;
    //av_dict_set(&opts, "refcounted_frames", "0", 0);
    if (avcodec_open2(_codecContext, _decoder, &opts) < 0) {
        logger_base.error("VideoReader: Couldn't open the context with the decoder in %s", _filename.c_str());
        return;
    }
}

static int64_t MStoDTS(int ms, double dtspersec)
{
    return (int64_t)(((double)ms * dtspersec) / 1000.0);
}

static int DTStoMS(int64_t dts , double dtspersec)
{
    if (dtspersec > 1000 && dtspersec < UINT_MAX) {
        int64_t dtsps = (int64_t)dtspersec;
        dts *= 1000;
        dts /= dtsps;
        return dts;
    }
    return (int)((1000.0 * (double)dts) / dtspersec);
}

int VideoReader::GetPos()
{
    return _curPos;
}

bool VideoReader::IsVideoFile(const std::string& filename)
{
    wxFileName fn(filename);
    auto ext = fn.GetExt().Lower().ToStdString();

    if (ext == "avi" ||
        ext == "mp4" ||
        ext == "mkv" ||
        ext == "mov" ||
        ext == "asf" ||
        ext == "flv" ||
        ext == "mpg" ||
        ext == "wmv" ||
        ext == "mpeg" ||
        ext == "m4v"
        )
    {
        return true;
    }

    return false;
}

long VideoReader::GetVideoLength(const std::string& filename)
{
    #if LIBAVFORMAT_VERSION_MAJOR < 58
    av_register_all();
    #endif

    AVFormatContext* formatContext = nullptr;
    int res = avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr);
    if (res != 0)
    {
        return 0;
    }

    if (avformat_find_stream_info(formatContext, nullptr) < 0)
    {
        avformat_close_input(&formatContext);
        return 0;
    }

    // Find the video stream
    AVCodec* cdc;
    int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &cdc, 0);
    if (streamIndex < 0)
    {
        avformat_close_input(&formatContext);
        return 0;
    }

    AVStream* videoStream = formatContext->streams[streamIndex];
    videoStream->discard = AVDISCARD_NONE;

    // get the video length in MS
    // Use the number of frames as the best possible way to calculate length
    long frames = (long)videoStream->nb_frames;
    long lengthMS = 0;

    if (frames > 0)
    {
        if (videoStream->avg_frame_rate.num != 0)
        {
            lengthMS = ((double)frames * (double)videoStream->avg_frame_rate.den * 1000.0) / (double)videoStream->avg_frame_rate.num;
        }
    }

    // If it does not look right try to base if off the duration
    if (lengthMS <= 0)
    {
        if (videoStream->avg_frame_rate.den != 0)
        {
            lengthMS = (double)formatContext->duration * (double)videoStream->avg_frame_rate.num / (double)videoStream->avg_frame_rate.den;
        }
    }

    // If it still doesnt look right
    if (lengthMS <= 0)
    {
        // This seems to work for .asf, .mkv, .flv
        lengthMS = (double)formatContext->duration / 1000.0;
    }

    avformat_close_input(&formatContext);

    return lengthMS;
}

VideoReader::~VideoReader()
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (_swsCtx != nullptr) {
        //logger_base.debug("Releasing sws Context.");
        sws_freeContext(_swsCtx);
        _swsCtx = nullptr;
    }
    if (_srcFrame != nullptr) {
        //logger_base.debug("Releasing srcFrame.");
        av_free(_srcFrame);
        _srcFrame = nullptr;
    }
    if (_srcFrame2 != nullptr) {
        //logger_base.debug("Releasing srcFrame2.");
        av_free(_srcFrame2);
        _srcFrame2 = nullptr;
    }
    if (_dstFrame != nullptr) {
        //logger_base.debug("Releasing dstFrame.");
        if (_dstFrame->data[0] != nullptr) {
            av_free(_dstFrame->data[0]);
        }
        av_free(_dstFrame);
        _dstFrame = nullptr;
    }
    if (_dstFrame2 != nullptr) {
        //logger_base.debug("Releasing dstFrame2.");
        if (_dstFrame2->data[0] != nullptr) {
            av_free(_dstFrame2->data[0]);
        }
        av_free(_dstFrame2);
        _dstFrame2 = nullptr;
    }
    if (_codecContext != nullptr) {

        if (_keyFrameCount != _codecContext->keyint_min)
        {
            logger_base.debug("Key frame count was adjusted from %d to %d.", _codecContext->keyint_min, _keyFrameCount);
        }

        //logger_base.debug("Releasing codecContext.");
        CleanupVideoToolbox(_codecContext, hwDecoderCache);
        hwDecoderCache = nullptr;
        avcodec_close(_codecContext);
		_codecContext = nullptr;
	}
	if (_formatContext != nullptr) {
        //logger_base.debug("Releasing formatContext.");
        avformat_close_input(&_formatContext);
		_formatContext = nullptr;
	}
    if (_hw_device_ctx != nullptr) {
        //logger_base.debug("Releasing hardware device context.");
        av_buffer_unref(&_hw_device_ctx);
        _hw_device_ctx = nullptr;
    }
}

void VideoReader::Seek(int timestampMS, bool readFrame)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // we have to be valid
	if (_valid) {
#ifdef VIDEO_EXTRALOGGING
        logger_base.info("VideoReader: Seeking to %d ms.", timestampMS);
#endif
        if (_atEnd && _videoToolboxAccelerated) {
            // once the end is reached, the hardware decoder is done
            // so we need to reopen it to be able continue decoding
            reopenContext();
        }

        if (timestampMS < _lengthMS) {
			_atEnd = false;
		} else {
			// dont seek past the end of the file
			_atEnd = true;
            avcodec_flush_buffers(_codecContext);
            av_seek_frame(_formatContext, _streamIndex, MStoDTS(_lengthMS, _dtspersec), AVSEEK_FLAG_FRAME);
            return;
		}

        avcodec_flush_buffers(_codecContext);

        if (timestampMS <= 0) {
            int f = av_seek_frame(_formatContext, _streamIndex, 0, AVSEEK_FLAG_FRAME);
            if (f != 0) {
                logger_base.info("       VideoReader: Error seeking to %d.", timestampMS);
            }
        } else {
            int f = av_seek_frame(_formatContext, _streamIndex, MStoDTS(timestampMS, _dtspersec), AVSEEK_FLAG_BACKWARD);
            if (f != 0) {
                logger_base.info("       VideoReader: Error seeking to %d.", timestampMS);
            }
        }

        _curPos = -1000;
        if (readFrame)
        {
            GetNextFrame(timestampMS, 0);
        }
	}
}

bool VideoReader::readFrame(int timestampMS) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    int rc = 0;
    if ((rc = avcodec_receive_frame(_codecContext, _srcFrame)) == 0) {
        if (_srcFrame->pts == 0x8000000000000000)
        {
            _curPos = (_srcFrame->pkt_dts * _lengthMS) / _frames;
        }
        else
        {
            _curPos = DTStoMS(_srcFrame->pts, _dtspersec);
        }
        if (_firstFramePos == -1) {
            _firstFramePos = _curPos;
        }
        if (_firstFramePos > timestampMS) {
            // some videos don't have any frames in the first part of a second so we'll use the first frame we DO
            // have for up to that.
            timestampMS = _firstFramePos;
        }
        //int curPosDTS = DTStoMS(_srcFrame->pkt_dts, _dtspersec);
        //printf("    Pos: %d    DTS: %d    Repeat: %d      PTS: %lld\n", _curPos, curPosDTS, _srcFrame->repeat_pict, _srcFrame->pts);
        bool unrefSrcFrame2 = false;
        if ((double)_curPos / (double)_frames >= ((double)timestampMS / (double)_frames) - 2.0) {
            #ifdef VIDEO_EXTRALOGGING
            logger_base.debug("    Decoding video frame %d.", _curPos);
            #endif
            bool hardwareScaled = false;
            if (IsVideoToolboxAcceleratedFrame(_srcFrame)) {
                if (_wantsHWType) {
                    hardwareScaled = true;
                    std::swap(_dstFrame2, _srcFrame);
                } else {
                    hardwareScaled = VideoToolboxScaleImage(_codecContext, _srcFrame, _dstFrame2, hwDecoderCache);
                }
            }

            if (!hardwareScaled) {

                AVFrame* f = nullptr;
#if LIBAVFORMAT_VERSION_MAJOR > 57
                if (IsHardwareAcceleratedVideo() && _codecContext->hw_device_ctx != nullptr && _srcFrame->format == __hw_pix_fmt && !_abandonHardwareDecode) {
    #ifdef __WXMSW__
                    bool hwscale = false;
                    if (__hw_pix_fmt == AV_PIX_FMT_DXVA2_VLD)
                    {
                        LPDIRECT3DSURFACE9 surface = (LPDIRECT3DSURFACE9)_srcFrame->data[3];

                        RECT srcRect = { 0, 0, _codecContext->width, _codecContext->height };
                        RECT tgtRect = { 0, 0, _width, _height };

                        LPDIRECT3DDEVICE9 device = nullptr;
                        HRESULT hr = surface->GetDevice(&device);
                        if (hr != D3D_OK || device == nullptr)
                        {
                            logger_base.error("Unable to get DirectX device 0x%x", hr);
                            _abandonHardwareDecode = true;
                        }
                        else
                        {
                            hr = device->BeginScene();
                            if (hr != D3D_OK)
                            {
                                logger_base.error("Unable to begin DirectX scene 0x%x", hr);
                                _abandonHardwareDecode = true;
                            }
                            else
                            {
                                D3DSURFACE_DESC surfaceDesc;
                                hr = surface->GetDesc(&surfaceDesc);
                                if (hr != D3D_OK)
                                {
                                    logger_base.error("Unable to get DirectX surface description 0x%x", hr);
                                    _abandonHardwareDecode = true;
                                }
                                else
                                {
                                    if (_swsCtx == nullptr) // used as a proxy for first frame
                                    {
                                        if (surfaceDesc.Format < 255)
                                        {
                                            logger_base.debug("Source surface format %d:0x%x %d,%d", (int)surfaceDesc.Format, (int)surfaceDesc.Format, surfaceDesc.Width, surfaceDesc.Height);
                                        }
                                        else
                                        {
                                            if (surfaceDesc.Format < 512)
                                            {
                                                logger_base.debug("Source surface format %d:0x%x %d,%d", (int)surfaceDesc.Format, (int)surfaceDesc.Format, surfaceDesc.Width, surfaceDesc.Height);
                                            }
                                            else
                                            {
                                                char ff[5];
                                                ff[0] = (char)(surfaceDesc.Format & 0xFF);
                                                ff[1] = (char)((surfaceDesc.Format >> 8) & 0xFF);
                                                ff[2] = (char)((surfaceDesc.Format >> 16) & 0xFF);
                                                ff[3] = (char)((surfaceDesc.Format >> 24) & 0xFF);
                                                ff[4] = 0x00;
                                                logger_base.debug("Source surface format %s %d,%d", ff, surfaceDesc.Width, surfaceDesc.Height);
                                            }
                                        }
                                    }

                                    //int pixsize = 4;
                                    //D3DFORMAT desired = D3DFORMAT::D3DFMT_A8R8G8B8;
// I WOULD LIKE TO DO CONVERSION TO RGB here but just as ARGB doesnt work so doesnt RGB .. but worse
                                    //if (!_wantAlpha)
                                    //{
                                    //    desired = D3DFORMAT::D3DFMT_R8G8B8;
                                    //    pixsize = 3;
                                    //}

                                    // Lets not try to do format conversion as it does not work.
                                    D3DFORMAT desired = surfaceDesc.Format;

                                    LPDIRECT3DSURFACE9 backBuffer = nullptr;

                                    hr = device->CreateRenderTarget(_width, _height, desired, surfaceDesc.MultiSampleType,
                                        surfaceDesc.MultiSampleQuality, true, &backBuffer, nullptr);
                                    if (hr != D3D_OK || backBuffer == nullptr)
                                    {
                                        logger_base.debug("Tried to create render target %d,%d multisample type %d quality %d",
                                            _width, _height, surfaceDesc.MultiSampleType, surfaceDesc.MultiSampleQuality);
                                        logger_base.error("Unable to create a render target 0x%x", hr);
                                        _abandonHardwareDecode = true;
                                    }

                                    if (hr == D3D_OK && backBuffer != nullptr)
                                    {
                                        D3DSAMPLERSTATETYPE growshrink = D3DSAMPLERSTATETYPE::D3DSAMP_MINFILTER;
                                        if (_codecContext->width + _codecContext->height < _width + _height)
                                        {
                                            growshrink = D3DSAMPLERSTATETYPE::D3DSAMP_MAGFILTER;
                                            if (_swsCtx == nullptr)
                                            {
                                                logger_base.debug("Expanding video.");
                                            }
                                        }
                                        else
                                        {
                                            if (_swsCtx == nullptr)
                                            {
                                                logger_base.debug("Shrinking video.");
                                            }
                                        }

                                        hr = 1;
                                        for (auto it = _dxva2_filters.begin(); it != _dxva2_filters.end() && hr != D3D_OK; ++it)
                                        {
                                            hr = device->SetSamplerState(0, growshrink, *it);
                                            if (hr == D3D_OK && _swsCtx == nullptr)
                                            {
                                                logger_base.debug("Chosen Min Filter Type %d", *it);
                                            }
                                        }

                                        // copy the current surface to my render target - shrink and convert format
                                        // D3DTEXF_POINT or D3DTEXF_CONVOLUTIONMONO or D3DTEXF_LINEAR or D3DTEXF_GAUSSIANQUAD or D3DTEXF_NONE
                                        // I am not enormously happy with this stretch rect ... it tends to produce really harsh looking video
                                        hr = 1;
                                        for (auto it = _dxva2_filters.begin(); it != _dxva2_filters.end() && hr != D3D_OK; ++it)
                                        {
                                            hr = device->StretchRect(surface, nullptr, backBuffer, nullptr, *it);
                                            if (hr == D3D_OK && _swsCtx == nullptr)
                                            {
                                                logger_base.debug("Chosen D3D Texture Filter Type %d", *it);
                                            }
                                        }

                                        if (hr != D3D_OK)
                                        {
                                            logger_base.error("Unable to scale and convert format in hardware 0x%x", hr);
                                            _abandonHardwareDecode = true;
                                        }
                                        else {
// THIS CODE IS INCOMPLETE AND LIKELY COMPLETELY WRONG
// AND I AM NOT SURE EXACTLY HOW AND WHEN THE SHADER WOULD RUN TO DO COLOUR CONVERSION
// DO I RESIZE FIRST THEN DO THE NV12->RGBA or RGB conversion
                                            //std::string shaderSource = "vec4 main(__sample s) { return s.rgba; }";
                                            //LPD3DBLOB shader = nullptr;
                                            //LPD3DBLOB errors = nullptr;
                                            //hr = D3DCompile(shaderSource.c_str(), shaderSource.size(), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &shader, &errors);
                                            //char* perr = (char*)errors->GetBufferPointer();
                                            //LPDIRECT3DPIXELSHADER9 shaderObject = nullptr;
                                            //hr = device->CreatePixelShader((DWORD*)shader->GetBufferPointer(), &shaderObject);
                                            //shader->Release();
                                            //errors->Release();

                                            // copy down the data into _srcFrame2
                                            D3DLOCKED_RECT LockedRect;
                                            hr = backBuffer->LockRect(&LockedRect, &tgtRect, D3DLOCK_READONLY);
                                            if (hr != D3D_OK || LockedRect.pBits == nullptr)
                                            {
                                                logger_base.error("Unable to lock rect to copy down data 0x%x", hr);
                                                _abandonHardwareDecode = true;
                                            }
                                            else
                                            {
                                                hr = backBuffer->GetDesc(&surfaceDesc);

                                                av_frame_unref(_srcFrame2);

                                                //if (_wantAlpha)
                                                //{
                                                //    _srcFrame2->format = AV_PIX_FMT_ARGB;
                                                //}
                                                //else
                                                //{
                                                //    _srcFrame2->format = AV_PIX_FMT_RGB24;
                                                //}
                                                _srcFrame2->format = AV_PIX_FMT_NV12;
                                                _srcFrame2->width = _width;
                                                _srcFrame2->height = _height;

                                                int ret = av_frame_get_buffer(_srcFrame2, 32);
                                                wxASSERT(ret >= 0);

                                                // This is the code for copying down RGB/RGBA data
                                                //for (int i = 0; i < _height; i++)
                                                //{
                                                //    memcpy(_srcFrame2->data[0] + i * _srcFrame2->linesize[0], (uint8_t*)LockedRect.pBits + i * LockedRect.Pitch, _width * pixsize);
                                                //}

                                                // This is a manual download of the NV12 data into an AVFrame ... but we do it from our resized surface
                                                // Copy Y
                                                if (_srcFrame2->data[0] != nullptr && LockedRect.pBits != nullptr) {
                                                    for (int i = 0; i < _height; i++) {
                                                        memcpy(_srcFrame2->data[0] + i * _srcFrame2->linesize[0], (uint8_t*)LockedRect.pBits + i * LockedRect.Pitch, _width * 1);
                                                    }
                                                }
                                                // Copy UV
                                                if (_srcFrame2->data[1] != nullptr && LockedRect.pBits != nullptr) {
                                                    for (int i = 0; i < _height / 2; i++) {
                                                        memcpy(_srcFrame2->data[1] + i * _srcFrame2->linesize[1], (uint8_t*)LockedRect.pBits + _height * LockedRect.Pitch + i * LockedRect.Pitch, _width * 2);
                                                    }
                                                }

                                                backBuffer->UnlockRect();

                                                if (_srcFrame2->data[0] != nullptr && _srcFrame2->data[1] != nullptr)
                                                {
                                                    hwscale = true;

                                                    // we will still run sws_scale on this but it should be faster as it is already the right size and in ARGB format
                                                    f = _srcFrame2;
                                                }
                                                else
                                                {
                                                    logger_base.warn("Failed to copy data from hardware decode because at least one of the frame data pointers was null");
                                                    _abandonHardwareDecode = true;
                                                }
                                            }
                                            //shaderObject->Release();
                                        }

                                        backBuffer->Release();
                                    }
                                    hr = device->EndScene();
                                }
                            }
                            device->Release();
                        }
                    }

                    if (!hwscale)
    #endif
                    {
                        /* retrieve data from GPU to CPU */
                        if (av_hwframe_transfer_data(_srcFrame2, _srcFrame, 0) < 0) {
                            f = _srcFrame;
                        }
                        else {
                            unrefSrcFrame2 = true;
                            f = _srcFrame2;
                        }
                    }

                    if (_abandonHardwareDecode && _swsCtx != nullptr)
                    {
                        logger_base.warn("VideoReader: This could get ugly ... we have abandoned hardware decode but we already had a sws Context.");
                    }
                }
                else
#endif
                {
                    f = _srcFrame;
                }

                // make sure f is valid
                if (f == nullptr) {
                    logger_base.warn("VideoReader: Strange f was not valid so setting it to the source frame.");
                    f = _srcFrame;
                }

                // first time through we wont have a scale context so create it
                if (_swsCtx == nullptr) {
                    if (_abandonHardwareDecode) {
                        logger_base.warn("VideoReader: Hardware decoding abandoned due to directx error.");
                    }
                    #if LIBAVFORMAT_VERSION_MAJOR > 57
                    if (IsHardwareAcceleratedVideo() && _codecContext->hw_device_ctx != nullptr && _srcFrame->format == __hw_pix_fmt && !_abandonHardwareDecode) {
                        logger_base.debug("Hardware format %s -> Software format %s.", av_get_pix_fmt_name((AVPixelFormat)_srcFrame->format), av_get_pix_fmt_name((AVPixelFormat)_srcFrame2->format));
                        _swsCtx = sws_getContext(f->width, f->height, (AVPixelFormat)f->format,
                            _width, _height, _pixelFmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
                        if (_swsCtx == nullptr) {
                            logger_base.error("VideoReader: Error creating SWSContext");
                        }
                        else {
                            logger_base.debug("Hardware Decoding Pixel format conversion %s -> %s.", av_get_pix_fmt_name((AVPixelFormat)_srcFrame2->format), av_get_pix_fmt_name(_pixelFmt));
                            logger_base.debug("Size conversion %d,%d -> %d,%d.", f->width, f->height, _width, _height);
                        }
                    }
                    else
                    #endif
                    {
                        // software decoding
                        logger_base.debug("Software format %s -> Software format %s.", av_get_pix_fmt_name((AVPixelFormat)f->format), av_get_pix_fmt_name((AVPixelFormat)_pixelFmt));
                        _swsCtx = sws_getContext(f->width, f->height, (AVPixelFormat)f->format,
                            _width, _height, _pixelFmt, SWS_BICUBIC, nullptr, nullptr, nullptr);
                        if (_swsCtx == nullptr) {
                            logger_base.error("VideoReader: Error creating SWSContext");
                        }
                        else {
                            logger_base.debug("Software Decoding Pixel format conversion %s -> %s.", av_get_pix_fmt_name(_codecContext->pix_fmt), av_get_pix_fmt_name(_pixelFmt));
                            logger_base.debug("Size conversion %d,%d -> %d,%d.", f->width, f->height, _width, _height);
                        }
                    }
                }

                if (_swsCtx != nullptr) {
                    sws_scale(_swsCtx, f->data, f->linesize, 0,
                        f->height, _dstFrame2->data,
                        _dstFrame2->linesize);
                }
            }
            std::swap(_dstFrame, _dstFrame2);
        }
        av_frame_unref(_srcFrame);
        if (unrefSrcFrame2) {
            av_frame_unref(_srcFrame2);
        }
        return true;
    } else if (rc != AVERROR(EAGAIN)) {
        logger_base.debug("avcodec_receive_frame failed %d - abandoning video read.", rc);
        _abort = true;
    }
    return false;
}


AVFrame* VideoReader::GetNextFrame(int timestampMS, int gracetime)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_valid || _frames == 0)
    {
        return nullptr;
    }

    if (timestampMS > _lengthMS)
    {
        _atEnd = true;
        return nullptr;
    }

#ifdef VIDEO_EXTRALOGGING
    logger_base.debug("Video %s getting frame %d.", (const char *)_filename.c_str(), timestampMS);
#endif

    int currenttime = GetPos();
    int timeOfNextFrame = currenttime + _frameMS;
    int timeOfPrevFrame = currenttime - _frameMS;
    
    if (_firstFramePos >= timestampMS) {
        //use the first frame in the file
        timestampMS = _firstFramePos;
    }
    
    if (timestampMS >= currenttime && timestampMS < timeOfNextFrame) {
        //same frame, just return
        return _dstFrame;
    }
    if (timestampMS >= timeOfPrevFrame && timestampMS < currenttime) {
        //prev frame, just return, avoids a seek
        return _dstFrame2;
    }

    // If the caller is after an old frame we have to seek first
    if (currenttime > timestampMS + gracetime)
    {
#ifdef VIDEO_EXTRALOGGING
        logger_base.debug("    Video %s seeking back from %d to %d.", (const char *)_filename.c_str(), currenttime, timestampMS);
#endif
        Seek(timestampMS, false);
        currenttime = GetPos();
    }

	if (timestampMS <= _lengthMS) {
        bool firstframe = false;
        if (currenttime <= 0 && timestampMS == 0) {
            firstframe = true;
        }

        bool seekedForward = false;
		while (!_abort && (firstframe || ((currenttime + (_frameMS / 2.0)) < timestampMS)) &&
               currenttime <= _lengthMS &&
               (av_read_frame(_formatContext, &_packet)) == 0) {
            // Is this a packet from the video stream?
			if (_packet.stream_index == _streamIndex) {

                // Decode video frame
                int decodeCount = 0;
                int ret = avcodec_send_packet(_codecContext, &_packet);
                while (!_abort && ret != 0) {
                    if (ret != AVERROR(EAGAIN) && _videoToolboxAccelerated) {
                        logger_base.debug("    Hardware video decoding failed for %s. Reverting to software decoding.", (const char*)_filename.c_str());
                        reopenContext(false);
                        Seek(timestampMS, false);
                        currenttime = GetPos();
                        ret = 0;
                    } else {
                        if (readFrame(timestampMS)) {
                            firstframe = false;
                            currenttime = _curPos;
                        } else {
                            decodeCount++;
                            if (decodeCount == 100) {
                                return nullptr;
                            }
                        }
                        ret = avcodec_send_packet(_codecContext, &_packet);
                    }
                }

                // I am taking _codecContext->keyint_min as likely keyframe frequency - if we are a long way short of the target time try seeking forward ... once
                // the 2 fudge factor is under the assumption that the cost of a seek forward 2 frames is more expensive than just reading the 2 frames
                // 2 may or may not be the best fudge factor
                if (currenttime != -1000 && currenttime < timestampMS - _frameMS * (_keyFrameCount + 2))
                {
                    if (seekedForward)
                    {
                        // we should not have gotten here so keyframecount must be too small
                        _keyFrameCount++;
                    }
                    else
                    {
                        seekedForward = true;
#ifdef VIDEO_EXTRALOGGING
                        logger_base.debug("    Video %s seeking forward from %d to %d.", (const char*)_filename.c_str(), currenttime, timestampMS);
#endif
                        Seek(timestampMS, false);
                        currenttime = GetPos();
                    }
                }
			}
			// Free the packet that was allocated by av_read_frame
			av_packet_unref(&_packet);
		}
    } else {
		_atEnd = true;
		return nullptr;
	}

	if (_dstFrame->data[0] == nullptr || currenttime > _lengthMS) {
		_atEnd = true;
		return nullptr;
	} else {
        int currenttime = GetPos();
        int timeOfNextFrame = currenttime + _frameMS;
        int timeOfPrevFrame = currenttime - _frameMS;
        if (timestampMS >= currenttime && timestampMS < timeOfNextFrame) {
            //same frame, just return
            return _dstFrame;
        }
        if (timestampMS >= timeOfPrevFrame && timestampMS < currenttime) {
            //prev frame, just return, avoids a seek
            return _dstFrame2;
        }

		return _dstFrame;
	}
}
