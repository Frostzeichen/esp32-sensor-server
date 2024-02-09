#include <Arduino.h>
#include <sensor.cpp.h>
#include <env.h>

void WiFiSetup (void *pvParameters);
void SensorSetup (void *pvParameters);
void ServerSetup (void *pvParameters);
void Test (void *pvParameters);

void setup () {
  Serial.begin(9600);
  delay(1000);

  xTaskCreatePinnedToCore(WiFiSetup, "wifisetup", 2500, NULL, 90, NULL, 0);
  xTaskCreatePinnedToCore(SensorSetup, "sensorsetup", 1000, NULL, 90, NULL, 1);
}

void loop () {

}

void WiFiSetup (void *pvParameters) {
  vTaskDelay((TickType_t) 1000 / portTICK_PERIOD_MS);
  ss.setup(SSID, PASS);
  xTaskCreatePinnedToCore(ServerSetup, "server", 2000, NULL, 90, NULL, 0);
  Serial.println(ss.address);
  vTaskDelay((TickType_t) 1000 / portTICK_PERIOD_MS);

  vTaskDelete(NULL);
}

String prepareJSON(); // prepared.
void ServerSetup (void *pvParameters) {
  WiFiServer server(80);
  server.begin();

  for (;;) {
    vTaskDelay((TickType_t) 50 / portTICK_PERIOD_MS);
    WiFiClient client = server.accept();

    if (client) {
      String request;
      while (client.connected()) {

          if (client.available()) {
              String line = client.readStringUntil('\r');

              if (line.startsWith("GET") && line.endsWith("HTTP/1.1") && !line.startsWith("GET /favicon.ico")) { // GET request decoder
                  request = line.substring(line.indexOf("GET"), line.lastIndexOf(" HTTP/1.1"));
              }

              if (line.length() == 1 && line[0] == '\n') { // response manager
                  if (request == "GET /") client.println(prepareJSON());
                  break;
              }
          }

          vTaskDelay((TickType_t) 5 / portTICK_PERIOD_MS);
      }

      while (client.available()) client.read();
      client.stop();
    }
  }
}

void SensorSetup (void *pvParameters) {
  pinMode(GPIO_NUM_36, INPUT);
  xTaskCreatePinnedToCore(Test, "test", 1000, NULL, 90, NULL, 1);

  vTaskDelete(NULL);
}

void Test (void *pvParameters) {
  for (;;) {
    Serial.println(analogRead(GPIO_NUM_36));
    vTaskDelay((TickType_t) 259 / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}

String prepareJSON() {
    String jsonResponse;
    String header = F("HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Connection: close\r\n"
                // "Refresh: 5\r\n"
                "\r\n");
    String body; // TODO: Make a transpiler for this.
        body += "{";
        body +=     "\"name\": ";
        body +=     "\"photoresistor\"";
        body += ", ";
        body +=     "\"format\": ";
        body +=     "\"analog\"";
        body += ", ";
        body +=     "\"reading\": ";
        body +=     analogRead(GPIO_NUM_36); // TODO: replace with proper pin.
        body += "}";
    jsonResponse.reserve(1024);
    jsonResponse += header;
    jsonResponse += body;
    return jsonResponse;
}

