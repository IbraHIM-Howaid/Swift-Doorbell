# Swift Doorbell W/ ESP-32
**Collaborators:** Ibrahim Al-Howaid &amp; Aidan Drakes 
**Project Start:** February 5, 2026 
**Status:** In Progress (Design Cycle 3)

# The Problem
The shop features a security door that locks automatically and can only be opened from the inside. This requires a person to be physically present near the door to grant entry. However, when the shop is busy or staff are in the main classroom area, visitors often go unnoticed. A reliable alert system is needed to bridge this gap.

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


<img width="512" height="384" alt="light demo" src="https://github.com/user-attachments/assets/abaa2b7c-1631-4b26-b2d8-9929517be56a" />



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

## Speaker Prototype
I built this prototype to test the speaker volume and functions while also connecting a button at the same time. The speaker was not playing at full volume yet, since it needs a better transistor to support the wattage.




https://github.com/user-attachments/assets/358c88d1-b70e-46f1-bb2e-e722ba1a3a29







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
Components: ESP32, 3.36V LiFePO4 battery, doorbell button (3-wire), 10kΩ resistor.

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

## Button Continuity Test Code
Uploaded to the button ESP32 to verify GPIO33 wiring before testing deep sleep. Serial Monitor (115200 baud) prints 1 when idle and 0 when the button is pressed.

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

## Button Deep Sleep Test Code
Tests the full button-side behaviour. On wake, it prints a message to Serial and blinks the onboard LED three times, then returns to deep sleep. The ESP32 wakes on GPIO33 going LOW (button pressed).

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

## Test Results
- Speaker tone test: successful, three tones played clearly through the BC547 circuit.
- Button continuity test: successful, GPIO33 reads 1 at idle and switches to 0 when the button is pressed.
- Deep sleep wake test: successful, Serial Monitor confirmed the message on each button press and the board returns to sleep correctly.
- Next step: implement ESP-NOW communication to link the button and speaker ESP32 boards.

## Final Installation
The system is built, tested, and installed in the shop. The speaker unit is set up in the classroom on wall power, and the battery-powered button unit is mounted outside by the door. Pressing the button outside plays the chime in the classroom, which solves the original problem of visitors going unnoticed at the auto-locking door.

# Tech Stack
- Microcontrollers: ESP32 (x2), earlier prototypes on Micro:bit and Arduino
- Wireless: ESP-NOW (peer-to-peer, router-free)
- Firmware: C++ (Arduino IDE)
- Power management: deep sleep with RTC-pin wake
- Electronics: BC547 transistor speaker driver, breadboard prototyping
- Enclosure: Fusion 360, 3D printing (Aidan)

# STAR Summary
**Situation.** The shop's auto-locking door meant visitors & students often went unnoticed when staff/students were busy or in the classroom, so we decided to design and build one.
 
**Task.** Design and build a rugged, loud, wireless doorbell from scratch, with a button that could run for months on a single battery.
 
**Action.** We prototyped on the Micro:bit, found it could not supply enough voltage, then moved to Arduino and built LED and speaker test circuits with debounce logic. When volume and installation became the limits, I rebuilt on two ESP32s with C++ firmware using ESP-NOW for router-free communication and deep sleep with an RTC-pin wake to preserve battery life. Aidan designed the 3D-printed enclosures.
 
**Result.** A complete, installed wireless doorbell. The speaker runs on wall power in the classroom and the battery-powered button is mounted outside, so pressing it now alerts staff inside. Built and tested across three design cycles.
