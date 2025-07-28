#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "esp_timer.h"
#include "esp32-hal-ledc.h"

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* botToken = "YOUR_BOT_TOKEN";
const char* chatID   = "YOUR_CHAT_ID";

String imagga_user   = "YOUR_IMAGGA_USER";
String imagga_secret = "YOUR_IMAGGA_SECRET";

#define PIR_PIN         13
#define FLASH_PIN       4
#define DOOR_SENSOR_PIN 12

WiFiClientSecure clientTCP;
unsigned long lastMotionTime = 0;
const unsigned long motionCooldown = 15000;
#define DEBUG false

String captureAndLabel(camera_fb_t* fb);
void sendPhotoTelegram(camera_fb_t* fb, String caption);

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(FLASH_PIN, OUTPUT);
  digitalWrite(FLASH_PIN, LOW);

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  echo("\n✅ WiFi Connected: " + WiFi.localIP().toString());

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = 5;
  config.pin_d1       = 18;
  config.pin_d2       = 19;
  config.pin_d3       = 21;
  config.pin_d4       = 36;
  config.pin_d5       = 39;
  config.pin_d6       = 34;
  config.pin_d7       = 35;
  config.pin_xclk     = 0;
  config.pin_pclk     = 22;
  config.pin_vsync    = 25;
  config.pin_href     = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn     = 32;
  config.pin_reset    = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count     = 2;
  config.fb_location  = CAMERA_FB_IN_PSRAM;

  if (esp_camera_init(&config) != ESP_OK) {
    echo("❌ Camera init failed");
    return;
  }

  echo("📷 Camera Ready");
}

void loop() {
  static bool lastDoorState = HIGH;
  bool doorState = digitalRead(DOOR_SENSOR_PIN);

  if (doorState != lastDoorState) {
    lastDoorState = doorState;

    if (doorState == HIGH) {
      echo("🚪 Door/Window OPENED!");

      camera_fb_t* fb = esp_camera_fb_get();
      if (fb) {
        sendPhotoTelegram(fb, "🚪 Door/Window OPENED!");
        esp_camera_fb_return(fb);
      }
    } else {
      echo("🔒 Door/Window CLOSED");
    }

    delay(100);
  }

  if (digitalRead(PIR_PIN) == HIGH && millis() - lastMotionTime > motionCooldown) {
    lastMotionTime = millis();
    echo("🚨 Motion detected!");

    digitalWrite(FLASH_PIN, HIGH);
    delay(300);

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      echo("❌ Camera capture failed");
      digitalWrite(FLASH_PIN, LOW);
      return;
    }

    digitalWrite(FLASH_PIN, LOW);

    String label = captureAndLabel(fb);
    String caption;
    if (label.startsWith("person:")) {
      String conf = label.substring(label.indexOf(":") + 1);
      caption = "🚶 Person Detected (" + conf + "%)";
    } else if (label.startsWith("animal:")) {
      String conf = label.substring(label.indexOf(":") + 1);
      caption = "🐾 Animal Detected (" + conf + "%)";
    } else if (label == "imagga_fail" || label == "wifi_fail" || label == "json_parse_fail")
      caption = "⚠ Motion detected, but classification failed!";
    else
      caption = "⚠ Motion Detected (Unknown)";

    sendPhotoTelegram(fb, caption);
    esp_camera_fb_return(fb);
  }
}

void sendPhotoTelegram(camera_fb_t* fb, String caption) {
  if (WiFi.status() != WL_CONNECTED || !fb) return;

  clientTCP.stop();
  clientTCP.setInsecure();
  if (!clientTCP.connect("api.telegram.org", 443)) {
    Serial.println("❌ Telegram connection failed");
    return;
  }

  String boundary = "------------------------ESP32CAMBoundary";

  String startRequest = "--" + boundary + "\r\n";
  startRequest += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
  startRequest += String(chatID) + "\r\n";
  startRequest += "--" + boundary + "\r\n";
  startRequest += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n";
  startRequest += caption + "\r\n";
  startRequest += "--" + boundary + "\r\n";
  startRequest += "Content-Disposition: form-data; name=\"photo\"; filename=\"image.jpg\"\r\n";
  startRequest += "Content-Type: image/jpeg\r\n\r\n";

  String endRequest = "\r\n--" + boundary + "--\r\n";
  int contentLength = startRequest.length() + fb->len + endRequest.length();

  String headers = "POST /bot" + String(botToken) + "/sendPhoto HTTP/1.1\r\n";
  headers += "Host: api.telegram.org\r\n";
  headers += "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
  headers += "Content-Length: " + String(contentLength) + "\r\n\r\n";

  clientTCP.print(headers);
  clientTCP.print(startRequest);
  clientTCP.write(fb->buf, fb->len);
  clientTCP.print(endRequest);

  delay(500);
  clientTCP.stop();
  Serial.println("✅ Sent to Telegram");
}

String captureAndLabel(camera_fb_t* fb) {
  if (!fb || fb->len == 0) return "no_image";
  if (WiFi.status() != WL_CONNECTED) return "wifi_fail";

  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect("api.imagga.com", 443)) {
    echo("❌ Failed to connect to Imagga");
    return "imagga_fail";
  }

  String boundary = "----ESP32CamBoundary";
  String contentType = "multipart/form-data; boundary=" + boundary;

  String bodyStart = "--" + boundary + "\r\n";
  bodyStart += "Content-Disposition: form-data; name=\"image\"; filename=\"photo.jpg\"\r\n";
  bodyStart += "Content-Type: image/jpeg\r\n\r\n";

  String bodyEnd = "\r\n--" + boundary + "--\r\n";

  int contentLength = bodyStart.length() + fb->len + bodyEnd.length();

  // Send upload request
  client.printf("POST /v2/uploads HTTP/1.1\r\n");
  client.printf("Host: api.imagga.com\r\n");
  client.printf("Authorization: Basic %s\r\n", base64::encode(imagga_user + ":" + imagga_secret).c_str());
  client.printf("Content-Type: %s\r\n", contentType.c_str());
  client.printf("Content-Length: %d\r\n", contentLength);
  client.printf("Connection: close\r\n\r\n");

  client.print(bodyStart);
  client.write(fb->buf, fb->len);
  client.print(bodyEnd);

  // Read upload response
  String response = "";
  long timeout = millis() + 7000;
  while (client.connected() && millis() < timeout) {
    while (client.available()) {
      char c = client.read();
      response += c;
    }
  }
  client.stop();

  if (DEBUG) {
    Serial.println("[UPLOAD RESPONSE]");
    Serial.println(response);
  }

  // Parse upload_id using JSON
  int jsonStart = response.indexOf('{');
  if (jsonStart == -1) return "json_parse_fail";

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, response.substring(jsonStart));
  if (error) {
    Serial.println("❌ Failed to parse Imagga upload JSON");
    return "json_parse_fail";
  }

  String uploadId = doc["result"]["upload_id"];
  if (uploadId == "") return "imagga_fail";
  if (DEBUG) Serial.println("✅ Upload ID: " + uploadId);

  // Get tags using upload_id
  HTTPClient http;
  WiFiClientSecure tagClient;
  tagClient.setInsecure();

  String tagURL = "https://api.imagga.com/v2/tags?image_upload_id=" + uploadId;
  http.begin(tagClient, tagURL);
  http.addHeader("Authorization", "Basic " + base64::encode(imagga_user + ":" + imagga_secret));
  int httpCode = http.GET();
  String tagResponse = http.getString();
  http.end();

  if (DEBUG) {
    Serial.println("[TAGS] Code: " + String(httpCode));
    Serial.println("[TAGS] Response: " + tagResponse);
  }

  if (httpCode != 200) return "imagga_fail";

  // Parse tag response
  DynamicJsonDocument tagDoc(2048);
  if (deserializeJson(tagDoc, tagResponse)) return "json_parse_fail";

  JsonArray tags = tagDoc["result"]["tags"];
  for (JsonObject tag : tags) {
    String label = tag["tag"]["en"];
    float confidence = tag["confidence"];
    label.toLowerCase();

    if (label.indexOf("person") != -1 || label.indexOf("man") != -1 || label.indexOf("woman") != -1 || label.indexOf("human") != -1)
      return "person:" + String(confidence, 1);
    
    if (label.indexOf("dog") != -1 || label.indexOf("cat") != -1 || label.indexOf("animal") != -1 || label.indexOf("puppy") != -1 || label.indexOf("kitten") != -1)
      return "animal:" + String(confidence, 1);
  }

  return "no_person";
}

void echo(String msg) {
  Serial.println(msg);  // still prints to Serial
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://api.telegram.org/bot" + String(botToken) + "/sendMessage");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String body = "chat_id=" + String(chatID) + "&text=" + msg;
    http.POST(body);
    http.end();
  }
}
