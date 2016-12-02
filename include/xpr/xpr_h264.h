#ifndef XPR_H264_H
#define XPR_H264_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XPR_H264_FrameInfo {
	float fps;
	int width;
	int height;
    int keyFrame;
} XPR_H264_FrameInfo;

typedef struct XPR_H264_HRD {
    uint32_t cpb_cnt_minus1;
    uint8_t bit_rate_scale;
    uint8_t cpb_size_scale;
    uint32_t bit_rate_value_minus1;
    uint32_t cpb_size_value_minus1;
    uint8_t cbr_flag;
    uint8_t initial_cpb_removal_delay_length_minus1;
    uint8_t cpb_removal_delay_length_minus1;
    uint8_t dpb_output_delay_length_minus1;
    uint8_t time_offset_length;
} XPR_H264_HRD;

typedef struct XPR_H264_VUI {
    uint8_t aspect_ratio_info_present_flag;
    uint8_t aspect_ratio_idc;
    uint32_t sar_width;
    uint32_t sar_height;

    uint8_t overscan_info_present_flag;
    uint8_t overscan_appropriate_flag;
    uint8_t video_signal_type_present_flag;
    uint8_t video_format;
    uint8_t video_full_range_flag;
    uint8_t colour_description_present_flag;
    uint8_t colour_parimaries;
    uint8_t transfer_characteristics;
    uint8_t matrix_coefficients;

    uint8_t chroma_loc_info_present_flag;
    uint32_t chroma_sample_loc_type_top_field;
    uint32_t chroma_sample_loc_type_bottom_field;

    uint8_t timing_info_present_flag;
    uint32_t num_units_in_tick;
    uint32_t time_scale;
    uint8_t fixed_frame_rate_flag;
    uint8_t nal_hrd_parameters_present_flag;
    uint8_t vcl_hrd_parameters_present_flag;
    uint8_t pic_struct_present_flag;
    uint8_t bitstream_restriction_flag;

    XPR_H264_HRD nal_hrd_parameters;
    XPR_H264_HRD vcl_hrd_parameters;
} XPR_H264_VUI;

typedef struct XPR_H264_SPS {
    uint8_t forbidden_zero_bit;     ///< u(1)
    uint8_t nal_ref_idc;            ///< u(2)
    uint8_t nal_unit_type;          ///< u(5)
    uint8_t profile_idc;            ///< u(8)
    uint8_t constraint_set0_flag;   ///< u(1)
    uint8_t constraint_set1_flag;   ///< u(1)
    uint8_t constraint_set2_flag;   ///< u(1)
    uint8_t constraint_set3_flag;   ///< u(1)
    uint8_t reserved_zero_4bits;    ///< u(4)
    uint8_t constraint_setN_flag;   ///< u(8)
    uint8_t level_idc;              ///< u(8)
    uint32_t seq_parameter_set_id;                  ///< ue(v)

    uint32_t chroma_format_idc;                     ///< ue(v)
    uint8_t separate_colour_plane_flag;             ///< u(1)
    uint32_t bit_depth_luma_minus8;                 ///< ue(v)
    uint32_t bit_depth_chroma_minus8;               ///< ue(v)
    uint8_t qpprime_y_zero_transform_bypass_flag;   ///< u(1)
    uint8_t seq_scaling_matrix_present_flag;        ///< u(1)

    uint32_t log2_max_frame_num_minus4;             ///< ue(v)

    uint32_t pic_order_cnt_type;                    ///< ue(v)
    uint32_t log2_max_pic_order_cnt_lsb_minus4;     ///< ue(v) 

    uint32_t max_num_ref_frames;                    ///< ue(v)
    uint32_t gaps_in_frame_num_value_allowed_flag;  ///< u(1)
    uint32_t pic_width_in_mbs_minus1;               ///< ue(v)
    uint32_t pic_height_in_map_units_minus1;        ///< ue(v)

    uint8_t frame_mbs_only_flag;                    ///< u(1)
    uint8_t mb_adaptive_frame_field_flag;           ///< u(1)
    uint8_t direct_8x8_inference_flag;              ///< u(1)

    uint8_t frame_cropping_flag;                    ///< u(1)
    uint32_t frame_crop_left_offset;                ///< ue(v)
    uint32_t frame_crop_right_offset;               ///< ue(v)
    uint32_t frame_crop_top_offset;                 ///< ue(v)
    uint32_t frame_crop_bottom_offset;              ///< ue(v)

    uint8_t vui_parameters_present_flag;            ///< u(1)

    XPR_H264_VUI vui_parameters;
    XPR_H264_HRD hrd_parameters;
} XPR_H264_SPS;

typedef struct XPR_H264_NALU {
    const uint8_t* data;
    unsigned int length;
} XPR_H264_NALU;

typedef enum XPR_H264_NALU_Type {
    XPR_H264_NALU_TYPE_P_SLC = 0x01,
    XPR_H264_NALU_TYPE_I_SLC = 0x05,
    XPR_H264_NALU_TYPE_SEI = 0x06,
    XPR_H264_NALU_TYPE_SPS = 0x07,
    XPR_H264_NALU_TYPE_PPS = 0x08,
    XPR_H264_NALU_TYPE_AUD = 0x09,

} XPR_H264_NALU_Type;

XPR_API int XPR_H264_ProbeFrameInfo(const uint8_t* data, unsigned int length,
                            XPR_H264_FrameInfo* fi);

XPR_API int XPR_H264_ProbeFrameInfoEx(XPR_H264_NALU nalus[], unsigned int naluCount,
                              XPR_H264_FrameInfo* fi);

XPR_API int XPR_H264_ParseSPS_NALU(const uint8_t* data, unsigned int length,
                           XPR_H264_SPS* sps);

XPR_API int XPR_H264_ScanNALU(const uint8_t* data, unsigned int length,
                      XPR_H264_NALU nalus[], unsigned int maxNalus);

XPR_API void XPR_H264_SPS_Dump(XPR_H264_SPS* sps, const char* indent);

#ifdef __cplusplus
}
#endif

#endif // XPR_H264_H

