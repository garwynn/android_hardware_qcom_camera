/* Copyright (c) 2012-2014, The Linux Foundataion. All rights reserved.
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

#ifndef __QCamera3_POSTPROC_H__
#define __QCamera3_POSTPROC_H__

extern "C" {
#include <mm_camera_interface.h>
#include <mm_jpeg_interface.h>
}
//#include "QCamera3HWI.h"
#include "QCameraQueue.h"
#include "QCameraCmdThread.h"

namespace qcamera {

class QCamera3Exif;
class QCamera3Channel;
class QCamera3PicChannel;
class QCamera3ReprocessChannel;
class QCamera3Stream;
class QCamera3Memory;

typedef struct {
    uint32_t jobId;                  // job ID
    uint32_t client_hdl;             // handle of jpeg client (obtained when open jpeg)
    mm_camera_super_buf_t *src_frame;// source frame (need to be returned back to kernel after done)
    mm_camera_super_buf_t *src_reproc_frame; // original source frame for reproc if not NULL
    mm_camera_super_buf_t *aux_frame;// source frame but from different stream
    QCamera3Channel *aux_channel;
    QCamera3Exif *pJpegExifObj;
} qcamera_hal3_jpeg_data_t;

typedef struct {
    uint32_t jobId;                  // job ID
    mm_camera_super_buf_t *src_frame;// source frame (need to be returned back to kernel after done)
} qcamera_hal3_pp_data_t;

#define MAX_HAL3_EXIF_TABLE_ENTRIES 22
class QCamera3Exif
{
public:
    QCamera3Exif();
    virtual ~QCamera3Exif();

    int32_t addEntry(exif_tag_id_t tagid,
                     exif_tag_type_t type,
                     uint32_t count,
                     void *data);
    uint32_t getNumOfEntries() {return m_nNumEntries;};
    QEXIF_INFO_DATA *getEntries() {return m_Entries;};

private:
    QEXIF_INFO_DATA m_Entries[MAX_HAL3_EXIF_TABLE_ENTRIES];  // exif tags for JPEG encoder
    uint32_t  m_nNumEntries;                            // number of valid entries
};

class QCamera3PostProcessor
{
public:
    QCamera3PostProcessor(QCamera3PicChannel *ch_ctrl);
    virtual ~QCamera3PostProcessor();

    int32_t init(jpeg_encode_callback_t jpeg_cb, void *user_data);
    int32_t deinit();
    int32_t start(QCamera3Memory *mMemory, int index,
                  QCamera3Channel *pInputChannel);
    int32_t stop();
    int32_t processData(mm_camera_super_buf_t *frame);
    int32_t processRawData(mm_camera_super_buf_t *frame);
    int32_t processPPData(mm_camera_super_buf_t *frame);
    int32_t processAuxiliaryData(mm_camera_buf_def_t *frame,
        QCamera3Channel* pAuxiliaryChannel);
    int32_t processPPMetadata(mm_camera_super_buf_t *frame);
    qcamera_hal3_jpeg_data_t *findJpegJobByJobId(uint32_t jobId);
    void releaseJpegJobData(qcamera_hal3_jpeg_data_t *job);

private:
    int32_t sendEvtNotify(int32_t msg_type, int32_t ext1, int32_t ext2);
    mm_jpeg_color_format getColorfmtFromImgFmt(cam_format_t img_fmt);
    mm_jpeg_format_t getJpegImgTypeFromImgFmt(cam_format_t img_fmt);
    int32_t getJpegEncodingConfig(mm_jpeg_encode_params_t& encode_parm,
                                  QCamera3Stream *main_stream,
                                  const cam_dimension_t &src_dim,
                                  const cam_dimension_t &dst_dim,
                                  const cam_dimension_t &thumb_dst_dim);
    int32_t encodeData(qcamera_hal3_jpeg_data_t *jpeg_job_data,
                       uint8_t &needNewSess);
    void releaseSuperBuf(mm_camera_super_buf_t *super_buf);
    static void releaseNotifyData(void *user_data, void *cookie);
    int32_t processRawImageImpl(mm_camera_super_buf_t *recvd_frame);

    static void releaseJpegData(void *data, void *user_data);
    static void releasePPInputData(void *data, void *user_data);
    static void releaseOngoingPPData(void *data, void *user_data);

    static void *dataProcessRoutine(void *data);

private:
    QCamera3PicChannel         *m_parent;
    jpeg_encode_callback_t     mJpegCB;
    void *                     mJpegUserData;
    mm_jpeg_ops_t              mJpegHandle;
    uint32_t                   mJpegClientHandle;
    uint32_t                   mJpegSessionId;

    int8_t                     m_bThumbnailNeeded;
    QCamera3Memory             *mJpegMem;
    int                        mJpegMemIndex;
    QCamera3ReprocessChannel *  m_pReprocChannel;

    QCameraQueue m_inputPPQ;            // input queue for postproc
    QCameraQueue m_ongoingPPQ;          // ongoing postproc queue
    QCameraQueue m_inputJpegQ;          // input jpeg job queue
    QCameraQueue m_ongoingJpegQ;        // ongoing jpeg job queue
    QCameraQueue m_inputRawQ;           // input raw job queue
    QCameraQueue m_inputMetaQ;          //input meta queue
    QCameraCmdThread m_dataProcTh;      // thread for data processing

     pthread_mutex_t mReprocJobLock;
};

}; // namespace qcamera

#endif /* __QCamera3_POSTPROC_H__ */
