#include "protocol.h"

extern reg_t g_i2c_reg_data[];

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

