/**
 * @brief  Project specific communication protocol defines and enums
 */
#ifndef MSG_DEFINITIONS_IRRIGATION_H_
#define MSG_DEFINITIONS_IRRIGATION_H_


#include "communication_base.h"
#include <algorithm>
#include <array>
#ifndef RPI
#include "stm32f3xx_hal.h"
#include "irrigation.h"
#include "plants.h"
#include "power.h"
#else
#include "../defines.h"
#include <cstring>

#define NAME_LENGTH 20
#define PLANTS_LENGTH 20

enum class batterystate_t: uint8_t{
	undetermined,
	charging,
	discharging
};

struct batteryerror_t{
	uint8_t overvoltage 	: 1;
	uint8_t overdischarge 	: 1;
	uint8_t overheated 		: 1;
	uint8_t overloaded 		: 1;
	uint8_t free1 			: 1;
	uint8_t free2 			: 1;
	uint8_t free3 			: 1;
	uint8_t free4 			: 1;
};

#pragma pack(push, 1)
struct pumpstatus_s {
	uint32_t state = 0;
	uint8_t id = 0;
	bool forced = false;
	bool cmd_consumed = false;
};

struct tankstatus_s {
	uint32_t state;
	uint8_t id;
};

struct plantstatus_s{
	float health;
	uint8_t id;
	char name[NAME_LENGTH];
};

struct sectorstatus_s {
	uint32_t state;
	uint8_t id;
	char plants[PLANTS_LENGTH];
};

struct batterystatus_s{
	uint16_t remaining_time_min;
	uint8_t id;
	uint8_t percentage;
	uint8_t status;
};
#pragma pack(pop)
#endif


#define PAYLOAD_SIZE 32
/**
 * @brief  Targets
 */
typedef enum _target_t : uint8_t{
	Generic 	= 0x00,
	Pump 		= 0x01,   
	Tank 		= 0x02,     
	Plant 		= 0x03,
	Sector		= 0x04,
	Power		= 0x0D,
	System		= 0x0E,
	All			= 0xFF
}target_t;

/**
 * @brief  Directions
 */
typedef enum _direction_t : uint8_t{
	RPiToIRM 	= 0xAA,
	IRMToRPi 	= 0xBB
} direction_t;

/**
 * @brief  Commands, 0x1X - Pump, 0x2X - Tank, 0x3X - Plant, 0xDX - Power Supply, 0xEX - System 
 */
typedef enum _command_t : uint8_t{
	None			= 0x00,
	Start 			= 0x10,
	Stop 			= 0x11,
	ForceStart		= 0x12,
	ForceStop		= 0x13,
	StartRev		= 0x14,
	ForceStartRev	= 0x15,
	GetLiquidLevel	= 0x20,
	GetTemperature	= 0x21,
	GetMoisture		= 0x30,
	GetVoltage		= 0xD0,
	GetCurrent		= 0xD1,
	GetChargeLevel	= 0xD2,
	GetCycles		= 0xD3,
	GetDescription	= 0xFB,
	SetSleep		= 0xFC,
	SetStandby		= 0xFD,
	GetState		= 0xFE,
	GetStatus		= 0xFF
} command_t;

#pragma pack(push,1)
struct servicecode_s{
	target_t reporter;
	uint8_t id;
	uint32_t code;
};

struct wframeDl_s {
	direction_t start;
	target_t target;
	uint8_t target_id;
	command_t cmd;
	uint8_t free[PAYLOAD_SIZE-5];
	uint8_t crc8;
};

struct wframeUl_s {
	direction_t start;
	target_t sender;
	uint8_t sender_id;
	union val32_u val; 				//<<Make sure this is 32bit
	char  desc[PAYLOAD_SIZE-8];
	uint8_t crc8;
};

struct cmd_s{
	target_t target;
	uint8_t target_id;
	command_t cmd;
	uint8_t subcmd1;
	uint8_t subcmd2;
};

struct confirmation_s{
	target_t target;
	uint8_t target_id;
	command_t cmd;
	uint8_t subcmd1;
	uint8_t subcmd2;
	bool consumed;
};

struct singlevalue_s{
	target_t target;
	uint8_t target_id;
	float value;
};

union servicecode_u{
	servicecode_s servicecode;
	uint8_t buffer[6];
};

union ulframe32byte_u{
	uint8_t 	buffer[PAYLOAD_SIZE];
	wframeUl_s	values;
};

union dlframe32byte_u{
	uint8_t 	buffer[PAYLOAD_SIZE];
	wframeDl_s 	values;
};
#pragma pack(pop)



class IrrigationMessage: Message{

private:

	direction_t								commdirection;
	std::array<uint8_t, PAYLOAD_SIZE> 		buffer;

public:

	dlframe32byte_u							*downlinkframe = nullptr;
	ulframe32byte_u							*uplinkframe = nullptr;

	IrrigationMessage(const direction_t & _commdirection):
	commdirection(_commdirection)
	{
		switch(this->commdirection){
		case direction_t::RPiToIRM:
			this->downlinkframe = new dlframe32byte_u;
			break;
		case direction_t::IRMToRPi:
			this->uplinkframe = new ulframe32byte_u;
			break;
		}
	};

	~IrrigationMessage()
	{
		if (this->downlinkframe != nullptr) delete this->downlinkframe;
		if (this->uplinkframe != nullptr) delete this->uplinkframe;
	};

	bool 									validateCRC() override;
	bool									setBuffer(uint8_t* _frame, const size_t & _buffer_size) override;
	std::array<uint8_t, PAYLOAD_SIZE>&		getBuffer();
	struct cmd_s							decodeCommand();
	struct tankstatus_s						decodeTank();
	struct pumpstatus_s						decodePump();
	struct plantstatus_s					decodePlant();
	struct sectorstatus_s					decodeSector();
	struct batterystatus_s					decodeBattery();
	struct confirmation_s					decodeConfirmation();
	std::array<uint8_t, PAYLOAD_SIZE>&			encode(const struct cmd_s &_cmd);
	std::array<uint8_t, PAYLOAD_SIZE>&			encode(const struct confirmation_s &_confirmation);
	std::array<uint8_t, PAYLOAD_SIZE>&			encode(const struct tankstatus_s &_tank);
	std::array<uint8_t, PAYLOAD_SIZE>&			encode(const struct pumpstatus_s &_pump);
	std::array<uint8_t, PAYLOAD_SIZE>&			encode(const struct plantstatus_s &_plant);
	std::array<uint8_t, PAYLOAD_SIZE>&			encode(const struct sectorstatus_s &_sector);
	std::array<uint8_t, PAYLOAD_SIZE>&			encode(const struct batterystatus_s &_battery);
	std::array<uint8_t, PAYLOAD_SIZE>&			encodeGeneric(const target_t & _target, const uint8_t & _id, const uint32_t & _state);

};

#endif /* MSG_DEFINITIONS_IRRIGATION_H_ */

