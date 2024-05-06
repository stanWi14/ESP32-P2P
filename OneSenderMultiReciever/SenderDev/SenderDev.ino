#include <esp_now.h>
#include <WiFi.h>

// Reciever MAC Address array
uint8_t broadcastAddresses[][6] = {
  {0x08, 0xB6, 0x1F, 0x3B, 0x51, 0xB8},
  {0xAC, 0x0B, 0xFB, 0xDB, 0x9A, 0x00},
  {0xEC, 0xFA, 0xBC, 0xA5, 0xE9, 0x8B},
  // Add more MAC addresses here if needed
};

// Structure data to send
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfoArray[sizeof(broadcastAddresses) / 6];

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
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
 
void loop() {
  // Set values to send
  strcpy(myData.a, "THIS IS A CHAR");
  myData.b = random(1,20);
  myData.c = 1.2;
  myData.d = false;
  
  // Send message via ESP-NOW to all peers
  for (int i = 0; i < sizeof(broadcastAddresses) / 6; i++) {
    esp_err_t result = esp_now_send(broadcastAddresses[i], (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }

  delay(2000);
}
