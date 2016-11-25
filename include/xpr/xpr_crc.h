#ifndef XPR_CRC_H
#define XPR_CRC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	// ���� 16 λ CRC ֵ
	uint16_t XPR_CRC16(uint16_t crc, const void *buf, size_t size);

	// ���� 32 λ CRC ֵ
	uint32_t XPR_CRC32(uint32_t crc, const void *buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif // XPR_CRC_H
