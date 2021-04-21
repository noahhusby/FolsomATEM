#include <SPI.h>

boolean previewRow [16] = {false};
boolean programRow [16] = {false};
boolean auxRow [16] = {false};
boolean macrowRow [16] = {false};

// set pin 10 as the slave select for the digital pot:

int enable = 7;
int latchPin = 8;

void setup() {
  Serial.begin(9600);
  pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);
pinMode(latchPin, OUTPUT);
pinMode(enable, OUTPUT);
pinMode(10, OUTPUT);
digitalWrite(10, LOW);
digitalWrite(enable, HIGH);
digitalWrite(latchPin, LOW);
delay(500);
//SPI.beginTransaction(settings);
digitalWrite(enable, LOW);
}

void loop() {
  for(int i = 0; i < 16; i++) {
    programRow[i] = true;
    updateLeds();
    delay(100);
  }
  for(int i = 0; i < 16; i++) {
    programRow[i] = false;
    updateLeds();
    delay(100);
  }
}

void setPreviewLeds(int value) {
  for(int i = 0; i < 16; i++) {
    previewRow[i] = false;
  }
  previewRow[value] = true;
}

void setProgramLeds(int value) {
  for(int i = 0; i < 16; i++) {
    programRow[i] = false;
  }
  programRow[value] = true;
}

void updateLeds() {
  digitalWrite(latchPin, LOW);
  shift(getFromBits(programRow, previewRow, 11));
  shift(getFromBits(macrowRow, auxRow, 11));
  shift(getFromBits(programRow, previewRow, 7));
  shift(getFromBits(macrowRow, auxRow, 7));
  shift(getFromBits(programRow, previewRow, 3));
  shift(getFromBits(macrowRow, auxRow, 3));
  shift(getFromBits(programRow, previewRow, 0));
  shift(getFromBits(macrowRow, auxRow, 0));
  digitalWrite(latchPin, HIGH);
 
}

void shift(byte b) {
  shiftOut(11, 13, MSBFIRST, b);
}

byte getFromBits(boolean arrayA[], boolean arrayB[], int start) {
  byte x = 0b00000000; 
  int c = 0;
  for(int i = start; i < start + 4; i++) {
    bitWrite(x, c, arrayA[i]);
    c++;
  }
  for(int i = start; i < start + 4; i++) {
    bitWrite(x, c, arrayB[i]);
    c++;
  }
  return x;
}
