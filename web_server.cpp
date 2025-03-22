#include "config_manager.h"
#include "web_server.h"
#include "data.h"
#include "log_manager.h"
#include "led_controller.h"
#include "filesystem.h"
#include "SPIFFS.h"
#include "version.h"
#include "web_file_manager.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void checkAndUpdateClients() {
    bool stateChanged = false;
    String json = "{";

    for (auto& entry : runtimeData.values) {
        static std::map<String, String> lastValues;

        if (lastValues[entry.first] != entry.second) {
            if (json.length() > 1) json += ",";  
            json += "\"" + entry.first + "\":\"" + entry.second + "\"";  
            lastValues[entry.first] = entry.second;
            stateChanged = true;
        }
    }

    json += "}";

    if (stateChanged) {
        ws.textAll(json);
    }
}

// WebSocket events
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    IPAddress clientIP = client->remoteIP();
    String ipStr = clientIP.toString();

    if (type == WS_EVT_CONNECT) {
        logEvent("WebSocket client connected: " + ipStr);
    } else if (type == WS_EVT_DISCONNECT) {
        logEvent("WebSocket client disconnected: " + ipStr);
    }
}


String getWebSocketScript() {
    return "<script>"
           "let ws = new WebSocket('ws://' + window.location.host + '/ws');"
           "ws.onmessage = function(event) {"
           "   try {"
           "       let data = JSON.parse(event.data);"
           "       for (let key in data) {"
           "           let el = document.getElementById(key);"
           "           if (el) {"
           "               if (el.tagName.toUpperCase() === 'INPUT') {"
           "                   if (el.type.toUpperCase() === 'RADIO') {"
           "                     let radio = document.querySelector('input[name=\"' + key + '\"][value=\"' + data[key] + '\"]');"
           "                     if (radio) radio.checked = true;"
           "                   } else if (el.type.toUpperCase() === 'RANGE') {"
           "                     el.value = data[key];"
           "                   }"
           "               } else {"
           "                   el.innerText = data[key];"
           "               }"
           "           }"
           "       }"
           "   } catch (e) {"
           "       console.error('JSON parse error:', e.message);"
           "       document.getElementById('ws_status').innerText = 'Invalid data received';"
           "       document.getElementById('ws_status').style.color = 'orange';"
           "   }"
           "};"
           "ws.onopen = () => ws.send('init');"
           "ws.onclose = function() {"
           "   document.getElementById('ws_status').innerText = 'WebSocket offline';"
           "   document.getElementById('ws_status').style.color = 'red';"
           "   setTimeout(() => location.reload(), 5000);"
           "};"
           "</script>";
}

String sendDataScript() {
    return "<script>"
"    document.addEventListener(\"DOMContentLoaded\", function () {"
"        document.querySelectorAll(\".editable\").forEach(function (btn) {"
"            btn.addEventListener(\"click\", handleInputChange);"
"            btn.addEventListener(\"touchend\", handleInputChange);"
"        });"
"        document.querySelectorAll(\".senddata\").forEach(function (btn) {"
"            btn.addEventListener(\"click\", handleSendData);"
"        });"
"    });"
""
"    function handleInputChange(event) {"
"        event.preventDefault();"
"        let key = this.getAttribute('data');"
"        if (!key) {"
"            alert('Error! Editable element must have a data attribute.');"
"            return;"
"        }"
""
"        let value = null;"
""
"        if (this.tagName.toUpperCase() === 'INPUT') {"
"            if (this.type.toUpperCase() === 'RADIO' && this.checked) {"
"                value = this.value;"
"            } else if (this.type.toUpperCase() === 'RANGE') {"
"                value = this.value;"
"            }"
"        } else if (this.tagName.toUpperCase() === 'BUTTON') {"
// "            let currentValue = this.innerText.trim();"
"            let currentValue = document.getElementById(key).innerText.trim();"
"            value = prompt('Enter new value for ' + key + ':', currentValue);"
"        }"
""
"        if (value !== null) {"
"            fetch('/set?param=' + key + '&value=' + encodeURIComponent(value))"
"                .then(response => response.text())"
"                .catch(error => console.error('Error:', error));"
"        }"
"    }"
""
"    function handleSendData(event) {"
"        event.preventDefault();"
"        let key = this.getAttribute('data');"
"        if (!key) {"
"            alert('Error! SendData element must have a data attribute.');"
"            return;"
"        }"
""
"        let value = this.getAttribute('value');"
"        if (!value) {"
"            alert('Error! SendData element must have a value attribute.');"
"            return;"
"        }"
""
"        let confirm_text = this.getAttribute('confirm_text');"
"        if (confirm_text) {"
"           if (!confirm(confirm_text)) {"
"             return;"
"           }"
"        }"
""
"        fetch('/set?param=' + key + '&value=' + encodeURIComponent(value))"
"            .then(response => response.text())"
"            .catch(error => console.error('Error:', error));"
"    }"
"</script>\n";
}

void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", 
"<!DOCTYPE html>\n<html>\n<head>\n<meta charset='UTF-8'>\n"
"<title>" + String(config["host_name"]) + "</title>\n"
// "<link rel='stylesheet' href='/bootstrap.min.css'>"
"<link rel='stylesheet' href='/bootstrap-grid.min.css'>"
+ getWebSocketScript() +
" <meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>body { font-family: Arial, sans-serif; }</style>"
    "</head>\n<body>\n"
    "<div class='container'>\n"
    "<div style='float: right; font-size: 77%; text-align: right;'>"
    "v" + FIRMWARE_VERSION + " b" + BUILD_NUMBER + "<br/>"
    "Uptime: <span id='uptime'></span><br/>"
    "<span id='datetime'></span> <span id='timezone'></span><br/>"
    "</div>\n"
    "<span id='ws_status'></span>\n"
    "<h1>LED Controller</h1>\n"

    " <div class='row'>\n"
    "  <div class='col'>\n"
    "<h2>PIR Sensors</h2>\n"
    "<p>PIR Sensor 1: <span id='pir1'>Loading...</span></p>\n"
    "<p>PIR Sensor 2: <span id='pir2'>Loading...</span></p>\n"
    "  </div>\n" // col

    "  <div class='col'>\n"
    "<h2>LED</h2>\n"
    "<label><input type='radio' name='led_mode' data='led_mode' id='led_mode' class='editable' value='manual'> Manual</label>\n"
    "<label><input type='radio' name='led_mode' data='led_mode' id='led_mode' class='editable' value='pir'> PIR Sensor</label>\n"
    "<br>\n"
    "<label for='currentBrightness'>CurrentBrightness:</label>\n"
    "<input type='range' id='currentBrightness' name='currentBrightness' data='currentBrightness' class='editable' min='0' max='100' step='5'>\n"
    // "<p>CurrentBrightness: <span id='currentBrightness'>Loading...</span></p>\n"
    "<p>Delay: <span id='led_delay'>Loading...</span> s <button class='btn btn-light editable' data='led_delay'>✎</button></p>\n"
    "  </div>\n" // col
    " </div>\n" // row

    " <div class='row'>\n"
    "<h2>Logs</h2><pre id='log'>Loading...</pre>\n"
    " </div>\n" // row

    " <div class='row'>\n"

    "<h2>Config</h2>\n"
    "  <div class='col'>\n"
    "<h2>Wi-Fi</h2>\n"
    "SSID: <span id='WIFI_SSID'>Loading...</span> <button class='btn btn-light editable' data='WIFI_SSID'>✎</button><br>\n"
    "PASS: <span id='WIFI_PASS'>Loading...</span> <button class='btn btn-light editable' data='WIFI_PASS'>✎</button><br>\n"
    "<br>\n"
    "  </div>\n" // col

    "  <div class='col'>\n"
    "<h2>Network</h2>\n"
    "Hostname: <span id='host_name'>Loading...</span> <button class='btn btn-light editable' data='host_name'>✎</button><br>\n"
    "mDNS: <span id='mDNS'>Loading...</span><br>\n"
    "<br>\n"
    "NTP_SERVER1: <span id='NTP_SERVER1'>Loading...</span> <button class='btn btn-light editable' data='NTP_SERVER1'>✎</button><br>\n"
    "NTP_SERVER2: <span id='NTP_SERVER2'>Loading...</span> <button class='btn btn-light editable' data='NTP_SERVER2'>✎</button><br>\n"
    "  </div>\n" // col
    "<br>\n"
    "<br>\n"
    "<button class='btn btn-light senddata' data='reboot' value='1' confirm_text='Reboot?'>reboot</button><br>\n"

    " </div>\n" // row

    "</div>\n" // container
  + sendDataScript() +
      // "<script src='/bootstrap.bundle.min.js'></script>\n";
    "</body></html>"
  );
}

// Обработчик запроса WebSocket (отправка текущего состояния)
void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
    if (len > 0 && strncmp((char*)data, "init", len) == 0) {
        String json = "{";
        bool first = true;

        for (auto& entry : runtimeData.values) {
            if (!first) json += ",";
            json += "\"" + entry.first + "\":\"" + entry.second + "\"";
            first = false;
        }

        json += "}";
        client->text(json);
    }
}

// Регистрация статических файлов
void registerStaticFiles() {
    struct FileMapping {
        const char *url;
        const char *path;
        const char *contentType;
    };

    FileMapping files[] = {
        {"/bootstrap-grid.min.css", "/bootstrap-grid.min.css.gz", "text/css"},
        {"/bootstrap.min.css", "/bootstrap.min.css.gz", "text/css"},
        // {"/style.css", "/style.css.gz", "text/css"}
        {"/bootstrap.bundle.min.js", "/bootstrap.bundle.min.js.gz", "application/javascript"},
    };

    for (auto &file : files) {
        server.on(file.url, HTTP_GET, [file](AsyncWebServerRequest *request) {
            if (fileExists(file.path)) {
                AsyncWebServerResponse *response = request->beginResponse(SPIFFS, file.path, file.contentType);
                response->addHeader("Content-Encoding", "gzip");
                request->send(response);
            } else {
                request->send(200, file.contentType, "/* File is loading... */");
            }
        });
    }
}


// Инициализация веб-сервера и WebSocket
void startWebServer() {
    // initFileSystem();

    server.on("/", HTTP_GET, handleRoot);

    registerStaticFiles();

    // server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    //     // request->send(SPIFFS, "/bootstrap.min.css", "text/css");
    //     // request->send(SPIFFS, "/bootstrap.min.css.gz", "text/css", false);
    //     AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/bootstrap.min.css.gz", "text/css");
    //     response->addHeader("Content-Encoding", "gzip");
    //     request->send(response);
    // });

    // server.on("/bootstrap-grid.min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    //     // request->send(SPIFFS, "/bootstrap.min.css", "text/css");
    //     // request->send(SPIFFS, "/bootstrap.min.css.gz", "text/css", false);
    //     AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/bootstrap-grid.min.css.gz", "text/css");
    //     response->addHeader("Content-Encoding", "gzip");
    //     request->send(response);
    // });

    // server.on("/bootstrap.bundle.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    //     // request->send(SPIFFS, "/bootstrap.bundle.min.js", "application/javascript");
    //     // request->send(SPIFFS, "/bootstrap.bundle.min.js.gz", "application/javascript", false);
    //     AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/bootstrap.bundle.min.js.gz", "application/javascript");
    //     response->addHeader("Content-Encoding", "gzip");
    //     request->send(response);
    // });

    server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("param") && request->hasParam("value")) {
            String param = request->getParam("param")->value();
            String value = request->getParam("value")->value();
            
            if (param == "led_delay") {
                // config[param] = value.toInt();
                setConfigValueInt(param.c_str(), value.toInt());
            } else if (param == "led_mode" || param == "WIFI_SSID" || param == "WIFI_PASS" || param == "NTP_SERVER1" || param == "NTP_SERVER2") {
                setConfigValue(param.c_str(), value.c_str());
            } else if (param == "currentBrightness") {
                setLEDBrightness(value.toInt());    
            } else if (param == "host_name") {
                setConfigValue(param.c_str(), value.c_str());
                runtimeData.values["mDNS"] = String(config["host_name"]) + ".local";
            } else if (param == "reboot") {                	
                ESP.restart();
            } else {
                request->send(400, "text/plain", "Invalid param");
                return;
            }

            // if param found
            // runtimeData.values[param] = String(config[param]);
            if (config.containsKey(param)) {
                runtimeData.values[param] = String(config[param]);
            } else {
                runtimeData.values[param] = param;
            }
            request->send(200, "text/plain", "OK");
            logEvent("Web: config update " + param + "=" + value);
        } else {
            request->send(400, "text/plain", "Missing parameters");
        }
    });

    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                  void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_DATA) {
            handleWebSocketMessage(client, arg, data, len);
        }
        onWebSocketEvent(server, client, type, arg, data, len);
    });


    setupFileManager(server, String(config["host_name"]));

    server.addHandler(&ws);
    server.begin();
    logEvent("Web server started.");
}

// Функция для вызова в loop()
void handleWebServerUpdates() {
    ws.cleanupClients();
    checkAndUpdateClients();
}

