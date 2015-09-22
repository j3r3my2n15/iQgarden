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

typedef struct{
   char*    caption;
   boolean    state;
   unsigned long   unix;
 } RELAY;

 typedef struct {
  char* caption;
  boolean state;
  float fValue;
  float fPValue;
  int iValue;
  int iPValue;
  unsigned long lastRead;
 } SENSOR;

 typedef struct {
  char *caption;
  int HourStart;
  int HourStop;
  int Led12Percent;
  int Led24Every;
  int Led24During;
  int PumpEvery;
  int PumpDuring;
  float TempChange;
  float MoistChange;
 }CONFIGMODE;

