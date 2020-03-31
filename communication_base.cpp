/*
 * wireless_base.cpp
 *
 *  Created on: 25.01.2020
 *      Author: Mati
 */

#include "communication_base.h"


/*****************************************************************
 * Wireless class
 *****************************************************************/

communicationtype_t& Wireless::typeGet(void){
	return this->commType;
}


/*****************************************************************
 * Message class
 *****************************************************************/

uint8_t Message::calculateCRC8(uint8_t *data, const size_t & _payload_size){

    uint8_t crc = 0xff;
    size_t i, j;
    for (i = 0; i < _payload_size-1; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if ((crc & 0x80) != 0)
                crc = (uint8_t)((crc << 1) ^ 0x31);
            else
                crc <<= 1;
        }
    }
	return crc;
}



