#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#else                  // _WIN32
// For the socket handling
#include <arpa/inet.h> // inet_aton
#include <netdb.h>
#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // open, close

#endif              // _WIN32
#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_file.h>
#include <xpr/xpr_h264.h>
#include <xpr/xpr_pes.h>

#ifdef _MSC_VER
#define ATTR_PACKED
#elif defined(__GNUC__)
#define ATTR_PACKED __attribute__((packed, aligned(1)))
#else
#define ATTR_PACKED
#endif

// 5112//8192//5112  // The number of bytes to read ahead
#define PES_PACKET_SIZE 5120
#define PES_PACKET_BUFFER_SIZE 8192
#define PES_MAX_CALLBACKS 16
#define PES_HEADER_FIXED_SIZE 112

struct XPR_PES
{
    void* outputFile;
    XPR_PES_WriteCallback writeCallback[PES_MAX_CALLBACKS];
    void* writeCallbackOpaque[PES_MAX_CALLBACKS];
    uint8_t* packetBuffer;
    uint32_t packetLength;
    int packetBufferSize;
    uint32_t totalPackets;
    int havePSM;
    float videoFPS;
    int videoWidth;
    int videoHeight;
    int audioCodec;
};

/// @brief pes Header
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
struct XPR_PES_Header
{
    uint8_t psHeader[16];
    uint8_t psMap[96];
} ATTR_PACKED;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
typedef struct ProgramPackHeader
{
    uint8_t pack_start_code[4]; //'0x000001BA'

    uint8_t sclk_28_29 : 2;
    uint8_t marker_bit : 1;
    uint8_t sclk_30_32 : 3;
    uint8_t fix_bit : 2; //'01'

    uint8_t sclk_20_27;

    uint8_t sclk_13_14 : 2;
    uint8_t marker_bit1 : 1;
    uint8_t sclk_15_19 : 5;

    uint8_t sclk_5_12; // bits[12..5]
    uint8_t sclk_ext_0_1 : 2;
    uint8_t marker_bit2 : 1;
    uint8_t sclk_0_4 : 5; // bits[4..0]

    uint8_t marker_bit3 : 1;
    uint8_t sclk_ext_2_8 : 7; // bits[8...2]

    uint8_t program_mux_rate1;

    uint8_t program_mux_rate2;
    uint8_t marker_bit5 : 1;
    uint8_t marker_bit4 : 1;
    uint8_t program_mux_rate3 : 6;

    uint8_t pack_stuffing_length : 3;
    uint8_t reserved : 5;

} ATTR_PACKED ProgramPackHeader;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
typedef struct PESPacketHeader
{
    uint8_t packet_start_code_prefix[3];
    uint8_t stream_id;
    uint8_t pes_packet_length[2];

    uint8_t original_or_copy : 1;
    uint8_t copyright : 1;
    uint8_t data_alignment_indicator : 1;
    uint8_t pes_priority : 1;
    uint8_t pes_scrambling_control : 2;
    uint8_t fix_bit : 2;

    uint8_t pes_extension_flag : 1;
    uint8_t pes_crc_flag : 1;
    uint8_t additional_copy_info_flag : 1;
    uint8_t dsm_trick_mode_flag : 1;
    uint8_t es_rate_flag : 1;
    uint8_t escr_flag : 1;
    uint8_t pts_dts_flags : 2;

    uint8_t pes_header_data_length;

    // PTS Tag [optional]
    // DTS Tag [optional]
    // ESCR Tag [optional]
    // DSMTrickMode Tag [optional]
    // CopyrightInfo Tag [optional]
    // CRC Tag [optional]
    // Externsion Tag [optional]
} ATTR_PACKED PESPacketHeader;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
typedef struct PESPacketPTSTag
{
    uint8_t marker_bit : 1;
    uint8_t pts1 : 3;
    uint8_t fix_bit : 4;

    uint8_t pts21;
    uint8_t marker_bit1 : 1;
    uint8_t pts22 : 7;

    uint8_t pts31;
    uint8_t marker_bit2 : 1;
    uint8_t pts32 : 7;

} ATTR_PACKED PESPacketPTSTag;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

// psm
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif

typedef struct PSMHeader
{
    uint8_t start_code[4]; // Must be 0x000001BC
    uint8_t psm_length[2]; // Total number of bytes in psm following this field.
    uint8_t next_indicator; // Current_next_indicator, reserved and
                            // program_stream_map_version.
    // It should be 0xE0
    uint8_t psm_marker; // Reserved and marker bit. It should be 0xFF
    uint8_t
        psi_length[2]; // Total length of the descriptors following this field.
    // Followed by PSMInfoDate and PSMInfoUnknown.
    // uint8_t esm_length[2];  //Followed by one or two PSMElementary.

} ATTR_PACKED PSMHeader;

typedef struct PSMInfoDate
{
    uint8_t desc_tag;      // Must be 0x40
    uint8_t desc_length;   // Must be 0x0E
    uint16_t company_name; // Must be 0x484B
    uint16_t unknown_type; // Must be 0x0001
    uint8_t year;          //***Must be same as PTS***
    uint8_t time[4];
    uint8_t reserved[5]; //***Must be set 07ffffffff
} ATTR_PACKED PSMInfoDate;

typedef struct PSMInfoUnknown
{
    uint8_t desc_tag;      // Must be 0x41;
    uint8_t desc_length;   // Must be 0x12
    uint16_t company_name; // Must be 0x484B
    uint8_t reserved[16];  // Should be 0x0
    uint8_t esm_length[2]; // Followed by one or two PSMElementary.
} ATTR_PACKED PSMInfoUnknown;

typedef struct PSMElementary
{
    uint8_t stream_type; // H.264-0x1B, PCMA-0x90, PCMU-0x91
    uint8_t stream_id;   // 0xE0 for video, 0xC0 for audio
    uint16_t esi_length; // The length of video info or audio info.
} ATTR_PACKED PSMElementary;

typedef struct PSMElementaryVideoInfo
{
    uint8_t desc_tag;      // Must be 0x42.
    uint8_t desc_length;   // Must be 0x0E.
    uint32_t unknown_type; // Must be 0x0000A021.
    uint16_t width;        // Frame width.
    uint16_t height;       // Frame height.
    uint32_t fix_bytes;    // Must be 0x121FFF00.
    uint16_t dur_time;     //(180*1000/fps+1)
} ATTR_PACKED PSMElementaryVideoInfo;

#if 0
typedef struct PSMElementaryAudioInfo {
    uint8_t desc_tag;      //Must be 0x43
    uint8_t desc_length;      //Must be 0x0A
    uint8_t reserved[10];  //We don't know it yet.
} ATTR_PACKED PSMElementaryAudioInfo;
#endif

typedef struct PSMESIAudio
{
    uint8_t stream_type;    // PCMA-0x90, PCMU-0x91
    uint8_t stream_id;      // 0xC0
    uint8_t esi_length[2];  // 0x000C
    uint8_t info_tag;       // 0x43
    uint8_t info_length;    // 0x0A
    uint32_t unknown_type;  // 0x0000FE00
    uint8_t audio_info1[2]; // 0x7D03
    uint8_t audio_info2[2]; // 711-0x03E8, 726-0x00FA
    uint16_t fix_bytes;     // 0x03FF
} ATTR_PACKED PSMESIAudio;

typedef struct PSMRDInfo
{
    uint8_t stream_type;
    uint8_t stream_id;
    uint16_t stream_length;
    uint32_t stuff_bytes;
} ATTR_PACKED PSMRDInfo;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

void XPR_PES_SetPTS(PESPacketPTSTag* tag, int64_t pts)
{
    pts = pts * 90 / 1000;
    tag->pts1 = (pts >> 30) & 0x07;
    tag->pts21 = (pts >> 22) & 0xFF;
    tag->pts22 = (pts >> 15) & 0x7F;
    tag->pts31 = (pts >> 7) & 0xFF;
    tag->pts32 = pts & 0x7F;
}

static int XPR_PES_DefaultWrite(const uint8_t* data, int length, void* opaque)
{
    XPR_PES* p = (XPR_PES*)opaque;
    int n = XPR_FileWrite(p->outputFile, data, length);
    return n;
}

static XPR_PES* XPR_PES_Alloc(void)
{
    return (XPR_PES*)calloc(sizeof(XPR_PES), 1);
}

static void XPR_PES_Free(XPR_PES* p)
{
    free((void*)p);
}

static int XPR_PES_Init(XPR_PES* p)
{
    p->writeCallback[0] = XPR_PES_DefaultWrite;
    p->writeCallbackOpaque[0] = p;
    p->packetBuffer = (uint8_t*)malloc(PES_PACKET_BUFFER_SIZE);
    p->packetBufferSize = PES_PACKET_BUFFER_SIZE;
    p->totalPackets = 1;
    p->havePSM = 1;
    p->audioCodec = AV_FOURCC_PCMA;
    return 0;
}

static void XPR_PES_Fini(XPR_PES* p)
{
    if (p->packetBuffer) {
        free(p->packetBuffer);
        p->packetBuffer = 0;
        p->packetBufferSize = 0;
    }
}

static void XPR_PES_PostPESPacket(XPR_PES* p, const uint8_t* data, int length)
{
    int i = 0;
    for (; i < PES_MAX_CALLBACKS; i++) {
        if (p->writeCallback[i])
            p->writeCallback[i](data, length, p->writeCallbackOpaque[i]);
    }
}

XPR_API XPR_PES* XPR_PES_Open(const char* url)
{
    XPR_PES* p = XPR_PES_Alloc();
    if (p) {
        if (XPR_PES_Init(p) < 0) {
            XPR_PES_Free(p);
            p = 0;
        }
        if (!url) {
            p->writeCallback[0] = 0;
            p->writeCallbackOpaque[0] = 0;
        }
        else if (strcmp(url, "icbx://dummy") == 0 ||
                 strcmp(url, "file://dummy") == 0) {
            p->writeCallback[0] = 0;
            p->writeCallbackOpaque[0] = 0;
        }
        else {
            if (strncmp(url, "file://", 7) == 0)
                p->outputFile = XPR_FileOpen(url + 7, "wc");
            else
                p->outputFile = XPR_FileOpen(url, "wc");
        }
    }
    return p;
}

XPR_API int XPR_PES_Close(XPR_PES* p)
{
    if (p->outputFile) {
        XPR_FileClose(p->outputFile);
        p->outputFile = 0;
    }
    XPR_PES_Fini(p);
    XPR_PES_Free(p);
    return 0;
}

XPR_API int XPR_PES_AddWriteCallback(XPR_PES* p, XPR_PES_WriteCallback cb,
                                     void* opaque)
{
    int i = 0;
    for (; i < 16; i++) {
        if (p->writeCallback[i] == 0) {
            p->writeCallback[i] = cb;
            p->writeCallbackOpaque[i] = opaque;
            return 0;
        }
    }
    return -1;
}

XPR_API int XPR_PES_DeleteWriteCallback(XPR_PES* p, XPR_PES_WriteCallback cb,
                                        void* opaque)
{
    int i = 0;
    for (; i < 16; i++) {
        if ((p->writeCallback[i] == cb) &&
            (p->writeCallbackOpaque[i] == opaque)) {
            p->writeCallback[i] = 0;
            p->writeCallbackOpaque[i] = 0;
            return 0;
        }
    }
    return -1;
}

XPR_API void setPsiDate(PSMInfoDate* info_date, int64_t pts)
{
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    if (!info_date)
        return;
    // Currently, we use system time.
    info_date->desc_tag = 0x40;
    info_date->desc_length = 0x0E;
    info_date->company_name = 0x4B48;
    info_date->unknown_type = 0x0100;
#if 0
    SYSTEMTIME sys_time;
    GetLocalTime(&sys_time);
    info_date->year = sys_time.wYear - 2000;
    uint8_t month = (uint8_t)sys_time.wMonth;
    uint8_t day = (uint8_t)sys_time.wDay;
    uint8_t hour = (uint8_t)sys_time.wHour;
    uint8_t minute = (uint8_t)sys_time.wMinute;
    uint8_t second = (uint8_t)sys_time.wSecond;
#endif
    info_date->year = tm->tm_year + 1900 - 2000;
    info_date->time[0] = ((tm->tm_mon + 1) << 4) & 0xF0;
    info_date->time[0] |= (tm->tm_mday >> 1) & 0xF;
    info_date->time[1] = (tm->tm_mday & 0x1) << 7;
    info_date->time[1] |= (tm->tm_hour << 2) & 0x7C;
    info_date->time[1] |= (tm->tm_min >> 4) & 0x3;
    info_date->time[2] = (tm->tm_min << 4) & 0xf0;
    info_date->time[2] |= (tm->tm_sec >> 2) & 0x0f;
    info_date->time[3] = (tm->tm_sec << 6) & 0xf0;
    info_date->time[3] |= 0x20;
    info_date->reserved[0] = 0x07;
    info_date->reserved[1] = 0xff;
    info_date->reserved[2] = 0xff;
    info_date->reserved[3] = 0xff;
    info_date->reserved[4] = 0xff;
}

static int XPR_PES_WritePSM(XPR_PES* p, int codec)
{
    int offset = 0;
    uint8_t* data = p->packetBuffer + p->packetLength;
    PSMHeader* psm_header = (PSMHeader*)(data);
    // PSMHeader psm_header;
    memset(psm_header, 0, sizeof(*psm_header));
    uint8_t CRC_32[4] = {0, 0, 0, 0};
    // PSM header
    psm_header->start_code[0] = 0x00;
    psm_header->start_code[1] = 0x00;
    psm_header->start_code[2] = 0x01;
    psm_header->start_code[3] = 0xBC;
    uint16_t psm_length =
        (sizeof(*psm_header) - sizeof(psm_header->start_code) -
         sizeof(psm_header->psm_length));
    psm_header->next_indicator = 0xE0;
    psm_header->psm_marker = 0xFF;
    uint16_t psi_length = sizeof(PSMInfoDate) + sizeof(PSMInfoUnknown);
    uint16_t esm_length = 0;
    if ((codec == AV_FOURCC_H264) || (codec == AV_FOURCC_JPEG)) {
        esm_length += sizeof(PSMElementary) + sizeof(PSMElementaryVideoInfo);
    }
    if ((p->audioCodec == AV_FOURCC_PCMA) ||
        (p->audioCodec == AV_FOURCC_PCMU)) {
        esm_length += sizeof(PSMESIAudio);
    }
    // uint16_t audio_length = sizeof(PSMESIAudio);
    psm_header->psi_length[0] = ((psi_length - 2) >> 8) & 0xFF;
    psm_header->psi_length[1] = (psi_length - 2) & 0xFF;
    // psm_length += psi_length  + esm_length + 14 + 4;
    psm_length += psi_length + esm_length + 4; // + 14 + 4;
    psm_header->psm_length[0] = 0x00;          //(psm_length >> 8) & 0xFF;
    psm_header->psm_length[1] = 0x5A;          // psm_length & 0xFF;
    offset += sizeof(*psm_header);
    // Program stream information.
    PSMInfoDate* info_date = (PSMInfoDate*)(data + offset);
    setPsiDate(info_date, 0);
    offset += sizeof(*info_date);
    PSMInfoUnknown* info_unknown = (PSMInfoUnknown*)(data + offset);
    info_unknown->desc_tag = 0x41;
    info_unknown->desc_length = 0x12;
    info_unknown->company_name = 0x4B48;
    memset(info_unknown->reserved, 0, 16);
    info_unknown->esm_length[0] = 0x00; //((esm_length+4) >> 8) & 0xFF;
    info_unknown->esm_length[1] = 0x2C; //(esm_length + 4) & 0xFF;
    offset += sizeof(*info_unknown);
    // Elementary stream information.
    if ((codec == AV_FOURCC_H264) || (codec == AV_FOURCC_JPEG)) {
        PSMElementary* es_map = (PSMElementary*)(data + offset);
        if (codec == AV_FOURCC_H264) {
            es_map->stream_type = 0x1B;
        }
        else if (codec == AV_FOURCC_JPEG) {
            es_map->stream_type = 0xB1;
        }
        es_map->stream_id = 0xE0;
        es_map->esi_length = 0x1000;
        offset += sizeof(*es_map);
        PSMElementaryVideoInfo* video_info =
            (PSMElementaryVideoInfo*)(data + offset);
        video_info->desc_tag = 0x42;
        video_info->desc_length = 0x0E;
        video_info->unknown_type = 0x21A00000;
        video_info->width = (uint16_t)p->videoWidth >> 8 &
                            0xFF; //海康私有必须要否则无法支持海康
        video_info->width |= ((uint16_t)p->videoWidth & 0x00FF) << 8;
        video_info->height = (uint16_t)p->videoHeight >> 8 &
                             0xFF; //海康私有必须要否则无法支持海康
        video_info->height |= ((uint16_t)p->videoHeight & 0x00FF) << 8;
        video_info->fix_bytes = 0x00FF1F12;
        video_info->dur_time =
            (180 * 1000 / 25 + 1) >> 8 & 0xFF; //海康私有必须要否则无法支持海康
        video_info->dur_time |= ((180 * 1000 / 25 + 1) & 0x00FF)
                                << 8; //海康私有必须要否则无法支持海康
        offset += sizeof(*video_info);
    }
    //海康额外数据
    PSMESIAudio* esi_audio = (PSMESIAudio*)(data + offset);
    memset(esi_audio, 0, sizeof(*esi_audio));
    if (p->audioCodec == AV_FOURCC_PCMA) {
        esi_audio->stream_type = 0x90;
    }
    else if (p->audioCodec == AV_FOURCC_PCMU) {
        esi_audio->stream_type = 0x91;
    }
    else {
        esi_audio->stream_type = 0x90;
    }
    // esi_audio->stream_type = es_map->stream_type;
    esi_audio->stream_id = 0xC0;
    esi_audio->esi_length[0] = 0x00;
    esi_audio->esi_length[1] = 0x0C;
    esi_audio->info_tag = 0x43;
    esi_audio->info_length = 0x0A;
    esi_audio->unknown_type = 0x00FE0000; // We should reverse the bytes order.
    esi_audio->audio_info1[0] = 0x7D;
    esi_audio->audio_info1[1] = 0x03;
    esi_audio->audio_info2[0] = 0x03;
    esi_audio->audio_info2[1] = 0xE8;
    esi_audio->fix_bytes = 0xFF03;
    offset += sizeof(*esi_audio);
    PSMRDInfo* rd_info = (PSMRDInfo*)(data + offset);
    rd_info->stream_type = 0xBD;
    rd_info->stream_id = 0xBD;
    rd_info->stream_length = 0x0;
    rd_info->stuff_bytes = 0x0000BFBF;
    offset += sizeof(*rd_info);
    memcpy(data + offset, CRC_32, 4);
    offset += 4;
    p->packetLength += offset;
    return offset;
}

/// @brief Write Program Pack Header
/// @return Program Pack Header length
static int XPR_PES_WriteProgramPackHeader(XPR_PES* p, int64_t pts, int codec)
{
    int offset = 0;
    ProgramPackHeader* ppHeader = (ProgramPackHeader*)p->packetBuffer;
    ppHeader->pack_start_code[0] = 0x00;
    ppHeader->pack_start_code[1] = 0x00;
    ppHeader->pack_start_code[2] = 0x01;
    ppHeader->pack_start_code[3] = 0xBA;
    ppHeader->fix_bit = 0x01;
    ppHeader->marker_bit = 0x01;
    ppHeader->marker_bit1 = 0x01;
    ppHeader->marker_bit2 = 0x01;
    ppHeader->marker_bit3 = 0x01;
    ppHeader->marker_bit4 = 0x01;
    ppHeader->marker_bit5 = 0x01;
    ppHeader->reserved = 0x1F;
    ppHeader->pack_stuffing_length = 0x06;
    ppHeader->sclk_ext_0_1 = 0;
    ppHeader->sclk_ext_2_8 = 0;
    int64_t ts = pts * 90;
    ts /= 1000;
    ppHeader->sclk_0_4 = ts & 0x1f;
    ppHeader->sclk_5_12 = ts >> 5 & 0xff;
    ppHeader->sclk_13_14 = ts >> 13 & 0x03;
    ppHeader->sclk_15_19 = ts >> 15 & 0x1f;
    ppHeader->sclk_20_27 = ts >> 20 & 0xff;
    ppHeader->sclk_28_29 = ts >> 28 & 0x03;
    ppHeader->sclk_30_32 = ts >> 30 & 0x07;
    ppHeader->program_mux_rate1 = 0x0;
    ppHeader->program_mux_rate2 = 0x0;
    ppHeader->program_mux_rate3 = 0x0;
    memset(((uint8_t*)ppHeader) + sizeof(*ppHeader), 0xFF,
           ppHeader->pack_stuffing_length);
    offset += sizeof(*ppHeader) + ppHeader->pack_stuffing_length - 4;
    p->packetBuffer[offset++] = (p->totalPackets >> 24) & 0xFF;
    p->packetBuffer[offset++] = (p->totalPackets >> 16) & 0xFF;
    p->packetBuffer[offset++] = (p->totalPackets >> 8) & 0xFF;
    p->packetBuffer[offset++] = (p->totalPackets >> 0) & 0xFF;
    p->packetLength = offset;
    p->totalPackets++;
    return offset;
}

#define NALU_AUD 0x09
#define NALU_SEI 0x06
#define NALU_SPS 0x07
#define NALU_PPS 0x08
#define NALU_I_SLC 0x05
#define NALU_P_SLC 0x01

static int XPR_PES_WritePESPacket(XPR_PES* p, const uint8_t* data, int length,
                                  int codec, int64_t pts, int firstPacket)
{
    int packets = 0;
    int total = length;
    int haveStartCode = 0;
    int naluType = 0;
    int offset, padLen, packetLen, esBlock;
    int isLastPacket = 0;
    // Check AVC/H264 Start Code
    if (codec == AV_FOURCC_H264 && length > 4) {
        if (data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 1) {
            haveStartCode = 1;
            naluType = data[4] & 0x1f;
        }
        else {
            total += 4; // Add start code bytes
            naluType = data[0] & 0x1f;
        }
    }
    //
    while (total > 0) {
        offset = p->packetLength;
        padLen = 0;
        packetLen = 0;
        esBlock = 0;
        PESPacketHeader* pesHeader =
            (PESPacketHeader*)(p->packetBuffer + offset);
        pesHeader->packet_start_code_prefix[0] = 0x00;
        pesHeader->packet_start_code_prefix[1] = 0x00;
        pesHeader->packet_start_code_prefix[2] = 0x01;
        pesHeader->fix_bit = 0x02;
        pesHeader->original_or_copy = 0;
        pesHeader->copyright = 0;
        if (packets == 0) {
            pesHeader->data_alignment_indicator = 1;
        }
        else {
            pesHeader->data_alignment_indicator = 0;
        }
        pesHeader->pes_priority = 1;
        pesHeader->pes_scrambling_control = 0;
        pesHeader->pes_extension_flag = 0;
        pesHeader->pes_crc_flag = 0;
        pesHeader->additional_copy_info_flag = 0;
        pesHeader->dsm_trick_mode_flag = 0;
        pesHeader->es_rate_flag = 0;
        pesHeader->escr_flag = 0;
        pesHeader->pts_dts_flags = 0;
        pesHeader->pes_header_data_length = 2;
        if ((codec == AV_FOURCC_H264) || (codec == AV_FOURCC_JPEG)) {
            pesHeader->stream_id = 0xE0;
        }
        else if ((codec == AV_FOURCC_AAC) || (codec == AV_FOURCC_PCMA) ||
                 (codec == AV_FOURCC_PCMU)) {
            pesHeader->stream_id = 0xC0;
        }
        padLen = 2;     // min stuff bytes
        packetLen += 5; // 3 fixed bytes + 2 stuff bytes
        offset += sizeof(*pesHeader);
        // Setup PTS Tag
        if (packets == 0 && firstPacket) {
            pesHeader->pts_dts_flags = 2;
            PESPacketPTSTag* pesPTS =
                (PESPacketPTSTag*)(p->packetBuffer + offset);
            pesPTS->fix_bit = 0x02;
            pesPTS->marker_bit = 0x01;
            pesPTS->marker_bit1 = 0x01;
            pesPTS->marker_bit2 = 0x01;
            XPR_PES_SetPTS(pesPTS, pts);
            pesHeader->pes_header_data_length += sizeof(*pesPTS);
            offset += sizeof(*pesPTS);
            packetLen += sizeof(*pesPTS);
        }
        // Setup xxxx
        // VIDEO_SPS-0xFC, VIDEO_PPS-0xFC, H264_Slice-FD, H264_Tail-0xFA,
        // VIDEO_MIDDLE-0xFF,MJPEG_FRONT-0xFD, MJPEG_Tail-0xFE, Audio_FRONT-0xFC
        if (total >
            (PES_PACKET_SIZE - packetLen - 6)) // 6 fixed pes header bytes
            esBlock = PES_PACKET_SIZE - 6 - packetLen;
        else
            esBlock = total;
        int a = (6 + packetLen + esBlock) % 4;
        a = a ? 4 - a : 0;
        padLen += a;
        pesHeader->pes_header_data_length += a;
        packetLen += a;
        if ((6 + packetLen + padLen + esBlock) > PES_PACKET_SIZE)
            esBlock -= a;
        if (PES_PACKET_SIZE > (6 + packetLen + padLen + esBlock))
            isLastPacket = 1;
        while (padLen-- > 1)
            p->packetBuffer[offset++] = 0xff;
        switch (codec) {
        case AV_FOURCC_PCMA:
        case AV_FOURCC_PCMU:
            p->packetBuffer[offset++] = 0xfc;
            break;
        case AV_FOURCC_H264:
            switch (naluType) {
            case NALU_SEI:
            case NALU_SPS:
                p->packetBuffer[offset++] = 0xfc;
                break;
            case NALU_PPS:
                p->packetBuffer[offset++] = 0xfc;
                break;
            case NALU_I_SLC:
            case NALU_P_SLC:
                if (packets == 0)
                    p->packetBuffer[offset++] = 0xfd;
                else if (isLastPacket)
                    p->packetBuffer[offset++] = 0xfa;
                else
                    p->packetBuffer[offset++] = 0xff;
                break;
            default:
                p->packetBuffer[offset++] = 0xff;
                break;
            }
            break;
        case AV_FOURCC_JPEG:
            if (packets == 0)
                p->packetBuffer[offset++] = 0xfd;
            else if (isLastPacket)
                p->packetBuffer[offset++] = 0xfe;
            else
                p->packetBuffer[offset++] = 0xff;
            break;
        default:
            p->packetBuffer[offset++] = 0xff;
            break;
        }
        // Setup ES Data
        if (packets == 0 && codec == AV_FOURCC_H264 && !haveStartCode) {
            p->packetBuffer[offset++] = 0;
            p->packetBuffer[offset++] = 0;
            p->packetBuffer[offset++] = 0;
            p->packetBuffer[offset++] = 1;
            total -= 4;
        }
        memcpy(p->packetBuffer + offset, (data + length - total), esBlock);
        offset += esBlock;
        packetLen += esBlock;
        // Setup packet length
        pesHeader->pes_packet_length[0] = (packetLen >> 8) & 0xFF;
        pesHeader->pes_packet_length[1] = packetLen & 0xFF;
        // Post out
        XPR_PES_PostPESPacket(p, p->packetBuffer, offset);
        //
        p->packetLength = 0;
        total -= esBlock;
        packets++;
    }
    return 0;
}

static int XPR_PES_WriteH264Frame(XPR_PES* p, const uint8_t* data, int length,
                                  int codec, int64_t pts)
{
    int i = 0;
    int count = 0;
    int offset = 0;
    int naluType = 0;
    int naluCount = 0;
    XPR_H264_FrameInfo fi;
    XPR_H264_NALU nalus[16];
    //
    naluCount = XPR_H264_ScanNALU(data, length, nalus, 16);
    if (naluCount < 1)
        return -1;
    if (XPR_H264_ProbeFrameInfoEx(nalus, naluCount, &fi) < 0)
        return -1;
    //
    p->videoFPS = fi.fps;
    p->videoWidth = fi.width;
    p->videoHeight = fi.height;
    //
    XPR_PES_WriteProgramPackHeader(p, pts, codec);
    //
    if (p->havePSM && fi.keyFrame)
        XPR_PES_WritePSM(p, codec);
    // Write each nalu
    for (i = 0; i < naluCount; i++) {
        naluType = nalus[i].data[0] & 0x1F;
        if (naluType ==
            XPR_H264_NALU_TYPE_AUD /* || naluType ==XPR_H264_NALU_TYPE_SEI*/)
            continue;
        XPR_PES_WritePESPacket(p, nalus[i].data - 4, nalus[i].length + 4, codec,
                               pts, count ? 0 : 1);
        count++;
    }
    (void)offset;
    return 0;
}

static int XPR_PES_WriteH264FramePartial(XPR_PES* p, const uint8_t* data,
                                         int length, int codec, int64_t pts,
                                         int firstPart)
{
    int haveStartCode = 0;
    int naluType = 0;
    XPR_H264_FrameInfo fi;
    XPR_H264_NALU nalu;
    //
    if (length > 4 &&
        (data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 1))
        haveStartCode = 1;
    if (haveStartCode) {
        nalu.data = data + 4;
        nalu.length = length - 4;
    }
    else {
        nalu.data = data;
        nalu.length = length;
    }
    //
    naluType = nalu.data[0] & 0x1f;
    //
    if (firstPart)
        XPR_PES_WriteProgramPackHeader(p, pts, codec);
    //
    if (naluType == XPR_H264_NALU_TYPE_SPS) {
        if (!p->videoWidth || !p->videoHeight) {
            if (XPR_H264_ProbeFrameInfoEx(&nalu, 1, &fi) == XPR_ERR_SUCCESS) {
                p->videoFPS = fi.fps;
                p->videoWidth = fi.width;
                p->videoHeight = fi.height;
            }
        }
        if (p->havePSM)
            XPR_PES_WritePSM(p, codec);
    }
    XPR_PES_WritePESPacket(p, data, length, codec, pts, firstPart);
    return 0;
}

static int detectJPEGSize(const uint8_t* data, int length, int* width,
                          int* height)
{
    int i = 0;
    //
    if (data[0] != 0xff && data[1] != 0xd8)
        return -1;
    *width = *height = 0;
    for (i = 0; i < length - 9; i++) {
        if (data[i] == 0xff && data[i + 1] == 0xc0) {
            *height = data[i + 5];
            *height <<= 8;
            *height |= data[i + 6];
            *width = data[i + 7];
            *width <<= 8;
            *width |= data[i + 8];
            break;
        }
    }
    return 0;
}

static int XPR_PES_WriteJPEGFrame(XPR_PES* p, const uint8_t* data, int length,
                                  int codec, int64_t pts)
{
    if (detectJPEGSize(data, length, &p->videoWidth, &p->videoHeight) < 0)
        return -1;
    XPR_PES_WriteProgramPackHeader(p, pts, codec);
    XPR_PES_WritePSM(p, codec);
    return XPR_PES_WritePESPacket(p, data, length, codec, pts, 1);
}

static int XPR_PES_WritePCMAFrame(XPR_PES* p, const uint8_t* data, int length,
                                  int codec, int64_t pts)
{
    p->packetLength = 0;
    p->audioCodec = codec;
    return XPR_PES_WritePESPacket(p, data, length, codec, pts, 1);
}

static int XPR_PES_WritePCMUFrame(XPR_PES* p, const uint8_t* data, int length,
                                  int codec, int64_t pts)
{
    p->packetLength = 0;
    p->audioCodec = codec;
    return XPR_PES_WritePESPacket(p, data, length, codec, pts, 1);
}

XPR_API int XPR_PES_WriteFrame(XPR_PES* p, const uint8_t* data, int length,
                               int codec, int64_t pts)
{
    int result = -1;
    if (!data || length < 8)
        return -1;
    switch (codec) {
    case AV_FOURCC_PCMA:
        result = XPR_PES_WritePCMAFrame(p, data, length, codec, pts);
        break;
    case AV_FOURCC_PCMU:
        result = XPR_PES_WritePCMUFrame(p, data, length, codec, pts);
        break;
    case AV_FOURCC_JPEG:
        result = XPR_PES_WriteJPEGFrame(p, data, length, codec, pts);
        break;
    case AV_FOURCC_H264:
        result = XPR_PES_WriteH264Frame(p, data, length, codec, pts);
        break;
    case AV_FOURCC_HKMI:
    case AV_FOURCC_MP2P:
        XPR_PES_PostPESPacket(p, data, length);
        result = 0;
        break;
    default:
        break;
    }
    return result;
}

XPR_API int XPR_PES_WriteFramePartial(XPR_PES* p, const uint8_t* data,
                                      int length, int codec, int64_t pts,
                                      int firstPart)
{
    int result = -1;
    if (!data || length < 2)
        return -1;
    switch (codec) {
    case AV_FOURCC_PCMA:
        result = XPR_PES_WritePCMAFrame(p, data, length, codec, pts);
        break;
    case AV_FOURCC_PCMU:
        result = XPR_PES_WritePCMUFrame(p, data, length, codec, pts);
        break;
    case AV_FOURCC_JPEG:
        result = XPR_PES_WriteJPEGFrame(p, data, length, codec, pts);
        break;
    case AV_FOURCC_H264:
        result = XPR_PES_WriteH264FramePartial(p, data, length, codec, pts,
                                               firstPart);
        break;
    case AV_FOURCC_HKMI:
    case AV_FOURCC_MP2P:
        XPR_PES_PostPESPacket(p, data, length);
        result = 0;
        break;
    default:
        break;
    }
    return result;
}

XPR_API int XPR_PES_WriteTailer(XPR_PES* p)
{
    return 0;
}

XPR_API int XPR_PES_WriteOpaque(XPR_PES* p, const uint8_t* data, int length)
{
    XPR_PES_PostPESPacket(p, data, length);
    return length;
}

XPR_API XPR_PES_Header* XPR_PES_HeaderNew(XPR_PES* p)
{
    XPR_PES_Header* h = (XPR_PES_Header*)calloc(sizeof(*h), 1);
    XPR_PES_HeaderInit(p, h);
    return h;
}

XPR_API void XPR_PES_HeaderInit(XPR_PES* p, XPR_PES_Header* hdr)
{
    // TODO: FIXME
}

XPR_API void XPR_PES_HeaderDestroy(XPR_PES_Header* hdr)
{
    free((void*)hdr);
}

XPR_API int XPR_PES_WriteHeader(XPR_PES* p, const XPR_PES_Header* hdr)
{
    if (!p || !hdr)
        return -1;
    return p->writeCallback[0]((const uint8_t*)hdr, PES_HEADER_FIXED_SIZE,
                               p->writeCallbackOpaque);
}
