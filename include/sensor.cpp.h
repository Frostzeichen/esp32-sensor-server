#ifndef SENSOR_CPP_H
#define SENSOR_CPP_H

#include <HardwareSerial.h>
#include <esp32-hal-gpio.h>
#include <WiFi.h>

// WiFi setup class which includes methods for driving the WiFi hardware.
class ssWifi {
    protected:
        // Sensor struct for each individual sensor connected to the controller.
        struct sensor {
            int id; // Controller-specific identifier.
            char model[30]; // Sensor model.
            char format[7]; // Analog | Digital.
            float reading = 0.00;

            // id: int,
            // model: string,
            // format: analog | digital,
            // reading: any,
            // time: string
        };


        sensor analog[5];
        sensor digital[5];

        String prepareJSON() {
            String jsonResponse;
            String header = F("HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/json\r\n"
                        "Connection: close\r\n"
                        "Refresh: 5\r\n"
                        "\r\n");
            String body = "{"; // TODO: Make a transpiler for this.
            body += "\"asd\": \"123\"";
            // for (int i = 0; i < sizeof(analog); i++) { // Returns a compiler warning.
            //     body += "{";
            //     body +=     "name: ";
            //     body +=     analog[i].model;
            //     body += ",";
            //     body +=     "format: ";
            //     body +=     analog[i].format;
            //     body += ",";
            //     body +=     "reading: ";
            //     body +=     analog[i].reading;
            //     body += ",";
            //     body += "}";
            // }
            // for (int i = 0; i < sizeof(digital); i++) { // Returns a compiler warning.
            //     body += "{";
            //     body +=     "name: ";
            //     body +=     digital[i].model;
            //     body += ",";
            //     body +=     "format: ";
            //     body +=     digital[i].format;
            //     body += ",";
            //     body +=     "reading: ";
            //     body +=     digital[i].reading;
            //     body += ",";
            //     body += "}";
            // }
            body += "}";
            jsonResponse.reserve(1024);
            jsonResponse += header;
            jsonResponse += body;
            return jsonResponse;
        }

    public:
        // Contains the current IP address.
        String address;

        /** 
         * Initiates WiFi mode.
         * 
         * **SSID** refers to name of WiFi access point.
         * 
         * **PASSWORD** for access point password. Leave blank string (aka a "") if no password.
         * 
         * **BAUDRATE** if you want the IP address to show up on the serial monitor.
        **/
        void setup (String SSID, String PASSWORD, int BAUDRATE = 0) {
            WiFi.mode(WIFI_STA);
            WiFi.begin(SSID, PASSWORD);

            while (WiFi.status() != WL_CONNECTED) vTaskDelay((TickType_t) 50 / portTICK_PERIOD_MS);
            address = WiFi.localIP().toString();
            if (BAUDRATE > 0) {
                Serial.println(address);
            }
        }

        void server () {
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
};

ssWifi ss;

#endif