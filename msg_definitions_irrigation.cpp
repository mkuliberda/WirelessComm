/*
 * msg_definitions_irrigation.cpp
 *
 *  Created on: 28.03.2020
 *      Author: Mati
 */
#include "msg_definitions_irrigation.h"


uint8_t DownlinkMessage::calculateCRC8(uint8_t *data){

    uint8_t crc = 0xff;
    size_t i, j;
    for (i = 0; i < RADIO1_PAYLOAD_SIZE-1; i++) {
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

bool DownlinkMessage::validateCRC(){

	if (this->calculateCRC8(this->downlinkframe.buffer) == this->downlinkframe.values.crc8){
		return true;
	}
	else{
		return true;//TODO:false;
	}
}

struct cmd_s DownlinkMessage::decode(){
	struct cmd_s cmd;
	cmd.target = this->downlinkframe.values.target;
	cmd.target_id = this->downlinkframe.values.target_id;
	cmd.cmd = this->downlinkframe.values.cmd;
	cmd.subcmd1 = this->downlinkframe.values.free[0];
	cmd.subcmd2 = this->downlinkframe.values.free[1];
	cmd.subcmd3 = this->downlinkframe.values.free[2];
	cmd.subcmd4 = this->downlinkframe.values.free[3];
	return cmd;
}

array<uint8_t, RADIO1_PAYLOAD_SIZE> DownlinkMessage::encode(struct cmd_s _cmd){

	array<uint8_t, RADIO1_PAYLOAD_SIZE> payload;

	this->downlinkframe.values.start = direction_t::RPiToIRM;
	this->downlinkframe.values.target = _cmd.target;
	this->downlinkframe.values.target_id = _cmd.target_id;
	this->downlinkframe.values.cmd = _cmd.cmd;
	this->downlinkframe.values.crc8 = this->calculateCRC8(this->downlinkframe.buffer);
	return payload;
}

bool DownlinkMessage::setBuffer(uint8_t* _frame){

	for (uint8_t i = 0; i < RADIO1_PAYLOAD_SIZE; ++i){
		this->downlinkframe.buffer[i] = _frame[i];
	}
	copy(begin(this->downlinkframe.buffer), end(this->downlinkframe.buffer), begin(this->buffer));
	return true;
}



