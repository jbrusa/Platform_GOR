//
// Created by Jordi Brusa on 30.05.17.
//

#ifndef PLATFORM_GOR_GOR_WEBSERVER_HPP
#define PLATFORM_GOR_GOR_WEBSERVER_HPP

#include "GOR_Platform.hpp"

#define HTTP_PORT       80

#define TAIL_SIZE       1000

WiFiServer server(HTTP_PORT);

extern String GetInfoESP();
extern String GetInfoWifi();



unsigned char h2int(char c)
{
    if (c >= '0' && c <='9')
        return((unsigned char)c - '0');

    if (c >= 'a' && c <='f')
        return((unsigned char)c - 'a' + 10);

    if (c >= 'A' && c <='F')
        return((unsigned char)c - 'A' + 10);

    return(0);
}


String urldecode(String str)
{

    String encodedString="";
    char c;
    char code0;
    char code1;
    for (int i =0; i < str.length(); i++){
        c=str.charAt(i);
        if (c == '+'){
            encodedString+=' ';
        }else if (c == '%') {
            code0=str.charAt(++i);
            code1=str.charAt(++i);
            c = (h2int(code0) << 4) | h2int(code1);
            encodedString+=c;
        } else {
            encodedString += c;
        }
    }

    return encodedString;
}


class GOR_WebServer {
public:



    void begin() {
        server.begin();
    }

    void loop() {
        WiFiClient client = server.available();

        if (!client)
            return;

        String responseFile = "";
        String response = "";
        String mimeType = "";
        String currentLine = "";

        uint32_t t = millis();

        while (client.connected()) {
            if(millis() - t > 2000)
                break;

            if (client.available()) {
                char c = client.read();

                if (c == '\n') {

                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0) {

                        // the content of the HTTP response follows the header:
                        if(response != "") {
                            client.println("HTTP/1.1 200 OK");
                            client.println(String("Content-type:") + mimeType);
                            client.println();
                            client.print(response);
                            client.println();

                            if(response == "Restarting") {
                                ESP.restart();
                            }
                        } else if(responseFile != "") {
                            if (SD.exists(responseFile)) {
                                File f = SD.open(responseFile);
                                client.println("HTTP/1.1 200 OK");
                                client.println("Content-type:" + mimeType);
                                client.println();
                                while (f.available()) {
                                    client.print((char) f.read());
                                }
                                client.println();
                            } else {
                                client.println("HTTP/1.1 OK");
                                client.println("Content-type:text/html");
                                client.println();
                                client.print(ESP.getFreeHeap());
                                client.print("<br/>");
                                client.print(responseFile);
                                client.println();
                            }
                        }

                        break;
                    } else {
                        if (currentLine.startsWith("GET /")) {
                            int end = currentLine.indexOf(' ', 4);

                            String query = currentLine.substring(4, end);
                            if (query == "/config") {
                                mimeType = "text/txt";
                                responseFile = CONFIG_FILE;
                            } else if (query == "/restart") {
                                response = "Restarting";
                            } else if (query.startsWith("/save_config?")) {
                                query = query.substring(20);
                                query = urldecode(query);
                                mimeType = "text/txt";

                                SD.rename(CONFIG_FILE, CONFIG_FILE_BAK);
                                File f = SD.open(CONFIG_FILE, FILE_WRITE);
                                f.print(query);
                                f.close();

                                mimeType = "text/html";
                                responseFile = "/wwwroot/index.html";
                            } else if (query == "/log") {
                                mimeType = "text/txt";
                                responseFile = LOG_FILE;
                            } else if (query == "/info-esp") {
                                mimeType = "application/json";
                                response = "{";
                                response += GetInfoESP();
                                response += "}";
                            } else if (query == "/info-wifi") {
                                mimeType = "application/json";
                                response = "{";
                                response += GetInfoWifi();
                                response += "}";
                            } else if (query == "/") {
                                mimeType = "text/html";
                                responseFile = "/wwwroot/index.html";
                            } else {
                                uint16_t lastDot = (uint16_t) query.lastIndexOf('.');
                                String ext = query.substring(lastDot + 1);

                                if (ext == "html" || ext == "htm") mimeType = "text/html";
                                if (ext == "png") mimeType = "image/png";
                                if (ext == "css") mimeType = "text/css";
                                if (ext == "js") mimeType = "application/javascript";

                                responseFile = String("/wwwroot/") + query;
                            }
                        }

                        currentLine = "";
                    }
                } else if (c != '\r') {  // if you got anything else but a carriage return character,
                    currentLine += c;      // add it to the end of the currentLine
                }
            }
        }

        client.flush();
        client.stop();

    }
};

#endif //PLATFORM_GOR_GOR_WEBSERVER_HPP
