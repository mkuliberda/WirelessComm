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




#endif /* COMMUNICATION_BASE_H_ */
