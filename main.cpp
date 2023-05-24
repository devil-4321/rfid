#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Define SS pin and create MFRC522 instance
#define SS_PIN D2
#define RST_PIN D3
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Wi-Fi credentials
const char* ssid = "Your_WiFi_SSID";
const char* password = "Your_WiFi_Password";

// Server details
const String serverURL = "http://yourserver.com/attendance"; // Replace with your server URL

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
      delay(200);
      return;
    }

    // Read RFID tag UID
    String tagUID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      tagUID += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      tagUID += String(mfrc522.uid.uidByte[i], HEX);
    }

    // Send attendance data to the server
    if (sendAttendanceData(tagUID)) {
      Serial.println("Attendance recorded: " + tagUID);
    } else {
      Serial.println("Failed to record attendance: " + tagUID);
    }

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    delay(2000);
  }
}

bool sendAttendanceData(String tagUID) {
  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Prepare attendance data
  String postData = "uid=" + tagUID;

  // Send POST request
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode == HTTP_CODE_OK) {
    String response = http.getString();
    if (response == "Success") {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }

  http.end();
}
