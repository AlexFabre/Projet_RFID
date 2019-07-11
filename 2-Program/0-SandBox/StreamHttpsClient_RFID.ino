/**
   StreamHTTPClient.ino

    Created on: 24.05.2015

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;

enum status_value_json {
  JSON_ERROR_LEN_BUFF,
  JSON_ERROR_KEY_INDEX,
  JSON_ERROR_VALUE_EMPTY,
  JSON_MISS_END_CHAR,
  JSON_FORMAT_OK
};

enum type_value_json {
  NONE,
  TAB,
  MAP,
  STRING,
  INT,
  FLOAT
};

struct parse_json {
  enum status_value_json status;
  enum type_value_json type; 
}parse_json;

String keyWord[] = {"recycler", "recyclage"};

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  Serial.println(sizeof(keyWord)/sizeof(String));
Serial.println(keyWord[0]);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Honor 10", "rienrien");

}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    Serial.println(get_packaging_info("3029330003533"));
  }

  Serial.println("Wait 10s before the next round...");
  delay(10000);
}

String get_packaging_info(String product_code){
    String packaging;
    packaging = "NONE";
  
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);

    Serial.print("[HTTPS] begin...\n");

    // configure server and url
    const uint8_t fingerprint[20] = {0x8D,0x4E,0x3C,0x0D,0xF8,0xE7,0x93,0x28,0x44,0x0B,0x63,0x62,0x96,0xE5,0x6C,0x2F,0x46,0x33,0x01,0x00};

    client->setFingerprint(fingerprint);
    
    HTTPClient https;

    if (https.begin(*client, "https://fr.openfoodfacts.org/api/v0/produit/" + product_code + ".json")) {
      packaging = "NOT_FOUND";
      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK) {
          // get lenght of document (is -1 when Server sends no Content-Length header)
          int len = https.getSize();

          Serial.print("Length: ");
          Serial.println(len);

          // create buffer for read
          static uint8_t buff[128] = { 0 };
          String data = "";
          int inc = 0;

          // read all data from server
          while (https.connected() && (len > 0 || len == -1)) {
            // get available data size
            size_t size = client->available();

            if (size) {
              // read up to 128 byte
              int c = client->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

              // write it to Serial
              for(int i = 0; i < c ; i++){
                data += char(buff[i]);
                Serial.print(char(buff[i]));
              }
              Serial.println();

              if(keyAvailable(data, "packaging")){
                String package = "NONE";
                struct parse_json valueJson = valueAvailable(data, "packaging", &package);

                if(valueJson.status == JSON_FORMAT_OK){
                  packaging = package;
                  len = -2;
                }else if(valueJson.status != JSON_MISS_END_CHAR){
                  Serial.println("MISS");
                  len = -2;
                }
              }else if(inc > 5){
                data = "";
                inc = 0;
              }

              if (len > 0) {
                len -= c;
              }
              inc++;
            }
            delay(1);
          }

          Serial.println();
          Serial.print("[HTTPS] connection closed or file end.\n");

        }
      } else {
        packaging = "ERROR_HTTP_CODE";
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      packaging = "ERROR_CONNECT_URL";
      Serial.printf("Unable to connect\n");
    }

      return packaging;
}

bool is_recyclable(String packaging){
  int nb_item = sizeof(keyWord)/sizeof(String);
  int recycle = 0;
  
  for(int i = 0 ; i < nb_item; i++){
    if(packaging.indexOf(keyWord[i])){
      recycle++;
    }
  }

  if(recycle > 0){
    return true;
  }

  return false;
}

bool keyAvailable(String buffer, String key){
  if(buffer.indexOf("\"" + key + "\"") > 0){
    return true;
  }
  return false;
}

struct parse_json valueAvailable(String buffer, String key, String *value){
  struct parse_json valueJson;
  String keyJson = "\"" + key + "\"";
  Serial.println(keyJson);
  int keyIndex = buffer.indexOf("\"" + key + "\"");

  valueJson.type = NONE;
  valueJson.status = JSON_ERROR_VALUE_EMPTY;
      
  if(keyIndex > 0){
    int endIndexValue;
    int indexValue = keyIndex + key.length() + 3;

    if(indexValue > (buffer.length() - 1)){
      valueJson.status = JSON_ERROR_LEN_BUFF;
    }
    
    if(buffer[indexValue] == '['){
      valueJson.type = TAB;
    }else if(buffer[indexValue] == ','){
      valueJson.type = NONE;
      valueJson.status = JSON_ERROR_VALUE_EMPTY;
    }else if(buffer[indexValue] == '{'){
      valueJson.type = MAP;
    }else if(buffer[indexValue] == '"'){
      endIndexValue = buffer.indexOf('"', indexValue + 1);
      valueJson.type = STRING;

      if(endIndexValue == -1){
        valueJson.status = JSON_MISS_END_CHAR;
      }else{
        valueJson.status = JSON_FORMAT_OK;
        *value = buffer.substring(indexValue + 1, endIndexValue);
      }
    }else if((buffer[indexValue] >= '0') && (buffer[indexValue] <= '9')){
      endIndexValue = buffer.indexOf(',', indexValue + 1);
      valueJson.type = STRING;

      if(endIndexValue == -1){
        endIndexValue = buffer.indexOf('}', indexValue + 1);
        if(endIndexValue == -1){
          valueJson.status = JSON_MISS_END_CHAR;
        }
      }
      
      if(endIndexValue != -1){
        valueJson.status = JSON_FORMAT_OK;
        *value = buffer.substring(indexValue + 1, endIndexValue);
        
        if(value->indexOf('.')){
          valueJson.type = FLOAT;
        }else{
          valueJson.type = INT;
        }
      }
    }
  }
  
  return valueJson;
}
