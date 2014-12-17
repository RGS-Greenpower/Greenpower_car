//#include <EEPROMex.h>
//#include <EEPROMVar.h>
//#include <SendOnlySoftwareSerial.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <SD.h>

const int speedo_pin = 2;
const int current_pin = A0;
const int voltage_pin = A1;
const int battery_temp_pin = A2;
const int motor_temp_pin = A3;
const float wheel_size_in = 20;
const float wheel_size_mi = wheel_size_in / 63360;
const float shunt_resistor_value_ohms = 0.1;
const float smoothing_factor = 0.9;
float speed;
float current;
float av_current = -1;
float voltage;
float battery_temp;
float motor_temp;
int speedo_last;
int timer;
String send;
String display;
String store;
char buffer[100];
String message;
const int chipSelect = 4;
const int resetPin = 3;
unsigned long SDnumber = 0;
String SDnumberString = "";
String filename;
//SendOnlySoftwareSerial lcd_data = SendOnlySoftwareSerial(5);
//SendOnlySoftwareSerial lcd_message = SendOnlySoftwareSerial(6);
LiquidCrystal lcd_data(8, 7, 6, 5, 4, 3);
LiquidCrystal lcd_message(14, 13, 12, 11, 10, 9);

void setup() {
  // put your setup code here, to run once:
  analogReadResolution(12);
  analogWriteResolution(12);
  digitalWrite(resetPin, HIGH);
  pinMode(resetPin, OUTPUT);
  Serial1.begin(9600);
  pinMode(speedo_pin, INPUT);
  digitalWrite(speedo_pin, HIGH);
  speedo_last = -1;
  pinMode(chipSelect, OUTPUT);
  SD.begin(chipSelect);
  //SDnumber = EEPROM.readLong(0);
  File EEPROMsdr = SD.open("EEPROM.txt", FILE_READ);
  if (EEPROMsdr) {
    while (EEPROMsdr.available() < 0) {
      SDnumberString += EEPROMsdr.read();
    }
    EEPROMsdr.close();
    SD.remove("EEPROM.txt");
    char SDnumberChar[SDnumberString.length()];
    SDnumberString.toCharArray(SDnumberChar, SDnumberString.length());
    SDnumber = long(SDnumberChar);
  }
  SDnumber++;
  File EEPROMsdw = SD.open("EEPROM.txt", FILE_WRITE);
  if (EEPROMsdw) {
    EEPROMsdw.print(String(SDnumber));
    EEPROMsdw.close();
  }
  //EEPROM.updateLong(0, SDnumber);
  filename = String("data") + String(SDnumber) + String(".csv");
  char filename_char[filename.length()];
  filename.toCharArray(filename_char, filename.length());
  File dataFile = SD.open(filename_char, FILE_WRITE);
  if (dataFile) {
    dataFile.println(String("Time (mins),Speed,Current,Av. Current,Voltage,Battery Temp,Motor Temp"));
    dataFile.close();
  }
  //lcd_data.begin(20, 4);
  //lcd_message.begin(20, 4);
}

void loop() {
  //calculate speed
  if (digitalRead(speedo_pin) == LOW) {
    if (speedo_last >= 0) {
      speed = wheel_size_mi / ((millis() - speedo_last) / 3600000);
    }
    speedo_last = millis();
  }
  //calculate current
  current = (analogRead(current_pin) * 5 / 1024) / shunt_resistor_value_ohms;
  //calculate exponential moving average current
  if (av_current == -1) {
    av_current = current;
  }
  av_current = (av_current * smoothing_factor) + (current * (1 - smoothing_factor));
  //calculate other values
  voltage = analogRead(voltage_pin) * 30 / 1024;
  battery_temp = ((analogRead(battery_temp_pin) * 5000 / 1024) - 500) / 10;
  motor_temp = ((analogRead(motor_temp_pin) * 5000 / 1024) - 500) / 10;
  //send values over Serial1
  send = String(int(speed * 10)) + String(int(current * 10)) + String(int(av_current * 10)) + String(int(voltage * 10)) + String(int(battery_temp * 10)) + String(int(motor_temp * 10));
  if (millis() > timer) {
    Serial1.println(send);
    timer = millis() + 1000;
  }
  store = String(float(millis()) / 60000) + String(speed) + String(",") + String(current) + String(",") + String(av_current) + String(",") + String(voltage) + String(",") + String(battery_temp) + String(",") + String(motor_temp);
  File dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(store);
    dataFile.close();
  }
  //print values on LCDs
  if (Serial1.available() > 0) {
    Serial1.readBytes(buffer, 100);
    buffer[15] = 0;
    if (String(buffer).equals(String("reset"))) {
      digitalWrite(resetPin, LOW);
    }
    message = String(buffer);
  }
  display = String("Speed: ") + String(speed) + String(" mph, Current: ") + String(current) + String(" A, Av Current: ") + String(av_current) + String(" A, Voltage: ") + String(voltage) + String(" V.");
  lcd_data.println(display);
  lcd_message.println(message);
  /*
  lcd_data.setCursor(0, 0);
  lcd_data.print("Speed: ");
  lcd_data.print(speed);
  lcd_data.print("mph");
  lcd_data.setCursor(0, 1);
  lcd_data.print("Current: ");
  lcd_data.print(current);
  lcd_data.print("A");
  lcd_data.setCursor(0, 2);
  lcd_data.print("Voltage: ");
  lcd_data.print(voltage);
  lcd_data.print("V");
  lcd_message.print(message);
  */
}
