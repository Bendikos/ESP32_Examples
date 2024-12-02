#include "mqtt_aliot.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "mbedtls/md5.h"
#include "mbedtls/md.h"
#include "esp_wifi.h"
#include "stdio.h"
#include "mqtt_aliot.h"
#include "aliot_dm.h"
#include "driver/gpio.h"

static esp_mqtt_client_handle_t mqtt_handle = NULL;
static char is_mqtt_connect = 0;
static const char *TAG = "aliot mqtt";

extern int aiotMqttSign(const char *productKey, const char *deviceName, const char *deviceSecret,
                        char clientId[150], char username[65], char password[65]);

void mqtt_event_callback(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t data = (esp_mqtt_event_handle_t)event_data;
    switch (event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "aliot mqtt connected");
        is_mqtt_connect = 1;
        esp_mqtt_client_subscribe_single(mqtt_handle, AliyunSubscribeTopic_user_get, 1);
        esp_mqtt_client_subscribe_single(mqtt_handle, AliyunSubscribeTopic_user_reply, 1);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "mqtt disconnected");
        is_mqtt_connect = 0;
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "mqtt published ack");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "mqtt subscribed ack");
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "topic->%.*s", data->topic_len, data->topic);
        ESP_LOGI(TAG, "payload->%.*s", data->data_len, data->data);
        if (strstr(data->topic, "/property/set"))
        {
            cJSON *property_js = cJSON_Parse(data->data);
            cJSON *params_js = cJSON_GetObjectItem(property_js, "params");
            if (params_js)
            {
                cJSON *light_js = cJSON_GetObjectItem(params_js, "LightSwitch");
                if (light_js)
                {
                    int value = cJSON_GetNumberValue(light_js);
                    gpio_set_level(GPIO_NUM_10, value);
                }
                aliot_property_ack(200, "success");
            }
        }
        break;
    default:
        break;
    }
}

const char *server_cert =
    "-----BEGIN CERTIFICATE-----\n"
    "MIID3zCCAsegAwIBAgISfiX6mTa5RMUTGSC3rQhnestIMA0GCSqGSIb3DQEBCwUA\n"
    "MHcxCzAJBgNVBAYTAkNOMREwDwYDVQQIDAhaaGVqaWFuZzERMA8GA1UEBwwISGFu\n"
    "Z3pob3UxEzARBgNVBAoMCkFsaXl1biBJb1QxEDAOBgNVBAsMB1Jvb3QgQ0ExGzAZ\n"
    "BgNVBAMMEkFsaXl1biBJb1QgUm9vdCBDQTAgFw0yMzA3MDQwNjM2NThaGA8yMDUz\n"
    "MDcwNDA2MzY1OFowdzELMAkGA1UEBhMCQ04xETAPBgNVBAgMCFpoZWppYW5nMREw\n"
    "DwYDVQQHDAhIYW5nemhvdTETMBEGA1UECgwKQWxpeXVuIElvVDEQMA4GA1UECwwH\n"
    "Um9vdCBDQTEbMBkGA1UEAwwSQWxpeXVuIElvVCBSb290IENBMIIBIjANBgkqhkiG\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAoK//6vc2oXhnvJD7BVhj6grj7PMlN2N4iNH4\n"
    "GBmLmMdkF1z9eQLjksYc4Zid/FX67ypWFtdycOei5ec0X00m53Gvy4zLGBo2uKgi\n"
    "T9IxMudmt95bORZbaph4VK82gPNU4ewbiI1q2loRZEHRdyPORTPpvNLHu8DrYBnY\n"
    "Vg5feEYLLyhxg5M1UTrT/30RggHpaa0BYIPxwsKyylQ1OskOsyZQeOyPe8t8r2D4\n"
    "RBpUGc5ix4j537HYTKSyK3Hv57R7w1NzKtXoOioDOm+YySsz9sTLFajZkUcQci4X\n"
    "aedyEeguDLAIUKiYicJhRCZWljVlZActorTgjCY4zRajodThrQIDAQABo2MwYTAO\n"
    "BgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUkWHoKi2h\n"
    "DlS1/rYpcT/Ue+aKhP8wHwYDVR0jBBgwFoAUkWHoKi2hDlS1/rYpcT/Ue+aKhP8w\n"
    "DQYJKoZIhvcNAQELBQADggEBADrrLcBY7gDXN8/0KHvPbGwMrEAJcnF9z4MBxRvt\n"
    "rEoRxhlvRZzPi7w/868xbipwwnksZsn0QNIiAZ6XzbwvIFG01ONJET+OzDy6ZqUb\n"
    "YmJI09EOe9/Hst8Fac2D14Oyw0+6KTqZW7WWrP2TAgv8/Uox2S05pCWNfJpRZxOv\n"
    "Lr4DZmnXBJCMNMY/X7xpcjylq+uCj118PBobfH9Oo+iAJ4YyjOLmX3bflKIn1Oat\n"
    "vdJBtXCj3phpfuf56VwKxoxEVR818GqPAHnz9oVvye4sQqBp/2ynrKFxZKUaJtk0\n"
    "7UeVbtecwnQTrlcpWM7ACQC0OO0M9+uNjpKIbksv1s11xu0=\n"
    "-----END CERTIFICATE-----";

void mqtt_start(void)
{
    char clientId[150] = {0};
    char username[65] = {0};
    char password[65] = {0};
    aiotMqttSign(ALIOT_PRODUCTKEY, ALIOT_DEVICENAME, ALIOT_DEVICESECRET, clientId, username, password);
    esp_mqtt_client_config_t mqtt_cfg = {

        .broker.verification.certificate = server_cert,
        .broker.address.transport = MQTT_TRANSPORT_OVER_SSL,
        .broker.address.hostname = ALIOT_MQTT_URL,
        .broker.address.port = 8883,
        .credentials.client_id = clientId,
        .credentials.username = username,
        .credentials.authentication.password = password,
    };
    mqtt_handle = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_handle, ESP_EVENT_ANY_ID, mqtt_event_callback, NULL);
    esp_mqtt_client_start(mqtt_handle);
}

void aliot_post_property_int(const char *name, int value)
{
    ALIOT_DM_DES *dm = aliot_malloc_des(ALIOT_DM_POST);
    aliot_set_dm_int(dm, name, value);
    aliot_dm_serialize(dm);
    char topic[128];
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/event/property/post", ALIOT_PRODUCTKEY, ALIOT_DEVICENAME);
    ESP_LOGI(TAG, "publish payload:%s", dm->dm_js_str);
    esp_mqtt_client_publish(mqtt_handle, topic, dm->dm_js_str, strlen(dm->dm_js_str), 1, 0);
    aliot_dm_free(dm);
}

void aliot_property_ack(int code,const char *message)
{
    ALIOT_DM_DES *dm = aliot_malloc_des(ALIOT_DM_SET_ACK);
    aliot_set_dm_property_ack(dm, code, message);
    aliot_dm_serialize(dm);
    char topic[128];
    snprintf(topic, sizeof(topic), "/sys/%s/%s/thing/service/property/set_reply", ALIOT_PRODUCTKEY, ALIOT_DEVICENAME);
    ESP_LOGI(TAG, "publish payload:%s", dm->dm_js_str);
    esp_mqtt_client_publish(mqtt_handle, topic, dm->dm_js_str, strlen(dm->dm_js_str), 1, 0);
    aliot_dm_free(dm);
}

char is_mqtt_connected(void)
{
    return is_mqtt_connect;
}
