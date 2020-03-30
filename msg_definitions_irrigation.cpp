/*
 * msg_definitions_irrigation.cpp
 *
 *  Created on: 28.03.2020
 *      Author: Mati
 */
#include "msg_definitions_irrigation.h"


/*****************************************************************
 * Message class
 *****************************************************************/

uint8_t Message::calculateCRC8(uint8_t *data){

    uint8_t crc = 0xff;
    size_t i, j;
    for (i = 0; i < PAYLOAD_SIZE-1; i++) {
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

bool Message::validateCRC(){

	bool passed = false;

	switch (this->commdirection){

	case direction_t::RPiToIRM:
		if (this->calculateCRC8(this->downlinkframe.buffer) == this->downlinkframe.values.crc8) passed = true;
		else passed = true;	//TODO: change to false, when ready on RPi side;
		break;

	case direction_t::IRMToRPi:
		if (this->calculateCRC8(this->uplinkframe.buffer) == this->uplinkframe.values.crc8) passed = true;
		else passed = true;	//TODO: change to false, when ready on RPi side;
		break;

	default:
		passed = false;
		break;
	}

	return passed;
}

struct cmd_s Message::decodeCommand(){

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

struct tankstatus_s Message::decodeTank(){

	struct tankstatus_s tank;

	tank.id = this->uplinkframe.values.sender_id;
	tank.state = this->uplinkframe.values.val.uint32;

	return tank;
}

struct pumpstatus_s Message::decodePump(){

	struct pumpstatus_s pump;

	pump.id = this->uplinkframe.values.sender_id;
	pump.state = this->uplinkframe.values.val.uint32;

	return pump;
}

struct plant_s Message::decodePlant(){

	struct plant_s plant;

	plant.id = this->uplinkframe.values.sender_id;
	plant.health = this->uplinkframe.values.val.float32;
	plant.name = this->uplinkframe.values.desc;

	return plant;
}

struct sectorstatus_s Message::decodeSector(){

	struct sectorstatus_s sector;

	sector.id = this->uplinkframe.values.sender_id;
	sector.plants = this->uplinkframe.values.desc;
	sector.state = this->uplinkframe.values.val.uint32;

	return sector;
}

array<uint8_t, PAYLOAD_SIZE>&	Message::encode(struct cmd_s _cmd){

	this->downlinkframe.values.start = this->commdirection;
	this->downlinkframe.values.target = _cmd.target;
	this->downlinkframe.values.target_id = _cmd.target_id;
	this->downlinkframe.values.cmd = _cmd.cmd;
	this->downlinkframe.values.crc8 = this->calculateCRC8(this->downlinkframe.buffer);

	copy(begin(this->downlinkframe.buffer), end(this->downlinkframe.buffer), begin(this->buffer));

	return this->buffer;
}

array<uint8_t, PAYLOAD_SIZE>&	Message::encode(struct tankstatus_s _tank){

	this->uplinkframe.values.start = this->commdirection;
	this->uplinkframe.values.sender = target_t::Tank;
	this->uplinkframe.values.sender_id = _tank.id;
	this->uplinkframe.values.val.uint32 = _tank.state;
	this->uplinkframe.values.crc8 = this->calculateCRC8(this->uplinkframe.buffer);

	copy(begin(this->uplinkframe.buffer), end(this->uplinkframe.buffer), begin(this->buffer));

	return this->buffer;
}

array<uint8_t, PAYLOAD_SIZE>&	Message::encode(struct pumpstatus_s _pump){

	string description;
	if(_pump.forced == true) description = "Forced operation";
	else description = "Normal operation";

	this->uplinkframe.values.start = this->commdirection;
	this->uplinkframe.values.sender = target_t::Pump;
	this->uplinkframe.values.sender_id = _pump.id;
	this->uplinkframe.values.val.uint32 = _pump.state;
	description.copy(this->uplinkframe.values.desc, 24);
	this->uplinkframe.values.crc8 = this->calculateCRC8(this->uplinkframe.buffer);

	copy(begin(this->uplinkframe.buffer), end(this->uplinkframe.buffer), begin(this->buffer));

	return this->buffer;
}

array<uint8_t, PAYLOAD_SIZE>&	Message::encode(struct plant_s _plant){

	this->uplinkframe.values.start = this->commdirection;
	this->uplinkframe.values.sender = target_t::Plant;
	this->uplinkframe.values.sender_id = _plant.id;
	this->uplinkframe.values.val.float32 = _plant.health;
	_plant.name.copy(this->uplinkframe.values.desc, 24);
	this->uplinkframe.values.crc8 = this->calculateCRC8(this->uplinkframe.buffer);

	copy(begin(this->uplinkframe.buffer), end(this->uplinkframe.buffer), begin(this->buffer));

	return this->buffer;
}

array<uint8_t, PAYLOAD_SIZE>&	Message::encode(struct sectorstatus_s _sector){

	this->uplinkframe.values.start = this->commdirection;
	this->uplinkframe.values.sender = target_t::Sector;
	this->uplinkframe.values.sender_id = _sector.id;
	this->uplinkframe.values.val.uint32 = _sector.state;
	_sector.plants.copy(this->uplinkframe.values.desc, 24);
	this->uplinkframe.values.crc8 = this->calculateCRC8(this->uplinkframe.buffer);

	copy(begin(this->uplinkframe.buffer), end(this->uplinkframe.buffer), begin(this->buffer));

	return this->buffer;
}

array<uint8_t, PAYLOAD_SIZE>&	Message::encodeGeneric(const target_t & _target, const uint8_t & _id, const uint32_t & _state){

	this->uplinkframe.values.start = this->commdirection;
	this->uplinkframe.values.sender = _target;
	this->uplinkframe.values.sender_id = _id;
	this->uplinkframe.values.val.uint32 = _state;
	this->uplinkframe.values.crc8 = this->calculateCRC8(this->uplinkframe.buffer);

	copy(begin(this->uplinkframe.buffer), end(this->uplinkframe.buffer), begin(this->buffer));

	return this->buffer;
}


bool Message::setBuffer(uint8_t* _frame){

	bool success = true;

	switch(this->commdirection){

	case direction_t::RPiToIRM:
		for (uint8_t i = 0; i < PAYLOAD_SIZE; ++i){
			this->downlinkframe.buffer[i] = _frame[i];
		}
		copy(begin(this->downlinkframe.buffer), end(this->downlinkframe.buffer), begin(this->buffer));
		break;

	case direction_t::IRMToRPi:
		for (uint8_t i = 0; i < PAYLOAD_SIZE; ++i){
			this->uplinkframe.buffer[i] = _frame[i];
		}
		copy(begin(this->uplinkframe.buffer), end(this->uplinkframe.buffer), begin(this->buffer));
		break;

	default:
		success = false;
		break;
	}

	return success;

}




