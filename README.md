# 17keynumpad
![calculators image](https://github.com/KacperBizon/17keynumpad/blob/main/calculators.jpg)
### board pinout
| pin/pin | B5(9) | (B6(14) | B2(16) | B3(10) 
|-------|:-------:|----:|--------:|--------:|
| B4(8) |backspace|  / |    *    |     -   |
|E6(7)  |   7     |  8 |    9    |    N/A  |
| D7(6) |   4     |  5 |    6    |     +   |
| C6(5) |   1     |  2 |    3    |    N/A  |
| D4(4) |  N/A    |  0 |    .    |   Enter |
 
### LCD I2C 16x2 pinout
| Arduino | I2C|
|----------|----------|
| GND   | GND|
| VCC   | VCC|
| 2  | SDA|
| 3  | SCL|

### keypresses
enter:
- first press - shows result
- second press - repeat the operation...
- changes value of "left" to result of the operation

del / num:
- press - deletes last pressed character
- press after the operation changes value of "left" to result of the operation and deletes last character
- hold - changes mode, saves left, middle, right
- hold after the operation changes value of "left" to result of the operation and changes mode

### display
  fp64lib 64 bit float  - IEE754 double with few compromises
  display up max        - 12 + 1 + 1 + 2 (numbers + positive/negative + operation + dots)
  display down          - 12 + 1 + 1 (result (truncated to 12 chars) + positive/negative + dot)

### parts list
- pcb
- 17 1n4148 diodes
- 3x2U pcb stabilizers
- pro micro ATMEGA32U4 (make sure to solder it on the bottom with 'front' orientation)
- 16x2 i2c lcd screen
- toggle switch
- 17 switches
- 17 keycaps
- 4 m2 screws
- 4 m3 screws
- 4 orings
- tp4056 (optional)
- lipo 800/600 mAh battery (optional)
