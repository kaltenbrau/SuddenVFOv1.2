// Morse Code Keyer (C) 2017 Doug Hoyte
// 2-Clause BSD License

//#include <EEPROM.h>

// PINS

const int pinMem1 = 0;
const int pinMem2 = 1;                                                                               ;
const int pinSpeedToneControl = 2;
const int pinKeyDit = 16;
const int pinKeyDah = 15;

const int pinStatusLed = 22;
const int pinMosfet = 17;
const int pinSpeaker = 23;

// I accidentally used a default closed push-button for memory 2 button. Change this to 0 if you did it properly :)
#define PINMEM2_INVERTED 0

// STATES

const int stateIdle = 0;
const int stateSettingSpeed = 1;
const int stateSettingTone = 2;

// KEYER MODE

const int keyerModeIambic = 0;
const int keyerModeVibroplex = 1;
const int keyerModeStraight = 2;

// SYMBOLS

const int symDit = 1;
const int symDah = 2;

// SAVE PACKET TYPES

const int packetTypeEnd = 0;
const int packetTypeSpeed = 1;
const int packetTypeFreq = 2;
const int packetTypeKeyerModeIambic = 3;
const int packetTypeKeyerModeVibroplex = 4;
const int packetTypeKeyerModeStraight = 5;
const int packetTypeMem0 = 20;
const int packetTypeMem1 = 21;
const int packetTypeMem2 = 22;

// STORAGE STUFF

const int storageSize = 2048;
const int storageMagic1 = 182;
const int storageMagic2 = 97;

// SAVED STATE

int toneFreq = 1000;
int ditMillis = 60;
int currKeyerMode = keyerModeIambic;

char memory[3][600];
size_t memorySize[3];


// RUN STATE

int currState = stateIdle;
int prevSymbol = 0; // 0=none, 1=dit, 2=dah
unsigned long whenStartedPress;
int recording = 0;
int currStorageOffset = 0;

void dumpSettingsToStorage();

void saveStorageEmptyPacket(int type) {
  if (currStorageOffset + 1 >= storageSize) {
    dumpSettingsToStorage();
    return;
  }
  EEPROM.write(currStorageOffset++, type);
  EEPROM.write(currStorageOffset, packetTypeEnd);
}

void saveStorageInt(int type, int value) {
  if (currStorageOffset + 1 + 2 >= storageSize) {
    dumpSettingsToStorage();
    return;
  }
  EEPROM.write(currStorageOffset++, type);
  EEPROM.write(currStorageOffset++, (value >> 8) & 0xFF);
  EEPROM.write(currStorageOffset++, value & 0xFF);
  EEPROM.write(currStorageOffset, packetTypeEnd);
}

void saveStorageMemory(int memoryId) {
  if (currStorageOffset + 1 + 2 + memorySize[memoryId] >= storageSize) {
    dumpSettingsToStorage();
    return;
  }

  int type = 0;
  if (memoryId == 0) type = packetTypeMem0;
  else if (memoryId == 1) type = packetTypeMem1;
  else if (memoryId == 2) type = packetTypeMem2;

  EEPROM.write(currStorageOffset++, type);
  EEPROM.write(currStorageOffset++, (memorySize[memoryId] >> 8) & 0xFF);
  EEPROM.write(currStorageOffset++, memorySize[memoryId] & 0xFF);

  for (size_t i = 0; i < memorySize[memoryId]; i++) EEPROM.write(currStorageOffset++, memory[memoryId][i]);

  EEPROM.write(currStorageOffset, packetTypeEnd);
}


void dumpSettingsToStorage() {
  currStorageOffset = 2;
  saveStorageInt(packetTypeSpeed, ditMillis);
  saveStorageInt(packetTypeFreq, toneFreq);
  if (currKeyerMode == keyerModeVibroplex) saveStorageEmptyPacket(packetTypeKeyerModeVibroplex);
  else if (currKeyerMode == keyerModeStraight) saveStorageEmptyPacket(packetTypeKeyerModeStraight);
  if (memorySize[0]) saveStorageMemory(0);
  if (memorySize[1]) saveStorageMemory(1);
  if (memorySize[2]) saveStorageMemory(2);
}




int delayInterruptable(int ms, int *pins, int *conditions, size_t numPins) {
  unsigned long finish = millis() + ms;

  while (1) {
    if (ms != -1 && millis() > finish) return -1;

    for (size_t i = 0; i < numPins; i++) {
      if (digitalRead(pins[i]) == conditions[i]) return pins[i];
    }
  }
}

void waitPin(int pin, int condition) {
  int pins[1] = { pin };
  int conditions[1] = { condition };
  delayInterruptable(-1, pins, conditions, 1);
  delay(250); // debounce
}

int playSymInterruptableVec(int sym, int transmit, int *pins, int *conditions, size_t numPins) {
  prevSymbol = sym;

  tone(pinSpeaker, toneFreq);
  digitalWrite(pinStatusLed, recording ? LOW : HIGH);

  // From version 0.11 the Tx clk output (clk0) is enabled on key down
  // from version 0.12 the Rx clk output (clk2) is disabled on key down
  if (transmit) {
    si5351.output_enable(SI5351_CLK2, 0);
    si5351.output_enable(SI5351_CLK0, 1);
    if (flag.vfo_A_active == true) {
      si5351.set_freq((u.vfo_A_value * 100ULL), SI5351_CLK0);
    }
    if (flag.vfo_B_active == true) {
      si5351.set_freq((u.vfo_B_value * 100ULL), SI5351_CLK0);
    }
    digitalWrite(pinMosfet, HIGH);
  }

  int ret = delayInterruptable(ditMillis * (sym == symDit ? 1 : 3), pins, conditions, numPins);

  noTone(pinSpeaker);
  digitalWrite(pinStatusLed, recording ? HIGH : LOW);

  // From version 0.11 the Tx clk output (clk0) is enabled on key down
  // from version 0.12 the Rx clk output (clk2) is disabled on key down
  si5351.output_enable(SI5351_CLK0, 0);
  si5351.output_enable(SI5351_CLK2, 1);
  if (flag.vfo_A_active == true) {
    si5351.set_freq((u.RIT_A_value * 100ULL), SI5351_CLK0);
  }
  if (flag.vfo_B_active == true) {
    si5351.set_freq((u.RIT_B_value * 100ULL), SI5351_CLK0);
  }

  digitalWrite(pinMosfet, LOW);

  if (ret != -1) return ret;

  ret = delayInterruptable(ditMillis, pins, conditions, numPins);
  if (ret != -1) return ret;

  return -1;
}

void playSym(int sym, int transmit) {
  playSymInterruptableVec(sym, transmit, NULL, NULL, 0);
}

int playSymInterruptable(int sym, int transmit, int pin, int condition) {
  int pins[1] = { pin };
  int conditions[1] = { condition };
  return playSymInterruptableVec(sym, transmit, pins, conditions, 1);
}

void memRecord(int memoryId, int value) {
  memory[memoryId][memorySize[memoryId]] = value;
  memorySize[memoryId]++;
}

void setMemory(int memoryId, int pin, int inverted) {
  memorySize[memoryId] = 0;
  digitalWrite(pinStatusLed, HIGH);
  recording = 1;

  unsigned long spaceStarted = 0;

  while (1) {
    int ditPressed = (digitalRead(pinKeyDit) == LOW);
    int dahPressed = (digitalRead(pinKeyDah) == LOW);

    if ((ditPressed || dahPressed) && spaceStarted) {
      // record a space
      double spaceDuration = millis() - spaceStarted;
      spaceDuration /= ditMillis;
      spaceDuration += 2.5;
      int toRecord = spaceDuration;
      if (toRecord > 255) toRecord = 255;
      memRecord(memoryId, toRecord);
      spaceStarted = 0;
    }

    if (ditPressed && dahPressed) {
      if (prevSymbol == symDah) {
        playSym(symDit, 0);
        memRecord(memoryId, 0);
      } else {
        playSym(symDah, 0);
        memRecord(memoryId, 1);
      }
    } else if (ditPressed) {
      playSym(symDit, 0);
      memRecord(memoryId, 0);
    } else if (dahPressed) {
      playSym(symDah, 0);
      memRecord(memoryId, 1);
    } else {
      if (prevSymbol) {
        spaceStarted = millis();
        prevSymbol = 0;
      }
    }

    if (memorySize[memoryId] >= sizeof(memory[memoryId]) - 2) break; // protect against overflow

    if (digitalRead(pin) == (inverted ? HIGH : LOW)) {
      delay(50);
      waitPin(pin, inverted ? LOW : HIGH);
      break;
    }
  }

  saveStorageMemory(memoryId);

  digitalWrite(pinStatusLed, LOW);
  recording = 0;

  tone(pinSpeaker, 1300);
  delay(300);
  tone(pinSpeaker, 900);
  delay(300);
  tone(pinSpeaker, 2000);

  for (int i = 0; i <= memoryId; i++) {
    digitalWrite(pinStatusLed, HIGH);
    delay(150);
    digitalWrite(pinStatusLed, LOW);
    delay(150);
  }

  noTone(pinSpeaker);
}

void playMemory(int memoryId) {
  if (memorySize[memoryId] == 0) {
    tone(pinSpeaker, 800);
    delay(200);
    tone(pinSpeaker, 500);
    delay(300);
    noTone(pinSpeaker);
    return;
  }

  int pins[2] = { pinKeyDit, pinKeyDah };
  int conditions[2] = { LOW, LOW };

  for (size_t i = 0; i < memorySize[memoryId]; i++) {
    int cmd = memory[memoryId][i];

    if (cmd == 0) {
      int ret = playSymInterruptableVec(symDit, 1, pins, conditions, 2);
      if (ret != -1) {
        delay(10);
        waitPin(ret, HIGH);
        return;
      }
    } else if (cmd == 1) {
      int ret = playSymInterruptableVec(symDah, 1, pins, conditions, 2);
      if (ret != -1) {
        delay(10);
        waitPin(ret, HIGH);
        return;
      }
    } else {
      int duration = cmd - 2;
      duration *= ditMillis;
      delay(duration);
    }
  }
}

void checkMemoryPin(int memoryId, int pin, int inverted) {
  if (digitalRead(pin) == (inverted ? HIGH : LOW)) {
    unsigned long whenStartedPress = millis();
    int doingSet = 0;

    delay(5);

    while (digitalRead(pin) == (inverted ? HIGH : LOW)) {
      if (millis() > whenStartedPress + 1000) {
        digitalWrite(pinStatusLed, HIGH);
        doingSet = 1;
      }
    }

    digitalWrite(pinStatusLed, LOW);
    delay(50);

    if (doingSet) setMemory(memoryId, pin, inverted);
    else playMemory(memoryId);
  }
}


int scaleDown(int orig, double factor, int lowerLimit) {
  int scaled = (int)((double)orig * factor);
  if (scaled == orig) scaled--;
  if (scaled < lowerLimit) scaled = lowerLimit;
  return scaled;
}

int scaleUp(int orig, double factor, int upperLimit) {
  int scaled = (int)((double)orig * factor);
  if (scaled == orig) scaled++;
  if (scaled > upperLimit) scaled = upperLimit;
  return scaled;
}




void factoryReset() {
  if (EEPROM.read(0) != storageMagic1) EEPROM.write(0, storageMagic1);
  if (EEPROM.read(1) != storageMagic2) EEPROM.write(1, storageMagic2);
  if (EEPROM.read(2) != packetTypeEnd) EEPROM.write(2, packetTypeEnd);

  currStorageOffset = 2;

  tone(pinSpeaker, 900);
  delay(300);
  tone(pinSpeaker, 600);
  delay(300);
  tone(pinSpeaker, 1500);
  delay(900);
  noTone(pinSpeaker);
}


void loadStorage() {
  int resetRequested = (digitalRead(pinMem1) == LOW && digitalRead(pinSpeedToneControl) == LOW);

  if (resetRequested || EEPROM.read(0) != storageMagic1 || EEPROM.read(1) != storageMagic2) factoryReset();

  currStorageOffset = 2;

  while (1) {
    int packetType = EEPROM.read(currStorageOffset);
    if (packetType == packetTypeEnd) {
      break;
    } else if (packetType == packetTypeSpeed) {
      ditMillis = (EEPROM.read(currStorageOffset + 1) << 8) | EEPROM.read(currStorageOffset + 2);
      currStorageOffset += 2;
    } else if (packetType == packetTypeFreq) {
      toneFreq = (EEPROM.read(currStorageOffset + 1) << 8) | EEPROM.read(currStorageOffset + 2);
      currStorageOffset += 2;
    } else if (packetType == packetTypeKeyerModeIambic) {
      currKeyerMode = keyerModeIambic;
    } else if (packetType == packetTypeKeyerModeVibroplex) {
      currKeyerMode = keyerModeVibroplex;
    } else if (packetType == packetTypeKeyerModeStraight) {
      currKeyerMode = keyerModeStraight;
    } else if (packetType >= packetTypeMem0 && packetType <= packetTypeMem2) {
      int memoryId = 0;
      if (packetType == packetTypeMem0) memoryId = 0;
      if (packetType == packetTypeMem1) memoryId = 1;
      if (packetType == packetTypeMem2) memoryId = 2;
      memorySize[memoryId] = (EEPROM.read(currStorageOffset + 1) << 8) | EEPROM.read(currStorageOffset + 2);
      for (size_t i = 0; i < memorySize[memoryId]; i++) {
        memory[memoryId][i] = EEPROM.read(currStorageOffset + 3 + i);
      }
      currStorageOffset += 2 + memorySize[memoryId];
    }

    currStorageOffset++; // packet type byte
  }
}

void playStraightKey(int releasePin) {
  tone(pinSpeaker, toneFreq);
  digitalWrite(pinStatusLed, HIGH);

  // From version 0.11 the Tx clk output (clk0) is enabled on key down
  // from version 0.12 the Rx clk output (clk2) is disabled on key down
  si5351.output_enable(SI5351_CLK2, 0);
  si5351.output_enable(SI5351_CLK0, 1);
  if (flag.vfo_A_active == true) {
    si5351.set_freq((u.vfo_A_value * 100ULL), SI5351_CLK0);
  }
  if (flag.vfo_B_active == true) {
    si5351.set_freq((u.vfo_B_value * 100ULL), SI5351_CLK0);
  }
  digitalWrite(pinMosfet, HIGH);

  while (digitalRead(releasePin) == LOW) {}

  noTone(pinSpeaker);
  digitalWrite(pinStatusLed, LOW);

  // From version 0.11 the Tx clk output (clk0) is enabled on key down
  // from version 0.12 the Rx clk output (clk2) is disabled on key down
  si5351.output_enable(SI5351_CLK0, 0);
  si5351.output_enable(SI5351_CLK2, 1);
  if (flag.vfo_A_active == true) {
    si5351.set_freq((u.RIT_A_value * 100ULL), SI5351_CLK0);
  }
  if (flag.vfo_B_active == true) {
    si5351.set_freq((u.RIT_B_value * 100ULL), SI5351_CLK0);
  }
  digitalWrite(pinMosfet, LOW);
}

// ****** End of File
