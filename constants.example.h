#define SERIAL_BAUD_RATE      115200
#define OFF                   HIGH
#define ON                    LOW

#define RC_RECEIVE_PIN        D1
#define RC_TRANSMIT_PIN       D2
#define ONEWIRE_PIN           D3
#define LED_PIN               D4
#define RELAY1_PIN            D5
#define RELAY2_PIN            D6
#define LIGHT_PIN             D7
#define LIGHT2_PIN            D8

#define WEATHER_UPDATE_MINS   60
#define TEMP_UPLOAD_MINS      5
#define NUM_OF_TEMP_SENSORS   4
#define DEBOUNCE_MS           100
#define LIGHT_DETECT_HOURS    72

#define WIFI_SSID             "***"
#define WIFI_PASSWORD         "***"
#define WIFI_SSID_2           "***"
#define WIFI_PASSWORD_2       "***"
#define WIFI_AP_SSID          "***"
#define WIFI_AP_PASSWORD      "***"

#define DHCP_CLIENTNAME       "***"

#define WEBSERVER_PORT        80
#define WEBSOCKET_PORT        81

#define WOL_TARGET_MAC        {0x***, 0x***, 0x***, 0x***, 0x***, 0x***}
#define WOL_UDP_PORT          9

#define WEATHER_URL           "***"
#define TEMP_UPLOAD_URL       "***"

#define RC_LAMPA_PROTOCOL     ***
#define RC_LAMPA_EVENT_LENGTH ***
#define RC_LAMPA_EVENT_ON     ***
#define RC_LAMPA_EVENT_OFF    ***

#define RC_KAPU_PROTOCOL      ***
#define RC_KAPU_EVENT_LENGTH  ***
#define RC_KAPU_EVENT_AUTO    ***
#define RC_KAPU_EVENT_GYALOG  ***

#define MQTT_BROKER           "***"
#define MQTT_BROKER_PORT      8883
#define MQTT_USER             "***"
#define MQTT_PASSWORD         "***"
