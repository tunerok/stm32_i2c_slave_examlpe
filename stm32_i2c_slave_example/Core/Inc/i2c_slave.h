#ifndef INC_I2C_SLAVE_H_
#define INC_I2C_SLAVE_H_

#include "main.h"

//Change this, or write your OWN 1us delay func (i2c_slave.c inside)
#define SYSTICKCLOCK 80000000ULL
#define SYSTICKPERUS (SYSTICKCLOCK / 1000000UL)
#define BYTE_TIMEOUT_US   (SYSTICKPERUS * 3 * 10)

#define I2C_RX_BUSY_CNTR	150

#define I2C_EVENT_ERROR               (1 << 1)
#define I2C_EVENT_ERROR_NO_SLAVE      (1 << 2)
#define I2C_EVENT_TRANSFER_COMPLETE   (1 << 3)
#define I2C_EVENT_TRANSFER_EARLY_NACK (1 << 4)
#define I2C_EVENT_ALL                 (I2C_EVENT_ERROR |  I2C_EVENT_TRANSFER_COMPLETE | I2C_EVENT_ERROR_NO_SLAVE | I2C_EVENT_TRANSFER_EARLY_NACK)

#define NoData         0 // the slave has not been addressed
#define ReadAddressed  1 // the master has requested a read from this slave (slave = transmitter)
#define WriteGeneral   2 // the master is writing to all slave
#define WriteAddressed 3 // the master is writing to this slave (slave = receiver)

typedef struct i2c_s {
	 uint32_t XferOperation;
	 volatile uint8_t event;
	 volatile int pending_start;
	 volatile uint8_t pending_slave_tx_master_rx;
	 volatile uint8_t pending_slave_rx_maxter_tx;
	 uint8_t *slave_rx_buffer;
	 volatile uint16_t slave_rx_buffer_size;
	 volatile uint16_t slave_rx_count;
	 I2C_HandleTypeDef *i2c_handler;
 }i2c_t;


int i2c_slave_init(I2C_HandleTypeDef *hi2c);
int i2c_slave_read(uint8_t *data, int length);
int i2c_slave_write(uint8_t *data, int length);
void i2c_slave_check_timeout(void);
int i2c_slave_receive(void);
void i2c_clear_pending(void);


#endif /* INC_I2C_SLAVE_H_ */
