/*
  17 switch keyboard

          B5(9)   B6(14)  B2(16)   B3(10)
  __________________________________________
  B4(8) |backspace  /       *        -
  E6(7) |   7       8       9       N/A
  D7(6) |   4       5       6        +
  C6(5) |   1       2       3       N/A
  D4(4) |  N/A      0       .      Enter

  LCD I2C 16x2
  ____________
  GND  GND
  VCC  VCC
  2    SDA
  3    SCL

  enter - first press - shows result
        - second press - repeat the operation...
        - changes value of "left" to result of the operation

  del / num - press - deletes last pressed character
            - press after the operation changes value of "left" to result of the operation and deletes last character
            - hold - changes mode, saves left, middle, right
            - hold after the operation changes value of "left" to result of the operation and changes mode

  DISPLAY
  fp64lib 64 bit float  - IEE754 double with few compromises
  display up max        - 12 + 1 + 1 + 2 (numbers + positive/negative + operation + dots)
  display down          - 12 + 1 + 1 (result (truncated to 12 chars) + positive/negative + dot)
*/

#include <Keyboard.h>

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

#include <fp64lib.h>

const byte rows[] = {8, 7, 6, 5, 4};
const int rowcount = sizeof(rows) / sizeof(rows[0]);

const byte cols[] = {9, 14, 16, 10};
const int colcount = sizeof(cols) / sizeof(cols[0]);

const uint8_t keymap[rowcount][colcount] = {{8, '/', '*', '-'},
                                            {'7', '8', '9', 32},
                                            {'4', '5', '6', '+'},
                                            {'1', '2', '3', 32},
                                            {32, '0', '.', 10}};

bool state[rowcount][colcount] = {};
unsigned long debounce[rowcount][colcount] = {0};
const byte debouncedelay = 6;

bool hybrid = 1;

String left = "", right = "", right2 = "";
byte middle = 0, middle2;
bool afterenter = 0;
byte shiftright = 0;

void setup() {
  delay(100);

  for (int k = 0; k < rowcount; k++)
    for (int m = 0; m < colcount; m++)
      state[k][m] = 1;

  for (int i = 0; i < rowcount; i++)
    pinMode(rows[i], INPUT);

  for (int j = 0; j < colcount; j++)
    pinMode(cols[j], INPUT_PULLUP);

  Wire.begin();
  lcd.begin(16, 2);
  lcd.noBlink();
  lcd.noCursor();
  lcd.backlight();
  lcd.setCursor(0, 0);

  Keyboard.begin();
}

void printingsequencetoprow() {
  String topStr = left;

  if (middle)
    topStr += char(middle);
  topStr += right;
  if (middle2)
    topStr += char(middle2);
  topStr += right2;

  lcd.clear();
  lcd.setCursor(0, 0);

  if (topStr.length() > 16) {
    lcd.print(topStr.substring(topStr.length() - 16));
  } else {
    lcd.print(topStr);
  }
}

void printingsequencebottomrow() {
  lcd.setCursor(0, 1);
  lcd.print(left);
}

void deletelastchar() {
  if (middle == 0) {
    int lastIndex = left.length() - 1;
    left.remove(lastIndex);
  } else if (right.toFloat() == 0) {
    middle = 0;
  } else if (middle2 == 0) {
    int lastIndex = right.length() - 1;
    right.remove(lastIndex);
  } else if (right2.toFloat() == 0) {
    middle2 = 0;
  } else {
    int lastIndex = right2.length() - 1;
    right2.remove(lastIndex);
  }
}

void calculate() {
  if (middle == '/') {
    left = fp64_to_string(fp64_div(fp64_atof(&left[0]), fp64_atof(&right[0])),
                          14, 14);
    if (right.toFloat() == 0) {
      lcd.clear();
      lcd.print("nie ma tak dobrze");
    }
  }
  if (middle == '*') {
    left = fp64_to_string(fp64_mul(fp64_atof(&left[0]), fp64_atof(&right[0])),
                          14, 14);
  }
  if (middle == '-') {
    left = fp64_to_string(fp64_sub(fp64_atof(&left[0]), fp64_atof(&right[0])),
                          14, 14);
  }
  if (middle == '+') {
    left = fp64_to_string(fp64_add(fp64_atof(&left[0]), fp64_atof(&right[0])),
                          14, 14);
  }
}

void calculate2() {
  if (middle2 != 0) {
    if (middle2 == '/') {
      right = fp64_to_string(
          fp64_div(fp64_atof(&right[0]), fp64_atof(&right2[0])), 14, 14);
      if (right2.toFloat() == 0) {
        lcd.clear();
        lcd.print("nie ma tak dobrze");
      }
    }
    if (middle2 == '*') {
      right = fp64_to_string(
          fp64_mul(fp64_atof(&right[0]), fp64_atof(&right2[0])), 14, 14);
    }
    if (middle2 == '-') {
      right = fp64_to_string(
          fp64_sub(fp64_atof(&right[0]), fp64_atof(&right2[0])), 14, 14);
    }
    if (middle2 == '+') {
      right = fp64_to_string(
          fp64_add(fp64_atof(&right[0]), fp64_atof(&right2[0])), 14, 14);
    }
  }
}

void handleDot() {
  if (middle == 0) {
    if (left.indexOf('.') == -1) {
      if (left.toFloat())
        left += ".";
      else
        left += "0.";
    }
  } else if (middle2 == 0) {
    if (right.indexOf('.') == -1) {
      if (right.toFloat())
        right += ".";
      else
        right += "0.";
    }
  } else {
    if (right2.indexOf('.') == -1) {
      if (right2.toFloat())
        right2 += ".";
      else
        right2 += "0.";
    }
  }
}

void handleDigit(char key) {
  if (middle == 0) {
    left += char(key);
  } else if (middle2 == 0) {
    right += char(key);
  } else {
    right2 += char(key);
  }
}

void handleHighPrecedence(char key) {
  if (middle == 0) {
    middle = key;
    if (left.toFloat() == 0)
      left = "0";
  } else if (right.toFloat() == 0) {
    middle = key;
  } else if (middle2 == 0) {
    if (middle == '*' || middle == '/') {
      calculate();
      middle = key;
      right = "";
    } else {
      middle2 = key;
    }
  } else if (right2.toFloat() == 0) {
    middle2 = key;
  } else {
    calculate2();
    middle2 = key;
    right2 = "";
  }
}

void handleLowPrecedence(char key) {
  if (middle == 0) {
    middle = key;
    if (left.toFloat() == 0)
      left = "0";
  } else if (right.toFloat() == 0) {
    middle = key;
  } else if (middle2 == 0) {
    calculate();
    middle = key;
    right = "";
  } else if (right2.toFloat() == 0) {
    middle2 = key;
  } else {
    calculate2();
    calculate();
    middle = key;
    right2 = "";
    middle2 = 0;
    right = "";
  }
}

bool checkToggleToCalculator(byte col) {
  int temp = 0;
  while (digitalRead(col) == 0) {
    temp += 1;
    delay(30);
    if (temp == 100) {
      hybrid = 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("calculator");
      delay(500);
      printingsequencetoprow();
      return true;
    }
  }
  return false;
}

bool checkToggleToNumpad(byte col) {
  int temp = 0;
  while (digitalRead(col) == 0) {
    temp += 1;
    delay(30);
    if (temp == 100) {
      hybrid = 0;
      if (afterenter == 1) {
        middle = 0;
        right = "";
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("numpad");
      delay(500);
      lcd.clear();
      return true;
    }
  }
  return false;
}

void processNumpadKey(char key, byte col) {
  Keyboard.press(key);
  if (key == 8) {
    checkToggleToCalculator(col);
  }
}

void processCalculatorKey(char key, byte col) {
  if (afterenter == 0) {
    if (key == 46) {
      handleDot();
    } else if (key > 47 && key < 58) {
      handleDigit(key);
    } else if (key == '*' || key == '/') {
      handleHighPrecedence(key);
    } else if (key == '+' || key == '-') {
      handleLowPrecedence(key);
    } else if (key == 10) {
      printingsequencetoprow();
      calculate2();
      calculate();
      afterenter = 1;
      printingsequencebottomrow();
      return;
    } else if (key == 8) {
      bool toggled = checkToggleToNumpad(col);
      if (!toggled) {
        deletelastchar();
      } else {
        return;
      }
    }

    if (key != 10) {
      printingsequencetoprow();
    }

  } else {
    if ((key > 47 && key < 58) || key == 46) {
      lcd.clear();
      if (key == 46)
        left = "0.";
      else
        left = char(key);
      middle = 0;
      right = "";
      middle2 = 0;
      right2 = "";
      afterenter = 0;
      printingsequencetoprow();
    } else if (key == '*' || key == '/' || key == '+' || key == '-') {
      middle = key;
      right = "";
      middle2 = 0;
      right2 = "";
      afterenter = 0;
      printingsequencetoprow();
    } else if (key == 10) {
      printingsequencetoprow();
      if (middle2 == 0)
        calculate();
      else {
        middle = middle2;
        right = right2;
        middle2 = 0;
        right2 = "";
        printingsequencetoprow();
        calculate();
      }
      printingsequencebottomrow();
    } else if (key == 8) {
      bool toggled = checkToggleToNumpad(col);
      if (!toggled) {
        right = "";
        middle = 0;
        right2 = "";
        middle2 = 0;
        deletelastchar();
        printingsequencetoprow();
      } else {
        return;
      }
      afterenter = 0;
    }
  }
}

void loop() {
  for (int i = 0; i < rowcount; i++) {
    byte row = rows[i];
    pinMode(row, OUTPUT);
    digitalWrite(row, LOW);

    for (int j = 0; j < colcount; j++) {
      byte col = cols[j];
      pinMode(col, INPUT_PULLUP);
      delay(1);

      if (digitalRead(col) == 0) {
        if (state[i][j] == 1) {
          char key = keymap[i][j];
          state[i][j] = 0;
          debounce[i][j] = millis();

          if (hybrid == 0) {
            processNumpadKey(key, col);
          } else {
            processCalculatorKey(key, col);
          }
        }
      }

      if (state[i][j] == 0) {
        if (digitalRead(col) == 1) {
          if (millis() - debounce[i][j] > debouncedelay) {
            state[i][j] = 1;
            Keyboard.release(keymap[i][j]);
          }
        }
      }
      pinMode(col, INPUT);
    }
    pinMode(row, INPUT);
  }
}