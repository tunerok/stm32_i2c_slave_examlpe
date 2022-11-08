/*
 * i2c_slave.c
 *
 *  Created on: Oct 14, 2022
 *      Author: ashirov
 */

#include "i2c_slave.h"


volatile i2c_t I2C_slave_obj;

// Delay has to constant expression
static void inline __attribute__((always_inline)) delayus(unsigned us_mul_5)
{
    uint32_t ticks = SYSTICKPERUS * us_mul_5;
    uint32_t start_tick = SysTick->VAL;

    while(SysTick->VAL - start_tick < ticks);
}


int i2c_slave_init(I2C_HandleTypeDef *hi2c){
	I2C_slave_obj.pending_slave_rx_maxter_tx = 0;
	I2C_slave_obj.pending_slave_tx_master_rx = 0;
	I2C_slave_obj.pending_start = 0;
	I2C_slave_obj.i2c_handler = hi2c;
	return 0;
}

int i2c_slave_read(uint8_t *data, int length)
{
    int count = 0;
    int ret = 0;
    uint32_t timeout = 0;
    int _length = 0;
	/*  We don't know in advance how many bytes will be sent by master so
	 *  we'll fetch one by one until master ends the sequence */
	_length = 1;
	I2C_slave_obj.slave_rx_buffer_size = length;
	I2C_slave_obj.slave_rx_count = 0;
	I2C_slave_obj.slave_rx_buffer = (uint8_t *)data;

    /*  Always use I2C_NEXT_FRAME as slave will just adapt to master requests */
    ret = HAL_I2C_Slave_Seq_Receive_IT(I2C_slave_obj.i2c_handler, (uint8_t *) data, _length, I2C_NEXT_FRAME);

    if (ret == HAL_OK) {
        timeout = BYTE_TIMEOUT_US * (_length + 1);
        while (I2C_slave_obj.pending_slave_rx_maxter_tx && (--timeout != 0)) {
        	delayus(1);
        }
        if (timeout != 0) {
           count = I2C_slave_obj.slave_rx_count;
        }
    }
    return count;
}

int i2c_slave_write(uint8_t *data, int length)
{
    int count = 0;
    int ret = 0;
    uint32_t timeout = 0;

    /*  Always use I2C_NEXT_FRAME as slave will just adapt to master requests */
    ret = HAL_I2C_Slave_Seq_Transmit_IT(I2C_slave_obj.i2c_handler, (uint8_t *) data, length, I2C_NEXT_FRAME);

    if (ret == HAL_OK) {
        timeout = BYTE_TIMEOUT_US * (length + 1);
        while (I2C_slave_obj.pending_slave_tx_master_rx && (--timeout != 0)) {
        	delayus(1);
        }

        if (timeout != 0) {
            count = length;
        } else {
            count = 0;
        }
    }

    return count;
}

int i2c_slave_receive(void)
{
    int retValue = NoData;

    if (I2C_slave_obj.pending_slave_rx_maxter_tx) {
        retValue = WriteAddressed;
    }

    if (I2C_slave_obj.pending_slave_tx_master_rx) {
        retValue = ReadAddressed;
    }

    return (retValue);
}

void i2c_slave_clear_pending(void){
	I2C_slave_obj.pending_slave_rx_maxter_tx = 0;
	I2C_slave_obj.pending_slave_tx_master_rx = 0;
}

static void ResetI2C(I2C_HandleTypeDef* rev_i2c){
	HAL_I2C_DeInit(rev_i2c);
	HAL_I2C_Init(rev_i2c);
}


void i2c_slave_check_timeout(void){

	static int rx_busy_counter = 0;
	HAL_I2C_StateTypeDef status = HAL_OK;

	 status = HAL_I2C_GetState(I2C_slave_obj.i2c_handler);

	  if (status == HAL_I2C_STATE_BUSY_RX_LISTEN){
		  rx_busy_counter++;
	  }
	  else{
		  rx_busy_counter = 0;
	  }

	  if (rx_busy_counter > I2C_RX_BUSY_CNTR){
		  	HAL_I2C_DisableListen_IT(I2C_slave_obj.i2c_handler);
			HAL_I2C_DeInit(I2C_slave_obj.i2c_handler);
			i2c_slave_clear_pending();
			HAL_I2C_Init(I2C_slave_obj.i2c_handler);
			HAL_I2C_EnableListen_IT(I2C_slave_obj.i2c_handler);
			rx_busy_counter = 0;
	  }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2C_slave_obj.event = I2C_EVENT_TRANSFER_COMPLETE;
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2C_slave_obj.event = I2C_EVENT_TRANSFER_COMPLETE;
}


void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
    /*  Transfer direction in HAL is from Master point of view */
    if (TransferDirection == I2C_DIRECTION_RECEIVE) {
    	I2C_slave_obj.pending_slave_tx_master_rx = 1;
    }

    if (TransferDirection == I2C_DIRECTION_TRANSMIT) {
    	I2C_slave_obj.pending_slave_rx_maxter_tx = 1;
    }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2C_slave_obj.pending_slave_tx_master_rx = 0;
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2C_slave_obj.slave_rx_count++;
        if (I2C_slave_obj.slave_rx_count < I2C_slave_obj.slave_rx_buffer_size) {
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, &(I2C_slave_obj.slave_rx_buffer[I2C_slave_obj.slave_rx_count]), 1, I2C_NEXT_FRAME);
        } else {
        	I2C_slave_obj.pending_slave_rx_maxter_tx = 0;
        }
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	I2C_slave_obj.pending_slave_rx_maxter_tx = 0;

    /* restart listening for master requests */
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    uint32_t event_code = 0;

    if ((hi2c->ErrorCode & HAL_I2C_ERROR_AF) == HAL_I2C_ERROR_AF) {
        /* Keep Set event flag */
        event_code = (I2C_EVENT_TRANSFER_EARLY_NACK) | (I2C_EVENT_ERROR_NO_SLAVE);
    }

    /* re-init IP to try and get back in a working state */
    ResetI2C(hi2c);

    HAL_I2C_EnableListen_IT(hi2c);
    /* Keep Set event flag */
    I2C_slave_obj.event = event_code | I2C_EVENT_ERROR;
}

