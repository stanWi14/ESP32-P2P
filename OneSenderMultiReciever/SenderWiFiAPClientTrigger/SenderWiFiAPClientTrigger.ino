// Send to targeted Device 
// Triggered by client connecting to WiFi AP
// You can use normal reciver for the basic implementation
#include <esp_now.h>
#include <WiFi.h>

// array to targeted device MAC Index
int connectTo[]= {0,2};

// REPLACE WITH YOUR RECEIVER MAC Addresses
uint8_t broadcastAddresses[][6] = {
  {0x08, 0xB6, 0x1F, 0x3B, 0x51, 0xB8},
  {0xAC, 0x0B, 0xFB, 0xDB, 0x9A, 0x00},
  {0xEC, 0xFA, 0xBC, 0xA5, 0xE9, 0x8B},
  // you can add more or less here
};

typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfoArray[sizeof(broadcastAddresses) / 6];

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

const char *ssid = "ESP32Master";
const char *password = "password";
const int ledPin = 2;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  WiFi.begin(ssid, password);
  Serial.print("Starting WiFi AP: ");
  Serial.println(ssid);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);

  // Register all peers
  for (int i = 0; i < sizeof(broadcastAddresses) / 6; i++) {
    memcpy(peerInfoArray[i].peer_addr, broadcastAddresses[i], 6);
    peerInfoArray[i].channel = 0;  
    peerInfoArray[i].encrypt = false;
    
    // Add peer        
    if (esp_now_add_peer(&peerInfoArray[i]) != ESP_OK){
      Serial.println("Failed to add peer");
      return;
    }
  }
}


// send signal 1 time only for each Device, retry when failed stop if success
void sendSignal(int index) {
  if (index < sizeof(broadcastAddresses) / 6) {
     Serial.print("Sending signal to index ");
     Serial.println(index);
     bool success = false;
     while (!success) {
        esp_err_t result = esp_now_send(broadcastAddresses[index], (uint8_t *) &myData, sizeof(myData));
        if (result == ESP_OK) {
            Serial.println("Sent with success");
            success = true;
        } else {
            Serial.println("Error sending the data, retrying...");
            delay(1000);
        }
     }
  } else {
    Serial.println("Invalid index");
  }
}


void loop() {
  int numClients = WiFi.softAPgetStationNum();
  
  // If there are clients connected, turn on LED and send data
  if (numClients > 0) {
    digitalWrite(ledPin, HIGH);
    strcpy(myData.a, "THIS IS A CHAR");
    myData.b = random(1,20);
    myData.c = 1.2;
    myData.d = false;
    
//     Send data to each device one by one
    for (int i = 0; i < sizeof(connectTo) / sizeof(connectTo[0]); i++) {
      int index = connectTo[i];
      sendSignal(index);
    }
    delay(5000);
  } else {
    digitalWrite(ledPin, LOW);
    // If no clients are connected, wait for a moment before checking again
    delay(2000);
  }
}
