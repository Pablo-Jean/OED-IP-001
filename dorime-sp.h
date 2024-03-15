/*
 * OEDIP_001.h
 *
 *  Created on: Mar 6, 2024
 *      Author: pablo-jean
 */

#ifndef DORIME_SP_H_
#define DORIME_SP_H_

/**
 * includes
 */

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "dorime-sp-interface.h"

/*
 * Data Packet Structure
 *
 * | Header | Version | Pck ID | ADDR | Len | Data | CRC | Tail |
 *    1B        2B        1B      2B     2B   Len    1B     1B
 *
 *    Header: The first packet, to indicate a start of a transmission,
 *    		and identify the Packet Type
 *    Version: The version of protocol, if an update occours, the lib
 *    		will be capable to handle old types (it's our desire)
 *    Pck ID: sends a packed id, that incremented every new transmission.
 *    		It's useful for user implement a retransmission, and preventing
 *    		a multiple processing of a same packet.
 *    Addr: The address of the receiver (0x0000 for Controller, 0xFFFF for
 *    		a broadcasting)
 *    Len: Length, in bytes, of the Data
 *    Data: The useful information
 *    CRC: CRC-8 Calculation for check if data is not corrupted.
 *    		The calculation starts in Header, ending on the Data
 *    Tail: Sinalyze the end of the communication
 *
 */

/**
 * Macros
 */
#define DORIME_VERSION_MAJOR		1
#define DORIME_VERSION_MINOR		0
#define DORIME_PACKET_VERSION		(DORIME_VERSION_MAJOR << 8) | (DORIME_VERSION_MINOR)

#define DORIME_INITIALIZED_FLAG		0xD0

#define DORIME_HEADER_START			0xA0
#define DORIME_HEADER_ACK			0xA2
#define DORIME_HEADER_DATA			0xA4
#define DORIME_HEADER_RESP			0xA6
#define DORIME_TAIL					0xED

#define DORIME_ADDR_CONTROLLER		0x0000
#define DORIME_ADDR_BROADCAST		0xFFFF

#define DORIME_ALIVE_CTR_2_TGT		"DORIME"
#define DORIME_ALIVE_TGT_2_CTR		"AMENO"

#define DORIME_FXN_SET_TX(handle, pTx)			handle->fxns._tx = pTx
#define DORIME_FXN_SET_RX(handle, pRx)			handle->fxns._rx = pRx
#define DORIME_FXN_SET_DIR(handle, pDir)		handle->fxns._dir = pDir
#define DORIME_FXN_SET_LOCK(handle, pLock)		handle>fxns._lock = pLock
#define DORIME_FXN_SET_UNLOCK(handle, pUnlock)	handle>fxns._unlock = pUnlock
#define DORIME_FXN_SET_TCKCNT(handle, pCnt)		handle>fxns._tickCont = pCnt
#define DORIME_FXN_SET_MALLOC(handle, pMalloc)	handle>fxns._memAlloc = pMalloc
#define DORIME_FXN_SET_FREE(handle, pFree)		handle>fxns._memFree = pFree

/**
 * Enumerates
 */

typedef enum{
	DORIME_EVT_ADDR_MATCH,
	DORIME_EVT_DATA_REC,
	DORIME_EVT_RESP_REC,
	DORIME_EVT_CRC_ERR,
	DORIME_EVT_ACK_OK
}dorime_event_e;

typedef enum{
	DORIME_TYPE_CONTROLLER,
	DORIME_TYPE_TARGET
}dorime_dev_type_e;

typedef enum{
	DORIME_OK,
	DORIME_FAILED,
	DORIME_NOT_ALLOWED,
	DORIME_NOT_INITIALIZED,
	DORIME_INVALID_PARAMETERS,
	DORIME_INVALID_HANDLE,

	DORIME_UNKNOWN = 0xFF
}dorime_err_e;

typedef enum{
	STATE_IDLE,
	STATE_ADDR_MATCH,
	STATE_WAITING_PACKET,
	STATE_RECEIVED_PACKET,
	STATE_PACKET_FAILED,

	STATE_START_SEND,
	STATE_START_WAITING_ACK,
	STATE_START_ACK,
	STATE_SEND_DATA,
	STATE_SEND_WAITING_ACK,
	STATE_SEND_ACK,

	STATE_TIMEOUT
}dorime_state_e;

/**
 * Typedefs and Structs
 */

typedef struct{
	uint16_t len;
	uint8_t *data;
}dorime_evparams_data_t;

typedef struct{
	uint16_t resp;
}dorime_evparams_resp_rec_t;

typedef struct{
	uint16_t addr;
}dorime_evparams_addr_match_t;

typedef struct{
	uint16_t u16Address;
	uint32_t u32MsPerCount;
	uint32_t u32Timeout;
	// fxns
	struct {
		serialTx _tx;
		serialRx _rx;
		dirSelection _dir;
		getCnt _tickCont;
		lock _lock;
		unlock _unlock;
		memAlloc _memAlloc;
		memFree _memFree;
	}fxns;
	// interns
	struct {
		dorime_dev_type_e enType;
		uint32_t u32Cnt;
		dorime_state_e enState;
		uint32_t u32DataLen;
		uint8_t u8InitFlag;
	}_internal;
}dorime_t;

/**
 * Publics
 */

dorime_err_e dorime_init(dorime_t *control);

dorime_err_e dorime_operation(dorime_t *control);

dorime_err_e dorime_send_data(dorime_t *control, uint8_t *data, uint16_t len);

dorime_err_e dorime_abort(dorime_t *control);

dorime_err_e dorime_rx_event(dorime_t *control);

dorime_err_e dorime_tx_event(dorime_t *control);

// callbacks

void dorime_cb_event(dorime_t *control, dorime_event_e event, void* params);

#endif /* DORIME_SP_H_ */
