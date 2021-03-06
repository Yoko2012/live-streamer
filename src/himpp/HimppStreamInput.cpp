/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * HimppStreamInput.cpp
 * Copyright (C) 2015 Watson Xu <xuhuashan@gmail.com>
 *
 * rtsp-streamer is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * rtsp-streamer is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <alloca.h>
#include <hi_type.h>
#include <hi_defines.h>
#include <hi_comm_sys.h>
#include <hi_comm_venc.h>
#include <mpi_sys.h>
#include <mpi_venc.h>
#include <mpi_sys.h>

#include <SimpleRTPSink.hh>
#include <H264VideoRTPSink.hh>
#include <H264VideoStreamDiscreteFramer.hh>

#include "HimppStreamInput.hh"


class HimppVideoStreamSource: public FramedSource
{
public:
    static HimppVideoStreamSource* createNew(UsageEnvironment& env, HimppVideoEncoder& encoder);
    //static unsigned getRefCount();

public:
    //EventTriggerId eventTriggerId; 
    // Note that this is defined here to be a static class variable, because this code is intended to illustrate how to
    // encapsulate a *single* device - not a set of devices.
    // You can, however, redefine this to be a non-static member variable.

protected:
    HimppVideoStreamSource(UsageEnvironment& env, HimppVideoEncoder& encoder);
    // called only by createNew(), or by subclass constructors
    virtual ~HimppVideoStreamSource();

private:
    // redefined virtual functions:
    virtual void doGetNextFrame();
    //virtual void doStopGettingFrames(); // optional
    static void backgroundHandler(void *clientData, int mask);
    //static void deliverFrame0(void* clientData);
    void deliverFrame();

private:
    //static unsigned referenceCount; // used to count how many instances of this class currently exist
    bool firstDeliverFrame;
    HimppVideoEncoder &fVideoEncoder;
};

//////////////////////////////////////////////////////////////////////////////
// HimppVideoStreamSource
//////////////////////////////////////////////////////////////////////////////

HimppVideoStreamSource*
HimppVideoStreamSource::createNew(UsageEnvironment& env, HimppVideoEncoder& encoder)
{
    return new HimppVideoStreamSource(env, encoder);
}

void HimppVideoStreamSource::backgroundHandler(void *clientData, int mask)
{
    HimppVideoStreamSource *stream_source = (HimppVideoStreamSource *)clientData; 

    //stream_source->envir().taskScheduler().triggerEvent(stream_source->eventTriggerId, stream_source);

    stream_source->deliverFrame();
}

HimppVideoStreamSource::HimppVideoStreamSource(UsageEnvironment& env, HimppVideoEncoder& encoder)
: FramedSource(env), fVideoEncoder(encoder), /*eventTriggerId(0),*/
  firstDeliverFrame(True)
{
    TaskScheduler &scheduler = envir().taskScheduler();

    // Any instance-specific initialization of the device would be done here:

    scheduler.setBackgroundHandling(encoder.fileDescriptor(), SOCKET_READABLE,
                                    (TaskScheduler::BackgroundHandlerProc*)backgroundHandler,
                                    this);

    fVideoEncoder.enable();
    // We arrange here for our "deliverFrame" member function to be called
    // whenever the next frame of data becomes available from the device.
    //
    // If the device can be accessed as a readable socket, then one easy way to do this is using a call to
    //     envir().taskScheduler().turnOnBackgroundReadHandling( ... )
    // (See examples of this call in the "liveMedia" directory.)
    //
    // If, however, the device *cannot* be accessed as a readable socket, then instead we can implement it using 'event triggers':
    // Create an 'event trigger' for this device (if it hasn't already been done):

    //eventTriggerId = envir().taskScheduler().createEventTrigger(deliverFrame0);
}

HimppVideoStreamSource::~HimppVideoStreamSource() {
    // Any instance-specific 'destruction' (i.e., resetting) of the device would be done here:
    envir().taskScheduler().disableBackgroundHandling(fVideoEncoder.fileDescriptor());

    // Reclaim our 'event trigger'
    //envir().taskScheduler().deleteEventTrigger(eventTriggerId);

    fVideoEncoder.disable();
}

void HimppVideoStreamSource::doGetNextFrame() {
    // This function is called (by our 'downstream' object) when it asks for new data.

    // Note: If, for some reason, the source device stops being readable (e.g., it gets closed), then you do the following:
    if (0 /* the source stops being readable */ /*%%% TO BE WRITTEN %%%*/) {
        handleClosure(this);
        return;
    }

    // If a new frame of data is immediately available to be delivered, then do this now:
    if (0 /* a new frame of data is immediately available to be delivered*/ /*%%% TO BE WRITTEN %%%*/) {
        deliverFrame();
    }

    // No new data is immediately available to be delivered.  We don't do anything more here.
    // Instead, our event trigger must be called (e.g., from a separate thread) when new data becomes available.
}

#if 0
void HimppVideoStreamSource::deliverFrame0(void* clientData) {
    if (clientData)
        ((HimppVideoStreamSource*)clientData)->deliverFrame();
}
#endif

void HimppVideoStreamSource::deliverFrame() {
    // This function is called when new frame data is available from the device.
    // We deliver this data by copying it to the 'downstream' object, using the following parameters (class members):
    // 'in' parameters (these should *not* be modified by this function):
    //     fTo: The frame data is copied to this address.
    //         (Note that the variable "fTo" is *not* modified.  Instead,
    //          the frame data is copied to the address pointed to by "fTo".)
    //     fMaxSize: This is the maximum number of bytes that can be copied
    //         (If the actual frame is larger than this, then it should
    //          be truncated, and "fNumTruncatedBytes" set accordingly.)
    // 'out' parameters (these are modified by this function):
    //     fFrameSize: Should be set to the delivered frame size (<= fMaxSize).
    //     fNumTruncatedBytes: Should be set iff the delivered frame would have been
    //         bigger than "fMaxSize", in which case it's set to the number of bytes
    //         that have been omitted.
    //     fPresentationTime: Should be set to the frame's presentation time
    //         (seconds, microseconds).  This time must be aligned with 'wall-clock time' - i.e., the time that you would get
    //         by calling "gettimeofday()".
    //     fDurationInMicroseconds: Should be set to the frame's duration, if known.
    //         If, however, the device is a 'live source' (e.g., encoded from a camera or microphone), then we probably don't need
    //         to set this variable, because - in this case - data will never arrive 'early'.
    // Note the code below.

    if (!isCurrentlyAwaitingData()) return; // we're not ready for the data yet

    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    VENC_PACK_S stPack;
    HI_S32 s32Ret;

    int chnid = fVideoEncoder.channelId();

    s32Ret = HI_MPI_VENC_Query(chnid, &stStat);
    if (HI_SUCCESS != s32Ret || stStat.u32CurPacks <= 0) {
        return;
    }

    stStream.pstPack = &stPack;
    stStream.u32PackCount = 1;
    stStream.u32Seq = 0;
    memset(&stStream.stH264Info, 0, sizeof(VENC_STREAM_INFO_H264_S));
    s32Ret = HI_MPI_VENC_GetStream(chnid, &stStream, HI_TRUE);
    if (HI_SUCCESS != s32Ret) {
        fprintf(stderr, "HI_MPI_VENC_GetStream %d failed [%#x]\n",
                chnid, s32Ret);
        return;
    }

    gettimeofday(&fPresentationTime, NULL);

    fFrameSize = 0;
#define ARRAY_SIZE(x)		(sizeof(x)/sizeof(x[0]))
    for (int i = 0; i < stStream.u32PackCount; i++) {
        for (int j = 0; j < ARRAY_SIZE(stStream.pstPack[i].pu8Addr); j++) {
            HI_U8 *p = stStream.pstPack[i].pu8Addr[j];
            HI_U32 len = stStream.pstPack[i].u32Len[j];

            if (len == 0)
                continue;

            if (len >= 3 && p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x01) {
                p += 3;
                len -= 3;
            }
            if (len >= 4 && p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00 && p[3] == 0x01) {
                p += 4;
                len -= 4;
            }

            if (fFrameSize + len > fMaxSize) {
                fprintf(stderr, "Package Length execute the fMaxSize %d\n", fMaxSize);
                break;
            }

            memmove(&fTo[fFrameSize], p, len);
            fFrameSize += len;
        }
    }

    s32Ret = HI_MPI_VENC_ReleaseStream(chnid, &stStream);
    if (HI_SUCCESS != s32Ret) {
        envir() << "HI_MPI_VENC_ReleaseStream failed [0x" << s32Ret << "]\n";
    }

    // After delivering the data, inform the reader that it is now available:
    if (isCurrentlyAwaitingData()) {
        FramedSource::afterGetting(this);
    }
}


//////////////////////////////////////////////////////////////////////////////
// HimppVideoServerMediaSubsession
//////////////////////////////////////////////////////////////////////////////


HimppVideoServerMediaSubsession*
HimppVideoServerMediaSubsession::createNew(UsageEnvironment& env, HimppVideoEncoder& encoder)
{
  return new HimppVideoServerMediaSubsession(env, encoder);
}

HimppVideoServerMediaSubsession
::HimppVideoServerMediaSubsession(UsageEnvironment& env, HimppVideoEncoder& encoder)
    : OnDemandServerMediaSubsession(env, True /* always reuse the first source */),
      fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL),
      fVideoEncoder(encoder)
{
}

HimppVideoServerMediaSubsession::~HimppVideoServerMediaSubsession()
{
    delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void* clientData) {
    HimppVideoServerMediaSubsession* subsess = (HimppVideoServerMediaSubsession*)clientData;
    subsess->afterPlayingDummy1();
}

void HimppVideoServerMediaSubsession::afterPlayingDummy1()
{
    // Unschedule any pending 'checking' task:
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    // Signal the event loop that we're done:
    setDoneFlag();
}

static void checkForAuxSDPLine(void* clientData) {
    HimppVideoServerMediaSubsession* subsess = (HimppVideoServerMediaSubsession*)clientData;
    subsess->checkForAuxSDPLine1();
}

void HimppVideoServerMediaSubsession::checkForAuxSDPLine1()
{
    char const* dasl;

    if (fAuxSDPLine != NULL) {
        // Signal the event loop that we're done:
        setDoneFlag();
    } else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
        fAuxSDPLine = strDup(dasl);
        fDummyRTPSink = NULL;

        // Signal the event loop that we're done:
        setDoneFlag();
    } else if (!fDoneFlag) {
        // try again after a brief delay:
        int uSecsToDelay = 100000; // 100 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
                (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const* HimppVideoServerMediaSubsession
::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
    if (fAuxSDPLine != NULL) return fAuxSDPLine; // it's already been set up (for a previous client)

    if (fDummyRTPSink == NULL) { // we're not already setting it up for another, concurrent stream
        // Note: For H264 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
        // until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
        // and we need to start reading data from our file until this changes.
        fDummyRTPSink = rtpSink;

        // Start reading the file:
        fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

        // Check whether the sink's 'auxSDPLine()' is ready:
        checkForAuxSDPLine(this);
    }

    envir().taskScheduler().doEventLoop(&fDoneFlag);

    return fAuxSDPLine;
}

FramedSource* HimppVideoServerMediaSubsession
::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate)
{
    estBitrate = fVideoEncoder.getBitrate(); // kbps, estimate

    // Create the video source:
    HimppVideoStreamSource *source = HimppVideoStreamSource::createNew(envir(), fVideoEncoder);
    return H264VideoStreamDiscreteFramer::createNew(envir(), source);
}

RTPSink* HimppVideoServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/)
{
    H264VideoRTPSink *rtp_sink = H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
    rtp_sink->setPacketSizes(1000, 1456 * 10);
    return rtp_sink;
}


////////////////////////////////////////////////////////////////////////////////
// Audio
////////////////////////////////////////////////////////////////////////////////


class HimppAudioStreamSource: public FramedSource
{
public:
    static HimppAudioStreamSource* createNew(UsageEnvironment& env, HimppAudioEncoder& encoder);
    //static unsigned getRefCount();

public:
    //EventTriggerId eventTriggerId; 
    // Note that this is defined here to be a static class variable, because this code is intended to illustrate how to
    // encapsulate a *single* device - not a set of devices.
    // You can, however, redefine this to be a non-static member variable.

protected:
    HimppAudioStreamSource(UsageEnvironment& env, HimppAudioEncoder& encoder);
    // called only by createNew(), or by subclass constructors
    virtual ~HimppAudioStreamSource();

private:
    // redefined virtual functions:
    virtual void doGetNextFrame();
    //virtual void doStopGettingFrames(); // optional
    static void backgroundHandler(void *clientData, int mask);
    //static void deliverFrame0(void* clientData);
    void deliverFrame();

private:
    //static unsigned referenceCount; // used to count how many instances of this class currently exist
    bool firstDeliverFrame;
    HimppAudioEncoder &fAudioEncoder;
};

HimppAudioStreamSource*
HimppAudioStreamSource::createNew(UsageEnvironment& env, HimppAudioEncoder& encoder)
{
    return new HimppAudioStreamSource(env, encoder);
}

void HimppAudioStreamSource::backgroundHandler(void *clientData, int mask)
{
    HimppAudioStreamSource *stream_source = (HimppAudioStreamSource *)clientData; 
    stream_source->deliverFrame();
}

HimppAudioStreamSource
::HimppAudioStreamSource(UsageEnvironment& env, HimppAudioEncoder& encoder)
    : FramedSource(env), fAudioEncoder(encoder),
      firstDeliverFrame(True)
{
    TaskScheduler &scheduler = envir().taskScheduler();

    scheduler.setBackgroundHandling(encoder.fileDescriptor(), SOCKET_READABLE,
                                    (TaskScheduler::BackgroundHandlerProc*)backgroundHandler,
                                    this);

    fAudioEncoder.enable();
}

HimppAudioStreamSource::~HimppAudioStreamSource() {
    envir().taskScheduler().disableBackgroundHandling(fAudioEncoder.fileDescriptor());

    fAudioEncoder.disable();
}

void HimppAudioStreamSource::doGetNextFrame() {
}

void HimppAudioStreamSource::deliverFrame() {
    if (!isCurrentlyAwaitingData()) return; // we're not ready for the data yet

    AUDIO_STREAM_S stStream;
    HI_S32 s32Ret;

    int chnid = fAudioEncoder.channelId();

    s32Ret = HI_MPI_AENC_GetStream(chnid, &stStream, HI_FALSE);
    if (HI_SUCCESS != s32Ret) {
        fprintf(stderr, "HI_MPI_AENC_GetStream %d failed [%#x]\n",
                chnid, s32Ret);
        return;
    }

    gettimeofday(&fPresentationTime, NULL);

    fFrameSize = 0;
    if (stStream.u32Len <= fMaxSize + 4) {
        fFrameSize = stStream.u32Len - 4;
        fNumTruncatedBytes = 0;
    }
    else {
        fFrameSize = fMaxSize;
        fNumTruncatedBytes = fFrameSize - fMaxSize;
    }
    memmove(&fTo[0], stStream.pStream + 4, fFrameSize);

    s32Ret = HI_MPI_AENC_ReleaseStream(chnid, &stStream);
    if (HI_SUCCESS != s32Ret) {
        envir() << "HI_MPI_AENC_ReleaseStream failed [0x" << s32Ret << "]\n";
    }

    // After delivering the data, inform the reader that it is now available:
    if (isCurrentlyAwaitingData()) {
        FramedSource::afterGetting(this);
    }
}


HimppAudioServerMediaSubsession* HimppAudioServerMediaSubsession
::createNew(UsageEnvironment& env, HimppAudioEncoder& encoder)
{
    return new HimppAudioServerMediaSubsession(env, encoder);
}

HimppAudioServerMediaSubsession
::HimppAudioServerMediaSubsession(UsageEnvironment& env, HimppAudioEncoder& encoder)
    : OnDemandServerMediaSubsession(env, True /* always reuse the first source */),
      fAudioEncoder(encoder)
{
}

HimppAudioServerMediaSubsession::~HimppAudioServerMediaSubsession()
{
}

FramedSource* HimppAudioServerMediaSubsession
::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
    estBitrate = 64; // kbps, estimate

    // Create the audio source:
    HimppAudioStreamSource *source = HimppAudioStreamSource::createNew(envir(), fAudioEncoder);

    return source;
}

RTPSink* HimppAudioServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
                   unsigned char rtpPayloadTypeIfDynamic,
				   FramedSource* inputSource)
{
    uint32_t samplerate = fAudioEncoder.getSampleRate();
    uint32_t nr_chans = 1;
    const char *mimeType = "";
    unsigned char payloadFormatCode = rtpPayloadTypeIfDynamic;

    IAudioEncoder::EncodingType encoding;
    encoding = fAudioEncoder.getEncoding();
    switch (encoding) {
    case IAudioEncoder::ADPCM:
        mimeType = "DVI4";
        break;
    case IAudioEncoder::LPCM:
        mimeType = "L16";
        break;
    case IAudioEncoder::G711A:
        mimeType = "PCMA";
        break;
    case IAudioEncoder::G711U:
        mimeType = "PCMU";
        break;
    case IAudioEncoder::G726:
        mimeType = "G726-40";
        break;
    }

    RTPSink *rtp_sink
        = SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                   payloadFormatCode, samplerate,
                                   "audio", mimeType, nr_chans);
    return rtp_sink;
}
