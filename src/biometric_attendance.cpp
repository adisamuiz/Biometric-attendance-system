#include <Arduino.h>
#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

uint8_t getIdNumber();
uint8_t enrollNewFingerprint();
int scanFingerprintAndGetId();
void setOledDisplay(const String& displayText, int textSize, int cursorYaxis, uint16_t textColor);

#define mySerial Serial1

int SCREEN_WIDTH = 128;
int SCREEN_HEIGHT = 64;
uint8_t SCREEN_ADDRESS = 0x3D; // Standard is 3D but proteus is 3C
int OLED_RESET = -1;

int button_1 = 25;
int button_2 = 26;
int RED_LED = 27;
int GREEN_LED = 28;

// initialize fingerprint library and oled
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // pinMode(button_1, INPUT_PULLUP);
  pinMode(button_2, INPUT_PULLUP);

  Serial.begin(9600);
  while (!Serial); 
  delay(100);

  // Initialize fingerprint sensor
  finger.begin(57600);
  if (!finger.verifyPassword()) {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed")); //"F" stores the text in flash memory instead of Ram to preserve space
    for(;;);
  }

  Serial.println("Welcome, place finger to scan!");
  // display.clearDisplay();
  // setOledDisplay("Welcome!", 2, 0, SSD1306_WHITE);
  // setOledDisplay("Place finger to scan!", 1, 30, SSD1306_WHITE);
}

void loop() {
  // volatile int value1 = digitalRead(button_1);
  volatile int value2 = digitalRead(button_2);

  if (value2 == LOW) {
    Serial.println("button 2 PRESSED");
    display.clearDisplay();
    setOledDisplay("Ready to enroll finger!", 1, 0, SSD1306_WHITE);
    setOledDisplay("Place finger to scan!", 1, 30, SSD1306_WHITE);
    enrollNewFingerprint();
  }
  Serial.println("Welcome, place finger to scan!");
  display.clearDisplay();
  setOledDisplay("Welcome!", 2, 0, SSD1306_WHITE);
  setOledDisplay("Place finger to scan!", 1, 30, SSD1306_WHITE);
  int fingerprintId = scanFingerprintAndGetId();
  if (fingerprintId > 0) {
    display.clearDisplay();
    setOledDisplay("Welcome, ID: " + String(fingerprintId), 2, 5, SSD1306_WHITE);
    delay(1000);
  }
}

uint8_t getIdNumber() {
    uint8_t num = 0;

    while (num == 0) {
      while (!Serial.available())
        ;
      num = Serial.parseInt();
    }
    return num;
}

uint8_t enrollNewFingerprint() {
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save "
                 "this finger as...");
  int id = getIdNumber();
  if (id == 0) {
    return 0;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);
  int p = -1; // set an initial invalid entry
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);

  // Scan fingerprint
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println("Place your finger...");
    }
  }

  // Convert fingerprint image to template
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Image conversion failed in slot1");
    return false;
  }
  Serial.println("Remove finger");
  delay(2000);

  // Second Scan
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println("Place your finger...");
    }
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Image conversion failed in slot2");
    return false;
  }

  Serial.print("Creating model for #");
  Serial.println(id);
  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Fingerprints unmatched!");
    return false;
  }

  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK) {
    Serial.println("Fingerprint not Enrolled!");
    return false;
  }
  Serial.println("Fingerprint Enrolled!");
  return true;
}

int scanFingerprintAndGetId() {
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (digitalRead(button_2) == LOW) {
      Serial.println("Button 2 pressed");
      return -1;
    }
    if (p == FINGERPRINT_NOFINGER) {
      continue;
    } 
    else if (p == FINGERPRINT_OK) {
      Serial.println("Finger scanned!");
      break;
    } 
    else {
      Serial.println("finger scan failed! try again");
      break;
    }
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Image conversion failed");
    return -1;
  }

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    Serial.println("finger match not found");
    return -1;
  }
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

void setOledDisplay(const String& displayText, int textSize, int cursorYaxis, uint16_t textColor) {
  // display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(textColor);
  display.setCursor(0, cursorYaxis);
  display.println(displayText);
  display.display();
}