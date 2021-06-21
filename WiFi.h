#ifndef __DHT11WiFi_h__
#define __DHT11WiFi_h__

//#include <WiFiClientSecure.h>
#include <WiFiClient.h>

class DHT11WiFi {
public:
    DHT11WiFi(const char * ssid, const char * key, IPAddress server_ip, int server_port, int sensor_pin);
    ~DHT11WiFi();

    bool isNetworkPresent();
    long sendData(const String & s);

    int sample(byte * temperature, byte * humidity);

    static void sleep(int microseconds);
    static void reset();

    void kick_the_dog();
    void kick() { this->kick_the_dog(); }

private:
    void enableWDT();
    void disableWDT();
    int sendMAC();
    bool joinNetwork();
    bool joinNetworkWPA2E();
    bool connectToServer();
//    int send(const byte * data, int len);

//    WiFiClientSecure _client;
    WiFiClient _client;

    const char * _ssid;
    const char * _key;
    IPAddress _server_ip;
    int _server_port;
    int _sensor_pin;
};




#endif //__DHT11WiFi_h__
