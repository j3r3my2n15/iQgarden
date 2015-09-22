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
//###          CONSTANTES            ###
//######################################
//Alignement
#define ALN_CURSOR 0
#define ALN_LEFT 1
#define ALN_CENTER 2
#define ALN_RIGHT 3
//Relais Statut
#define RELAY_OFF HIGH
#define RELAY_ON LOW
//divers
#define BUFF_MAX 128
//DHT
#define DHT_ALL 1
#define DHT_TEMP 2
#define DHT_MOIST 3
//Definition des modes de l'iQgarden. la valeur du mode correspond à la valeur de la carte RFID utilisée
#define MODE_NORMAL -20703
#define MODE_DEMO 32563
#define MODE_MAKERFAIRE -19666 
#define MODE_BASILIC -16573
#define MODE_TOMATE 20683                                        
#define MODE_EPILEPTIQUE 23803
