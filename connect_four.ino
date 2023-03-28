#define LED_PIN 9
#define NUM_LEDS 256
#define LED_ROWS 16
#define LED_COLS 16
#define GAME_ROWS 7
#define GAME_COLS 7
#define NUM_BUTTONS 4

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

Square grid[GAME_ROWS][GAME_COLS];
Square bigGrid[LED_ROWS][LED_COLS];
const byte buttonPins[] = {3, 4, 5};
byte placementCol = 3;
bool p1Turn = true;

byte translateToBigRow(byte row) {
  return 2 + row * 2;
}

byte translateToBigCol(byte col) {
  return 1 + col * 2;
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

  //     currentPosition.set(row, col);

  //     if (snake.body[0].row() == currentPosition.row() && snake.body[0].col() == currentPosition.col()) {
  //       // Snake's head
  //       Serial.print("H ");
  //     } else if (snake.occupiesPosition(currentPosition)) {
  //       // Snake's body
  //       Serial.print("S ");
  //     } else if (food.row() == currentPosition.row() && food.col() == currentPosition.col()) {
  //       // Food
  //       Serial.print("F ");
  //     } else {
  //       // Empty cell
  //       Serial.print(". ");
  //     }
  //   }
  //   Serial.println(); // Move to the next row
  // }
  Serial.println(); // Add an extra line to separate board states
}

void set(byte row, byte col, Square val) {
  grid[row][col] = val;
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
  Serial.begin(9600);
  Serial.println();
  set(5, 1, P1_TILE);
  set(6, 1, P2_TILE);

  //setup pin change interrupts
  PCICR |= B00000100;
  initializeButtons(buttonPins);

  
  printBoard();

}

void drop(byte col)
{
  byte curRow = 0;
  // set()

  while (curRow != GAME_ROWS && grid[curRow][col] == EMPTY)
  {
    curRow++;
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

  printBoard();




  // if (newDirection != snake.direction) {
  //   snake.direction = newDirection;
  //   snake.directionLocked = true;
  // }
}

ISR (PCINT2_vect) {
  Serial.println("pressed");
  for (int i = 0; i < NUM_BUTTONS; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      handleButtonPressed(buttonPins[i]);
    }
  }
}

void loop() {}


