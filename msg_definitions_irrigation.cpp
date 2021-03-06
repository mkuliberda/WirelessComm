/*
 * msg_definitions_irrigation.cpp
 *
 *  Created on: 28.03.2020
 *      Author: Mati
 */
#include "msg_definitions_irrigation.h"


/*****************************************************************
 * IrrigationMessage class
 *****************************************************************/

bool IrrigationMessage::validateCRC(){

	bool passed = false;

	switch (this->commdirection){

	case direction_t::RPiToIRM:
		if (this->calculateCRC8(this->downlinkframe->buffer, PAYLOAD_SIZE) == this->downlinkframe->values.crc8) passed = true;
		else passed = false;
		break;

	case direction_t::IRMToRPi:
		if (this->calculateCRC8(this->uplinkframe->buffer, PAYLOAD_SIZE) == this->uplinkframe->values.crc8) passed = true;
		else passed = false;
		break;

	default:
		passed = false;
		break;
	}

	return passed;
}

struct cmd_s IrrigationMessage::decodeCommand(){

	struct cmd_s cmd;

	cmd.target = this->downlinkframe->values.target;
	cmd.target_id = this->downlinkframe->values.target_id;
	cmd.cmd = this->downlinkframe->values.cmd;
	cmd.subcmd1 = this->downlinkframe->values.free[0];
	cmd.subcmd2 = this->downlinkframe->values.free[1];

	return cmd;
}

struct confirmation_s IrrigationMessage::decodeConfirmation(){

	struct confirmation_s confirmation;

	confirmation.target = this->uplinkframe->values.sender;
	confirmation.target_id = this->uplinkframe->values.sender_id;
	confirmation.cmd = static_cast<command_t>(this->uplinkframe->values.val.uint8[0]);
	confirmation.consumed = this->uplinkframe->values.val.uint8[1] == 1 ? true : false;
	confirmation.subcmd1 = this->uplinkframe->values.val.uint8[2];
	confirmation.subcmd2 = this->uplinkframe->values.val.uint8[3];

	return confirmation;
}

struct tankstatus_s IrrigationMessage::decodeTank(){

	struct tankstatus_s tank;

	tank.id = this->uplinkframe->values.sender_id;
	tank.state = this->uplinkframe->values.val.uint32;

	return tank;
}

struct pumpstatus_s IrrigationMessage::decodePump(){

	struct pumpstatus_s pump;

	pump.id = this->uplinkframe->values.sender_id;
	pump.state = this->uplinkframe->values.val.uint32;

	return pump;
}

struct plantstatus_s IrrigationMessage::decodePlant(){

	struct plantstatus_s plant;

	plant.id = this->uplinkframe->values.sender_id;
	plant.health = this->uplinkframe->values.val.float32;
	for (uint8_t i = 0; i < NAME_LENGTH; ++i) plant.name[i] = '\0';
	for (uint8_t i = 0; i < std::min(NAME_LENGTH, PAYLOAD_SIZE-8); ++i) plant.name[i] = this->uplinkframe->values.desc[i];

	return plant;
}

struct sectorstatus_s IrrigationMessage::decodeSector(){

	struct sectorstatus_s sector;

	sector.id = this->uplinkframe->values.sender_id;
	for (uint8_t i = 0; i < std::min(PLANTS_LENGTH, PAYLOAD_SIZE-8); ++i) sector.plants[i] = this->uplinkframe->values.desc[i];
	//sector.plants = this->uplinkframe->values.desc;
	sector.state = this->uplinkframe->values.val.uint32;

	return sector;
}

struct batterystatus_s IrrigationMessage::decodeBattery(){

	struct batterystatus_s battery;

	battery.id = this->uplinkframe->values.sender_id;
	battery.percentage = this->uplinkframe->values.val.uint8[0];
	battery.status = this->uplinkframe->values.val.uint8[1];
	battery.remaining_time_min = static_cast<uint16_t>(this->uplinkframe->values.val.uint8[2] << 8 | this->uplinkframe->values.val.uint8[3]);

	return battery;
}

std::array<uint8_t, PAYLOAD_SIZE>&	IrrigationMessage::encode(const struct cmd_s &_cmd){

	this->downlinkframe->values.start = this->commdirection;
	this->downlinkframe->values.target = _cmd.target;
	this->downlinkframe->values.target_id = _cmd.target_id;
	this->downlinkframe->values.cmd = _cmd.cmd;
	this->downlinkframe->values.crc8 = this->calculateCRC8(this->downlinkframe->buffer, PAYLOAD_SIZE);

	std::copy(std::begin(this->downlinkframe->buffer), std::end(this->downlinkframe->buffer), std::begin(this->buffer));

	return this->buffer;
}

std::array<uint8_t, PAYLOAD_SIZE>&	IrrigationMessage::encode(const struct confirmation_s &_confirmation){

	this->uplinkframe->values.start = direction_t::RPiToIRM;
	this->uplinkframe->values.sender = _confirmation.target;
	this->uplinkframe->values.sender_id = _confirmation.target_id;
	this->uplinkframe->values.val.uint8[0] = _confirmation.cmd;
	this->uplinkframe->values.val.uint8[1] = _confirmation.consumed ? 1 : 0;
	this->uplinkframe->values.val.uint8[2] = _confirmation.subcmd1;
	this->uplinkframe->values.val.uint8[3] = _confirmation.subcmd2;
	for (uint8_t i = 0; i < PAYLOAD_SIZE-8; ++i) this->uplinkframe->values.desc[i] = '\0'; //clean description from junk
	this->uplinkframe->values.crc8 = this->calculateCRC8(this->uplinkframe->buffer, PAYLOAD_SIZE);

	std::copy(std::begin(this->uplinkframe->buffer), std::end(this->uplinkframe->buffer), std::begin(this->buffer));

	return this->buffer;
}

std::array<uint8_t, PAYLOAD_SIZE>&	IrrigationMessage::encode(const struct tankstatus_s &_tank){

	this->uplinkframe->values.start = this->commdirection;
	this->uplinkframe->values.sender = target_t::Tank;
	this->uplinkframe->values.sender_id = _tank.id;
	this->uplinkframe->values.val.uint32 = _tank.state;
	for (uint8_t i = 0; i < PAYLOAD_SIZE-8; ++i) this->uplinkframe->values.desc[i] = '\0'; //clean description from junk
	this->uplinkframe->values.crc8 = this->calculateCRC8(this->uplinkframe->buffer, PAYLOAD_SIZE);
	std::copy(std::begin(this->uplinkframe->buffer), std::end(this->uplinkframe->buffer), std::begin(this->buffer));

	return this->buffer;
}

std::array<uint8_t, PAYLOAD_SIZE>&	IrrigationMessage::encode(const struct pumpstatus_s &_pump){

	std::string description;
	if(_pump.forced == true) description = "Forced operation";
	else description = "Normal operation";

	this->uplinkframe->values.start = this->commdirection;
	this->uplinkframe->values.sender = target_t::Pump;
	this->uplinkframe->values.sender_id = _pump.id;
	this->uplinkframe->values.val.uint32 = _pump.state;
	description.copy(this->uplinkframe->values.desc, PAYLOAD_SIZE-8);
	this->uplinkframe->values.crc8 = this->calculateCRC8(this->uplinkframe->buffer, PAYLOAD_SIZE);

	std::copy(std::begin(this->uplinkframe->buffer), std::end(this->uplinkframe->buffer), std::begin(this->buffer));

	return this->buffer;
}

std::array<uint8_t, PAYLOAD_SIZE>&	IrrigationMessage::encode(const struct plantstatus_s &_plant){

	this->uplinkframe->values.start = this->commdirection;
	this->uplinkframe->values.sender = target_t::Plant;
	this->uplinkframe->values.sender_id = _plant.id;
	this->uplinkframe->values.val.float32 = _plant.health;
	for (uint8_t i = 0; i < PAYLOAD_SIZE-8; ++i) this->uplinkframe->values.desc[i] = '\0'; //clean description from junk
	for (uint8_t i = 0; i < std::min(NAME_LENGTH, PAYLOAD_SIZE-8); ++i) this->uplinkframe->values.desc[i] = _plant.name[i];
	this->uplinkframe->values.crc8 = this->calculateCRC8(this->uplinkframe->buffer, PAYLOAD_SIZE);

	std::copy(std::begin(this->uplinkframe->buffer), std::end(this->uplinkframe->buffer), std::begin(this->buffer));

	return this->buffer;
}

std::array<uint8_t, PAYLOAD_SIZE>&	IrrigationMessage::encode(const struct sectorstatus_s &_sector){

	this->uplinkframe->values.start = this->commdirection;
	this->uplinkframe->values.sender = target_t::Sector;
	this->uplinkframe->values.sender_id = _sector.id;
	this->uplinkframe->values.val.uint32 = _sector.state;
	for (uint8_t i = 0; i < PAYLOAD_SIZE-8; ++i) this->uplinkframe->values.desc[i] = '\0'; //clean description from junk
	for (uint8_t i = 0; i < std::min(PLANTS_LENGTH, PAYLOAD_SIZE-8); ++i) this->uplinkframe->values.desc[i] = _sector.plants[i]; //copy plants to buffer
	this->uplinkframe->values.crc8 = this->calculateCRC8(this->uplinkframe->buffer, PAYLOAD_SIZE);

	std::copy(std::begin(this->uplinkframe->buffer), std::end(this->uplinkframe->buffer), std::begin(this->buffer));

	return this->buffer;
}

std::array<uint8_t, PAYLOAD_SIZE>&	IrrigationMessage::encode(const struct batterystatus_s &_battery){

	this->uplinkframe->values.start = this->commdirection;
	this->uplinkframe->values.sender = target_t::Power;
	this->uplinkframe->values.sender_id = _battery.id;
	this->uplinkframe->values.val.uint8[0] = _battery.percentage;
	this->uplinkframe->values.val.uint8[1] = _battery.status;
	this->uplinkframe->values.val.uint8[2] = static_cast<uint8_t>(_battery.remaining_time_min >> 8);
	this->uplinkframe->values.val.uint8[3] = static_cast<uint8_t>(_battery.remaining_time_min);
	for (uint8_t i = 0; i < PAYLOAD_SIZE-8; ++i) this->uplinkframe->values.desc[i] = '\0'; //clean description from junk
	this->uplinkframe->values.crc8 = this->calculateCRC8(this->uplinkframe->buffer, PAYLOAD_SIZE);

	std::copy(std::begin(this->uplinkframe->buffer), std::end(this->uplinkframe->buffer), std::begin(this->buffer));

	return this->buffer;
}

std::array<uint8_t, PAYLOAD_SIZE>&	IrrigationMessage::encodeGeneric(const target_t & _target, const uint8_t & _id, const uint32_t & _state){

	this->uplinkframe->values.start = this->commdirection;
	this->uplinkframe->values.sender = _target;
	this->uplinkframe->values.sender_id = _id;
	this->uplinkframe->values.val.uint32 = _state;
	for (uint8_t i = 0; i < PAYLOAD_SIZE-8; ++i) this->uplinkframe->values.desc[i] = '\0'; //clean description from junk
	this->uplinkframe->values.crc8 = this->calculateCRC8(this->uplinkframe->buffer, PAYLOAD_SIZE);

	std::copy(std::begin(this->uplinkframe->buffer), std::end(this->uplinkframe->buffer), std::begin(this->buffer));

	return this->buffer;
}


bool IrrigationMessage::setBuffer(uint8_t* _frame, const size_t & _buffer_size){

	bool success = true;

	if (_buffer_size > 0){

		switch(this->commdirection){
		case direction_t::RPiToIRM:
			for (size_t i = 0; i < _buffer_size; ++i){
				this->downlinkframe->buffer[i] = _frame[i];
			}
			std::copy(std::begin(this->downlinkframe->buffer), std::end(this->downlinkframe->buffer), std::begin(this->buffer));
			break;

		case direction_t::IRMToRPi:
			for (size_t i = 0; i < _buffer_size; ++i){
				this->uplinkframe->buffer[i] = _frame[i];
			}
			std::copy(std::begin(this->uplinkframe->buffer), std::end(this->uplinkframe->buffer), std::begin(this->buffer));
			break;

		default:
			success = false;
			break;
		}
	}
	else{
		success = false;
	}

	return success;

}

std::array<uint8_t, PAYLOAD_SIZE>& IrrigationMessage::getBuffer(void){
	return this->buffer;
}




