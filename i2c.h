/* 
 * File:   i2c.h
 * Author: matthew
 *
 * Created on 25 September 2015, 15:56
 */

#include <stdbool.h>

void i2c_Init(void);
void i2c_DataReceived(void);
bool i2c_IsTransferActive(void);