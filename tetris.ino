#include "LedControl.h"
#include "binary.h"
#define DNNPIN 12
#define CLKPIN 11
#define CSPIN 10
#define XPIN A0
#define YPIN A1

LedControl lc=LedControl(DNNPIN, CSPIN, CLKPIN, 1);

bool points[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0, 0, 0}
};

int blockSize = 4;
int currentBlockX[4] = {0,0,0,0};
int currentBlockY[4] = {0,0,0,0};

bool falling = false;
bool gameOver = false;
int counter = 0;
int currentRotation = 0;

void setup() {
  lc.shutdown(0, false);
  lc.setIntensity(0, 1);
  lc.clearDisplay(0);

  pinMode(XPIN, INPUT);
  pinMode(YPIN, INPUT);

  Serial.begin(9600);
}

void loop() {
  calculateScreen();
  if(falling) {
    moveBlockDown();
    moveBlockX();
    rotate();
    delay(100);
  } else if(!gameOver) {
    createNewBlock();
  }
}

void createNewBlock() {
  if(points[0][7] || points[1][7]) {
    clearGame();
    return;
  }
  falling = true;
  currentRotation = 0;
  long rdm = random(0, 3);
  if(rdm == 0) {
    currentBlockX[0] = 0;
    currentBlockX[1] = 1;
    currentBlockX[2] = 0;
    currentBlockX[3] = 1;
    currentBlockY[0] = 7;
    currentBlockY[1] = 7;
    currentBlockY[2] = 7;
    currentBlockY[3] = 7;
  } else if(rdm == 1) {
    currentBlockX[0] = 1;
    currentBlockX[1] = 1;
    currentBlockX[2] = 0;
    currentBlockX[3] = 1;
    currentBlockY[0] = 8;
    currentBlockY[1] = 8;
    currentBlockY[2] = 7;
    currentBlockY[3] = 7;
  } else if(rdm == 2) {
    currentBlockX[0] = 0;
    currentBlockX[1] = 0;
    currentBlockX[2] = 0;
    currentBlockX[3] = 0;
    currentBlockY[0] = 7;
    currentBlockY[1] = 8;
    currentBlockY[2] = 7;
    currentBlockY[3] = 8;
  }
}
  

void moveBlockDown() {
  if(counter == 5 || analogRead(XPIN) > 1020) {
    counter = 0;
  } else {
    counter++;
    return;
  }
  for(int i = 0; i < blockSize; i++) {
    int x = currentBlockX[i];
    int y = currentBlockY[i];
    if(points[x][y - 1] || y == 0) {
      solidBlock();
      createNewBlock();
      return;
    }
  }
  for(int i = 0; i < blockSize; i++) {
    currentBlockY[i] -= 1;
  }
}

void moveBlockX() {
  bool passLeft = false;
  bool passRight = false;
  for(int i = 0; i < blockSize; i++) {
    int x = currentBlockX[i];
    int y = currentBlockY[i];
    if(analogRead(YPIN) > 1020 && x > 0 && !points[x - 1][y]) {
      passLeft = true;
    } else if(passLeft) {
      passLeft = false;
      break;
    }
    if(analogRead(YPIN) < 20 && x < 7 && !points[x + 1][y]) {
      passRight = true;
    } else if(passRight) {
      passRight = false;
      break;
    }
  }
  for(int i = 0; i < blockSize; i++) {
    if(passLeft) {
      currentBlockX[i] -= 1;
    } else if(passRight) {
      currentBlockX[i] += 1;
    }
  }
}

void rotate() {
  if(analogRead(XPIN) > 20) {
    return;
  }
  //todo: rotation
  int y0 = currentBlockY[0];
  int y1 = currentBlockY[1];
  int y2 = currentBlockY[2];
  int y3 = currentBlockY[3];
  if(currentRotation == 0) {
    currentBlockY[0] = y3;
    currentBlockY[1] = y2;
    currentBlockY[2] = y1;
    currentBlockY[3] = y0;
  }
  currentRotation++;
  if(currentRotation > 0) {
    currentRotation = 0;
  }
}

void solidBlock() {
  for(int i = 0; i < blockSize; i++) {
    points[currentBlockX[i]][currentBlockY[i]] = true;
  }
  checkLine();
}

void checkLine() {
  for(int y = 0; y < 8; y++) {
    int i = 0;
    for(int x = 0; x < 8; x++) {
      if(points[x][y]) {
        i++;
      }
    }
    if(i == 8) {
      for(int y2 = y; y2 < 7; y2++) {
        for(int x = 0; x < 8; x++) {
          points[x][y2] = points[x][y2 + 1];
        }
      }
      checkLine();
      return;
    }
  }
}

void calculateScreen() {
  for(int x = 0; x < 8; x++) {
    for(int y = 0; y < 8; y++) {
      lc.setLed(0, x, y, points[x][y]);
      for(int i = 0; i < blockSize; i++) {
        if(currentBlockX[i] == x && currentBlockY[i] == y) {
          lc.setLed(0, x, y, true);
        }
      }
    }
  }
}

void clearGame() {
  gameOver = true;
  for(int y = 0; y < 8; y++) {
    for(int x = 0; x < 8; x++) {
      points[x][y] = 0;
      delay(10);
      calculateScreen();
    }
  }
}
