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
	DORIME_INTERFACE_DIRECTION_SLEEP
}dorime_interface_dir_e;

/** Function Typedefs **/

typedef uint8_t  (*serialTx)(uint8_t *data, uint16_t len);
typedef uint8_t  (*serialRx)(uint8_t *data, uint16_t len);
typedef uint8_t  (*dirSelection)(dorime_interface_dir_e dir);
typedef uint32_t (*getCnt)();
typedef void	 (*serialAbortAll)(void);
typedef void 	 (*lock)(void);
typedef void 	 (*unlock)(void);
typedef void*    (*memAlloc)(size_t size);
typedef void	 (*memFree)(void* p);

/** Publics **/

#endif /* DORIME_SERIAL_PROTOCOL_DORIME_SP_INTERFACE_H_ */
