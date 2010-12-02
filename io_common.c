/* io_common.c */

/* spolecne funkce
 * - zapis a cteni do konfiguracniho souboru
 * */

#include "io_common.h"
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

char *conf_file = "/etc/myweb-io.conf";

//reset all values in the structure
void io_default_config (tIO *sIo){
	unsigned char i;

	//inputs
	for(i=0;i<NR_EXT_INPUTS;i++){
		sIo->ext_inputs[i].current_state = 0;
		sIo->ext_inputs[i].default_state = 0;
	}

	//outputs
	for(i=0;i<NR_EXT_OUTPUTS;i++){
		sIo->ext_outputs[i].current_state = 0;
		sIo->ext_outputs[i].default_state= 0;
	}
}

// read config from file to the structure
int io_read_config (tIO *conf_data)
{
	FILE *fp = NULL;
	char input[128];
	char token[64];
	int line = 0;

	fp = fopen (conf_file, "r");
	if (fp == NULL) {
//		fprintf(stderr, "can't open %s: %s\n", conf_files[chan], strerror(errno));
		return (-1);
	}

	while (fgets(input, sizeof (input), fp)!=NULL){
		char *ptr = input;
		line ++;

		ptr = get_token (ptr, token, sizeof (token));
		if (token[0] == '#')
			continue;

		if (strcasecmp (token, "ext_inputs") == 0) {

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
				else if (strcasecmp (token, "ch_1_current_state") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					conf_data->ext_inputs[0].current_state = atol (token);
				}
				else if (strcasecmp (token, "ch_2_current_state") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					conf_data->ext_inputs[1].current_state = atol (token);
				}
			}
		}
		else if (strcasecmp (token, "ext_outputs") == 0) {
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
				else if (strcasecmp (token, "ch_1_current_state") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					conf_data->ext_outputs[0].current_state = atol (token);
				}
				else if (strcasecmp (token, "ch_2_current_state") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					conf_data->ext_outputs[1].current_state = atol (token);
				}
				else if (strcasecmp (token, "ch_1_default_state") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					conf_data->ext_outputs[0].default_state = atol (token);
				}
				else if (strcasecmp (token, "ch_2_default_state") == 0) {
					ptr = get_token (ptr, token, sizeof (token));
					conf_data->ext_outputs[1].default_state = atol (token);
				}
			}
		}
	}

	fclose (fp);
	return 0;

err:
//	fprintf(stderr, "Error %s in %d\n", &conf_files[chan], line);
	if (fp)	fclose (fp);
	return -1;
}

//write structure into the file
int io_write_config (tIO *conf_data)
{
	int i;
	FILE *fp = NULL;

	fp = fopen (conf_file, "w");
	if (fp == NULL) {
		fprintf(stderr, "can't open %s: %s\n", conf_file, strerror(errno));
		return (-1);
	}

	fprintf (fp, "# %s (Automatically generated) \n", conf_file);
	fprintf (fp, "ext_inputs {\n");
	for(i=0;i<NR_EXT_INPUTS;i++){
		fprintf (fp, "\tch_%d_current_state\t%d\n", i+1, conf_data->ext_inputs[i].current_state);
		//fprintf (fp, "ch_%d_current_state\t%d\n", i+1, conf_data->ext_inputs[i].default_state);
	}

	fprintf (fp, "}\n");
	fprintf (fp, "\n");

	fprintf (fp, "ext_outputs {\n");
	for(i=0;i<NR_EXT_INPUTS;i++){
		fprintf (fp, "\tch_%d_current_state\t%d\n", i+1, conf_data->ext_outputs[i].current_state);
		fprintf (fp, "\tch_%d_default_state\t%d\n", i+1, conf_data->ext_outputs[i].default_state);
	}

	fprintf (fp, "}\n");

	fclose (fp);
	return 0;
}
