#include "FS.h"

//holds the current upload
File fsUploadFile;
String folderMap[100];
int folderSelect = 1;
int fileSelect = 1;

String getRootHtml()
{
  File f = SPIFFS.open("/root.htm", "r");
  
}

void setup_Filesystem() {
  if (!SPIFFS.begin()) {
    Serial.println(F("No SPIFFS found. Format it"));
    if (SPIFFS.format()) {
      SPIFFS.begin();
    } else {
      Serial.println(F("No SPIFFS found. Format it"));
    }
  } else {
    Serial.println(F("SPIFFS file list"));
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      Serial.print(dir.fileName());
      Serial.print(F(" - "));
      Serial.println(dir.fileSize());
    }
  }
}

String getContentType(String filename) {
  if (httpServer.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  Serial.printf_P(PSTR("handleFileRead: %s\r\n"), path.c_str());
  if (path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = httpServer.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (httpServer.uri() != "/edit") return;
  HTTPUpload& upload = httpServer.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    Serial.printf_P(PSTR("handleFileUpload Name: %s\r\n"), filename.c_str());
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.printf_P(PSTR("handleFileUpload Data: %d\r\n"), upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    Serial.printf_P(PSTR("handleFileUpload Size: %d\r\n"), upload.totalSize);
  }
}

void handleFileDelete() {
  if (httpServer.args() == 0) return httpServer.send(500, "text/plain", "BAD ARGS");
  String path = httpServer.arg(0);
  Serial.printf_P(PSTR("handleFileDelete: %s\r\n"), path.c_str());
  if (path == "/")
    return httpServer.send(500, "text/plain", "BAD PATH");
  if (!SPIFFS.exists(path))
    return httpServer.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  httpServer.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (httpServer.args() == 0)
    return httpServer.send(500, "text/plain", "BAD ARGS");
  String path = httpServer.arg(0);
  Serial.printf_P(PSTR("handleFileCreate: %s\r\n"), path.c_str());
  if (path == "/")
    return httpServer.send(500, "text/plain", "BAD PATH");
  if (SPIFFS.exists(path))
    return httpServer.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if (file)
    file.close();
  else
    return httpServer.send(500, "text/plain", "CREATE FAILED");
  httpServer.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if (!httpServer.hasArg("dir")) {
    httpServer.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = httpServer.arg("dir");
  Serial.printf_P(PSTR("handleFileList: %s\r\n"), path.c_str());
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  output += "]";
  httpServer.send(200, "text/json", output);
}

void handleMinimalUpload() {
  char temp[700];

  snprintf ( temp, 700,
             "<!DOCTYPE html>\
    <html>\
      <head>\
        <title>ESP8266 Upload</title>\
        <meta charset=\"utf-8\">\
        <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
      </head>\
      <body>\
        <form action=\"/edit\" method=\"post\" enctype=\"multipart/form-data\">\
          <input type=\"file\" name=\"data\">\
          <input type=\"text\" name=\"path\" value=\"/\">\
          <button>Upload</button>\
         </form>\
      </body>\
    </html>"
           );
  httpServer.send ( 200, "text/html", temp );
}

void handleSpiffsFormat() {
  SPIFFS.format();
  httpServer.send(200, "text/json", "format complete");
}

void readFolders() {
  if (SPIFFS.exists("/Folders.txt")) {
    File f = SPIFFS.open("/Folders.txt", "r");
    int Index = 0;
    String temp;
    while ( f.available()) {
      temp = f.readStringUntil(10);
      temp.replace("\r", "");
      temp.toLowerCase();
      Index = temp.substring(0, 2).toInt();
      folderMap[Index] = temp.substring(3);
    }
    f.close();
  }
}

void getFolder(String folderSearch) {
  if (folderSearch.length() < 3) {
    folderSelect = folderSearch.toInt();
  } else {
    int Index;
    String temp;
    folderSearch.toLowerCase();
    for (Index = 99; Index > 0; Index--) {
      if (folderMap[Index].indexOf(folderSearch) >= 0) {
        folderSelect = Index;
        break;
      }
    }
  }
  if (folderSelect < 1 || folderSelect > 99) folderSelect = 0;
}

void setup_Filesystem_Webhandlers()
{
  Serial.println(F("Set up Web command handlers"));
  //Simple upload
  httpServer.on("/upload", handleMinimalUpload);
  httpServer.on("/format", handleSpiffsFormat);
  httpServer.on("/list", HTTP_GET, handleFileList);
  //load editor
  httpServer.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) httpServer.send(404, "text/plain", "FileNotFound");
  });
  //create file
  httpServer.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  httpServer.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  httpServer.on("/edit", HTTP_POST, []() {
    httpServer.send(200, "text/plain", "");
  }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  httpServer.onNotFound([]() {
    if (!handleFileRead(httpServer.uri())) httpServer.send(404, "text/plain", "FileNotFound");
  });
}
