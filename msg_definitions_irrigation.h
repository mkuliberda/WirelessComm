/**
 * @brief  Project specific communication protocol defines and enums
 */
#ifndef MSG_DEFINITIONS_IRRIGATION_H_
#define MSG_DEFINITIONS_IRRIGATION_H_

#ifndef RPI
#include "stm32f3xx_hal.h"
#else
#include "../defines.h"
#endif
#include "communication_base.h"
#include <algorithm>
#include <array>

#define RADIO1_PAYLOAD_SIZE 32
/**
 * @brief  Targets
 */
typedef enum target_t {
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
typedef enum _direction_t {
	RPiToIRM 	= 0xAA,
	IRMToRPi 	= 0xBB
} direction_t;

/**
 * @brief  Commands, 0x1X - Pump, 0x2X - Tank, 0x3X - Plant, 0xDX - Power Supply, 0xEX - System 
 */
typedef enum _command_t {
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
	uint8_t free[27];
	uint8_t crc8;
};

struct wframeUl_s {
	direction_t start;
	target_t sender;
	uint8_t sender_id;
	union val32_u val; 				//<<Make sure this is 32bit
	char  desc[24];
	uint8_t crc8;
};

struct cmd_s{
	target_t target;
	uint8_t target_id;
	command_t cmd;
	uint8_t subcmd1;
	uint8_t subcmd2;
	uint8_t subcmd3;
	uint8_t subcmd4;
};

union servicecode_u{
	servicecode_s servicecode;
	uint8_t buffer[6];
};

union ulframe32byte_u{
	uint8_t 	buffer[32];
	wframeUl_s	values;
};

union dlframe32byte_u{
	uint8_t 	buffer[32];
	wframeDl_s 	values;
};

using namespace std;

//class UplinkMessage{
//
//private:
//
//	uint8_t& calculateCRC();
//
//
//public:
//
//	uint8_t 	buffer[32];
//
//	UplinkMessage(){};
//
//	UplinkMessage(){};
//
//	void encode(void);
//
//
//
//
//};

class DownlinkMessage{

private:

	array<uint8_t, RADIO1_PAYLOAD_SIZE> 	buffer;
	dlframe32byte_u							downlinkframe;

	uint8_t									calculateCRC8(uint8_t *data);

public:

	DownlinkMessage(){};

	~DownlinkMessage(){};

	bool 									validateCRC();
	struct cmd_s 							decode();
	array<uint8_t, RADIO1_PAYLOAD_SIZE>		encode(struct cmd_s _cmd);
	bool									setBuffer(uint8_t* _frame);

};

#endif /* MSG_DEFINITIONS_IRRIGATION_H_ */

