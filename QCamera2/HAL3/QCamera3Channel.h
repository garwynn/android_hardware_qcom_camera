/* Copyright (c) 2012-2013, The Linux Foundataion. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __QCAMERA3_CHANNEL_H__
#define __QCAMERA3_CHANNEL_H__

#include <hardware/camera3.h>
#include "QCamera3Stream.h"
#include "QCamera3Mem.h"
#include "QCamera3PostProc.h"
#include "QCamera3HALHeader.h"
#include "utils/Vector.h"

extern "C" {
#include <mm_camera_interface.h>
}

using namespace android;

namespace qcamera {

typedef void (*channel_cb_routine)(mm_camera_super_buf_t *metadata,
                                camera3_stream_buffer_t *buffer,
                                uint32_t frame_number, void *userdata);

class QCamera3Channel
{
public:
    QCamera3Channel(uint32_t cam_handle,
                   mm_camera_ops_t *cam_ops,
                   channel_cb_routine cb_routine,
                   cam_padding_info_t *paddingInfo,
                   void *userData);
    QCamera3Channel();
    virtual ~QCamera3Channel();

    int32_t addStream(cam_stream_type_t streamType,
                              cam_format_t streamFormat,
                              cam_dimension_t streamDim,
                              uint8_t minStreamBufnum);
    virtual int32_t start();
    int32_t stop();
    int32_t bufDone(mm_camera_super_buf_t *recvd_frame);

    uint32_t getStreamTypeMask();
    uint32_t getStreamID(uint32_t streamMask);
    virtual int32_t initialize() = 0;
    virtual int32_t request(buffer_handle_t * /*buffer*/,
                uint32_t /*frameNumber*/){ return 0;};
    virtual int32_t request(buffer_handle_t * /*buffer*/,
                uint32_t /*frameNumber*/,
                jpeg_settings_t* /*mJpegSettings*/,
                mm_camera_buf_def_t* /*pInputBuffer*/,
                QCamera3Channel* /*pInputChannel*/){ return 0;};
    virtual void streamCbRoutine(mm_camera_super_buf_t *super_frame,
                            QCamera3Stream *stream) = 0;

    virtual int32_t registerBuffer(buffer_handle_t *buffer) = 0;
    virtual QCamera3Memory *getStreamBufs(uint32_t len) = 0;
    virtual void putStreamBufs() = 0;

    QCamera3Stream *getStreamByHandle(uint32_t streamHandle);
    uint32_t getMyHandle() const {return m_handle;};
    uint8_t getNumOfStreams() const {return m_numStreams;};
    QCamera3Stream *getStreamByIndex(uint8_t index);

    static void streamCbRoutine(mm_camera_super_buf_t *super_frame,
                QCamera3Stream *stream, void *userdata);
    void *mUserData;
    cam_padding_info_t *mPaddingInfo;
    QCamera3Stream *mStreams[MAX_STREAM_NUM_IN_BUNDLE];
    uint8_t m_numStreams;
protected:

   virtual int32_t init(mm_camera_channel_attr_t *attr,
                         mm_camera_buf_notify_t dataCB);
    int32_t allocateStreamInfoBuf(camera3_stream_t *stream);

    uint32_t m_camHandle;
    mm_camera_ops_t *m_camOps;
    bool m_bIsActive;

    uint32_t m_handle;


    mm_camera_buf_notify_t mDataCB;


    QCamera3HeapMemory *mStreamInfoBuf;
    channel_cb_routine mChannelCB;
    //cam_padding_info_t *mPaddingInfo;
};

/* QCamera3RegularChannel is used to handle all streams that are directly
 * generated by hardware and given to frameworks without any postprocessing at HAL.
 * Examples are: all IMPLEMENTATION_DEFINED streams, CPU_READ streams. */
class QCamera3RegularChannel : public QCamera3Channel
{
public:
    QCamera3RegularChannel(uint32_t cam_handle,
                    mm_camera_ops_t *cam_ops,
                    channel_cb_routine cb_routine,
                    cam_padding_info_t *paddingInfo,
                    void *userData,
                    camera3_stream_t *stream,
                    cam_stream_type_t stream_type);
    QCamera3RegularChannel(uint32_t cam_handle,
                    mm_camera_ops_t *cam_ops,
                    channel_cb_routine cb_routine,
                    cam_padding_info_t *paddingInfo,
                    void *userData,
                    camera3_stream_t *stream,
                    uint32_t width, uint32_t height,
                    cam_stream_type_t stream_type);

    virtual ~QCamera3RegularChannel();

    virtual int32_t start();
    virtual int32_t initialize();
    virtual int32_t request(buffer_handle_t *buffer, uint32_t frameNumber);
    virtual void streamCbRoutine(mm_camera_super_buf_t *super_frame,
                                            QCamera3Stream *stream);

    virtual QCamera3Memory *getStreamBufs(uint32_t le);
    virtual void putStreamBufs();
    mm_camera_buf_def_t* getInternalFormatBuffer(buffer_handle_t* buffer);
    virtual int32_t registerBuffer(buffer_handle_t *buffer);

public:
    static int kMaxBuffers;
private:
    int32_t initialize(struct private_handle_t *priv_handle);

    camera3_stream_t *mCamera3Stream;
    uint32_t mNumBufs;
    Vector<buffer_handle_t *> mCamera3Buffers;

    QCamera3GrallocMemory mMemory;
    // width and height of internal stream may be different than what's
    // specified in camera3_stream_t. For example: ZSL stream size is
    // always the active region size, but internally we use the JPEG
    // size.
    uint32_t mWidth, mHeight;
    cam_stream_type_t mStreamType; // Stream type
};

/* QCamera3MetadataChannel is for metadata stream generated by camera daemon. */
class QCamera3MetadataChannel : public QCamera3Channel
{
public:
    QCamera3MetadataChannel(uint32_t cam_handle,
                    mm_camera_ops_t *cam_ops,
                    channel_cb_routine cb_routine,
                    cam_padding_info_t *paddingInfo,
                    void *userData);
    virtual ~QCamera3MetadataChannel();

    virtual int32_t initialize();

    virtual int32_t request(buffer_handle_t *buffer, uint32_t frameNumber);
    virtual void streamCbRoutine(mm_camera_super_buf_t *super_frame,
                            QCamera3Stream *stream);

    virtual QCamera3Memory *getStreamBufs(uint32_t le);
    virtual void putStreamBufs();
    virtual int32_t registerBuffer(buffer_handle_t * /*buffer*/)
            { return NO_ERROR; };

private:
    QCamera3HeapMemory *mMemory;
};

/* QCameraRawChannel is for Dumping raw stream generated by camera daemon. */
class QCameraRawChannel : public QCamera3Channel
{
public:
    QCameraRawChannel(uint32_t cam_handle,
                    mm_camera_ops_t *cam_ops,
                    channel_cb_routine cb_routine,
                    cam_padding_info_t *paddingInfo,
                    void *userData,
                    cam_dimension_t *raw_dim);
    virtual ~QCameraRawChannel();

    virtual int32_t initialize();

    virtual int32_t request(buffer_handle_t *buffer, uint32_t frameNumber);
    virtual int32_t registerBuffers(uint32_t num_buffers,
                buffer_handle_t **buffers);
    virtual void streamCbRoutine(mm_camera_super_buf_t *super_frame,
                            QCamera3Stream *stream);

    virtual QCamera3Memory *getStreamBufs(uint32_t le);
    virtual void putStreamBufs();
    void dumpRawSnapshot(mm_camera_buf_def_t *frame);
    virtual int32_t registerBuffer(buffer_handle_t * /*buffer*/)
            { return NO_ERROR; };

private:
    QCamera3HeapMemory *mMemory;
    uint32_t mWidth, mHeight;
    uint32_t mMaxBuffers;
};


/* QCamera3PicChannel is for JPEG stream, which contains a YUV stream generated
 * by the hardware, and encoded to a JPEG stream */
class QCamera3PicChannel : public QCamera3Channel
{
public:
    QCamera3PicChannel(uint32_t cam_handle,
            mm_camera_ops_t *cam_ops,
            channel_cb_routine cb_routine,
            cam_padding_info_t *paddingInfo,
            void *userData,
            camera3_stream_t *stream);
    ~QCamera3PicChannel();

    virtual int32_t initialize();

    virtual int32_t request(buffer_handle_t *buffer,
            uint32_t frameNumber, jpeg_settings_t* mJpegSettings,
            mm_camera_buf_def_t* pInputBuffer,QCamera3Channel* pInputChannel);
    virtual void streamCbRoutine(mm_camera_super_buf_t *super_frame,
            QCamera3Stream *stream);

    virtual QCamera3Memory *getStreamBufs(uint32_t le);
    virtual void putStreamBufs();
    bool isWNREnabled() {return m_bWNROn;};
    bool needOnlineRotation();
    void getThumbnailSize(cam_dimension_t &dim);
    int getJpegQuality();
    int getJpegThumbnailQuality();
    int getJpegRotation();
    bool isRawSnapshot();
    QCamera3Exif *getExifData();
    static void jpegEvtHandle(jpeg_job_status_t status,
            uint32_t /*client_hdl*/,
            uint32_t jobId,
            mm_jpeg_output_t *p_output,
            void *userdata);
    static void dataNotifyCB(mm_camera_super_buf_t *recvd_frame,
            void *userdata);

    void queueMetadata(mm_camera_super_buf_t *metadata_buf,
                                       QCamera3Channel *pMetaChannel,
                                       bool relinquish);
    virtual int32_t registerBuffer(buffer_handle_t *buffer);

public:
    static int kMaxBuffers;
    QCamera3PostProcessor m_postprocessor; // post processor
    cam_dimension_t m_max_pic_dim;

private:
    camera3_stream_t *mCamera3Stream;
    uint32_t mNumBufs;
    jpeg_settings_t* mJpegSettings;
    int32_t mCurrentBufIndex;
    bool m_bWNROn;
    Vector<buffer_handle_t *> mCamera3Buffers;

    QCamera3GrallocMemory mMemory;
    QCamera3HeapMemory *mYuvMemory;
    QCamera3Channel *m_pMetaChannel;
    mm_camera_super_buf_t *mMetaFrame;
};

// reprocess channel class
class QCamera3ReprocessChannel : public QCamera3Channel
{
public:
    QCamera3ReprocessChannel(uint32_t cam_handle,
                            mm_camera_ops_t *cam_ops,
                            channel_cb_routine cb_routine,
                            cam_padding_info_t *paddingInfo,
                            void *userData, void *ch_hdl);
    QCamera3ReprocessChannel();
    virtual ~QCamera3ReprocessChannel();
    // online reprocess
    int32_t doReprocess(mm_camera_super_buf_t *frame,
                        mm_camera_super_buf_t *meta_frame);
    // offline reprocess
    int32_t doReprocess(int buf_fd, uint32_t buf_length, int32_t &ret_val,
                        mm_camera_super_buf_t *meta_buf);
    virtual QCamera3Memory *getStreamBufs(uint32_t len);
    virtual void putStreamBufs();
    virtual int32_t initialize();
    virtual void streamCbRoutine(mm_camera_super_buf_t *super_frame,
                            QCamera3Stream *stream);
    static void dataNotifyCB(mm_camera_super_buf_t *recvd_frame,
                                       void* userdata);
    int32_t addReprocStreamsFromSource(cam_pp_feature_config_t &config,
                                       QCamera3Channel *pSrcChannel,
                                       QCamera3Channel *pMetaChannel);
    QCamera3Stream *getStreamBySourceHandle(uint32_t srcHandle);
    int32_t metadataBufDone(mm_camera_super_buf_t *recvd_frame);
    virtual int32_t registerBuffer(buffer_handle_t * /*buffer*/)
            { return NO_ERROR; };

public:
    void *picChHandle;
private:
    uint32_t mSrcStreamHandles[MAX_STREAM_NUM_IN_BUNDLE];
    QCamera3Channel *m_pSrcChannel; // ptr to source channel for reprocess
    QCamera3Channel *m_pMetaChannel;
    QCamera3HeapMemory *mMemory;
};

}; // namespace qcamera

#endif /* __QCAMERA_CHANNEL_H__ */
