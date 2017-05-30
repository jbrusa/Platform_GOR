//
// Created by Jordi Brusa on 30.05.17.
//

#ifndef PLATFORM_GOR_GOR_WEBSERVER_HPP
#define PLATFORM_GOR_GOR_WEBSERVER_HPP

#define HTTP_PORT       80

WiFiServer server(80);

class GOR_WebServer {
public:


    void begin() {
        server.begin();
    }

    void loop() {
        WiFiClient client = server.available();
        String responseFile = "";

        if (client) {
            String currentLine = "";
            while (client.connected()) {
                if (client.available()) {
                    char c = client.read();
                    Serial.write(c);
                    if (c == '\n') {

                        // if the current line is blank, you got two newline characters in a row.
                        // that's the end of the client HTTP request, so send a response:
                        if (currentLine.length() == 0) {

                            Serial.println(String("Response file : ") + responseFile);
                            // the content of the HTTP response follows the header:
                            if (SD.exists(String("/wwwroot/") + responseFile)) {
                                File f = SD.open(String("/wwwroot/") + responseFile);
                                client.println("HTTP/1.1 200 OK");
                                client.println("Content-type:text/html");
                                client.println();
                                while (f.available()) {
                                    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                                    // and a content-type so the client knows what's coming, then a blank line:

                                    client.print((char) f.read());
                                }
                                client.println();
                            } else {
                                client.println("HTTP/1.1 404 Not Found");
                                client.println();
                            }

                            break;
                        } else {    // if you got a newline, then clear currentLine:
                            currentLine = "";
                        }
                    } else if (c != '\r') {  // if you got anything else but a carriage return character,
                        currentLine += c;      // add it to the end of the currentLine
                    }

                    // Check to see if the client request was "GET /H" or "GET /L":
                    if (currentLine.startsWith("GET /")) {
                        int end = currentLine.indexOf(' ', 4);
                        responseFile = currentLine.substring(4, end - 4);

                        if (responseFile == "/")
                            responseFile = "index.html";
                    }
                }
            }
            client.stop();
        }
    }
};

#endif //PLATFORM_GOR_GOR_WEBSERVER_HPP
