#define LED_PIN 9
#define NUM_LEDS 256
#define LED_ROWS 16
#define LED_COLS 16
#define GAME_ROWS 7
#define GAME_COLS 7
#define NUM_BUTTONS 3

#include <FastLED.h>

enum Square {
  EMPTY,
  P1_TILE,
  P2_TILE,
};
// (row, col) -> (2 + row * 2, 1 + col * 2)

struct Coordinate {
  uint8_t data; // 4 bits for row, 4 bits for col

  uint8_t row() const { return (data >> 4) & 0x0F; }
  uint8_t col() const { return data & 0x0F; }

  void set(uint8_t newRow, uint8_t newCol) {
    data = (newRow << 4) | (newCol & 0x0F);
  }
};

CRGB LEDs[NUM_LEDS];
Square grid[GAME_ROWS][GAME_COLS];
Square bigGrid[LED_ROWS][LED_COLS];
byte brightnessLevel = 1;

const byte buttonPins[] = {3, 4, 5};
const unsigned long debounceDelay = 200; // debounce time in milliseconds
unsigned long lastDebounceTime = 0; // initializes a variable to store last debounce time

byte placementCol = 3;
bool p1Turn = true;

byte translateToBigRow(byte row) {
  return 2 + row * 2;
}

byte translateToBigCol(byte col) {
  return 1 + col * 2;
}

int coordToLEDIndex(byte row, byte col) {
  return row * LED_COLS + (row % 2 ? col : LED_COLS - 1 - col);
}

void printBoard() {
  for (int row = 0; row < LED_ROWS; ++row) {
    for (int col = 0; col < LED_COLS; ++col) {
      Square currentSquare = bigGrid[row][col];
      if ((col == 0 || col == LED_COLS - 1) && row > 1) {
        Serial.print("H ");
      } 
      else if ((col == translateToBigCol(placementCol) || col == translateToBigCol(placementCol) + 1) && row < 2) {
        Serial.print(p1Turn ? "1 " : "2 ");
      }
      else if (currentSquare == P1_TILE) {
        Serial.print("1 ");
      } else if (currentSquare == P2_TILE) {
        Serial.print("2 ");
      } else {
        Serial.print(". ");
      }
    }
    Serial.println();
  }

  Serial.println();
  Serial.println();

}

void updateBoard() {
  for (int row = 0; row < LED_ROWS; ++row) {
    for (int col = 0; col < LED_COLS; ++col) {
      Square currentSquare = bigGrid[row][col];
      int LEDIndex = coordToLEDIndex(row, col);

      if ((col == 0 || col == LED_COLS - 1) && row > 1) {
        LEDs[LEDIndex] = CRGB(0, 0, brightnessLevel);
      } 
      // else if ((col == translateToBigCol(placementCol) || col == translateToBigCol(placementCol) + 1) && row < 2) {
      //   LEDs[LEDIndex] = p1Turn ? CRGB(6, 0, 0) : CRGB(3, 3, 0);
      // }
      else if (currentSquare == P1_TILE) {
        LEDs[LEDIndex] = CRGB(2 * brightnessLevel, 0, 0);
      } else if (currentSquare == P2_TILE) {
        LEDs[LEDIndex] = CRGB(brightnessLevel, brightnessLevel, 0);
      } else {
        LEDs[LEDIndex] = CRGB(0, 0, 0);
      }
    }
  }
  FastLED.show();
}


void set(byte row, byte col, Square val) {
  if (row >= 0 && col >= 0) {
    grid[row][col] = val;
  }
  byte bigRow = translateToBigRow(row);
  byte bigCol = translateToBigCol(col);
  bigGrid[bigRow][bigCol] = val;
  bigGrid[bigRow + 1][bigCol] = val;
  bigGrid[bigRow][bigCol + 1] = val;
  bigGrid[bigRow + 1][bigCol + 1] = val;
} 

void initializeButtons(byte* buttonPins) {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    // make sure to attach pullup resistors before enabling interrupts, otherwise
    // this triggers the interrupts a few times
    pinMode(buttonPins[i], INPUT_PULLUP);
    // turn on interrupts for specific pins
    PCMSK2 |= 1 << buttonPins[i];
  }
}

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, LED_PIN, GRB>(LEDs, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();


  Serial.begin(9600);
  Serial.println();

  //setup pin change interrupts
  // PCICR |= B00000100;
  initializeButtons(buttonPins);

  // printBoard();
  set(-1, placementCol, p1Turn ? P1_TILE : P2_TILE);
  updateBoard();

}

void drop(byte col)
{
  byte curRow = 0;
  // set()

  while (curRow != GAME_ROWS && grid[curRow][col] == EMPTY)
  {
    set(curRow, col, p1Turn ? P1_TILE : P2_TILE);
    set(curRow - 1, col, EMPTY);
    updateBoard();
    curRow++;
    delay(50);
    //timedelay
  }
  if (curRow > 0)
  {
    set(curRow - 1, col, p1Turn ? P1_TILE : P2_TILE);
    p1Turn = !p1Turn;
    return;
  }
}

void handleButtonPressed(byte pin) {
  Serial.println("pressed");
  byte oldPlacementCol = placementCol;
  switch (pin) {
    case 3:
      drop(placementCol);
      break;
    case 4:
      placementCol -= placementCol ? 1 : 0;
      break;
    case 5:
      placementCol += placementCol == GAME_COLS - 1 ? 0 : 1;
      break;
  }

  set(-1, oldPlacementCol, EMPTY);
  set(-1, placementCol, p1Turn ? P1_TILE : P2_TILE);

  // printBoard();
  updateBoard();
}

bool debounceCheck() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    return true;
  }
  return false;
}

void loop() {
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttonPins[i]) == LOW && debounceCheck()) {
      handleButtonPressed(buttonPins[i]);
      break;
    }
  }
  Serial.println(placementCol);
}


