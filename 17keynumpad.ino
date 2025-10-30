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

//keyboard
#include <Keyboard.h>

//lcd
#include <Wire.h> // Library for I2C communication
#include <LiquidCrystal_I2C.h> // Library for LCD
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x27, 16, 2);

//library for 64 bit float
#include <fp64lib.h>

//pins
const byte rows[] = {8, 7, 6, 5, 4};
const int rowcount = sizeof(rows) / sizeof(rows[0]);

const byte cols[] = {9, 14, 16, 10};
const int colcount = sizeof(cols) / sizeof(cols[0]);

//keymap (comment up)
const uint8_t keymap[rowcount][colcount] = {8, '/', '*', '-', '7', '8', '9', 32, '4', '5', '6', '+', '1', '2', '3', 32, 32, '0', '.', 10}; // 17 (+3)

//state of key
bool state[rowcount][colcount] = {}; // 17 (+3)

//debounce timer for every switch
unsigned long debounce[rowcount][colcount] = {0}; // 17 (+3)

//debounce delay
const byte debouncedelay = 6;

//change to calculator
bool hybrid = 1;

//for calculator
String left = "", right = "", right2 = "";

byte middle = 0, middle2;

//other functions
bool afterenter = 0;
byte shiftright = 0;

void setup() {
  //state 1 (inactive)
  for (int k = 0; k < rowcount; k++)
    for (int m = 0; m < colcount; m++)
      state[k][m] = 1;

  //matrix inputs
  for (int i = 0; i < rowcount; i++)
    pinMode(rows[i], INPUT);

  for (int j = 0; j < colcount; j++)
    pinMode(cols[j], INPUT_PULLUP);

  lcd.begin();
  lcd.noBlink();
  lcd.noCursor();
  //lcd.noBacklight();
  lcd.setCursor(0, 0);

  Keyboard.begin();
}

void printingsequencetoprow()
{
  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print(left);

  shiftright = left.length() + right.length() + right2.length();

  if (middle)
  {
    lcd.print(char(middle));
    shiftright++;
  }


  lcd.print(right);

  if (middle2)
  {
    lcd.print(char(middle2));
    shiftright++;
  }

  lcd.print(right2);

  //shifting the top display
  for (byte i = 16; i < shiftright; i++)
  {
    lcd.scrollDisplayLeft();
  }
}

void printingsequencebottomrow()
{
  if (shiftright > 15)////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  {
    shiftright -= 16;
    lcd.setCursor(shiftright, 1);
  }
  else
  {
    lcd.setCursor(0, 1);
  }

  lcd.print(left);
}

void deletelastchar()
{
  if (middle == 0)
  {
    int lastIndex = left.length() - 1;
    left.remove(lastIndex);
  }
  else if (right.toFloat() == 0)
  {
    middle = 0;
  }
  else if (middle2 == 0)
  {
    int lastIndex = right.length() - 1;
    right.remove(lastIndex);
  }
  else if (right2.toFloat() == 0)
  {
    middle2 = 0;
  }
  else
  {
    int lastIndex = right2.length() - 1;
    right2.remove(lastIndex);
  }
}

//left and right
void calculate()
{
  if (middle == '/')
  {
    left = fp64_to_string( fp64_div (fp64_atof(&left[0]), fp64_atof(&right[0])), 14, 14 );

    if (right.toFloat() == 0)
    {
      lcd.clear();

      lcd.print("nie ma tak dobrze");
    }
  }

  if (middle == '*')
  {
    left = fp64_to_string( fp64_mul (fp64_atof(&left[0]), fp64_atof(&right[0])), 14, 14 );
  }

  if (middle == '-')
  {
    left = fp64_to_string( fp64_sub (fp64_atof(&left[0]), fp64_atof(&right[0])), 14, 14 );
  }

  if (middle == '+')
  {
    left = fp64_to_string( fp64_add (fp64_atof(&left[0]), fp64_atof(&right[0])), 14, 14 );
  }
}

//right and right 2
void calculate2()
{
  if (middle2 != 0)
  {
    if (middle2 == '/')
    {
      right = fp64_to_string( fp64_div (fp64_atof(&right[0]), fp64_atof(&right2[0])), 14, 14 );

      if (right2.toFloat() == 0)
      {
        lcd.clear();

        lcd.print("nie ma tak dobrze");
      }
    }

    if (middle2 == '*')
    {
      right = fp64_to_string( fp64_mul (fp64_atof(&right[0]), fp64_atof(&right2[0])), 14, 14 );
    }

    if (middle2 == '-')
    {
      right = fp64_to_string( fp64_sub (fp64_atof(&right[0]), fp64_atof(&right2[0])), 14, 14 );
    }

    if (middle2 == '+')
    {
      right = fp64_to_string( fp64_add (fp64_atof(&right[0]), fp64_atof(&right2[0])), 14, 14 );
    }
  }
}

void loop() {
  //normal keyboard operating mode
  //i - row j - col
  for (int i = 0; i < rowcount; i++)
  {
    byte row = rows[i];

    pinMode(row, OUTPUT);
    digitalWrite(row, LOW);

    for (int j = 0; j < colcount; j++)
    {
      byte col = cols[j];

      pinMode(col, INPUT_PULLUP);
      delay(1);

      //press
      if (digitalRead(col) == 0)
      {
        if (state[i][j] == 1)
        {
          //assign the pressed key
          char key = keymap[i][j];

          if (hybrid == 0)
          {
            //numpad mode
            state[i][j] = 0;
            debounce[i][j] = millis();
            Keyboard.press(key);
           
            //to calculator
            if (key == 8)
            {
              int temp = 0;
              while (digitalRead(col) == 0)
              {

                temp += 1;
                delay(30);

                if (temp == 100)
                {
                  hybrid = !hybrid;

                  lcd.clear();

                  lcd.setCursor(0, 0);

                  lcd.print("calculator");

                  delay(500);

                  //printing sequence top row
                  printingsequencetoprow();
                }
              }
            }
          }
          else
          {
            //calculator mode
            if (afterenter == 0)
            {
              //'.'
              if (key == 46)
              {
                if (middle == 0)
                {
                  if (left.indexOf('.') == -1)
                  {
                    if (left.toFloat())
                      left += ".";
                    else
                      left += "0.";
                  }
                }
                else if (middle2 == 0)
                {
                  if (right.indexOf('.') == -1)
                  {
                    if (right.toFloat())
                      right += ".";
                    else
                      right += "0.";
                  }
                }
                else
                {
                  if (right2.indexOf('.') == -1)
                  {
                    if (right2.toFloat())
                      right2 += ".";
                    else
                      right2 += "0.";
                  }
                }
              }

              //'0-10'
              if (key > 47 && key < 58)
              {
                if (middle == 0)
                {
                  left += char(key);
                }
                else if (middle2 == 0)
                {
                  right += char(key);
                }
                else
                {
                  right2 += char(key);
                }
              }

              //'/, *, -, +'
              if (key == '/')
              {
                if (middle == 0)
                {
                  middle = '/';

                  if (left.toFloat() == 0)
                    left = "0";
                }
                else if (right.toFloat() == 0)
                {
                  middle = '/';
                }
                else if (middle2 == 0)
                {
                  if (middle == '*' || middle == '/')
                  {
                    calculate();

                    middle = '/';
                    right = "";
                  }
                  else
                  {
                    middle2 = '/';
                  }
                }
                else if (right2.toFloat() == 0)
                {
                  middle2 = '/';
                }
                else
                {
                  calculate2();

                  middle2 = '/';
                  right2 = "";
                }
              }


              if (key == '*')
              {
                if (middle == 0)
                {
                  middle = '*';

                  if (left.toFloat() == 0)
                    left = "0";
                }
                else if (right.toFloat() == 0)
                {
                  middle = '*';
                }
                else if (middle2 == 0)
                {
                  if (middle == '*' || middle == '/')
                  {
                    calculate();

                    middle = '*';
                    right = "";
                  }
                  else
                  {
                    middle2 = '*';
                  }
                }
                else if (right2.toFloat() == 0)
                {
                  middle2 = '*';
                }
                else
                {
                  calculate2();

                  middle2 = '*';
                  right2 = "";
                }
              }


              if (key == '+')
              {
                if (middle == 0)
                {
                  middle = '+';

                  if (left.toFloat() == 0)
                    left = "0";
                }
                else if (right.toFloat() == 0)
                {
                  middle = '+';
                }
                else if (middle2 == 0)
                {
                  calculate();

                  middle = '+';
                  right = "";
                }
                else if (right2.toFloat() == 0)
                {
                  middle2 = '+';
                }
                else
                {
                  calculate2();

                  calculate();

                  middle = '+';

                  right2 = "";
                  middle2 = 0;
                  right = "";
                }
              }

              if (key == '-')
              {
                if (middle == 0)
                {
                  middle = '-';

                  if (left.toFloat() == 0)
                    left = "0";
                }
                else if (right.toFloat() == 0)
                {
                  middle = '-';
                }
                else if (middle2 == 0)
                {
                  calculate();

                  middle = '-';
                  right = "";
                }
                else if (right2.toFloat() == 0)
                {
                  middle2 = '-';
                }
                else
                {
                  calculate2();

                  calculate();

                  middle = '-';

                  right2 = "";
                  middle2 = 0;
                  right = "";
                }
              }

              //printing sequence top row
              printingsequencetoprow();

              //enter
              if (key == 10)
              {
                calculate2();

                calculate();

                afterenter = 1;

                //printing sequence bottom row
                printingsequencebottomrow();
              }

              if (key == 8)
              {
                bool dontremove = 0;
                byte temp = 0;

                //to numpad
                while (digitalRead(col) == 0)
                {
                  temp += 1;
                  delay(30);

                  if (temp == 100)
                  {
                    dontremove = 1;

                    hybrid = !hybrid;

                    lcd.clear();

                    lcd.setCursor(0, 0);

                    lcd.print("numpad");

                    delay(500);

                    lcd.clear();
                  }
                }

                if (dontremove == 0)
                {
                  //del / num
                  deletelastchar();

                  //printing sequence top row
                  printingsequencetoprow();
                }
              }
            }

            //afterenter = 1
            else
            {
              //'0-10' || "."
              if ((key > 47 && key < 58) || (key == 46))
              {
                lcd.clear();

                left = char(key);

                middle = 0;

                right = "";

                middle2 = 0;

                right2 = "";

                if (key == 46)
                {
                  left = "0.";
                }

                afterenter = 0;

                //printing sequence top row
                printingsequencetoprow();
              }

              //'/, *, -, +'
              if (key == '*' || key == '/' || key == '+' || key == '-')
              {
                middle = key;

                right = "";

                middle2 = 0;

                right2 = "";

                afterenter = 0;

                //printing sequence top row
                printingsequencetoprow();
              }

              //enter
              if (key == 10)
              {
                //printing sequence top row
                printingsequencetoprow();

                if (middle2 == 0)
                {
                  calculate();
                }
                else
                {
                  middle = middle2;
                  right = right2;

                  middle2 = 0;
                  right2 = "";

                  //printing sequence top row
                  printingsequencetoprow();

                  calculate();
                }


                //printing sequence bottom row
                printingsequencebottomrow();
              }

              //del / num
              if (key == 8)
              {
                bool dontremove = 0;
                byte temp = 0;

                while (digitalRead(col) == 0)
                {
                  temp += 1;
                  delay(30);

                  //to calculator
                  if (temp == 100)
                  {
                    dontremove = 1;
                    hybrid = !hybrid;

                    middle = 0;

                    right = "";

                    lcd.clear();

                    lcd.setCursor(0, 0);

                    lcd.print("numpad");

                    delay(500);

                    lcd.clear();
                  }
                }
                if (dontremove == 0)
                {
                  right = "";

                  middle = 0;

                  right2 = "";

                  middle2 = 0;

                  //del
                  deletelastchar();

                  //printing sequence top row
                  printingsequencetoprow();
                }
                afterenter = 0;
              }
            }
          }
          state[i][j] = 0;
        }
      }

      //release
      if (state[i][j] == 0)
      {
        if (digitalRead(col) == 1)
        {
          if (millis() - debounce[i][j] > debouncedelay)
          {
            {
              state[i][j] = 1;
              Keyboard.release(keymap[i][j]);
            }
          }
        }
      }
      pinMode(col, INPUT);
    }
    pinMode(row, INPUT);
  }
}
