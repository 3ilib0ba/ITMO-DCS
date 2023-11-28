/*
 * kb.h
 *
 *  Created on: Nov 28, 2023
 *      Author: kastr
 */

#ifndef INC_KB_H_
#define INC_KB_H_

#include "i2c.h"

#define ROW1 0xFE
#define ROW2 0xFD
#define ROW3 0xFB
#define ROW4 0xF7


uint8_t Check_Row( uint8_t  Nrow );
uint8_t Get_Key( uint8_t  Nrow );
HAL_StatusTypeDef Set_Keyboard( void );

#endif /* INC_KB_H_ */
