# Copyright (C) 2018-2019 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

ie_add_sample(NAME videoDecode
              SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/video.cpp"
              OPENCV_DEPENDENCIES highgui videoio imgproc
              DEPENDENCIES avcodec avdevice avfilter avformat avutil swresample swscale)
