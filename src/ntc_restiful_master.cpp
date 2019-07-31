#include "ntc_restiful_master.h"
#include <assert.h>
#include <time.h>



#if TEST_MODE
const char* kafka_topic[__TOPIC_MAX]={"IPD_HTTP_IP_LOG", "IPD_SSL_IP_LOG", "IPD_DNS_IP_LOG", "NTC_CONN_RECORD_LOG", "NTC_COLLECT_RADIUS_LOG"};
#else
const char* kafka_topic[__TOPIC_MAX]={"IPD-HTTP-IP-LOG", "IPD-SSL-IP-LOG", "IPD-DNS-IP-LOG", "NTC-CONN-RECORD-LOG", "NTC-COLLECT-RADIUS-LOG"};
#endif

const char* g_conf_file = "./conf/ntc_restiful.conf";
global_info_t g_info;
const char *module_name = "ntc_restiful";

void free_data(void *data)
{
	free(data);
	data = NULL;
}


void init_parameters(const char* g_conf_file)
{
	int ret=0;
	//log
	MESA_load_profile_int_def(g_conf_file, "SYSTEM", "log_level", &g_info.log_level, 10);
	MESA_load_profile_string_def(g_conf_file, "SYSTEM", "log_path", g_info.run_log_path, MAX_PATH_LEN, "./log/ntc_restiful.log");
		
	//restiful
	MESA_load_profile_string_def(g_conf_file, "RESTIFUL", "restiful_listen_ip", g_info.restiful_listen_ip, MAX_IP_LEN, "127.0.0.1");
	short restiful_port_tmp = 0;
	MESA_load_profile_short_def(g_conf_file, "RESTIFUL", "restiful_listen_port", &restiful_port_tmp, 80);
	g_info.restiful_listen_port = (unsigned short)restiful_port_tmp;
	MESA_load_profile_uint_def(g_conf_file, "RESTIFUL", "restiful_listen_timeout", &g_info.restiful_listen_timeout, 120);

}
void read_maat_init_param(const char* profile, const char* section, struct maat_init_param* param)
{
	int tmp;
	MESA_load_profile_string_nodef(profile, section, "instance_name", param->instance_name, sizeof(param->instance_name));
	MESA_load_profile_string_nodef(profile, section, "table_info_path", param->table_info_path, sizeof(param->table_info_path));
	MESA_load_profile_string_def(profile, section, "redis_ip",  param->redis_ip, 32, "127.0.0.1");
	MESA_load_profile_int_def(profile, section, "redis_port",&(tmp), 6379);
	param->redis_port=(unsigned short)tmp;
	MESA_load_profile_int_def(profile, section, "redis_index",&(param->redis_index), 0);
	return;
}
Maat_feather_t maat_init(const struct maat_init_param* param, void* logger, int thread_num)
{
	//init maat_read_redis
	int scan_interval_ms=10;
	int effective_interval_ms=30;
	Maat_feather_t target=NULL;
	target=Maat_feather(thread_num, param->table_info_path, logger);
	Maat_set_feather_opt(target,MAAT_OPT_INSTANCE_NAME, param->instance_name, strlen(param->instance_name)+1);
/*
#if TEST_MODE
	Maat_set_feather_opt(target, MAAT_OPT_JSON_FILE_PATH, "conf/maat_test.json", strlen("conf/maat_test.json"));
#else
*/
	Maat_set_feather_opt(target, MAAT_OPT_REDIS_IP, param->redis_ip, strlen(param->redis_ip)+1);
	Maat_set_feather_opt(target, MAAT_OPT_REDIS_PORT, &(param->redis_port), sizeof(unsigned short ));
	Maat_set_feather_opt(target, MAAT_OPT_REDIS_INDEX, &(param->redis_index), sizeof(int));
//#endif
	Maat_set_feather_opt(target, MAAT_OPT_SCANDIR_INTERVAL_MS,&scan_interval_ms, sizeof(scan_interval_ms));
	Maat_set_feather_opt(target, MAAT_OPT_EFFECT_INVERVAL_MS,&effective_interval_ms, sizeof(effective_interval_ms));
	Maat_initiate_feather(target);
	return target;
}


void url_hit_compile_handler(struct evhttp_request *req, void *arg)
{
	cJSON* output = cJSON_CreateObject();
	//get uri
	int http_return_code = HTTP_OK;//200
	const char* uri;
	uri = evhttp_request_uri(req);
	char* decoded_uri;
	decoded_uri = evhttp_decode_uri(uri);
	//get request param
	char* url;
	struct evkeyvalq params;
	evhttp_parse_query(decoded_uri, &params);
	char* url = (char*)evhttp_find_header(&params, "url");
	if(NULL == url)
	{
		cJSON_AddStringToObject(output, "errmsg", "wrong format. eg.http://192.168.10.8:8080/v1/url_hit_compile?url=www.baidu.com");
		http_return_code = HTTP_BADREQUEST;//400
		goto OUTPUT;
	}
	
	//maat_scan
	Maat_rule_t result[MAX_CONFIG_NUM];
	memset((void*)result, 0, sizeof(Maat_rule_t));
	int found_pos=0;
	scan_status_t *mid = NULL;
	int hit_num = Maat_full_scan_string(g_info.maat_feather, g_info.url_config_table, CHARSET_NONE, url, strlen(url), result, &found_pos, MAX_CONFIG_NUM, mid, 1);
	if(hit_num > 0)
	{
		cJSON_AddNumberToObject(output, "hit_num", hit_num);
		cJSON* pz_id_array = cJSON_CreateArray();
		int i;
		for(i = 0; i < hit_num; i++)
		{
			cJSON_AddItemToArray(pz_id_array, cJSON_CreateNumber(result[i].config_id));
		}
		cJSON_AddItemToObject(output, "pz_id_set", pz_id_array);
	}

	//output
OUTPUT:
	evhttp_add_header(req->output_headers, "Server", "NTC_restiful");
	evhttp_add_header(req->output_headers, "Content-Type", "application/json");
	evhttp_add_header(req->output_headers, "Connection", "close");
	struct evbuffer *buf = evbuffer_new();
	char* output_str = cJSON_Print(output);
	evbuffer_add_printf(buf, "%s", output_str);
	evhttp_send_reply(req, http_return_code, "OK", buf);
	evbuffer_free(buf);
	cJSON_Delete(output);
	free(decoded_uri);
	free(output_str);
}


void url_label_handler(struct evhttp_request *req, void *arg)
{
	cJSON* output = cJSON_CreateObject();
	//get uri
	int http_return_code = HTTP_OK;//200
	const char* uri;
	uri = evhttp_request_uri(req);
	char* decoded_uri;
	decoded_uri = evhttp_decode_uri(uri);
	//get request param
	char* url;
	struct evkeyvalq params;
	evhttp_parse_query(decoded_uri, &params);
	char* url = (char*)evhttp_find_header(&params, "url");
	char* label = (char*)evhttp_find_header(&params, "label");
	
	if(NULL == url && NULL == label)
	{//todo: is label input Chinese?
		cJSON_AddStringToObject(output, "errmsg", "wrong format. eg.http://192.168.10.8:8080/v1/label?url=www.baidu.com or http://192.168.10.8:8080/v1/label?label=ÐÂÎÅ");
		http_return_code = HTTP_BADREQUEST;//400
		goto OUTPUT;
	}

	if(NULL != url)
	{
		
	}

	if(NULL != label)
	{
		
	}

	//output
OUTPUT:
	evhttp_add_header(req->output_headers, "Server", "NTC_restiful");
	evhttp_add_header(req->output_headers, "Content-Type", "application/json");
	evhttp_add_header(req->output_headers, "Connection", "close");
	struct evbuffer *buf = evbuffer_new();
	char* output_str = cJSON_Print(output);
	evbuffer_add_printf(buf, "%s", output_str);
	evhttp_send_reply(req, http_return_code, "OK", buf);
	evbuffer_free(buf);
	cJSON_Delete(output);
	free(decoded_uri);
	free(output_str);
}


int main(int argc, char **argv)
{
	memset(&g_info, 0, sizeof(global_info_t));	
	init_parameters(g_conf_file);
	//init log
	g_info.runtime_log =  MESA_create_runtime_log_handle(g_info.run_log_path, g_info.log_level);
	if(g_info.runtime_log== NULL)
	{
		printf("<%s>%d: MESA_create_runtime_log_handle failed ...\n", __FILE__, __LINE__);
		assert(0);
	}

	read_maat_init_param(g_conf_file, "MAAT", &(g_info.maat_param));
	g_info.maat_feather=maat_init( &(g_info.maat_param), g_info.runtime_log, 1);
	//todo:table name
	g_info.url_config_table = Maat_table_register(g_info.maat_feather, "URL_PZ");
	if(g_info.url_config_table < 0)
	{
		MESA_handle_runtime_log(g_info.runtime_log, RLOG_LV_FATAL, module_name, "<%s>%d: register maat_table failed", __FILE__, __LINE__);
		assert(0);
	}

	//init restiful
	event_init();
	struct evhttp* httpd;
	httpd = evhttp_start(g_info.restiful_listen_ip, g_info.restiful_listen_port);
	evhttp_set_timeout(httpd, g_info.restiful_listen_timeout);

	evhttp_set_cb(httpd, "/v1/url_hit_compile", url_hit_compile_handler, NULL);

	evhttp_set_cb(httpd, "/v1/label", url_label_handler, NULL);
	
	event_dispatch();
	evhttp_free(httpd);

	pthread_join(kafka_focus_domain_consumer_pid[0], NULL);
	
	return 0;
}
