/*
 * wireless_base.h
 *
 *  Created on: 25.01.2020
 *      Author: Mati
 */

#ifndef COMMUNICATION_BASE_H_
#define COMMUNICATION_BASE_H_

#ifndef RPI
#include "stm32f3xx_hal.h"
#else
#include "../defines.h"
#endif


enum class communicationtype_t{
	undetermined,
	point_to_point,
	wifi
};

union val32_u{
	float float32;
	uint32_t uint32;
	uint8_t uint8[4];
};

union usartbuffer32_u{
	uint32_t status;
	uint8_t buffer[4];
};


class Wireless{

protected:

	communicationtype_t 			commType;

	communicationtype_t& 			typeGet(void);


public:

	Wireless(){};

	virtual ~Wireless(){};

};


class Message{

protected:

	uint8_t									calculateCRC8(uint8_t *data,const size_t & _payload_size);

public:

	Message(){};

	virtual ~Message(){};

	virtual bool 							validateCRC() = 0;
	virtual bool							setBuffer(uint8_t* _frame, const size_t & _buffer_size) = 0;
};




#endif /* COMMUNICATION_BASE_H_ */
