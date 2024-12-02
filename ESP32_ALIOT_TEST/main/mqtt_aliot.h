#ifndef _MQTT_ALIOT_H__
#define _MQTT_ALIOT_H__



#define ALIOT_MQTT_URL "iot-06z009vbywh7j17.mqtt.iothub.aliyuncs.com"
#define AliyunPublishTopic_user_update "/"ALIOT_PRODUCTKEY"/"ALIOT_DEVICENAME"/user/update"
#define AliyunSubscribeTopic_user_get "/"ALIOT_PRODUCTKEY"/"ALIOT_DEVICENAME"/user/get"
#define AliyunSubscribeTopic_user_reply "/sys/"ALIOT_PRODUCTKEY"/"ALIOT_DEVICENAME"/thing/event/property/post_reply"

void mqtt_start(void);

void aliot_post_property_int(const char *name, int value);

void aliot_property_ack(int code,const char *message);

char is_mqtt_connected(void);

#endif