/*
 * dorime-sp.c
 *
 *  Created on: Mar 6, 2024
 *      Author: pablo-jean
 */


#include "dorime-sp.h"

/*
 * Structs and Typedefs
 */

/*
 * Privates
 */

inline dorime_err_e  _tx(dorime_t *control, uint8_t *data, uint16_t len){
	if (control->fxns._tx(data, len) != 0)
		return DORIME_FAILED;
	return DORIME_OK;
}

inline dorime_err_e  _rx(dorime_t *control, uint8_t *data, uint16_t len){
	if (control->fxns._rx(data, len) != 0)
		return DORIME_FAILED;
	return DORIME_OK;
}

inline dorime_err_e  _dir(dorime_t *control, dorime_interface_dir_e dir){
	if (control->fxns._dir != NULL){
		if (control->fxns._dir(dir) != 0)
			return DORIME_FAILED;
	}
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

inline void* _malloc(dorime_t *control, uint32_t size){
	return control->fxns._memAlloc(size);
}

inline void	_free(dorime_t *control, void* p){
	control->fxns._memFree(p);
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
	return DORIME_OK;
}

dorime_err_e dorime_operation(dorime_t *control);

dorime_err_e dorime_send_data(dorime_t *control, uint8_t *data, uint16_t len);

dorime_err_e dorime_abort(dorime_t *control);

dorime_err_e dorime_rx_event(dorime_t *control);

dorime_err_e dorime_tx_event(dorime_t *control);
