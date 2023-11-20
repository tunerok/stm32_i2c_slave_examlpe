/*
 * i2c_slave.c
 *
 *  Created on: Oct 14, 2022
 *      Author: ashirov
 */

#include "i2c_slave.h"
#include "registers.h"


volatile i2c_t I2C_slave_obj;

extern reg_t g_i2c_reg_data[];

// Delay has to constant expression
static void inline __attribute__((always_inline)) delayus(unsigned us_mul_5)
{
    uint32_t ticks = SYSTICKPERUS * us_mul_5;
    uint32_t start_tick = SysTick->VAL;

    while(SysTick->VAL - start_tick < ticks);
}


void i2c_slave_clear(void){
	I2C_slave_obj.reg_address = 0;
	I2C_slave_obj.curr_idx = NONE;
	I2C_slave_obj.ready_to_answer = 0;
	I2C_slave_obj.ready_to_write = 0;

}

int i2c_slave_init(I2C_HandleTypeDef *hi2c){
	I2C_slave_obj.i2c_handler = hi2c;
	i2c_slave_clear();
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
			HAL_I2C_Init(I2C_slave_obj.i2c_handler);
			HAL_I2C_EnableListen_IT(I2C_slave_obj.i2c_handler);
			rx_busy_counter = 0;
	  }
}

//******************
//-------i2c--------
//******************


void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode){
	UNUSED(AddrMatchCode);
	//	Если мастер пишет, слушаем необходимое количество байт
	if(TransferDirection == I2C_DIRECTION_TRANSMIT){
		// Первый запрос на запись всегда составляет 1 байт запрошенного регистрового адреса.
		// Сохранить его в I2C_slave_obj.reg_address
		if(!I2C_slave_obj.reg_addr_rcvd)
			HAL_I2C_Slave_Sequential_Receive_IT(hi2c, &I2C_slave_obj.reg_address, 1, I2C_FIRST_FRAME);
	}
	else {
		// Если мастер отправляет запрос на чтение, вернуть значение данных в запрошенном регистре.
		I2C_slave_obj.curr_idx = reg_get_idx(I2C_slave_obj.reg_address);
		if ((I2C_slave_obj.curr_idx != NONE)&& (I2C_slave_obj.curr_idx != ECHO)&& (g_i2c_reg_data[I2C_slave_obj.curr_idx].access != WRITE_ONLY)){
			HAL_I2C_Slave_Sequential_Transmit_IT(hi2c, (uint8_t*)&g_i2c_reg_data[I2C_slave_obj.curr_idx].value.uint16_val, reg_get_len(I2C_slave_obj.curr_idx), I2C_LAST_FRAME);
		}
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c){
	// Это вызывается после основного запроса на запись. в первый раз это будет адрес регистра.
	// Второй раз, если это запрос на запись, это будет полезная нагрузка
	if(!I2C_slave_obj.reg_addr_rcvd){
		// Если reg_addr_rcvd имеет значение false, это означает, что мастер ждет данные
		I2C_slave_obj.reg_addr_rcvd = 1;
		I2C_slave_obj.curr_idx = reg_get_idx(I2C_slave_obj.reg_address);
		if ((I2C_slave_obj.curr_idx != NONE)&& (I2C_slave_obj.curr_idx != ECHO)&& (g_i2c_reg_data[I2C_slave_obj.curr_idx].access != READ_ONLY)){
			HAL_I2C_Slave_Sequential_Receive_IT(hi2c, (uint8_t*)&g_i2c_reg_data[I2C_slave_obj.curr_idx].value.uint16_val, reg_get_len(I2C_slave_obj.curr_idx), I2C_NEXT_FRAME);
		}
	} else {
		// Если reg_addr_rcvd установлен, это означает, что этот обратный вызов был возвращен после получения данных регистра
		I2C_slave_obj.reg_addr_rcvd = 0;
		//добавим быструю обработку
		protocol_reg_ctrl(I2C_slave_obj.curr_idx);
		I2C_slave_obj.curr_idx = NONE;


	}
	HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_ListenCpltCallback (I2C_HandleTypeDef *hi2c){
	HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c){
	I2C_slave_obj.reg_addr_rcvd = 0;
	HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	//HAL_I2C_ERROR_NONE       0x00000000U    /*!< No error           */
	//HAL_I2C_ERROR_BERR       0x00000001U    /*!< BERR error         */
	//HAL_I2C_ERROR_ARLO       0x00000002U    /*!< ARLO error         */
	//HAL_I2C_ERROR_AF         0x00000004U    /*!< Ack Failure error  */
	//HAL_I2C_ERROR_OVR        0x00000008U    /*!< OVR error          */
	//HAL_I2C_ERROR_DMA        0x00000010U    /*!< DMA transfer error */
	//HAL_I2C_ERROR_TIMEOUT    0x00000020U    /*!< Timeout Error      */
	uint32_t error_code = HAL_I2C_GetError(hi2c);
	if (error_code != HAL_I2C_ERROR_AF){}
	HAL_I2C_EnableListen_IT(hi2c);
}


