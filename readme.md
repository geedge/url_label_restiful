# 安装步骤

## 运行环境

| 操作系统 | Centos7.4             |
| -------- | --------------------- |
| 内存     | 128G                  |
| CPU      | Inter E5 2630 V3      |
| 网卡     | 双万兆网卡Intel 82599 |
| 硬盘     | 500G                  |

## 初始化

Redis动态配置库初始化

时间同步		主机时区和时间必须一致

主机名			不重复，不能有下划线

系统编码		UTF-8

系统参数		句柄数、内存参数、TCP参数优化

网卡名			业务口网卡名一致；管理口网卡名一致

## 依赖库

librdkafka.so.1  

libhiredis_vip.so.0.4  

libmaatframe.so.2.5	

libMESA_handle_logger.so 	

libMESA_htable.so		

libMESA_prof_load.so		

libMESA_field_stat2.so		

librulescan.so

openssl	1.0.1e-16以上版本，Python			2.6版本或以上

## 配置文件

全部配置都在`web_focus/bin/conf/web_focus.conf`文件中。

系统参数：

```
[SYSTEM]
thread_num=1 #线程数
domain_min_voter_num=1 #domain\sip对的不同client ip的数量大于等于此值时认为有效
dyn_config_expire_time=300 #下发的动态配置超时时间，单位秒
raidus_config_expire_time=86400 #下发的Radius（Subscribe ID)配置的超时时间，单位秒
```

IP复用业务配置:

```
[IP_REUSE]
ir_pick_cli_num=100	#为IP Reuse业务每次下发的客户端IP数量
tcp_record_sample_rate=0.05  #tcp日志的采样比例，取值0.001~1.0
```

MAAT 配置： 

```
[NTC_MAAT] #只读
instance_name=ntc
table_info_path=./conf/ntc_table_info.conf
redis_ip = 10.4.34.4
redis_port = 6380
redis_index = 2

[PROXY_MAAT] #只读
instance_name=proxy
table_info_path=./conf/proxy_table_info.conf
redis_ip = 10.4.34.4
redis_port = 6380
redis_index = 4

[OUTPUT_MAAT] #写入
instance_name=wbf
table_info_path=./conf/output_maat_table_info.conf
redis_ip = 10.4.20.151
redis_port = 6379
redis_index = 0
```

调试选项：

```
[DEBUG]
#no more than 64, seperate with ","
trace_keys=www.mesalab.cn, ietf.org, askar_legalkz.in
```



## 安装步骤

编译

`unzip web_focus.zip`

`cd web_focus/src`

 `make clean;make`

运行

`cd web_focus/bin/`

 `./web_focus` or ` ./wf_r2`

状态检查

`watch -d cat ipd_fs.stat`

## 常见问题

1. 链接错误

现象 `cannot find -lsqlite3`

解决方法：`ln -s  /usr/lib64/libsqlite3.so.0 /usr/lib64/libsqlite3.so`

