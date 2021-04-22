// LED Rows
boolean previewRow [16] = {false};
boolean programRow [16] = {false};
boolean auxRow [16] = {false};
boolean macrowRow [16] = {false};

int ledLatch = 8;
int ledClock = 12;
int ledData = 11;

void setup() {
  pinMode(ledLatch, OUTPUT);
  pinMode(ledClock, OUTPUT);
  pinMode(ledData, OUTPUT);

  delay(500);

  // Prepare LEDS
  digitalWrite(ledData, LOW);
  digitalWrite(ledClock, LOW);
  digitalWrite(ledLatch, HIGH);
}

void loop() { 
  for(int i = 0; i < 16; i++) {
    setPreviewLeds(i);
    updateLeds();
    delay(100);
  }
  delay(1000);
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
