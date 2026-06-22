/**
 * @file doorbell_button.ino
 * @brief Button-side control software for the Shop Doorbell project (ESP32 + ESP-NOW).
 * @author Ibrahim Al-Howaid
 * @date June 2026
 *
 * This program runs on the battery-powered button ESP32. The board spends almost
 * all of its time in Deep Sleep drawing near-zero power. When the physical button
 * is pressed, GPIO33 is pulled LOW, waking the board. On wake it fires a single
 * ESP-NOW packet to the speaker ESP32, then immediately returns to Deep Sleep.
 */

#include <esp_now.h>
#include <WiFi.h>
#include <esp_sleep.h>

// --- WAKE PIN ---
// GPIO33 is an RTC pin, so it stays active during Deep Sleep and can wake the board.
#define BUTTON_WAKE_PIN GPIO_NUM_33

// --- TARGET ADDRESS ---
// CHANGE THIS to your speaker ESP32's MAC address.
uint8_t speakerMAC[] = {0xB4, 0xE6, 0x2D, 0xD5, 0xFB, 0x75};

/**
 * @struct DoorbellMessage
 * @brief Tiny payload sent over ESP-NOW. Kept identical on both boards.
 */
typedef struct {
  bool ring;
} DoorbellMessage;

/**
 * @class EspNowSender
 * @brief Encapsulates ESP-NOW initialization and one-shot transmission.
 */
class EspNowSender {
  private:
    uint8_t peerMAC[6];

  public:
    // Store the target MAC address for this sender.
    EspNowSender(uint8_t mac[6]) {
      memcpy(peerMAC, mac, 6);
    }

    /**
     * @brief Brings up Wi-Fi in station mode and registers the speaker as a peer.
     * @return true if ESP-NOW initialized and the peer was added, false otherwise.
     */
    bool begin() {
      WiFi.mode(WIFI_STA);

      if (esp_now_init() != ESP_OK) {
        return false;
      }

      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, peerMAC, 6);
      peerInfo.channel = 0;      // 0 = use current channel (no router needed)
      peerInfo.encrypt = false;

      return esp_now_add_peer(&peerInfo) == ESP_OK;
    }

    /**
     * @brief Sends a single "ring" packet to the speaker ESP32.
     */
    void sendRing() {
      DoorbellMessage msg;
      msg.ring = true;
      esp_now_send(peerMAC, (uint8_t *)&msg, sizeof(msg));
      delay(100); // give the radio time to finish transmitting before sleep
    }
};

// --- OBJECT INSTANTIATION ---
EspNowSender doorbellSender(speakerMAC);

void setup() {
  Serial.begin(115200);
  delay(50);

  if (doorbellSender.begin()) {
    doorbellSender.sendRing();
    Serial.println("Ring sent!");
  } else {
    Serial.println("ESP-NOW init failed");
  }

  // Arm wake-up on button press (GPIO33 going LOW) and return to Deep Sleep.
  esp_sleep_enable_ext0_wakeup(BUTTON_WAKE_PIN, 0);
  Serial.println("Going to sleep...");
  delay(50);
  esp_deep_sleep_start();
}

void loop() {
  // Never reached. All work happens once on wake in setup().
}
