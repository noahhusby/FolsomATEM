/*
 FolsomATEM
 Copyright (c) 2021 Noah Husby

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 */
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>    

#include <OSCBundle.h>
#include <OSCBoards.h>

#define ledLatch 8
#define ledData 11
#define ledClock 12

// LED Rows
boolean previewRow [16] = {false};
boolean programRow [16] = {false};
boolean auxRow [16] = {false};
boolean macrowRow [16] = {false};

// OSC UDP
EthernetUDP udp;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress controllerIp(192, 168, 1, 81);
IPAddress computerIp(192, 168, 1, 82);
const unsigned int inPort = 8888;
const unsigned int outPort = 3333;

boolean oscEnabled = false;

void setup() {
  pinMode(ledLatch, OUTPUT);
  pinMode(ledClock, OUTPUT);
  pinMode(ledData, OUTPUT);

  delay(500);

  //initOsc();

  // Prepare LEDS
  digitalWrite(ledData, LOW);
  digitalWrite(ledClock, LOW);
  digitalWrite(ledLatch, HIGH);
}

void loop() { 
  if(oscEnabled) {
    OSCBundle bundleIn;
    int size;
    if((size = udp.parsePacket()) > 0) {
      while(size--) {
        bundleIn.fill(udp.read());
      }
      if(!bundleIn.hasError()) {
        // Add routes here
        bundleIn.route("/atem/program", routeProgram);
      }
    }
  }
  for(int i = 0; i < 16; i++) {
    setPreviewLeds(i);
    updateLeds();
    delay(100);
  }
  delay(1000);
}

/**
 * OSC Methods
 */
void initOsc() {
  Ethernet.begin(mac, controllerIp);
  udp.begin(inPort);
  oscEnabled = true;
}

void sendMessage(OSCMessage msg) {
  udp.beginPacket(computerIp, outPort);
  msg.send(udp);
  udp.endPacket();
  msg.empty();
}

/**
 * OSC Routes
 */
void routeProgram(OSCMessage &msg, int addrOffset) {
  
}

/**
 * LED Methods
 */
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
  digitalWrite(ledLatch, LOW);
  shift(getFromBits(programRow, previewRow, 11));
  shift(getFromBits(macrowRow, auxRow, 11));
  shift(getFromBits(programRow, previewRow, 7));
  shift(getFromBits(macrowRow, auxRow, 7));
  shift(getFromBits(programRow, previewRow, 3));
  shift(getFromBits(macrowRow, auxRow, 3));
  shift(getFromBits(programRow, previewRow, 0));
  shift(getFromBits(macrowRow, auxRow, 0));
  digitalWrite(ledLatch, HIGH);
}


void shift(uint8_t b) {
  shiftOut(ledData, ledClock, MSBFIRST, b);
}

uint8_t getFromBits(boolean arrayA[], boolean arrayB[], int start) {
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
