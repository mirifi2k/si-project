#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// 16x2 I2C LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;

// const char *sensorsNames[8] = {"OUT", "IN", "6", "5", "4", "3", "2", "1"};
// const unsigned short sensors[8] = {A0, A1, A2, A3, 13, 12, 11, 10};

const unsigned short parkingLotsPins[6] = {
  10, 11, 12, 13, A3, A2
};

const unsigned short leavingSensorPin = A0;
const unsigned short enteringSensorPin = A1;

int enter, leave; // value readings for leaving/entering sensors

void setup() {
  Serial.begin(9600);

  /*
   * Pins Setup.
   */

  pinMode(leavingSensorPin, INPUT);
  pinMode(enteringSensorPin, INPUT);
  
  for (int i = 0; i < 6; i++) {
    pinMode(parkingLotsPins[i], INPUT);
  }

  /*
   * Servo setup.
   */

  servo.attach(7);
  servo.write(0);

  /*
   * LCD Setup.
   */

  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("Free park spots:");
}

void loop() {
  unsigned short freeSpots;
  lcd.setCursor(0, 1);
  
  for (int i = 0; i < 6; i++) {
    int val;
    if (i < 4) {
      if ((val = digitalRead(parkingLotsPins[i])) == HIGH) {
        lcd.print(i + 1);
        lcd.print(",");
      }
    }
    else {
      if ((val = analogRead(parkingLotsPins[i])) > 550) {
        lcd.print(i + 1);
        lcd.print(",");
      }
    }
  }

  lcd.print("\b     ");
  
  enter = analogRead(enteringSensorPin); // reading of the parking entering sensor
  leave = analogRead(leavingSensorPin); // reading of the parking leaving sensor
  
  if ((freeSpots = getFreeSpots()) < 1) { // in case there are no free spots => barrier closes and displays message
    servo.write(0);
    
    delay(1000);

    lcd.setCursor(0, 1);
    lcd.print("Parking is full.");
  }

  if (leave <= 550) { // if someone wants to leave
    for (int i = 0; i <= 60; i++) { // open the barrier
      servo.write(i);
      delay(30);
    }

    while ((leave = analogRead(leavingSensorPin)) <= 550); // barrier stays up till the car leaves

    delay(500); // wait 500ms
    for (int i = 60; i >= 0; i--) { // close the barrier
      servo.write(i);
      delay(30);
    }
  }

  if (enter <= 550) {
    for (int i = 0; i <= 60; i++) { // open the barrier
      servo.write(i);
      delay(30);
    }
    
    while ((enter = analogRead(enteringSensorPin)) <= 550); // barrier stays up till the car enters the parking

    delay(500); // wait 500ms
    for (int i = 60; i >= 0; i--) { // close the barrier
      servo.write(i);
      delay(30);
    }
  }

  if ((freeSpots = getFreeSpots()) > 0) {
    lcd.setCursor(13, 1);
    lcd.print("C:");
    lcd.print(freeSpots);
  }
}

unsigned short getFreeSpots() { // function to return the free spots count
  unsigned short count = 0x06;

  for (unsigned short i = 0x0; i < 0x6; i++) {
    if (i < 4) {
      if (digitalRead(parkingLotsPins[i]) == LOW) { // spots 1-4 are connected to digital pins
        count --;
      }
    } else {
      if (analogRead(parkingLotsPins[i]) <= 550) { // spots 5-6 are connected to analog pins
        count --;
      }
    }
  }
  
  return count;
}