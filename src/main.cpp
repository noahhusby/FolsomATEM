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

#include <OSCBundle.h>

#define transitionBar A0

#define ledLatchA 22
#define ledDataA 24
#define ledClockA 26

#define ledLatchB 23
#define ledDataB 25
#define ledClockB 27

// Latch, Data, Clock
const int driverA [3] = {ledLatchA, ledDataA, ledClockA};
const int driverB [3] = {ledLatchB, ledDataB, ledClockB};

// LED Rows
boolean previewRow [16] = {false};
boolean programRow [16] = {false};
boolean auxRow [16] = {false};
boolean macrowRow [16] = {false};

boolean groupRow [8] = {false};
boolean keyRow [8] = {false};
boolean pipRow [8] = {false};
boolean functionRow [8] = {false};


// OSC UDP
EthernetUDP udp;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress controllerIp(192, 168, 1, 81);
IPAddress computerIp(192, 168, 1, 238);
const unsigned int inPort = 8888;
const unsigned int outPort = 3333;

boolean oscEnabled = false;

double lastTbarValue = 0;

void initOsc();
void updateLeds();
void shift(uint8_t b, const int driver[]);
void route(OSCBundle &bundle);
void setPreviewLeds(int value);
void setProgramLeds(int value);
void sendMessage(OSCMessage &msg);
void routeProgram(OSCMessage &msg, int addrOffset);
uint8_t getFromBits(const boolean arrayA[], const boolean arrayB[], int start);

void setup() {
    //Serial.begin(9600);
    pinMode(driverA[0], OUTPUT);
    pinMode(driverA[1], OUTPUT);
    pinMode(driverA[2], OUTPUT);
    pinMode(driverB[0], OUTPUT);
    pinMode(driverB[1], OUTPUT);
    pinMode(driverB[2], OUTPUT);

    pinMode(transitionBar, INPUT);

    delay(500);

    initOsc();

    // Prepare LEDs
    digitalWrite(driverA[0], LOW);
    digitalWrite(driverA[1], LOW);
    digitalWrite(driverA[2], LOW);
    digitalWrite(driverB[0], LOW);
    digitalWrite(driverB[1], LOW);
    digitalWrite(driverB[2], LOW);

    for(int i = 0; i < 16; i++) {
        macrowRow[i] = true;
        auxRow[i] = true;
        programRow[i] = true;
        previewRow[i] = true;
        updateLeds();
        delay(50);
    }

    for(int i = 0; i < 8; i++) {
        functionRow[i] = true;
        pipRow[i] = true;
        keyRow[i] = true;
        groupRow[i] = true;
        updateLeds();
        delay(50);
    }

    for(int i = 7; i > -1; i--) {
        functionRow[i] = false;
        pipRow[i] = false;
        keyRow[i] = false;
        groupRow[i] = false;
        updateLeds();
        delay(50);
    }

    for(int i = 15; i > -1; i--) {
        macrowRow[i] = false;
        auxRow[i] = false;
        programRow[i] = false;
        previewRow[i] = false;
        updateLeds();
        delay(50);
    }
};

void loop() {
    if(oscEnabled) {
        OSCBundle bundleIn;
        int size;
        if((size = udp.parsePacket()) > 0) {
            while(size--) {
                bundleIn.fill(udp.read());
            }
            route(bundleIn);
        }
        double tbar = ((int32_t)analogRead(transitionBar)) / 1023.0;
        if(tbar != lastTbarValue) {
            lastTbarValue = tbar;
            OSCMessage msg("/atem/transition/bar");
            msg.add(tbar);
            udp.beginPacket(computerIp, outPort);
            msg.send(udp);
            udp.endPacket();
            msg.empty();
        }
    }
}

/**
 * OSC Methods
 */
void initOsc() {
    EthernetClass::begin(mac, controllerIp);
    udp.begin(inPort);
    oscEnabled = true;
}

void sendMessage(OSCMessage &msg) {
    udp.beginPacket(computerIp, outPort);
    msg.send(udp);
    udp.endPacket();
    msg.empty();
}

/**
 * OSC Routes
 */
void routeProgram(OSCMessage &msg, int addrOffset) {
    if(msg.fullMatch("/atem/program", 0)) {
        int in = msg.getInt(0) - 1;
        if(in > -1 && in < 16) {
            setProgramLeds(in);
            updateLeds();
        }
    }
}

void routePreview(OSCMessage &msg, int addrOffset) {
    if(msg.fullMatch("/atem/preview", 0)) {
        int in = msg.getInt(0) - 1;
        if(in > -1 && in < 16) {
            setPreviewLeds((in));
            updateLeds();
        }
    }
}

void route(OSCBundle &bundle) {
    bundle.route("/atem/program", routeProgram);
    bundle.route("/atem/preview", routePreview);
}


/**
* LED Methods
*/
void setPreviewLeds(int value) {
    for(bool & i : previewRow) {
        i = false;
    }
    previewRow[value] = true;
}

void setProgramLeds(int value) {
    for(bool & i : programRow) {
        i = false;
    }
    programRow[value] = true;
}

void updateLeds() {
    digitalWrite(driverA[0], LOW);
    shift(getFromBits(keyRow, groupRow, 3), driverA);
    shift(getFromBits(functionRow, pipRow, 3), driverA);
    shift(getFromBits(keyRow, groupRow, 0), driverA);
    shift(getFromBits(functionRow, pipRow, 0), driverA);
    digitalWrite(driverA[0], HIGH);
    digitalWrite(driverB[0], LOW);
    shift(getFromBits(programRow, previewRow, 11), driverB);
    shift(getFromBits(macrowRow, auxRow, 11), driverB);
    shift(getFromBits(programRow, previewRow, 7), driverB);
    shift(getFromBits(macrowRow, auxRow, 7), driverB);
    shift(getFromBits(programRow, previewRow, 3), driverB);
    shift(getFromBits(macrowRow, auxRow, 3), driverB);
    shift(getFromBits(programRow, previewRow, 0), driverB);
    shift(getFromBits(macrowRow, auxRow, 0), driverB);
    digitalWrite(driverB[0], HIGH);
}

void shift(uint8_t b, const int driver[]) {
    shiftOut(driver[1], driver[2], MSBFIRST, b);
}

uint8_t getFromBits(const boolean arrayA[], const boolean arrayB[], int start) {
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
