#ifndef INC_PROTOCOL_H_
#define INC_PROTOCOL_H_

#include <stdint.h>
#include "registers.h"
#include "i2c_slave.h"


//Parsing data and events received via i2c
int protocol_i2c_parse(int i2c_event);

//Data processing in registers
void protocol_reg_ctrl(reg_idx_t idx);

#endif /* INC_PROTOCOL_H_ */
