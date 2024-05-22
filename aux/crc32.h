/*
 * crc32.h
 *
 *  Created on: May 20, 2024
 *      Author: pablo-jean
 */

#ifndef DORIME_SERIAL_PROTOCOL_AUX_CRC32_H_
#define DORIME_SERIAL_PROTOCOL_AUX_CRC32_H_

#include <stdint.h>

unsigned int
xcrc32 (const unsigned char *buf, int len, unsigned int init);

#endif /* DORIME_SERIAL_PROTOCOL_AUX_CRC32_H_ */
