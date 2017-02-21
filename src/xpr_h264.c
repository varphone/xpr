#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_bitvector.h>
#include <xpr/xpr_h264.h>
#include <xpr/xpr_utils.h>

int XPR_H264_HaveStartCode(const uint8_t* data, unsigned int length)
{
    if (length < 4)
        return 0;
    if (data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 1)
        return 1;
    return 0;
}

int64_t XPR_H264_VUI_CalcDuration(XPR_H264_VUI* vui, int64_t base)
{
    if (!vui->timing_info_present_flag || !vui->num_units_in_tick || !vui->time_scale)
        return 0;
    base = base ? 1000000 : base;
    return base * vui->num_units_in_tick / vui->time_scale * 2;
}

float XPR_H264_VUI_CalcFPS(XPR_H264_VUI* vui)
{
    if (!vui->timing_info_present_flag)
        return 0.0;
    return (float)vui->time_scale / (float)vui->num_units_in_tick / 2;
}

int64_t XPR_H264_VUI_CalcPTSSteps(XPR_H264_VUI* vui, int64_t base)
{
    if (!vui->timing_info_present_flag)
        return 0;
    base = base ? 1000000 : base;
    return base * vui->num_units_in_tick / vui->time_scale * 2;
}

int XPR_H264_ProbeFrameInfo(const uint8_t* data, unsigned int length,
                            XPR_H264_FrameInfo* fi)
{
    int i = 0;
    int n = 0;
    XPR_H264_NALU nalus[16];
    //
    n = XPR_H264_ScanNALU(data, length, nalus, 16);
	return XPR_H264_ProbeFrameInfoEx(nalus, n, fi);
}

int XPR_H264_ProbeFrameInfoEx(XPR_H264_NALU nalus[], unsigned int naluCount,
                              XPR_H264_FrameInfo* fi)
{
    int i = 0;
    XPR_H264_SPS sps;
    //
    fi->keyFrame = XPR_FALSE;
	for (i = 0; i < (int)naluCount; i++) {
		if ((nalus[i].data[0] & 0x1f) == XPR_H264_NALU_TYPE_SPS) {
            if (XPR_H264_ParseSPS_NALU(nalus[i].data, nalus[i].length, &sps) != XPR_ERR_SUCCESS)
                break;
		    fi->fps = XPR_H264_VUI_CalcFPS(&sps.vui_parameters);
			fi->width = (sps.pic_width_in_mbs_minus1+1) * 16;
			fi->height = (sps.pic_height_in_map_units_minus1+1) * 16;
            fi->keyFrame = XPR_TRUE;
            return XPR_ERR_SUCCESS;
		}
	}
	return XPR_ERR_ERROR;
}

static void ParseSPS_HRD(XPR_BitVector* bv, XPR_H264_SPS* sps,
                         XPR_H264_HRD* hrd)
{
    unsigned int SchedSelIdx = 0;
    hrd->cpb_cnt_minus1 = XPR_BitVectorGetExpGolomb(bv);
    hrd->bit_rate_scale = XPR_BitVectorGetBits(bv, 4);
    hrd->cpb_size_scale = XPR_BitVectorGetBits(bv, 4);
    for (SchedSelIdx = 0; SchedSelIdx <= hrd->cpb_cnt_minus1;
         SchedSelIdx++) {
        hrd->bit_rate_value_minus1 = XPR_BitVectorGetExpGolomb(bv);
        hrd->cpb_size_value_minus1 = XPR_BitVectorGetExpGolomb(bv);
        hrd->cbr_flag = XPR_BitVectorGet1Bit(bv);
    }
    hrd->initial_cpb_removal_delay_length_minus1 = XPR_BitVectorGetBits(
                bv, 5);
    hrd->cpb_removal_delay_length_minus1 = XPR_BitVectorGetBits(bv, 5);
    hrd->dpb_output_delay_length_minus1 = XPR_BitVectorGetBits(bv, 5);
    hrd->time_offset_length = XPR_BitVectorGetBits(bv, 5);
}

static void ParseSPS_VUI(XPR_BitVector* bv, XPR_H264_SPS* sps,
                         XPR_H264_VUI* vui)
{
    vui->aspect_ratio_info_present_flag = XPR_BitVectorGet1Bit(bv);
    if (vui->aspect_ratio_info_present_flag) {
        vui->aspect_ratio_idc = XPR_BitVectorGetBits(bv, 8);
        if (vui->aspect_ratio_idc == 255/*Extended_SAR*/) {
            vui->sar_width = XPR_BitVectorGetBits(bv, 16); // sar_width
            vui->sar_height = XPR_BitVectorGetBits(bv, 16); //sar_height
        }
    }
    vui->overscan_info_present_flag = XPR_BitVectorGet1Bit(bv);
    if (vui->overscan_info_present_flag) {
        vui->overscan_appropriate_flag = XPR_BitVectorGet1Bit(
                                             bv); // overscan_appropriate_flag
    }
    vui->video_signal_type_present_flag = XPR_BitVectorGet1Bit(bv);
    if (vui->video_signal_type_present_flag) {
        vui->video_format = XPR_BitVectorGetBits(bv, 3); // video_format
        vui->video_full_range_flag = XPR_BitVectorGet1Bit(
                                         bv); // video_full_range_flag
        vui->colour_description_present_flag = XPR_BitVectorGet1Bit(
                bv); // colour_description_present_flag
        if (vui->colour_description_present_flag) {
            vui->colour_parimaries = XPR_BitVectorGetBits(bv,
                                     8); // colour_primaries
            vui->transfer_characteristics = XPR_BitVectorGetBits(bv,
                                            8); // transfer_characteristics
            vui->matrix_coefficients = XPR_BitVectorGetBits(bv,
                                       8); // matrix_coefficients
        }
    }
    vui->chroma_loc_info_present_flag = XPR_BitVectorGet1Bit(bv);
    if (vui->chroma_loc_info_present_flag) {
        vui->chroma_sample_loc_type_top_field = XPR_BitVectorGetExpGolomb(
                bv); // chroma_sample_loc_type_top_field
        vui->chroma_sample_loc_type_bottom_field = XPR_BitVectorGetExpGolomb(
                    bv); // chroma_sample_loc_type_bottom_field
    }
    vui->timing_info_present_flag = XPR_BitVectorGet1Bit(bv);
    if (vui->timing_info_present_flag) {
#if 0 //VW: Use new stype value caculator, 2012/11/23
#if defined(_BOARD_A5S)
        // num_units_in_tick in Ambarella A5S 1080p frame added 8 bits.
        if (sps->pic_width_in_mbs_minus1 >= 119 &&
            sps->pic_height_in_map_units_minus1 >= 67 &&
            sps->profile_idc == 77 &&
            sps->level_idc == 41) {
            XPR_BitVectorSkip(bv, 24);
            vui->num_units_in_tick = XPR_BitVectorGetBits(bv, 16);
        }
        else {
            vui->num_units_in_tick = XPR_BitVectorGetBits(bv, 32);
        }
#elif defined(_BOARD_A2)
        // num_units_in_tick in Ambarella A2 480p frame added 8 bits.
        if (sps->profile_idc == 77 &&
            sps->level_idc == 31 &&
            sps->pic_width_in_mbs_minus1 == 44 &&
            sps->pic_height_in_map_units_minus1 == 29) {
            XPR_BitVectorSkip(bv, 24);
            vui->num_units_in_tick = XPR_BitVectorGetBits(bv, 16);
        }
        else {
            vui->num_units_in_tick = XPR_BitVectorGetBits(bv, 32);
        }
#else
        vui->num_units_in_tick = XPR_BitVectorGetBits(bv, 32);
#endif
        vui->time_scale = XPR_BitVectorGetBits(bv, 32);
#else // VW: New style value caculator.
        vui->num_units_in_tick = XPR_BitVectorGetBits(bv, 32);
        vui->time_scale = XPR_BitVectorGetBits(bv, 32);
        if (vui->num_units_in_tick == 0 ||
            vui->time_scale == 0 ||
            (vui->time_scale / vui->num_units_in_tick) > 240) {
            XPR_BitVectorRollback(bv, 64);
            XPR_BitVectorSkip(bv, 24);
            vui->num_units_in_tick = XPR_BitVectorGetBits(bv, 16);
            vui->time_scale = XPR_BitVectorGetBits(bv, 32);
        }
#endif
        vui->fixed_frame_rate_flag = XPR_BitVectorGet1Bit(bv);
    }
    vui->nal_hrd_parameters_present_flag = XPR_BitVectorGet1Bit(bv);
    if (vui->nal_hrd_parameters_present_flag) {
        ParseSPS_HRD(bv, sps, &vui->nal_hrd_parameters);
    }
    vui->vcl_hrd_parameters_present_flag = XPR_BitVectorGet1Bit(bv);
    if (vui->vcl_hrd_parameters_present_flag) {
        ParseSPS_HRD(bv, sps, &vui->vcl_hrd_parameters);
    }
    if (vui->nal_hrd_parameters_present_flag ||
        vui->vcl_hrd_parameters_present_flag) {
        XPR_BitVectorSkip(bv, 1); // low_delay_hrd_flag
    }
    XPR_BitVectorSkip(bv, 1); // pic_struct_present_flag
    vui->bitstream_restriction_flag = XPR_BitVectorGet1Bit(bv);
    if (vui->bitstream_restriction_flag) {
        XPR_BitVectorSkip(bv, 1); // motion_vectors_over_pic_boundaries_flag
        (void)XPR_BitVectorGetExpGolomb(bv); // max_bytes_per_pic_denom
        (void)XPR_BitVectorGetExpGolomb(bv); // max_bits_per_mb_denom
        (void)XPR_BitVectorGetExpGolomb(bv); // log2_max_mv_length_horizontal
        (void)XPR_BitVectorGetExpGolomb(bv); // log2_max_mv_length_vertical
        (void)XPR_BitVectorGetExpGolomb(bv); // max_num_reorder_frames
        (void)XPR_BitVectorGetExpGolomb(bv); // max_dec_frame_buffering
    }
}

int XPR_H264_ParseSPS_NALU(const uint8_t* data, unsigned int length,
                           XPR_H264_SPS* sps)
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int lastScale = 0;
    unsigned int nextScale = 0;
    unsigned int sizeOfScalingList = 0;
    unsigned int delta_scale = 0;
    unsigned int seq_scaling_list_present_flag = 0;
    unsigned int num_ref_frames_in_pic_order_cnt_cycle = 0;
    unsigned char bvb[XPR_BITVECTOR_NOB_SIZE] = {0};
    XPR_BitVector* bv = 0;
    //
    if (XPR_H264_HaveStartCode(data, length)) {
        data += 4;
        length -= 4;
    }
    bv = XPR_BitVectorNOB(bvb, XPR_BITVECTOR_NOB_SIZE,
                          (uint8_t*)data, 0, 8 * length);
    if (!bv)
        return XPR_ERR_ERROR;
    //
    sps->forbidden_zero_bit = XPR_BitVectorGetBits(bv, 1);
    sps->nal_ref_idc = XPR_BitVectorGetBits(bv, 2);
    sps->nal_unit_type = XPR_BitVectorGetBits(bv, 5);
    sps->profile_idc = XPR_BitVectorGetBits(bv, 8);
    sps->constraint_setN_flag = XPR_BitVectorGetBits(bv,
                                8); // also "reserved_zero_2bits" at end
    sps->level_idc = XPR_BitVectorGetBits(bv, 8);
    sps->seq_parameter_set_id = XPR_BitVectorGetExpGolomb(bv);
    if (sps->profile_idc == 100 ||
        sps->profile_idc == 110 ||
        sps->profile_idc == 122 ||
        sps->profile_idc == 244 ||
        sps->profile_idc == 44 ||
        sps->profile_idc == 83 ||
        sps->profile_idc == 86 ||
        sps->profile_idc == 118 ||
        sps->profile_idc == 128) {
        sps->chroma_format_idc = XPR_BitVectorGetExpGolomb(bv);
        if (sps->chroma_format_idc == 3) {
            sps->separate_colour_plane_flag = XPR_BitVectorGetBoolean(bv);
        }
        (void)XPR_BitVectorGetExpGolomb(bv); // bit_depth_luma_minus8
        (void)XPR_BitVectorGetExpGolomb(bv); // bit_depth_chroma_minus8
        XPR_BitVectorSkip(bv, 1); // qpprime_y_zero_transform_bypass_flag
        sps->seq_scaling_matrix_present_flag = XPR_BitVectorGet1Bit(bv);
        if (sps->seq_scaling_matrix_present_flag) {
            for (i = 0; i < (unsigned int)((sps->chroma_format_idc != 3) ? 8 : 12); ++i) {
                seq_scaling_list_present_flag = XPR_BitVectorGet1Bit(bv);
                if (seq_scaling_list_present_flag) {
                    sizeOfScalingList = i < 6 ? 16 : 64;
                    lastScale = 8;
                    nextScale = 8;
                    for (j = 0; j < sizeOfScalingList; j++) {
                        if (nextScale != 0) {
                            delta_scale = XPR_BitVectorGetExpGolomb(bv);
                            nextScale = (lastScale + delta_scale + 256) % 256;
                        }
                        lastScale = (nextScale == 0) ? lastScale : nextScale;
                    }
                }
            }
        }
    }
    sps->log2_max_frame_num_minus4 = XPR_BitVectorGetExpGolomb(bv);
    sps->pic_order_cnt_type = XPR_BitVectorGetExpGolomb(bv);
    if (sps->pic_order_cnt_type == 0) {
        sps->log2_max_pic_order_cnt_lsb_minus4 = XPR_BitVectorGetExpGolomb(
                    bv);
    }
    else if (sps->pic_order_cnt_type == 1) {
        XPR_BitVectorSkip(bv, 1); // delta_pic_order_always_zero_flag
        (void)XPR_BitVectorGetExpGolomb(bv); // offset_for_non_ref_pic
        (void)XPR_BitVectorGetExpGolomb(bv); // offset_for_top_to_bottom_field
        num_ref_frames_in_pic_order_cnt_cycle = XPR_BitVectorGetExpGolomb(bv);
        for (i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
            (void)XPR_BitVectorGetExpGolomb(bv); // offset_for_ref_frame[i]
        }
    }
    sps->max_num_ref_frames = XPR_BitVectorGetExpGolomb(bv);
    sps->gaps_in_frame_num_value_allowed_flag = XPR_BitVectorGet1Bit(bv);
    sps->pic_width_in_mbs_minus1 = XPR_BitVectorGetExpGolomb(bv);
    sps->pic_height_in_map_units_minus1 = XPR_BitVectorGetExpGolomb(bv);
    sps->frame_mbs_only_flag = XPR_BitVectorGetBoolean(bv);
    if (!sps->frame_mbs_only_flag) {
        sps->mb_adaptive_frame_field_flag = XPR_BitVectorGet1Bit(bv);
    }
    sps->direct_8x8_inference_flag = XPR_BitVectorGet1Bit(bv);
    sps->frame_cropping_flag = XPR_BitVectorGet1Bit(bv);
    if (sps->frame_cropping_flag) {
        sps->frame_crop_left_offset = XPR_BitVectorGetExpGolomb(
                                          bv); // frame_crop_left_offset
        sps->frame_crop_right_offset = XPR_BitVectorGetExpGolomb(
                                           bv); // frame_crop_right_offset
        sps->frame_crop_top_offset = XPR_BitVectorGetExpGolomb(
                                         bv); // frame_crop_top_offset
        sps->frame_crop_bottom_offset = XPR_BitVectorGetExpGolomb(
                                            bv); // frame_crop_bottom_offset
    }
    sps->vui_parameters_present_flag = XPR_BitVectorGet1Bit(bv);
    if (sps->vui_parameters_present_flag) {
        ParseSPS_VUI(bv, sps, &sps->vui_parameters);
    }
    XPR_BitVectorDestroy(bv);
    return XPR_ERR_SUCCESS;
}

int XPR_H264_ScanNALU(const uint8_t* data, unsigned int length,
                      XPR_H264_NALU nalus[], unsigned int maxNalus)
{
    static const uint8_t kStartCode[4] = { 0x00, 0x00, 0x00, 0x01 };
    int i = 0;
    int l = MIN(length, 1024);
    int n = 0;
    int m = 0;
    for (i = 0; i < l; i++) {
        if (data[i] != kStartCode[m]) {
            m = 0;
            continue;
        }
        if (++m > 3) {
            nalus[n].data = data + i + 1;
            if (n > 0)
                nalus[n-1].length = (unsigned int)(nalus[n].data - nalus[n-1].data - 4);
            if (++n > (int)(maxNalus-1))
                break;
            m = 0;
        }
    }
    if (n > 0)
		nalus[n - 1].length = (unsigned int)(data + length - nalus[n - 1].data);
    return n;
}

void XPR_H264_HRD_Dump(XPR_H264_HRD* hrd, const char* indent)
{
    printf("%scpb_cnt_minus1                          : 0x%x\n", indent,
           hrd->cpb_cnt_minus1);
    printf("%sbit_rate_scale                          : 0x%hhx\n", indent,
           hrd->bit_rate_scale);
    printf("%scpb_size_scale                          : 0x%hhx\n", indent,
           hrd->cpb_size_scale);
    printf("%sbit_rate_value_minus1                   : 0x%hhx\n", indent,
           hrd->bit_rate_value_minus1);
    printf("%scpb_size_value_minus1                   : 0x%hhx\n", indent,
           hrd->cpb_size_value_minus1);
    printf("%scbr_flag                                : 0x%hhx\n", indent,
           hrd->cbr_flag);
    printf("%sinitial_cpb_removal_delay_length_minus1 : 0x%hhx\n", indent,
           hrd->initial_cpb_removal_delay_length_minus1);
    printf("%scpb_removal_delay_length_minus1         : 0x%hhx\n", indent,
           hrd->cpb_removal_delay_length_minus1);
    printf("%sdpb_output_delay_length_minus1          : 0x%hhx\n", indent,
           hrd->dpb_output_delay_length_minus1);
    printf("%stime_offset_length                      : 0x%hhx\n", indent,
           hrd->time_offset_length);
}

void XPR_H264_VUI_Dump(XPR_H264_VUI* vui, const char* indent)
{
    printf("%saspect_ratio_info_present_flag      : 0x%hhx\n", indent,
           vui->aspect_ratio_info_present_flag);
    printf("%saspect_ratio_idc                    : 0x%hhx\n", indent,
           vui->aspect_ratio_idc);
    printf("%ssar_width                           : %d\n", indent,
           vui->sar_width);
    printf("%ssar_height                          : %d\n", indent,
           vui->sar_height);
    printf("%soverscan_info_present_flag          : 0x%hhx\n", indent,
           vui->overscan_info_present_flag);
    printf("%soverscan_appropriate_flag           : 0x%hhx\n", indent,
           vui->overscan_appropriate_flag);
    printf("%svideo_signal_type_present_flag      : 0x%hhx\n", indent,
           vui->video_signal_type_present_flag);
    printf("%svideo_format                        : 0x%hhx\n", indent,
           vui->video_format);
    printf("%svideo_full_range_flag               : 0x%hhx\n", indent,
           vui->video_full_range_flag);
    printf("%scolour_description_present_flag     : 0x%hhx\n", indent,
           vui->colour_description_present_flag);
    printf("%scolour_parimaries                   : 0x%x\n", indent,
           vui->colour_parimaries);
    printf("%stransfer_characteristics            : 0x%x\n", indent,
           vui->transfer_characteristics);
    printf("%smatrix_coefficients                 : 0x%x\n", indent,
           vui->matrix_coefficients);
    printf("%schroma_loc_info_present_flag        : 0x%hhx\n", indent,
           vui->chroma_loc_info_present_flag);
    printf("%stiming_info_present_flag            : 0x%hhx\n", indent,
           vui->timing_info_present_flag);
    printf("%snum_units_in_tick                   : 0x%x\n", indent,
           vui->num_units_in_tick);
    printf("%stime_scale                          : 0x%x\n", indent,
           vui->time_scale);
    printf("%sfixed_frame_rate_flag               : 0x%hhx\n", indent,
           vui->fixed_frame_rate_flag);
    printf("%snal_hrd_parameters_present_flag     : 0x%hhx\n", indent,
           vui->nal_hrd_parameters_present_flag);
    if (vui->nal_hrd_parameters_present_flag)
        XPR_H264_HRD_Dump(&vui->nal_hrd_parameters, "        ");
    printf("%svlc_hrd_parameters_present_flag     : 0x%hhx\n", indent,
           vui->vcl_hrd_parameters_present_flag);
    if (vui->vcl_hrd_parameters_present_flag)
        XPR_H264_HRD_Dump(&vui->vcl_hrd_parameters, "        ");
    printf("%spic_struct_present_flag             : 0x%hhx\n", indent,
           vui->pic_struct_present_flag);
    printf("%sbitstream_restriction_flag          : 0x%hhx\n", indent,
           vui->bitstream_restriction_flag);
}

void XPR_H264_SPS_Dump(XPR_H264_SPS* sps, const char* indent)
{
    indent = indent ? indent : "";
    printf("%sforbidden_zero_bit                      : 0x%hhx\n", indent,
           sps->forbidden_zero_bit);
    printf("%snal_ref_idc                             : %d\n", indent,
           sps->nal_ref_idc);
    printf("%snal_unit_type                           : %d\n", indent,
           sps->nal_unit_type);
    printf("%sconstraint_setN_flag                    : 0x%hhx\n", indent,
           sps->constraint_setN_flag);
    printf("%sprofile_idc                             : %d\n", indent,
           sps->profile_idc);
    printf("%slevel_idc                               : %d\n", indent,
           sps->level_idc);
    printf("%sseq_parameter_set_id                    : 0x%x\n", indent,
           sps->seq_parameter_set_id);
    if (sps->profile_idc == 100 || sps->profile_idc == 110 ||
        sps->profile_idc == 122 ||
        sps->profile_idc == 244 || sps->profile_idc == 44 ||
        sps->profile_idc == 83 ||
        sps->profile_idc == 86) {
        printf("%schroma_format_idc                       : %d\n", indent,
               sps->chroma_format_idc);
        printf("%sseparate_colour_plane_flag              : 0x%hhx\n", indent,
               sps->separate_colour_plane_flag);
        printf("%sbit_depth_luma_minus8                   : %d\n", indent,
               sps->bit_depth_luma_minus8);
        printf("%sbit_depth_chroma_minus8                 : %d\n", indent,
               sps->bit_depth_chroma_minus8);
        printf("%sqpprime_y_zero_transform_bypass_flag    : 0x%hhx\n", indent,
               sps->qpprime_y_zero_transform_bypass_flag);
    }
    printf("%slog2_max_frame_num_minus4               : %d\n", indent,
           sps->log2_max_frame_num_minus4);
    printf("%spic_order_cnt_type                      : 0x%hhx\n", indent,
           sps->pic_order_cnt_type);
    if (sps->pic_order_cnt_type == 0) {
        printf("%s    log2_max_pic_order_cnt_lsb_minus4   : %d\n", indent,
               sps->log2_max_pic_order_cnt_lsb_minus4);
    }
    else {
    }
    printf("%smax_num_ref_frames                      : %d\n", indent,
           sps->max_num_ref_frames);
    printf("%sgaps_in_frame_num_value_allowed_flag    : 0x%hhx\n", indent,
           sps->gaps_in_frame_num_value_allowed_flag);
    printf("%spic_width_in_mbs_minus1                 : %d\n", indent,
           sps->pic_width_in_mbs_minus1);
    printf("%spic_height_in_map_units_minus1          : %d\n", indent,
           sps->pic_height_in_map_units_minus1);
    printf("%sframe_mbs_only_flag                     : 0x%hhx\n", indent,
           sps->frame_mbs_only_flag);
    printf("%sdirect_8x8_inference_flag               : 0x%hhx\n", indent,
           sps->direct_8x8_inference_flag);
    printf("%sframe_cropping_flag                     : 0x%hhx\n", indent,
           sps->frame_cropping_flag);
    printf("%sframe_crop_left_offset                  : %d\n", indent,
           sps->frame_crop_left_offset);
    printf("%sframe_crop_right_offset                 : %d\n", indent,
           sps->frame_crop_right_offset);
    printf("%sframe_crop_top_offset                   : %d\n", indent,
           sps->frame_crop_top_offset);
    printf("%sframe_crop_bottom_offset                : %d\n", indent,
           sps->frame_crop_bottom_offset);
    printf("%svui_parameters_present_flag             : 0x%hhx\n", indent,
           sps->vui_parameters_present_flag);
    if (sps->vui_parameters_present_flag)
        XPR_H264_VUI_Dump(&sps->vui_parameters, "    ");
}

