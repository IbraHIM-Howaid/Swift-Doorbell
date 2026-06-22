/**
 * @file doorbell_speaker.ino
 * @brief Speaker-side control software for the Shop Doorbell project (ESP32 + ESP-NOW).
 * @author Ibrahim Al-Howaid
 * @date June 2026
 *
 * This program runs on the wall-powered speaker ESP32. It stays on continuously,
 * listening for an ESP-NOW packet from the button ESP32. When a "ring" packet is
 * received, it plays one of three randomized musical melodies through the speaker
 * driven by the BC547 transistor on GPIO25.
 */

#include "pitches.h"
#include <esp_now.h>
#include <WiFi.h>

// --- SPEAKER PIN ---
#define SPEAKER_PIN 25

// --- MELODY ARRAYS (unchanged from the original sketch) ---
int melody1[] = {NOTE_E6, NOTE_C6};
int durations1[] = {4, 2};
int length1 = 2;

int melody2[] = {NOTE_C6, NOTE_G5, NOTE_G5, NOTE_A5, NOTE_G5, 0, NOTE_B5, NOTE_C6};
int durations2[] = {4, 8, 8, 4, 4, 4, 4, 4};
int length2 = 8;

int melody3[] = {NOTE_A5, NOTE_F5, NOTE_G5, NOTE_C5, REST, REST, NOTE_C4, NOTE_G4, NOTE_A4, NOTE_F4};
int durations3[] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
int length3 = 10;

/**
 * @struct DoorbellMessage
 * @brief Tiny payload received over ESP-NOW. Kept identical on both boards.
 */
typedef struct {
  bool ring;
} DoorbellMessage;

// Flag set by the receive callback, handled in loop() so the callback stays fast.
volatile bool ringRequested = false;

/**
 * @class SpeakerController
 * @brief Manages audio output, frequencies, and melody playback.
 *        (Unchanged from the original Arduino sketch.)
 */
class SpeakerController {
  private:
    int pin;

  public:
    // initialize speaker pin
    SpeakerController(int p) {
      pin = p;
    }

    // Initializes the hardware pin and ensures transistor is off
    void begin() {
      pinMode(pin, OUTPUT);
      noTone(pin);
    }

    /**
     * @brief Iterates through arrays to play a specific musical melody.
     * @param mel Array of note frequencies.
     * @param dur Array of note durations.
     * @param len Number of notes in the melody array.
     */
    void playMelody(int mel[], int dur[], int len) {
      for (int thisNote = 0; thisNote < len; thisNote++) {
        if (mel[thisNote] == 0 || mel[thisNote] == REST) {
          noTone(pin); // Ensure transistor is OFF during rests
        } else {
          tone(pin, mel[thisNote]);
        }

        // Calculate note duration (1 second / note type)
        int noteDuration = 1000 / dur[thisNote];
        delay(noteDuration);

        // Stop tone and pause briefly before the next note
        noTone(pin);
        int pauseBetweenNotes = noteDuration * 1.01;
        delay(pauseBetweenNotes);
      }
    }

    /**
     * @brief Picks one of the three melodies at random and plays it.
     */
    void playRandom() {
      int choice = random(1, 4); // Pick random number between 1 and 3
      if (choice == 1) {
        playMelody(melody1, durations1, length1);
      } else if (choice == 2) {
        playMelody(melody2, durations2, length2);
      } else {
        playMelody(melody3, durations3, length3);
      }
    }
};

// --- OBJECT INSTANTIATION ---
SpeakerController doorbellSpeaker(SPEAKER_PIN);

/**
 * @brief ESP-NOW receive callback. Kept short: just validates and raises a flag.
 */
void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  DoorbellMessage msg;
  memcpy(&msg, data, sizeof(msg));
  if (msg.ring) {
    ringRequested = true;
  }
}

void setup() {
  Serial.begin(115200);

  doorbellSpeaker.begin();
  randomSeed(analogRead(0));

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_register_recv_cb(onReceive);
  Serial.println("Speaker ready, listening...");
}

void loop() {
  // Play the chime outside the callback so the radio callback stays fast.
  if (ringRequested) {
    ringRequested = false;
    Serial.println("Ring received!");
    doorbellSpeaker.playRandom();
  }
}
