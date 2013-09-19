/*
 * i2cutils.h
 *
 *  Created on: Jul 9, 2013
 *      Author: tgorski
 */

#ifndef I2CUTILS_H_
#define I2CUTILS_H_

#include "xiic.h"
#include "xparameters.h"
#include "xbasic_types.h"

#define SI5324A_I2C_ADDR		(0x68)			// 7 bit address
#define SI5324C_I2C_ADDR		(0x69)			// 7 bit address
#define DS25CP104_I2C_ADDR		(0x50)			// 7 bit address

void init_DS25CP104(void);

void init_SI5324A(void);

void check_SI5324A(void);

void init_SI5324C(void);

void check_SI5324C(void);


#endif /* I2CUTILS_H_ */
