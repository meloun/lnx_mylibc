/* pm_common.h */

#ifndef _PM_COMMON_H
#define _PM_COMMON_H

/* user defines */
#define NR_LINES	3

/* end of users parameters*/

//data structure for each line
typedef struct{
	signed int current;
	signed int voltage;
	unsigned int power;
	unsigned int energy;
	unsigned int pf;
}tPM_LINE;

//data structure
typedef struct{
	unsigned int frequence;
	signed int temperature;
	tPM_LINE sLines[NR_LINES];	//3 lines
	tPM_LINE sNeutral;			//
}tPM;

extern char *conf_file;

extern void pm_default_config(tPM *sPm);
extern int pm_read_config(tPM *sPm);
extern int pm_write_config(tPM *sPm);

#endif	/* _PM_COMMON_H */
