#include <Adafruit_Fingerprint.h>
#define mySerial Serial1

// initialize fingerprint library
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t readnumber(void) {
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
  int id = readnumber();
  if (id == 0) { // ID #0 not allowed, try again!
    return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print("Place your finger");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Image conversion failed");
    return false;
  }


  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");


  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert image slot 2");
    return false;
  }

  Serial.print("Creating model for #");
  Serial.println(id);

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Prints unmatched!");
    return false;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK) {
    Serial.println("not Stored!");
    return false;
  }
  Serial.println("Stored!");
  return true;

}

int scanFingerprintAndGetId() {
  int p = -1;
  // delay(3000);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    Serial.println("waiting for finger");
    if (p == FINGERPRINT_OK) {
      Serial.println("finger found");
      break;
    } 
    // else {
  //     Serial.println("finger scanned");
  //     break;
  //   }
  }

  // while (p != FINGERPRINT_OK) {
  //   p = finger.getImage();
  //   switch (p) {
  //   case FINGERPRINT_OK:
  //     Serial.println("Image taken");
  //     break;
  //   case FINGERPRINT_NOFINGER:
  //     Serial.print(".");
  //     break;
  //   case FINGERPRINT_PACKETRECIEVEERR:
  //     Serial.println("Communication error");
  //     break;
  //   case FINGERPRINT_IMAGEFAIL:
  //     Serial.println("Imaging error");
  //     break;
  //   default:
  //     Serial.println("Unknown error");
  //     break;
  //   }
  // }
  // uint8_t p = finger.getImage();
  // if (p != FINGERPRINT_OK)
  // Serial.println("finger scan done");
  // Serial.println(p);

  //   return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)
    return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    Serial.println("finger not matched");
    return -1;
  }
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

int button_1 = 25;
int button_2 = 26;
void setup() {
  pinMode(button_1, INPUT_PULLUP);
  pinMode(button_2, INPUT_PULLUP);
  Serial.begin(9600);
  while (!Serial); 
  delay(100);
  Serial.println("\nHardware Serial Fingerprint Test");
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } 
  else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }
}

void loop() {
  int value1 = digitalRead(button_1);
  int value2 = digitalRead(button_2);
  if (value1 == LOW && value2 == HIGH ) {
    Serial.println("button 1 PRESSED");
    int fingerprintId = scanFingerprintAndGetId();
    if (fingerprintId > 0) {
      Serial.println("found fingerprint with id: ");
      Serial.println(fingerprintId);
    }
  }
  if (value1 == HIGH && value2 == LOW) {
    Serial.println("button 2 PRESSED");
    enrollNewFingerprint();
  }
  
  // Serial.println("system ready");

}
