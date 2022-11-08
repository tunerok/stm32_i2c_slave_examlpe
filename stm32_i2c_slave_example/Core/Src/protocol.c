#include "protocol.h"

extern reg_t g_i2c_reg_data[];


int protocol_i2c_parse(int i2c_event){

	reg_idx_t idx;
	static uint8_t buff[5] = {0};

	switch (i2c_event) {
	//We want to read reg
	  case ReadAddressed:
	  {
		  //Get reg idx by address
		  idx = reg_get_idx(buff[0]);
		  //Check if we can get access to this reg
		  if ((idx != NONE) && (idx != ECHO) && (g_i2c_reg_data[idx].reg_addr != WRITE_ONLY)){
			  //If OK, send data contains by reg
			  i2c_slave_write((uint8_t *)&g_i2c_reg_data[idx].value, reg_get_len(idx));
		  }
		  else{
			  //If not OK, send double 0xAA as the error msg
			  buff[0] = 0xAA;
			  buff[1] = 0xAA;
			  i2c_slave_write(buff, 2);
		  }
		  break;
	  }
	  case WriteGeneral:
	  {
		 i2c_slave_read(buff, 1);
		  break;
	  }
	  //We want to write to reg. in fact, we always get here for any request from the master
	  //Therefore, we need to check the length of the data packet.
	  //The first data received will be the address of the register.
	  //If the message length is greater than 1, then we are going to write to the register.
	  case WriteAddressed:
	  {
		  int data_cnt = 0;
		  data_cnt = i2c_slave_read(buff, 3);
		  if (data_cnt > 1){
			  //This is write request
			  //Get reg idx by address
			  idx = reg_get_idx(buff[0]);
			  //Check if we can get access to this reg
			  if ((idx != NONE) && (idx != ECHO) && (g_i2c_reg_data[idx].reg_addr != READ_ONLY)){
				  //Change data in the regs by idx
				  switch (g_i2c_reg_data[idx].value_type) {
					case UINT16:
						g_i2c_reg_data[idx].value.uint16_val = (uint16_t)(buff[1] | buff[2] << 8);
						break;
					case BOOL:
						g_i2c_reg_data[idx].value.char_val = buff[1] & 0x01;
						break;
					case CHAR:
						g_i2c_reg_data[idx].value.char_val = buff[1];
						break;
					default:
						return 0;
				}
				//We can also add some processing on the received values
				protocol_reg_ctrl(idx);
			  }
		  }
		  //This is NOT write request, so we drops to ReadAddressed by the next switch roll
		  break;
	   }
	  default:
		  break;
	}
	return 1;
}


void protocol_reg_ctrl(reg_idx_t idx){
	  switch (idx){
	  case UINT16_RW:
		  g_i2c_reg_data[UINT16_RO].value.uint16_val++;
		  break;
	  case INT16_RW:
		  g_i2c_reg_data[INT16_RO].value.int16_val++;
		  break;
	  default:
		  break;
	  }
}

