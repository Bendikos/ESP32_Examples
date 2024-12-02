#ifndef _ALIOT_DM_H__
#define _ALIOT_DM_H__
#include "cJSON.h"

typedef enum
{
    ALIOT_DM_POST,
    ALIOT_DM_SET_ACK,
} ALIOT_DM_TYPE;
typedef struct
{
    cJSON *dm_js;
    char *dm_js_str;
} ALIOT_DM_DES;

ALIOT_DM_DES *aliot_malloc_des(ALIOT_DM_TYPE type);
void aliot_set_dm_int(ALIOT_DM_DES *dm, const char *name, int value);
void aliot_dm_serialize(ALIOT_DM_DES *dm);
void aliot_dm_free(ALIOT_DM_DES *dm);
void aliot_set_dm_property_ack(ALIOT_DM_DES *dm, int code, const char *message);

#endif