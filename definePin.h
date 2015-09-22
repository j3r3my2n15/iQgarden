/*
  iQgarden for the Arduino.

  This code implements all features of the iQgarden:

  Author:          Jeremy DENIS <jeremy.denis@gmail.com>
  Available from:  https://github.com/jeremydenis51/iQgarden
 

  Common Creatives license:
  
  You are free to:

 - Share — copy and redistribute the material in any medium or format
 - Adapt — remix, transform, and build upon the material

The licensor cannot revoke these freedoms as long as you follow the license terms.

Attribution — You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.

NonCommercial — You may not use the material for commercial purposes.

ShareAlike — If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original.

   
  You should have received a copy of the Creative Commons License
  along with this program.  If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0//>.
   
*/

//######################################
//###           PIN DEFINE           ###
//######################################
#define LCD_ROWS 2
#define LCD_COLS 16
//DHT22
#define PIN_DHT 6
//BMP180
#define PIN_BMP_SDA 4
#define PIN_BMP_SCL 5
//Light Sensor
#define PIN_LIGHT 0
//RTC
#define PIN_RTC_SDA 20
#define PIN_RTC_SCL 21
//Relays
#define PIN_RLY_LED24 36
#define PIN_RLY_LED12 34
#define PIN_RLY_FANINJ 32
#define PIN_RLY_FANEXT 30
#define PIN_RLY_PUMP 28
#define PIN_RLY_FANREF 26
#define PIN_RLY_NOT1 24
#define PIN_RLY_NOT2 22
//capteur de niveau d'eau
#define PIN_WATER 1
//lecteur RFID
#define SS_PIN 53 //RFID
#define RST_PIN 9 //RFID
//Tone
#define PIN_TONE 8
