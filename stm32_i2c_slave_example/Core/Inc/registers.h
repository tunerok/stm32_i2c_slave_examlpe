#ifndef INC_REGISTERS_H_
#define INC_REGISTERS_H_

#include <stdint.h>

//Register Addresses
#define REG_VERSION_ADDR		0x00
#define REG_UINT16_RW_ADDR		0x01
#define REG_INT16_RW_ADDR		0x02
#define REG_BOOL_RW_ADDR		0x03
#define REG_CHAR_RW_ADDR		0x04
#define REG_UINT16_RO_ADDR		0x11
#define REG_INT16_RO_ADDR		0x12
#define REG_BOOL_RO_ADDR		0x13
#define REG_CHAR_RO_ADDR		0x14

//Register value types
typedef enum val_type_t {
    EMPTY,
    UNDEFINED,
    BOOL,
    UINT16,
	INT16,
    CHAR
} val_type_t;


//Register indices
typedef enum reg_idx_t {
    NONE = -1,
    ECHO = 0,
    VERSION,
	UINT16_RW,
	INT16_RW,
	BOOL_RW,
	CHAR_RW,
	UINT16_RO,
	INT16_RO,
	BOOL_RO,
	CHAR_RO,
	REGISTER_NUM
} reg_idx_t;

//Access to registers
typedef enum reg_mode_t {
    RESERVED = 0,
    WRITE_ONLY,
    READ_ONLY,
    FULL_ACCESS
} reg_mode_t;

//Type for register values
typedef union var_t {
    uint8_t  bool_val;
    uint16_t uint16_val;
    int16_t  int16_val;
    char     char_val;
} var_t;


//Register structure
typedef struct reg_t reg_t;

struct reg_t {
	uint8_t		  		access;		//Access
	uint8_t 	  		reg_addr;	//Address
	val_type_t    		value_type; //Value type
    const var_t 		def_val;     //Default values
    var_t       		value;       //Current values
};


reg_idx_t reg_get_idx(uint8_t address);
void reg_factory(void);
int reg_get_len(reg_idx_t idx);


#endif
