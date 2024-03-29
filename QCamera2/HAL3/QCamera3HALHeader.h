/* Copyright (c) 2013, The Linux Foundataion. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*	notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*	copyright notice, this list of conditions and the following
*	disclaimer in the documentation and/or other materials provided
*	with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*	contributors may be used to endorse or promote products derived
*	from this software without specific prior written permission.
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
#ifndef __QCAMERA_HALHEADER_H__
#define __QCAMERA_HALHEADER_H__

extern "C" {
#include <mm_camera_interface.h>
#include <mm_jpeg_interface.h>
}

using namespace android;

namespace qcamera {

class QCamera3Channel;

    typedef enum {
        INVALID,
        VALID,
    } stream_status_t;

    typedef struct {
        int32_t jpeg_orientation;
        uint8_t jpeg_quality;
        uint8_t jpeg_thumb_quality;
        cam_dimension_t thumbnail_size;
        int32_t sensor_sensitivity;
        int64_t sensor_exposure_time;
        float lens_focal_length;
        int32_t max_jpeg_size;
        int exposure_compensation;
        cam_rational_type_t exposure_comp_step;
        int64_t* gps_timestamp;
        double* gps_coordinates[3];
        char gps_processing_method[35];
        bool is_jpeg_format;
        uint32_t min_required_pp_mask;
        int32_t sharpness;
    } jpeg_settings_t;

    typedef struct {
        camera3_stream_t *stream;
        stream_status_t status;
        QCamera3Channel *channel;
    } stream_info_t;

    typedef struct {
        int32_t iso_speed;
        int64_t exposure_time;
    } metadata_response_t;

};//namespace qcamera


#endif
