# Swift Doorbell W/ ESP-32
**Collaborators:** Ibrahim Al-Howaid &amp; Aidan Drakes 
**Project Start:** February 5, 2026 
**Status:** Complete and Installed (Design Cycle 3)

# Summary (STAR)
**Situation.** The shop's auto-locking door meant visitors often went unnoticed when staff were busy or in the classroom, and no commercial doorbell fit the space.

**Task.** Design and build a rugged, loud, wireless doorbell from scratch, with a button that could run for months on a single battery.

**Action.** We prototyped on the Micro:bit, found it could not supply enough voltage, then moved to Arduino and built LED and speaker test circuits with debounce logic. When volume and installation became the limits, I rebuilt on two ESP32s with C++ firmware using ESP-NOW for router-free communication and deep sleep with an RTC-pin wake to preserve battery life. Aidan designed the 3D-printed enclosures.

**Result.** A complete, installed wireless doorbell. The speaker runs on wall power in the classroom and the battery-powered button is mounted outside, so pressing it now alerts staff inside. Built and tested across three design cycles.

# Overview
A wireless doorbell built from two ESP32 microcontrollers that talk directly over ESP-NOW, designed and built from scratch with a partner. The battery-powered button is mounted outside by the door and the wall-powered speaker sits in the classroom, so a press outside plays a chime inside. The project went through three design cycles (Micro:bit, Arduino, ESP32) and is now fully built and installed.

# The Problem
The shop has a door that locks automatically and can only be opened from the inside. This requires a person to be physically present near the door to grant entry. However, when the shop is busy or staff are in the main classroom area, visitors often go unnoticed. A reliable alert system is needed to bridge this gap.

# Solution & Criteria
Our goal is to build a rugged, high-volume doorbell system tailored for a shop environment. The design must meet the following technical requirements:
- Audio Output: Minimum 80-90 dB (loud enough to be heard in the main classroom over shop machinery).
- Durability: Built to withstand the physical strain of daily heavy use.
- Weatherproofing: Sealed enough to survive snowy or wet conditions and protect the circuit.
- Maintenance: Securely wall-mounted but features an accessible opening for battery replacement and/or upgrades.
- Housing: Large enough to hold the circuit and the battery pack.

# Items Needed
- 2x Microcontrollers
- 2x Battery cases
- Input: [1x LED Button](https://www.amazon.ca/Nilight-Stainless-Latching-Switches-Waterproof/dp/B0BMVR2RRM/ref=sr_1_8?crid=3DV3F8GTZKL0R&dib=eyJ2IjoiMSJ9.97goyb5WPq66coA7I0-qGGkc5qGwcRxVoJKl1xU28IjYykzkQNahYAXcRwZTmlVES3VoNbFmwZmDoBAbxSqtlbhDXuIbAw5QVDgR0aZ2Ie-xfd5OuiJyO2GmL9eO22_1iPOTQBalzfOo5TQJE91X7btb4CFVfOXBCWGJGw59nURIjXEbiTSKT4x1ewGS9_vGJyfRQH1krc2IU4mF_woiDGFFvpyxNaxMBdQQ5fBcr4tzAzwjXNwKerOarS1jqylmm4Xa4xb8qPsjQ-4Q7q6oaNh7gxDILPuPfzBIUymTj_4.mRPwragvJzCY2GkOvKQD-k9MTDU-2wn6HpS9VTMSgkU&dib_tag=se&keywords=Gravity%3A+LED+Button&qid=1771351114&sprefix=micro+bit+large+button%2Caps%2C502&sr=8-8)
- Output: [1x Speaker](https://gikfun.com/products/gikfun-round-micro-speaker-diameter-28mm-8ohm-8r-2w-for-arduino-pack-of-2pcs)
- Housing: 2x Custom 3D-printed cases (Designed by Aidan)

![IMG_4123 (1)](https://github.com/user-attachments/assets/0077188b-1b6c-4fb7-9bf6-87c2bc424323)

# Design Cycle 1: Micro:bit Prototype
- **Setup:** One "Emitter" (button) and one "Receiver" (speaker), communicating over the Micro:bit radio.
- **Conclusion:** We discovered that the Micro:bit did not adequately support the voltage required for the external button and speaker simultaneously, so we moved to Arduino.

# Design Cycle 2: Arduino Prototypes
The Micro:bit could not drive the button and speaker, so we switched to Arduino, which supports the required voltage and is much better to work with overall. Before building the full system, I made a couple of prototypes to practice the wiring and get each component working on its own.

## LED Prototype
I built this prototype to confirm the button could connect to the Arduino and toggle an LED on and off, using a state machine and debounce logic so a single press registers cleanly.



<img width="512" height="384" alt="light demo" src="https://github.com/user-attachments/assets/fffa8c63-f332-4a32-92e2-d1d7d545c263" />




<details>
<summary>View LED prototype code</summary>

```cpp
// Pin constants
const int buttonPin = 7;
const int ledPin = 8;

// States definition
enum SystemState {
  STATE_OFF,
  STATE_ON
};

SystemState currentState = STATE_OFF; // Start in the OFF state

// Variables for debounce logic
int lastButtonState = LOW;
int stableButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 30;

void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);

  // Initialize output based on starting state
  updateHardware();
}

void loop() {
  int reading = digitalRead(buttonPin);

  // --- Step 1: Debounce Logic ---
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the reading has been stable, check if it's a new press
    if (reading != stableButtonState) {
      stableButtonState = reading;

      // --- Step 2: Case Transition (Triggered on HIGH) ---
      if (stableButtonState == LOW) {
        handleButtonPress();
      }
    }
  }

  lastButtonState = reading;
}

// This function handles the logic for switching between cases
void handleButtonPress() {
  switch (currentState) {
    case STATE_OFF:
      currentState = STATE_ON;
      break;

    case STATE_ON:
      currentState = STATE_OFF;
      break;
  }

  updateHardware();
}

// Apply the current state to the actual LED
void updateHardware() {
  switch (currentState) {
    case STATE_OFF:
      digitalWrite(ledPin, LOW);
      break;

    case STATE_ON:
      digitalWrite(ledPin, HIGH);
      break;
  }
}
```

</details>


## Speaker Prototype
I built this prototype to test the speaker volume and functions while also connecting a button at the same time. The speaker was not playing at full volume yet, since it needs a better transistor to support the wattage.




https://github.com/user-attachments/assets/fc04b203-32aa-46dd-899c-0fce6ca284f3

<img width="4032" height="3024" alt="Doorbell Design 2 (1)" src="https://github.com/user-attachments/assets/82a205eb-86eb-4e3d-aacd-317fc792dec9" />



<img width="3024" height="4032" alt="IMG_4395" src="https://github.com/user-attachments/assets/612358d9-3566-488f-9dfb-51be3ad9db69" />

<img width="4032" height="3024" alt="IMG_4398" src="https://github.com/user-attachments/assets/09a6001f-b95a-4a94-bfa9-4ce1949f80a3" />

# Design Cycle 3: ESP32 Rebuild
Two problems pushed us past the basic Arduino setup:
- The speaker was not loud enough to carry from the shop to the classroom.
- Installation of the doorbell might have been too difficult to manage.

Using an Arduino with external RF radio modules was considered but replaced by two ESP32 microcontrollers communicating via ESP-NOW, for the following reasons:
- ESP32 has a built-in Wi-Fi antenna, so no extra radio modules or wiring are needed.
- ESP-NOW is a peer-to-peer protocol that works without a Wi-Fi router.
- ESP32s are programmed in C++ using the same Arduino IDE, so existing code logic transfers over.

## Hardware Decisions
The system uses two ESP32 boards. The speaker side is powered by a USB wall adapter so it can stay on 24/7 listening for a signal, set up in the classroom. The button side runs on a 3.36V battery connected directly to the 3V3 pin, bypassing the onboard regulator to avoid wasting energy as heat. Deep Sleep is used on the button side so the battery lasts for months, as the ESP32 draws near-zero power while asleep and only wakes when the button is pressed.

## Speaker Side Wiring
Components: ESP32, BC547 transistor, passive speaker, 1kΩ resistor, 10kΩ resistor, 1N4007 diode (optional), USB wall adapter.

1. USB wall adapter plugged into ESP32 USB port
2. ESP32 5V pin → breadboard + rail
3. ESP32 GND pin → breadboard − rail
4. ESP32 GPIO25 → 1kΩ resistor → BC547 Base (middle leg)
5. 10kΩ resistor from BC547 Base → GND rail (prevents floating)
6. BC547 Emitter (right leg) → GND rail
7. BC547 Collector (left leg) → Speaker (−)
8. Speaker (+) → breadboard + rail (5V)


## Button Side Wiring
Components: ESP32, 3.36V battery, doorbell button (3-wire), 10kΩ resistor.

9. Battery + (red wire) → ESP32 3V3 pin (bypasses regulator, battery voltage matches 3.3V)
10. Battery − (black wire) → ESP32 GND pin
11. Button black wire → GND rail
12. Button white wire → ESP32 GPIO33 pin
13. 10kΩ resistor from GPIO33 → 3V3 rail (pull-up, keeps pin HIGH when idle)

*GPIO33 is an RTC pin, so it stays active during deep sleep and a button press can wake the ESP32.*
*GPIO33 reads HIGH normally and drops LOW when the button is pressed, which triggers the wake-up.*
*No transistor needed, the button signals the wake pin directly.*

## Speaker Tone Test Code
Uploaded to the speaker ESP32 to confirm the BC547 circuit and speaker were wired correctly. Three tones play at increasing pitch then stop.

<details>
<summary>View tone test sketch</summary>

```cpp
void setup() {
  tone(25, 1000, 500);   // GPIO25, 1000Hz, 500ms
  delay(600);
  tone(25, 1500, 500);
  delay(600);
  tone(25, 2000, 500);
  delay(600);
  noTone(25);
}

void loop() {
}
```

</details>

## Button Continuity Test Code
Uploaded to the button ESP32 to verify GPIO33 wiring before testing deep sleep. Serial Monitor (115200 baud) prints 1 when idle and 0 when the button is pressed.

<details>
<summary>View continuity test sketch</summary>

```cpp
void setup() {
  Serial.begin(115200);
  pinMode(33, INPUT_PULLUP);
}

void loop() {
  Serial.println(digitalRead(33));
  delay(200);
}
```

</details>

## Button Deep Sleep Test Code
Tests the full button-side behaviour. On wake, it prints a message to Serial and blinks the onboard LED three times, then returns to deep sleep. The ESP32 wakes on GPIO33 going LOW (button pressed).

<details>
<summary>View deep sleep test sketch</summary>

```cpp
#define BUTTON_PIN GPIO_NUM_33
#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("Button pressed - awake!");

  pinMode(LED_PIN, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }

  esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 0);
  Serial.println("Going back to sleep...");
  delay(100);
  esp_deep_sleep_start();
}

void loop() {
}
```

</details>

## Final Firmware
With both boards wired and tested, I wrote the final firmware. The two sketches share an identical `DoorbellMessage` struct so the packet lines up on both sides, and both are organized into classes (`EspNowSender` on the button, `SpeakerController` on the speaker) to keep the logic clean and readable.

**Button side** (`doorbell_button.ino`): the board wakes from deep sleep on a button press, fires one ESP-NOW packet to the speaker's MAC address, then goes straight back to sleep. All the work happens once in `setup()`, so `loop()` is never reached.

<details>
<summary>View the full firmware (both sketches)</summary>

```cpp
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
```

**Speaker side** (`doorbell_speaker.ino`): the board stays on, listens for the ESP-NOW packet, and on receipt plays one of three randomized melodies through the BC547-driven speaker. The receive callback only raises a flag so it stays fast, and the actual chime plays in `loop()`.

```cpp
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

// --- MELODY ARRAYS ---
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
```

</details>


*Note: the melody frequencies come from `pitches.h`, the standard Arduino tone-library header that maps note names (like `NOTE_E6`) to their frequencies in Hz.*

## Test Results
- Speaker tone test: successful, three tones played clearly through the BC547 circuit.
- Button continuity test: successful, GPIO33 reads 1 at idle and switches to 0 when the button is pressed.
- Deep sleep wake test: successful, Serial Monitor confirmed the message on each button press and the board returns to sleep correctly.
- ESP-NOW link: successful, the button and speaker ESP32 boards communicate reliably over ESP-NOW.

## Final Installation
The system is built, tested, and installed in the shop. The speaker unit is set up in the classroom on wall power, and the battery-powered button unit is mounted outside by the door. Pressing the button outside plays the chime in the classroom, which solves the original problem of visitors going unnoticed at the auto-locking door.

# Tech Stack
- Microcontrollers: ESP32 (x2), earlier prototypes on Micro:bit and Arduino
- Wireless: ESP-NOW (peer-to-peer, router-free)
- Firmware: C++ (Arduino IDE)
- Power management: deep sleep with RTC-pin wake
- Electronics: BC547 transistor speaker driver, breadboard prototyping
- Enclosure: Fusion 360, 3D printing (Aidan)
