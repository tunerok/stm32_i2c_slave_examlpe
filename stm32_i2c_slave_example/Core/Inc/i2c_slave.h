#ifndef INC_I2C_SLAVE_H_
#define INC_I2C_SLAVE_H_

#include "main.h"
#include "registers.h"

//Change this, or write your OWN 1us delay func (i2c_slave.c inside)
#define SYSTICKCLOCK 80000000ULL
#define SYSTICKPERUS (SYSTICKCLOCK / 1000000UL)
#define BYTE_TIMEOUT_US   (SYSTICKPERUS * 3 * 10)

#define I2C_RX_BUSY_CNTR	150


typedef struct i2c_s {
	 volatile reg_idx_t curr_idx;
	 volatile uint8_t reg_addr_rcvd;
	 volatile uint8_t reg_address;
	 volatile uint8_t ready_to_answer;
	 volatile uint8_t ready_to_write;
	 I2C_HandleTypeDef *i2c_handler;
 }i2c_t;



int i2c_slave_init(I2C_HandleTypeDef *hi2c);
void i2c_slave_check_timeout(void);


#endif /* INC_I2C_SLAVE_H_ */
