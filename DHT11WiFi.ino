#include "WiFi.h"

const bool DEBUG = true;

const char * ssid = "SSID";
const char * key  = "KEY";
IPAddress server_ip(192,168,0,1);
const uint16_t server_port = 1700;

const uint16_t data_len = 2;
const uint64_t sleep_period =  120E6; // 120 seconds in microseconds
int8_t data[data_len];

uint64_t wkup_time = 0;

#define max(a,b) (a>b)?a:b
#define min(a,b) (a<b)?a:b

void setup()
{
    wkup_time = micros();

    /******** INITIALIZE SERIAL ********/
    if(DEBUG) {
        Serial.begin(115200); //TODO: only enable serial if USB is connected
        Serial.setDebugOutput(false);
        Serial.println("\nDHT11WiFi");
    }

    application_routine();
}

void application_routine()
{
    DHT11WiFi dht11wifi(ssid, key, server_ip, server_port, D5);
    dht11wifi.kick();

    byte temp, hum;
    temp = hum = -127;
    if(dht11wifi.sample(&temp, &hum)) {
        if(DEBUG) Serial.println("Failed to read from sensor\n");
    }
    else if(DEBUG) {
        Serial.print("Sample OK: ");
        Serial.print((int)temp); Serial.print(" *C, ");
        Serial.print((int)hum); Serial.println(" H");
        dht11wifi.kick();
    
        String message = "{\"temp\":" + String(temp) + ",\"hum\":" + String(hum) + "}";
        if(DEBUG) Serial.println(message);
        dht11wifi.kick();
    
        int sent = dht11wifi.sendData(message);
        dht11wifi.kick();
        if(DEBUG) {
            Serial.print("Sent (bytes): ");
            Serial.println(sent);
        }
    }

    uint64_t time_to_sleep = sleep_period - (micros() - wkup_time);
    if (time_to_sleep > sleep_period) {
        if(DEBUG) Serial.println("Resetting...\r\n");
        dht11wifi.kick();
        DHT11WiFi::reset();
    }
    else
    {
      if(DEBUG) {
          Serial.print("Sleep Tight (t=");
          Serial.print(time_to_sleep/1E6);
          Serial.println("s).");
          Serial.flush();
      }
      DHT11WiFi::sleep(time_to_sleep);
    }

    Serial.println("I have insomnia..."); // program shouldn't reach this point
}
void loop() {
    Serial.println("I'm now a zombie..."); // program shouldn't reach this point
}
