/*
 * dorime-sp-interface.h
 *
 *  Created on: Mar 6, 2024
 *      Author: pablo-jean
 */

#ifndef DORIME_SERIAL_PROTOCOL_DORIME_SP_INTERFACE_H_
#define DORIME_SERIAL_PROTOCOL_DORIME_SP_INTERFACE_H_

/** Includes **/

#include <stdint.h>

/** Typedefs **/

typedef enum{
	DORIME_INTERFACE_DIRECTION_TX,
	DORIME_INTERFACE_DIRECTION_RX,
	DORIME_INTERFACE_DIRECTION_SLEED
}dorime_interface_dir_e;

/** Function Typedefs **/

typedef uint8_t  (*serialTx)(uint8_t *data, uint16_t len);
typedef uint8_t  (*serialRx)(uint8_t *data, uint16_t len);
typedef uint8_t  (*dirSelection)(dorime_interface_dir_e dir);
typedef uint32_t (*getCnt)();
typedef void 	 (*lock)();
typedef void 	 (*unlock)();
typedef void*    (*memAlloc)(uint32_t size);
typedef void	 (*memFree)(void* p);

/** Publics **/
/** Must be source code defined for the chipset **/

#endif /* DORIME_SERIAL_PROTOCOL_DORIME_SP_INTERFACE_H_ */
