/* io_common.h */

#ifndef _IO_COMMON_H
#define _IO_COMMON_H

/* user defines */
#define NR_EXT_INPUTS	2
#define NR_EXT_OUTPUTS	2

/* end of users parameters*/

typedef struct{
	int current_state;
	int default_state;
}tEXTERNAL_INPUT;

typedef struct{
	int current_state;
	int default_state;
}tEXTERNAL_OUTPUT;



typedef struct{
	tEXTERNAL_INPUT ext_inputs[NR_EXT_INPUTS];
	tEXTERNAL_OUTPUT ext_outputs[NR_EXT_OUTPUTS];
}tIO;

extern char *conf_file;

extern void io_default_config(tIO *sIO);
extern int io_read_config(tIO *sIO);
extern int io_write_config(tIO *sIO);

#endif	/* _IO_COMMON_H */
