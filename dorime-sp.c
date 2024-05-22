/*
 * dorime-sp.c
 *
 *  Created on: Mar 6, 2024
 *      Author: pablo-jean
 */


#include "dorime-sp.h"

/**
 * Macros
 */

#define _SET_MACHINE_STATE(x, y)	x->_internal.enState = y;

#define _PCK_DATA_SIZEOF_WO_DATA		(sizeof(dorime_pck_data_t) - 4)
#define _PCK_START_SIZEOF				(sizeof(dorime_pck_start_t))

/*
 * Structs and Typedefs
 */

/*
 * Privates
 */

inline dorime_err_e  _tx(dorime_t *control, uint8_t *data, uint16_t len){
	if (control->fxns._tx(data, len) != 0)
		return DORIME_FAILED;
	control->_internal.TxIsWaiting = true;
	return DORIME_OK;
}

inline dorime_err_e  _rx(dorime_t *control, uint8_t *data, uint16_t len){
	if (control->fxns._rx(data, len) != 0)
		return DORIME_FAILED;
	control->_internal.RxIsWaiting = true;
	return DORIME_OK;
}

inline void _abort(dorime_t *control){
	if (control->fxns._abort != NULL){
		control->fxns._abort();
	}
	control->_internal.TxIsWaiting = false;
	control->_internal.RxIsWaiting = false;
}

inline dorime_err_e  _dir(dorime_t *control, dorime_interface_dir_e dir){
	if (control->fxns._dir != NULL){
		if (control->fxns._dir(dir) != 0)
			return DORIME_FAILED;
	}
	control->_internal.enInterfaceDir = dir;
	return DORIME_OK;
}

inline uint32_t _getCnt(dorime_t *control){
	return control->fxns._tickCont();
}

inline void _lock(dorime_t *control){
	if (control->fxns._lock != NULL){
		control->fxns._lock();
	}
}

inline void _unlock(dorime_t *control){
	if (control->fxns._unlock != NULL){
		control->fxns._unlock();
	}
}

inline void* _malloc(dorime_t *control, size_t size){
	if (control->fxns._memAlloc != NULL){
		return control->fxns._memAlloc(size);
	}
	else{
		return malloc(size);
	}
}

inline void	_free(dorime_t *control, void* p){
	if (control->fxns._memFree != NULL){
		control->fxns._memFree(p);
	}
	else{
		free(p);
	}
}

/** Dorime Auxiliary Functions **/

void _dorime_send_ack(dorime_t *control, uint16_t DestAddr){
	control->packet.Ack.header = DORIME_HEADER_ACK;
	control->packet.Ack.version = DORIME_PACKET_VERSION;
	control->packet.Ack.addr = DestAddr;
	control->packet.Ack.ack = DORIME_PACKET_ACK;
	control->packet.Ack.crc = dorime_crc32((uint8_t*)&control->packet.Ack, sizeof(control->packet.Ack) - 5);
	control->packet.Ack.tail = DORIME_TAIL;
	_dir(control, DORIME_INTERFACE_DIRECTION_RX);
	_tx(control, (uint8_t*)&control->packet.Ack, sizeof(control->packet.Ack));
}

void _dorime_wait_for_start_packet(dorime_t *control){
	_dir(control, DORIME_INTERFACE_DIRECTION_RX);
	_rx(control, control->packet.Start, _PCK_START_SIZEOF);
}

void _dorime_send_start_packet(dorime_t *control){
	dorime_pck_start_t *start;

	start = &control->packet.Start;

	start->header = DORIME_HEADER_START;
	start->version = DORIME_PACKET_VERSION;
	start->ID = control->_internal.u32ID;
	start->addr = control->_internal.u16DestAddr;
	start->len = control->_internal.u16DataLen;
	start->tail = DORIME_TAIL;

	control->_internal.ID++;
	_dir(control, DORIME_INTERFACE_DIRECTION_TX);
	_tx(control, (uint8_t*)start, _PCK_START_SIZEOF);
}

/** Dorime States **/

void _dorime_state_idle(dorime_t *control){
	if (control->_internal.RxIsWaiting == false){
		_dorime_wait_for_start_packet(control);
	}
}

void _dorime_state_addr_match(dorime_t *control){
	if (control->packet.Start.header != DORIME_HEADER_START ||
		control->packet.Start.tail != DORIME_TAIL){
		_SET_MACHINE_STATE(control, DOR_STATE_COOLDOWN);
		return;
	}

	// TODO Check the CRC Tail

	if (control->packet.Start.addr == control->u16Address ||
			control->packet.Start.addr == DORIME_ADDR_BROADCAST){
		uint16_t AckToAddr;

		// If my device is a Target, the device can only communicate
		// with the Controller
		if (control->_internal.enType == DORIME_TYPE_TARGET){
			AckToAddr = DORIME_ADDR_CONTROLLER;
		}
		else{
			// TODO: Check what Controller will do in this case
		}
		control->_internal.u64TimeoutWaiting = control->_internal.u64Tick + control->_internal.u32ReceiveTimeout;
		control->packet.BufferLen = _PCK_DATA_SIZEOF_WO_DATA + control->packet.Start.len;
		control->packet.Buffer = (uint8_t*)_malloc(control, control->packet.BufferLen);

		_SET_MACHINE_STATE(control, DOR_STATE_SEND_ACK);
	}
	else{
		_SET_MACHINE_STATE(control, DOR_STATE_COOLDOWN);
	}
}

void _dorime_state_send_ack(dorime_t *control){
	_dir(control, DORIME_INTERFACE_DIRECTION_TX);
	_dorime_send_ack(control, AckToAddr);
	_SET_MACHINE_STATE(control, DOR_STATE_WAITING_ACK_SENDED);
}

void _dorime_state_waiting_ack_sended(dorime_t *control){
	if (control->_internal.u64Tick >= control->_internal.u64TimeoutWaiting){
		_SET_MACHINE_STATE(control, DOR_STATE_COOLDOWN);
	}
}

void _dorime_state_waiting_packet(dorime_t *control){
	if (control->_internal.u64Tick >= control->_internal.u64TimeoutWaiting){
		_SET_MACHINE_STATE(control, DOR_STATE_COOLDOWN);
	}
}

void _dorime_state_received_packet(dorime_t *control){
	dorime_pck_data_t *PckD;
	uint8_t *Bff;
	uint32_t i;

	Bff = control->packet.Buffer;
	PckD = &control->packet.Data;
	// Mount Received Buffer into data packet
	i = 0;
	PckD->header = Bff[i];
	i += sizeof(uint8_t);
	PckD->version = Bff[i];
	i+= sizeof(uint16_t);
	PckD->ID = Bff[i];
	i+= sizeof(uint32_t);
	PckD->addr = Bff[i];
	i += sizeof(uint16_t);
	PckD->len = Bff[i];
	i += sizeof(uint16_t);
	PckD->data = &Bff[i];
	i += sizeof(uint16_t);
	PckD->crc = Bff[i];
	i += sizeof(uint32_t);
	PckD->tail = Bff[i];
	i += sizeof(uint8_t);

	if (PckD->header != DORIME_HEADER_DATA){
		_SET_MACHINE_STATE(control, DOR_STATE_PACKET_FAILED);
		return;
	}
	if (PckD->tail != DORIME_TAIL){
		_SET_MACHINE_STATE(control, DOR_STATE_PACKET_FAILED);
		return;
	}
	if (dorime_crc32(PckD->data, PckD->len) != PckD->crc){
		_SET_MACHINE_STATE(control, DOR_STATE_PACKET_FAILED);
		return;
	}

	// If received packet is OK, call the callback
	dorime_evparams_data_t params;

	params.data = PckD->data;
	params.len = PckD->len;
	// the user MUST copy the data in the params var
	// They will be freed soon
	dorime_cb_event(control, DORIME_EVT_DATA_REC, &params);

	_SET_MACHINE_STATE(control, DOR_STATE_ABORT);
}

void _dorime_state_packet_failed(dorime_t *control){
	dorime_cb_event(control, DORIME_EVT_REC_FAILED, NULL);
	_SET_MACHINE_STATE(control, DOR_STATE_ABORT);
}

void _dorime_state_start_send(dorime_t *control){
	_dorime_send_start_packet(control);
	control->_internal.u64TimeoutWaiting = control->_internal.u64Tick + control->_internal.u32SendTimeout;
	_SET_MACHINE_STATE(control, DOR_STATE_WAITING_START_SENDED);
}

void _start_waiting_ack(dorime_t *control){

}

void _dorime_state_start_ack(dorime_t *control){

}

void _dorime_state_send_data(dorime_t *control){

}

void _dorime_state_send_waiting_ack(dorime_t *control){

}

void _dorime_state_send_ack(dorime_t *control){

}

void _dorime_state_cooldown(dorime_t *control){
	// Do a garbage checker
	if (control->_internal.isCoolDownCounting == false){
		control->_internal.isCoolDownCounting = true;
		control->_internal.u64TimeoutWaiting = control->_internal.u64Tick + control->_internal.u32CooldownTimeout;

	}
	if (control->_internal.TxIsWaiting == true || control->_internal.RxIsWaiting == true){
		_abort(control);
	}
	if (control->packet.Buffer != NULL){
		_free(control, control->packet.Buffer);
		control->packet.Buffer = NULL;
	}
	if (control->_internal.u64Tick >= control->_internal.u64TimeoutWaiting){
		_SET_MACHINE_STATE(control, DOR_STATE_ABORT);
	}
}

void _dorime_state_abort(dorime_t *control){
	if (control->_internal.TxIsWaiting == true || control->_internal.RxIsWaiting == true){
		_abort(control);
	}
	if (control->packet.Buffer != NULL){
		_free(control, control->packet.Buffer);
		control->packet.Buffer = NULL;
	}
	_SET_MACHINE_STATE(control, DOR_STATE_IDLE);
}

void _dorime_state_timeout(dorime_t *control){
	_SET_MACHINE_STATE(control, DOR_STATE_ABORT);
}

/*
 * Publics
 */

dorime_err_e dorime_init(dorime_t *control){
	if (control == NULL){
		return DORIME_INVALID_HANDLE;
	}
	// check mandatory functions
	if (control->fxns._rx == NULL ||
			control->fxns._tx == NULL ||
			control->fxns._tickCont == NULL){
		return DORIME_INVALID_PARAMETERS;
	}
	// check for invalid parameters
	if (control->u32MsPerCount == 0 ||
			control->u32Timeout == 0){
		return DORIME_INVALID_PARAMETERS;
	}

	// if no Memory Allocation fxn was declared, we will use
	// `malloc`and `free`.
	if (control->fxns._memAlloc == NULL ||
			control->fxns._memFree == NULL){
		control->fxns._memAlloc = malloc;
		control->fxns._memFree = free;
	}

	if (control->u16Address != DORIME_TYPE_CONTROLLER){
		control->_internal.enType = DORIME_TYPE_CONTROLLER;
	}
	else{
		control->_internal.enType = DORIME_TYPE_TARGET;
	}
	control->_internal.u8InitFlag = DORIME_INITIALIZED_FLAG;
	control->_internal.enState = DOR_STATE_IDLE;
	control->_internal.u64Tick = 0;
	control->_internal.u16DataLen = 0;
	control->_internal.RxIsWaiting = false;
	control->_internal.TxIsWaiting = false;
	control->_internal.isCoolDownCounting = false;
	control->_internal.enInterfaceDir = DORIME_INTERFACE_DIRECTION_SLEEP;
	control->_internal.u32ID = 0;

	control->packet.Buffer = NULL;

	return DORIME_OK;
}

dorime_err_e dorime_handler(dorime_t *control){
	if (control == NULL){
		return DORIME_INVALID_HANDLE;
	}
	if (control->_internal.u8InitFlag != DORIME_INITIALIZED_FLAG){
		return DORIME_NOT_INITIALIZED;
	}

	switch (control->_internal.enState){
	case DOR_STATE_IDLE:
		_dorime_state_idle(control);
		break;
	case DOR_STATE_ADDR_CHECK:
		_dorime_state_addr_match(control);
		break;
	case DOR_STATE_SEND_ACK:
		_dorime_state_send_ack(control);
		break;
	case DOR_STATE_WAITING_ACK_SENDED:
		_dorime_state_waiting_ack_sended(control);
		break;
	case DOR_STATE_SEND_ACK_FINISHED:
		_dorime_state_send_ack_finished(control);
		break;
	case DOR_STATE_WAITING_PACKET:
		_dorime_state_waiting_packet(control);
		break;
	case DOR_STATE_RECEIVED_PACKET:
		_dorime_state_received_packet(control);
		break;
	case DOR_STATE_PACKET_FAILED:
		_dorime_state_packet_failed(control);
		break;

	case DOR_STATE_START_SEND:
		_dorime_state_start_send(control);
		break;
	case DOR_STATE_WAITING_START_SENDED:

		break;
	case DOR_STATE_SEND_START_FINISHED:

		break;
	case DOR_STATE_START_WAITING_ACK:
		_dorime_state_start_waiting_ack(control);
		break;
	case DOR_STATE_START_REC_ACK:
		_dorime_state_start_ack(control);
		break;
	case DOR_STATE_SEND_DATA:
		_dorime_state_send_data(control);
		break;
	case DOR_STATE_SEND_WAITING_ACK:
		_dorime_state_send_waiting_ack(control);
		break;
	case DOR_STATE_SEND_REC_ACK:
		_dorime_state_send_ack(control);
		break;

	case DOR_STATE_COOLDOWN:
		_dorime_state_cooldown(control);
		break;
	case DOR_STATE_ABORT:
		_dorime_state_abort(control);
		break;
	case DOR_STATE_TIMEOUT:
		_dorime_state_timeout(control);
		break;
	default:
		control->_internal.enState = DOR_STATE_IDLE;
		break;
	}



	return DORIME_OK;
}

dorime_err_e dorime_tick(dorime_t *control, size_t TickRate){
	if (control == NULL){
		return DORIME_INVALID_HANDLE;
	}
	if (control->_internal.u8InitFlag != DORIME_INITIALIZED_FLAG){
		return DORIME_NOT_INITIALIZED;
	}

	control->_internal.u64Tick += TickRate;

	return DORIME_OK;
}

dorime_err_e dorime_send_data(dorime_t *control, uint8_t *data, uint16_t len){
	if (control == NULL){
		return DORIME_INVALID_HANDLE;
	}
	if (control->_internal.u8InitFlag != DORIME_INITIALIZED_FLAG){
		return DORIME_NOT_INITIALIZED;
	}

	return DORIME_OK;
}

dorime_err_e dorime_abort(dorime_t *control){
	if (control == NULL){
		return DORIME_INVALID_HANDLE;
	}
	if (control->_internal.u8InitFlag != DORIME_INITIALIZED_FLAG){
		return DORIME_NOT_INITIALIZED;
	}

	_SET_MACHINE_STATE(control, DOR_STATE_ABORT);

	return DORIME_OK;
}

dorime_err_e dorime_rx_event(dorime_t *control){
	if (control == NULL){
		return DORIME_INVALID_HANDLE;
	}
	if (control->_internal.u8InitFlag != DORIME_INITIALIZED_FLAG){
		return DORIME_NOT_INITIALIZED;
	}

	switch (control->_internal.enState){
	case DOR_STATE_WAITING_PACKET:
		_SET_MACHINE_STATE(control, DOR_STATE_RECEIVED_PACKET);
		break;
	}

	return DORIME_OK;
}

dorime_err_e dorime_tx_event(dorime_t *control){
	if (control == NULL){
		return DORIME_INVALID_HANDLE;
	}
	if (control->_internal.u8InitFlag != DORIME_INITIALIZED_FLAG){
		return DORIME_NOT_INITIALIZED;
	}

	switch (control->_internal.enState){
	case DOR_STATE_WAITING_ACK_SENDED:
		_dir(control, DORIME_INTERFACE_DIRECTION_RX);
		_rx(control, control->packet.Buffer, control->packet.BufferLen);
		_SET_MACHINE_STATE(control, DOR_STATE_WAITING_PACKET);
		break;
	default:

		break;
	}

	return DORIME_OK;
}

/*
 * Weak Routines: These routines can be customized by developer,
 * to reach better performance
 */

__weak_symbol uint32_t dorime_crc32(uint8_t *arr, uint32_t len){
	return xcrc32(arr, len, 0x00);
}
