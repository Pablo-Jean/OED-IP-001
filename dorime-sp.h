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
#include <string.h>

#include "dorime-sp-interface.h"

/**
 * Macros
 */

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

#define DORIME_FXN_SET_TX(handle, fxn)		handle->fxns._tx = fxn
#define DORIME_FXN_SET_RX(handle, fxn)
#define DORIME_FXN_SET_DIR(handle, fxn)
#define DORIME_FXN_SET_LOCK(handle, fxn)
#define DORIME_FXN_SET_UNLOCK(handle, fxn)
#define DORIME_FXN_SET_TCKCNT(handle, fxn)

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
	dorime_dev_type_e enType;
	uint32_t u32MsPerCount;
	uint32_t u32Timeout;
	// fxns
	struct fxns{
		serialTx _tx;
		serialRx _rx;
		dirSelection _dir;
		getCnt _lock;
		lock _unlock;
		unlock _tickCont;
	};
	// interns
	struct internal{
		uint32_t u32Cnt;
		dorime_state_e enState;
		uint32_t u32DataLen;
		uint8_t u8InitFlag;
	};
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
