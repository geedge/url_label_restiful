#ifndef H_IP_DISCOVERY_MASTER_H
#define H_IP_DISCOVERY_MASTER_H
#include <pthread.h>
#include <stdlib.h>

#include <MESA/MESA_handle_logger.h>
#include <MESA/MESA_prof_load.h>
#include <MESA/Maat_rule.h>
#include <MESA/Maat_command.h>
#include <pthread.h>
#include <event.h>
#include <evhttp.h>
#include "cJSON.h"
#include "url_classification.h"

#ifndef MAX
#define MAX(a, b)  	(((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)  	(((a) < (b)) ? (a) : (b))
#endif

#define MAX_CONFIG_NUM 128

struct maat_init_param
{
	char instance_name[32];
	char table_info_path[MAX_PATH_LEN];
	char redis_ip[32];
	unsigned short redis_port;
	int redis_index;
};


typedef struct __global_info
{
	//log
	char run_log_path[256];
	void *runtime_log;
	int log_level;

	//maat
	struct maat_init_param maat_param;
	Maat_feather_t maat_feather;
	int url_config_table;
	
	//restiful
	char restiful_listen_ip[MAX_IP_LEN];
	unsigned short restiful_listen_port;
	unsigned int restiful_listen_timeout;//in seconds
}global_info_t;

#endif
