#include "web_file_manager.h"
#include "filesystem.h"
#include "log_manager.h"

String getPageScript() {
    return "<script>\n"
           "function deleteFile(filename) {\n"
           "console.log('Deleting:', filename);\n"
           "fetch('/delete', {\n"
           "  method: 'POST',\n"
           "  headers: { 'Content-Type': 'application/x-www-form-urlencoded' },\n"
           "  body: 'file=' + encodeURIComponent(filename) "
           "})\n"
           ".then(response => response.text())\n"
           ".then((text) => {\n"
           "  console.log('Server response:', text);\n"
          //  "  location.reload();\n"
           "})\n"
           ".catch(err => console.error('Delete failed:', err));\n"
           "}\n"
           "\n"
           "function formatSPIFFS() {\n"
           "if (confirm('Are you sure you want to format SPIFFS? This will delete all files!')) {\n"
           "fetch('/format', { method: 'POST' })\n"
           ".then(response => response.text())\n"
           ".then(text => { console.log('Format response:', text); location.reload(); })\n"
           ".catch(err => console.error('Format failed:', err));\n"
           "}\n"
           "}\n"
           "</script>\n";
}

void setupFileManager(AsyncWebServer &server, String title) {
    server.on("/fs", HTTP_GET, [title](AsyncWebServerRequest *request) {
        String html = "<!DOCTYPE html>\n<html>\n<head>\n<meta charset='UTF-8'>\n<title>" + title + " - FS</title>\n";
        html += "<style>table { border-collapse: collapse; } th, td { padding: 8px; border: 1px solid black; }</style>";
        html += getPageScript();
        html += "</head>\n<body>\n<h2>File System</h2>\n<table>\n<tr><th>File</th><th>Size (bytes)</th><th>Action</th></tr>\n";

        String path = "/";
        File root = SPIFFS.open(path);
        File file = root.openNextFile();
        while (file) {
            html += "<tr><td>" + path + String(file.name()) + "</td>";
            html += "<td>" + String(file.size()) + "</td>";
            html += "<td><button onclick=\"deleteFile('" + path + String(file.name()) + "')\">Delete</button></td></tr>\n";
            file = root.openNextFile();
        }
        html += "</table>\n";
        html += "<br><br>\n";
        html += "<button onclick=\"formatSPIFFS()\">Format SPIFFS</button>\n";
        html += "</body>\n</html>\n";

        request->send(200, "text/html", html);
    });

    server.on("/delete", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("file", true)) {
            String filename = request->getParam("file", true)->value();
            if (SPIFFS.exists(filename)) {
                SPIFFS.remove(filename);
                // logEvent("Deleted file: " + filename);
                logEvent("Trying to delete file: '" + filename + "'");
                bool removed = SPIFFS.remove(filename);
                logEvent(String("Remove result: ") + (removed ? "Success" : "Failed"));
                request->send(200, "text/plain", "OK");
            } else {
                logEvent("Can't delete file: '" + filename + "' (file not found)");
                request->send(404, "text/plain", "File not found");
            }
        } else {
            request->send(400, "text/plain", "Bad request");
        }
    });

    server.on("/format", HTTP_POST, [](AsyncWebServerRequest *request) {
        bool success = formatSPIFFS();

        if (success) {
            request->send(200, "text/plain", "SPIFFS formatted successfully");
            ESP.restart();
        } else {
            request->send(500, "text/plain", "SPIFFS format failed");
        }
    });

    server.on("/startDownloadTask", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "starting...");
        startDownloadTask();
    });
}
