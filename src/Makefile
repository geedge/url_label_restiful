CCC=g++

VPATH=./cJSON

INC_PATH=-I./include -I./include/hiredis-vip -I./include/librdkafka
CFLAGS=-Wall -g -fPIC -std=c++11 $(INC_PATH)
#CFLAGS+=-DREDIS_SINGLE_MODE
#CFLAGS+=-DFRAG_INDEX_VOIP
#CFLAGS+=-DFRAG_INDEX_AV

LDFLAGS = 
LIBS = -lhiredis_vip -lrdkafka -lmaatframe -lMESA_handle_logger -lMESA_htable -lMESA_prof_load -lpthread -lMESA_field_stat2 -lrulescan -lrt -lpcre -lsqlite3 -levent
#LIBS += ./lib/libmaatframe.a
#LIBS += -lmaatframe
#LIBS += ./lib/libhiredis_vip.a
#LIBS += -lhiredis_vip
#LIBS += -lmaatframe -lrulescan
#LIBS += -lrulescan
#LIBS += ./lib/libmaatframe.a
#LIBS += -lrdkafka
#LIBS += ./lib/librdkafka.a
#LIBS +=./lib/libhiredis_vip.a 

OBJS = cJSON.o ip_discovery_master.o web_focus_restiful.o
DEPS = $(OBJS:.o=.d)

TARGET_EXE=web_focus

ALL:$(TARGET_EXE)

$(TARGET_EXE):$(OBJS)
	$(CCC) $(LDFLAGS) $^ -o $@ $(LIBS)
	cp $@ ../bin/

.c.o:
	$(CCC) $(CFLAGS) -c $<

.cpp.o:
	$(CCC) $(CFLAGS) -c $<

-include $(DEPS)

clean:
	rm -rf $(OBJS) $(DEPS) $(TARGET_EXE)
