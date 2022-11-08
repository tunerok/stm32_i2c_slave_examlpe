#include "registers.h"

volatile reg_t g_i2c_reg_data[] =
{
		[VERSION] = 		{ READ_ONLY, 	REG_VERSION_ADDR, 	CHAR,   {.char_val = 0x01}, 	{0} },
		[UINT16_RW] = 		{ FULL_ACCESS, 	REG_UINT16_RW_ADDR, UINT16,   {.uint16_val = 0x00}, 	{0} },
		[INT16_RW] = 		{ FULL_ACCESS, 	REG_INT16_RW_ADDR, 	INT16, {.int16_val = 0x00}, 	{0} },
		[BOOL_RW] = 		{ FULL_ACCESS, 	REG_BOOL_RW_ADDR, 	BOOL, {.bool_val = 0x00}, 	{0} },
		[CHAR_RW] = 		{ FULL_ACCESS, 	REG_CHAR_RW_ADDR, 	CHAR, {.char_val = 0x00}, 	{0} },
		[UINT16_RO] =		{ READ_ONLY, 	REG_UINT16_RO_ADDR, UINT16, {.uint16_val = 0x3344}, {0} },
		[INT16_RO] = 		{ READ_ONLY, 	REG_INT16_RO_ADDR, 	INT16, {.int16_val = 0x2233}, 	{0} },
		[BOOL_RO] = 		{ READ_ONLY, 	REG_BOOL_RO_ADDR, 	BOOL,   {.bool_val = 0x01}, 	{0} },
		[CHAR_RO] = 		{ READ_ONLY, 	REG_CHAR_RO_ADDR, 	CHAR, {.char_val = 0x15},		{0} },
};

reg_idx_t reg_get_idx(uint8_t address){
	for(int i = 1; i < REGISTER_NUM; i++){
		if (g_i2c_reg_data[i].reg_addr == address){
			return i;
		}
	}
	return NONE;
}

int reg_get_len(reg_idx_t idx){
	int data_len = 0;
	switch (g_i2c_reg_data[idx].value_type){
	case BOOL:
	case CHAR:
		data_len = 1;
		break;
	case UINT16:
	case INT16:
		data_len = 2;
		break;
	default:
		break;
	}
	return data_len;
}


//Restoring factory settings
void reg_factory(void) {
	for (int idx = 1; idx < REGISTER_NUM - 1; idx++) {
		reg_idx_t reg_idx = idx;
		switch (g_i2c_reg_data[reg_idx].value_type) {
		case UNDEFINED:
			break;
		case BOOL:
			g_i2c_reg_data[reg_idx].value.bool_val =
					g_i2c_reg_data[reg_idx].def_val.bool_val;
			break;
		case UINT16:
			g_i2c_reg_data[reg_idx].value.uint16_val =
					g_i2c_reg_data[reg_idx].def_val.uint16_val;
			break;
		case INT16:
			g_i2c_reg_data[reg_idx].value.int16_val =
					g_i2c_reg_data[reg_idx].def_val.int16_val;
			break;
		case CHAR:
			g_i2c_reg_data[reg_idx].value.char_val =
					g_i2c_reg_data[reg_idx].def_val.char_val;
			break;
		default:
			break;
		}
	}
}
