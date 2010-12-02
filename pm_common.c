/* pm_common.c */

/* spolecne funkce
 * - zapis a cteni do konfiguracniho souboru
 * */

#include <stdio.h>
#include "pm_common.h"
#include "s2e_common.h"


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

char *pm_conf_file = "/etc/pm.conf";

//reset all values in the structure
void pm_default_config (tPM *sPm){
	int i;

	//
	sPm->frequence = 1;
	sPm->temperature = 2;

	//lines default values
	for(i=0; i<NR_LINES; i++){
		sPm->sLines[i].voltage = 0;
		sPm->sLines[i].current = 0;
		sPm->sLines[i].power = 0;
		sPm->sLines[i].energy = 0;
		sPm->sLines[i].pf = 0;
	}

	//neutral default values
	sPm->sNeutral.current = 0;
	sPm->sNeutral.voltage = 0;
	sPm->sNeutral.power = 0;
	sPm->sNeutral.energy = 0;
}

// read config from file to the structure
int pm_read_config (tPM *sPm)
{
	FILE *fp = NULL;
	char input[128];
	char token[64];
	int line = 0;

	fp = fopen (pm_conf_file, "r");
	if (fp == NULL) {
		fprintf(stderr, "can't open %s: %s\n", pm_conf_file, strerror(errno));
		return (-1);
	}

	while (fgets(input, sizeof (input), fp)!=NULL){
		char *ptr = input;
		line ++;

		ptr = get_token (ptr, token, sizeof (token));
		if (token[0] == '#')
			continue;

		if (strcasecmp (token, "pm") == 0) {

			ptr = get_token (ptr, token, sizeof (token));
			if (strcmp (token, "{") != 0)
				goto err;

			while (fgets(input, sizeof (input), fp)!=NULL){
				char *ptr = input;
				line++;

				ptr = get_token (ptr, token, sizeof (token));
				if (token[0] == '#')
					continue;

				if (strcmp (token, "}") == 0) {
					break;
				}

				//FREQUENCE
				else if (strcasecmp (token, "frequence") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->frequence = atol (token);
				}
				//TEMPERATURE
				else if (strcasecmp (token, "temperature") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->temperature = atol (token);
				}

				//VOLTAGE
				else if (strcasecmp (token, "l1_voltage") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[0].voltage = atol (token);
				}
				else if (strcasecmp (token, "l2_voltage") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[1].voltage = atol (token);
				}
				else if (strcasecmp (token, "l3_voltage") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[2].voltage = atol (token);
				}

				//CURRENT
				else if (strcasecmp (token, "l1_current") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[0].current = atol (token);
				}
				else if (strcasecmp (token, "l2_current") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[1].current = atol (token);
				}
				else if (strcasecmp (token, "l3_current") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[2].current = atol (token);
				}

				//POWER
				else if (strcasecmp (token, "l1_power") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[0].power = atol (token);
				}
				else if (strcasecmp (token, "l2_power") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[1].power = atol (token);
				}
				else if (strcasecmp (token, "l3_power") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[2].power = atol (token);
				}

				//ENERGY
				else if (strcasecmp (token, "l1_energy") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[0].energy = atol (token);
				}
				else if (strcasecmp (token, "l2_energy") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[1].energy = atol (token);
				}
				else if (strcasecmp (token, "l3_energy") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[2].energy = atol (token);
				}

				//POWER FACTOR
				else if (strcasecmp (token, "l1_pf") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[0].pf = atol (token);
				}
				else if (strcasecmp (token, "l2_pf") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[1].pf = atol (token);
				}
				else if (strcasecmp (token, "l3_pf") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					sPm->sLines[2].pf = atol (token);
				}

			}
		}
	}

	fclose (fp);
	return 0;

err:
	fprintf(stderr, "Error %s in %d\n", &pm_conf_file, line);
	if (fp)	fclose (fp);
	return -1;
}

//write structure into the file
int pm_write_config (tPM *sPm)
{
	int i;
	FILE *fp = NULL;

	fp = fopen (pm_conf_file, "w");
	if (fp == NULL) {
		fprintf(stderr, "can't open %s: %s\n", pm_conf_file, strerror(errno));
		return (-1);
	}

	fprintf (fp, "# %s (Automatically generated) \n", pm_conf_file);
	fprintf (fp, "pm {\n");

	fprintf (fp, "\tfrequence\t%u\n", sPm->frequence);
	fprintf (fp, "\ttemperature\t%d\n", sPm->temperature);
	for(i=0; i<NR_LINES; i++){
		fprintf (fp, "\tl%d_current\t%d\n", i+1, sPm->sLines[i].current);
		fprintf (fp, "\tl%d_voltage\t%d\n", i+1, sPm->sLines[i].voltage);
		fprintf (fp, "\tl%d_power\t%d\n", i+1, sPm->sLines[i].power);
		fprintf (fp, "\tl%d_energy\t%d\n", i+1, sPm->sLines[i].energy);
		fprintf (fp, "\tl%d_pf\t\t%d\n", i+1, sPm->sLines[i].pf);
	}

	fprintf (fp, "}\n");
	fprintf (fp, "\n");

	fclose (fp);
	return 0;
}
