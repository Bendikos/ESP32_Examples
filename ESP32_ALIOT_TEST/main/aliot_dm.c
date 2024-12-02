#include "aliot_dm.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

static int s_aliot_id = 0;

ALIOT_DM_DES *aliot_malloc_des(ALIOT_DM_TYPE type)
{
    ALIOT_DM_DES *dm = (ALIOT_DM_DES *)malloc(sizeof(ALIOT_DM_DES));
    if (dm)
    {
        memset(dm, 0, sizeof(ALIOT_DM_DES));
        dm->dm_js = cJSON_CreateObject();
        char id[10];
        snprintf(id, sizeof(id), "%d", s_aliot_id++);
        cJSON_AddStringToObject(dm->dm_js, "id", id);
        switch (type)
        {
        case ALIOT_DM_POST:
            cJSON_AddStringToObject(dm->dm_js, "version", "1.0");
            cJSON_AddStringToObject(dm->dm_js, "method", "thing.event.property.post");
            cJSON_AddObjectToObject(dm->dm_js, "params");
            break;
        case ALIOT_DM_SET_ACK:
            cJSON_AddStringToObject(dm->dm_js, "version", "1.0");
            cJSON_AddObjectToObject(dm->dm_js, "data");
            break;
        }
        return dm;
    }
    return NULL;
}

void aliot_set_dm_int(ALIOT_DM_DES *dm, const char *name, int value)
{
    if (dm)
    {
        cJSON *params_js = cJSON_GetObjectItem(dm->dm_js, "params");
        if (params_js)
        {
            cJSON_AddNumberToObject(params_js, name, value);
        }
    }
}
void aliot_set_dm_property_ack(ALIOT_DM_DES *dm, int code,const char *message)
{
    if (dm)
    {
        cJSON_AddNumberToObject(dm->dm_js, "code", code);
        cJSON_AddStringToObject(dm->dm_js, "message", message);
    }
}
void aliot_dm_serialize(ALIOT_DM_DES *dm)
{
    if (dm)
    {
        if (dm->dm_js_str)
        {
            cJSON_free(dm->dm_js_str);
            dm->dm_js_str = NULL;
        }
        dm->dm_js_str = cJSON_PrintUnformatted(dm->dm_js);
    }
}
void aliot_dm_free(ALIOT_DM_DES *dm)
{
    if (dm)
    {
        if (dm->dm_js_str)
        {
            cJSON_free(dm->dm_js_str);
            dm->dm_js_str = NULL;
        }
        if (dm->dm_js)
        {
            cJSON_Delete(dm->dm_js);
            dm->dm_js = NULL;
        }
        free(dm);
    }
}