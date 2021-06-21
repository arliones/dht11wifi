#include "WiFi.h"
#include <ESP8266WiFi.h>
#include <string.h>
#include <SimpleDHT.h>

DHT11WiFi::DHT11WiFi(const char * ssid, const char * key, IPAddress server_ip, int host_port, int sensor_pin)
    : _ssid(ssid), _key(key), _server_ip(server_ip), _server_port(host_port), _sensor_pin(sensor_pin)
{
    pinMode(D0, WAKEUP_PULLUP);
 //   _client.setTimeout(30E3); //30s
}

DHT11WiFi::~DHT11WiFi() {
    WiFi.disconnect();
}

bool DHT11WiFi::isNetworkPresent() {

    int numSsid = WiFi.scanNetworks();
    kick();
  
    if (numSsid <= 0) return false;
  
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
        if(WiFi.SSID(thisNet).compareTo(_ssid) == 0)
          return true;
    }

    return false;
}

int DHT11WiFi::sendMAC() {
    byte mac[6];
    WiFi.macAddress(mac);

    String macstr = "";
    if(mac[0] < 0x10) macstr += "0";
    macstr += String(mac[0],HEX);
    for(int i = 1; i < 6; i ++) {
        macstr += "-";
        if(mac[i] < 0x10) macstr += "0";
        macstr += String(mac[i],HEX);
    }
    kick();
    _client.println(macstr);
    return macstr.length() + 2; // 2 - "\r\n" from println
}

long DHT11WiFi::sendData(const String & s)
{
    kick();
    if(!isNetworkPresent()) return -1;
    kick();
    if(!joinNetwork()) return -2;
    kick();
    if(!connectToServer()) return -3;
    kick();
    int sent = 0;

    int mac_size = sendMAC();
    sent += mac_size;

    _client.print(s);
    sent += s.length();

    kick();
    disableWDT();
    long recvd = _client.parseInt();
    enableWDT();
    kick();

    return recvd;
}

void DHT11WiFi::enableWDT()
{
    kick();
    ESP.wdtEnable(10000);
    *((volatile uint32_t*) 0x60000900) |= 1; // Hardware WDT ON
}
void DHT11WiFi::disableWDT()
{
    kick();
    ESP.wdtDisable();
    *((volatile uint32_t*) 0x60000900) &= ~1; // Hardware WDT OFF
}
void DHT11WiFi::kick_the_dog()
{
    ESP.wdtFeed();
}

void DHT11WiFi::sleep(int microseconds)
{
    ESP.deepSleep(microseconds, WAKE_RF_DEFAULT);
}

void DHT11WiFi::reset()
{
    ESP.reset();
}

int DHT11WiFi::sample(byte * temperature, byte * humidity)
{
    SimpleDHT11 dht11;
    int err = SimpleDHTErrSuccess;
    kick();
    if ((err = dht11.read(_sensor_pin, temperature, humidity, NULL)) != SimpleDHTErrSuccess) {
        *temperature = -100;
        *humidity = -100;
        return err;
    }
    kick();

    return 0; // success
}

void print_wl_status(wl_status_t st) {
  switch(st) {
      case WL_CONNECTED:
          Serial.print("WL_CONNECTED");
          break;
      case WL_NO_SHIELD:
          Serial.print("WL_NO_SHIELD");
          break;
      case WL_IDLE_STATUS:
          Serial.print("WL_IDLE_STATUS");
          break;
      case WL_NO_SSID_AVAIL:
          Serial.print("WL_NO_SSID_AVAIL");
          break;
      case WL_SCAN_COMPLETED:
          Serial.print("WL_SCAN_COMPLETED");
          break;
      case WL_CONNECT_FAILED:
          Serial.print("WL_CONNECT_FAILED");
          break;
      case WL_CONNECTION_LOST:
          Serial.print("WL_CONNECTION_LOST");
          break;
      case WL_DISCONNECTED:
          Serial.print("WL_DISCONNECTED");
          break;
      case WL_WRONG_PASSWORD:
          Serial.print("WL_WRONG_PASSWORD");
          break;
  }
}

bool DHT11WiFi::joinNetwork()
{
    wl_status_t st = WiFi.begin(_ssid,_key);
//    wl_status_t st = WiFi.begin(_ssid);
    int tries = 60;
    while(st != WL_CONNECTED && tries--) {
        st = WiFi.status();
        delay(500);
        kick();
    }

    if(tries > 0) {
        Serial.println("\nJoined network.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("GW address: ");
        Serial.println(WiFi.gatewayIP());
        kick();
        return true;
    } else {
        Serial.print("\nWiFi not connected (cause = ");
        print_wl_status(st);
        Serial.println(").");
        kick();
        return false;
    }
}

bool DHT11WiFi::connectToServer()
{
    kick();
    if(_client.connect(_server_ip, _server_port) < 0)
    {
        kick();
        return false;
    }

//    if(!_client.verify(_fingerprint, _host))
//        return false;

    kick();
    Serial.println("Connected to server.");
    return true;
}



extern "C" {
//#include "user_interface.h"
#include "wpa2_enterprise.h"
//#include "c_types.h"
}

// SSID to connect to
char username[] = "WPA2E user";
char identity[] = "WPA2E id";
char password[] = "WPA2E passwd";

bool DHT11WiFi::joinNetworkWPA2E()
{
  WiFi.mode(WIFI_STA);
  kick();

  delay(1000);
  Serial.setDebugOutput(true);
  Serial.printf("SDK version: %s\n", system_get_sdk_version());
  Serial.printf("Free Heap: %4d\n",ESP.getFreeHeap());
  kick();
  
  // Setting ESP into STATION mode only (no AP mode or dual mode)
  wifi_set_opmode(STATION_MODE);
  kick();

  struct station_config wifi_config;

  memset(&wifi_config, 0, sizeof(wifi_config));
  strcpy((char*)wifi_config.ssid, _ssid);
  strcpy((char*)wifi_config.password, password);

  wifi_station_set_config(&wifi_config);
  byte target_esp_mac[6];
  WiFi.macAddress(target_esp_mac);
  wifi_set_macaddr(STATION_IF,target_esp_mac);
  kick();
  

  wifi_station_set_wpa2_enterprise_auth(1);
  kick();

  // Clean up to be sure no old data is still inside
  wifi_station_clear_cert_key();
  kick();
  wifi_station_clear_enterprise_ca_cert();
  kick();
  wifi_station_clear_enterprise_identity();
  kick();
  wifi_station_clear_enterprise_username();
  kick();
  wifi_station_clear_enterprise_password();
  kick();
  wifi_station_clear_enterprise_new_password();
  kick();
  
  wifi_station_set_enterprise_identity((uint8*)identity, strlen(identity));
  kick();
  wifi_station_set_enterprise_username((uint8*)username, strlen(username));
  kick();
  wifi_station_set_enterprise_password((uint8*)password, strlen((char*)password));
  kick();

  
  wifi_station_connect();
  kick();
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  kick();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}
