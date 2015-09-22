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
//###           LIBRARY              ###
//######################################
#include <SPI.h>
#include <Ethernet.h> //eth
#include <LiquidCrystal.h>
#include <config.h>
#include <ds3231.h>
#include <Wire.h>
#include <DHT22.h>
#include <SFE_BMP180.h>
#include <RFID.h>
#include <stdio.h>
#include "definePin.h"
#include "defineConstante.h"
#include "structs.h"
#include "tonePitch.h"
#include "chars.h"

//DEBUG MODE // DEBUG MODE // DEBUG MODE
#define DEBUG_MODE false
//DEBUG MODE // DEBUG MODE // DEBUG MODE




//######################################
//###            Musique             ###
//######################################
// notes in the melody:
int melody[] = {  NOTE_C4, NOTE_G3,NOTE_G3, NOTE_A3, NOTE_G3,0, NOTE_B3, NOTE_C4};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {  4, 8, 8, 4,4,4,4,4 };
//######################################
//###      Variables Globales        ###
//######################################
//logical
long lastNow = 0;
long timeNow = 0;

int currentMode=0;

//relais
int state_Led24 = RELAY_ON;
int state_Led12 = RELAY_ON;
int state_FanInj = RELAY_ON;
int state_FanExt = RELAY_ON;
int state_Pump = RELAY_ON;
int state_FanRef = RELAY_ON;
RELAY rly_Led24;
RELAY rly_Led12;
RELAY rly_FanInj;
RELAY rly_FanExt;
RELAY rly_Pump;
RELAY rly_FanRef;
//Capteur
SENSOR snr_Light;
SENSOR snr_Temp;
SENSOR snr_Moist;
SENSOR snr_Press;
SENSOR snr_Level;
SENSOR snr_Alt;
//Mode
CONFIGMODE cmNormal;
CONFIGMODE cmBasilic;
CONFIGMODE cmTomate;
//LCD
LiquidCrystal lcd(12,11,5,4,3,2);
//DHT22
DHT22 sensor_DHT(PIN_DHT);
//Time
char* time;
//pression
SFE_BMP180 sensor_Pressur;
double baseline;
// RFID
RFID rfid(SS_PIN,RST_PIN);
//Tone
int toneOk = 0;
//Ethernet
#define DEVICEID "3103"
char passphrase[] = "efmlIJzdjdugaçèè!54TY3Bhjdbz"; 
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192,168,1,248);
EthernetClient client;
IPAddress server(192,168,1,130);
//char server[] = "api.iqgarden.me"; // IP Adres (or name) of server to dump data to
//int  interval = 10000; // Wait between dumps 15mn (900000)
unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 10*10;  //delay between updates to pachube.com

//######################################
//###             SETUP              ###
//######################################  
void setup()
{
  // Initialisation des la lisison serie
  Serial.begin(57600);
  Serial.println("Launching");
  // Initialisation du RTC : horloge
  Wire.begin();
  DS3231_init(0x06);

  //tone sur la pin 8
  tone(PIN_TONE,NOTE_DS6,500);

  SPI.begin();
  rfid.init();

  LCD_logo();
  LCD_display(4,1,"garden.me",ALN_CURSOR,0,false);

  // Initialisation des PIN Relais to OUTPOUT
  Serial.println("PIN Relay to Output -> START");
  pinMode(PIN_RLY_LED24, OUTPUT);
  pinMode(PIN_RLY_LED12, OUTPUT);
  pinMode(PIN_RLY_FANINJ, OUTPUT);
  pinMode(PIN_RLY_FANEXT, OUTPUT);
  pinMode(PIN_RLY_PUMP, OUTPUT);
  pinMode(PIN_RLY_FANREF, OUTPUT);
  pinMode(PIN_RLY_NOT1, OUTPUT);
  pinMode(PIN_RLY_NOT2, OUTPUT);
  digitalWrite(PIN_RLY_NOT1, HIGH);
  digitalWrite(PIN_RLY_NOT2, HIGH);
  Serial.println("PIN Relay to Output -> DONE");

  
//relais
  relay_AllOff();
  rly_Led24.caption = "Led 24v  ";
  rly_Led12.caption = "Led 12v  ";
  rly_FanInj.caption = "Fan Inject";
  rly_FanExt.caption = "Fan Extrac";
  rly_Pump.caption = "Pompe";
  rly_FanRef.caption = "Refroidiss";
//capteur
  snr_Light.caption = "Luminosite";
  snr_Temp.caption = "Temperature";
  snr_Moist.caption = "Humidite";
  snr_Press.caption = "Pression";
  snr_Level.caption = "Niveau";
  snr_Alt.caption = "Altitude";
  snr_Light.state = snr_Temp.state = snr_Moist.state = snr_Press.state = snr_Level.state = snr_Alt.state = 0;

//CONFIG MODE
  cmNormal.caption = "Normal";
  cmNormal.HourStart = 7;
  cmNormal.HourStop = 23;
  cmNormal.Led12Percent = 75;
  cmNormal.Led24Every = 10;
  cmNormal.Led24During = 1;
  cmNormal.PumpEvery = 10;
  cmNormal.PumpDuring = 10;
  cmNormal.TempChange = 23;
  cmNormal.MoistChange = 60;
  cmBasilic.caption = "Basilic";
  cmBasilic.HourStart = 7;
  cmBasilic.HourStop = 23;
  cmBasilic.Led12Percent = 65;
  cmBasilic.Led24Every = 15;
  cmBasilic.Led24During = 1;
  cmBasilic.PumpEvery = 10;
  cmBasilic.PumpDuring = 10;
  cmBasilic.TempChange = 23;
  cmBasilic.MoistChange = 60;
  cmTomate.caption = "Tomate";
  cmTomate.HourStart = 7;
  cmTomate.HourStop = 23;
  cmTomate.Led12Percent = 60;
  cmTomate.Led24Every = 0;
  cmTomate.Led24During = 0;
  cmTomate.PumpEvery = 10;
  cmTomate.PumpDuring = 20;
  cmTomate.TempChange = 30;
  cmTomate.MoistChange = 80;
  
//pression
  if (sensor_Pressur.begin()) Serial.println("BMP180 init success");

  else{
    Serial.println("BMP180 init fail (disconnected?)\n\n");
    while(1); // Pause forever.
  }
//premiere mesure pour chauffer les capteurs
  baseline = getPressure();
  getTempOrMoist(DHT_ALL);
//delais pour reprendre une mesure
  baseline = getPressure();
  getTempOrMoist(DHT_ALL);
/*
  // Initialisation de la connexion Ethernet
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // DHCP failed, so use a fixed IP address:
    Ethernet.begin(mac, ip);
  }
*/
  Serial.println("Launched !");
  
  delay(3000);
  LCD_initChar();
  lastNow = RTC_getUnix();
  getAllMesure();
}

//######################################
//###             LOOP               ###
//######################################  
void loop(){
  //check du delais depuis le dernier controle
  if(needCheck() == 1) getAllMesure();

  int cardID = checkCardRFID();
  if( cardID != 0){//changement de mode
    relay_AllOff();
    currentMode = cardID;
    switch(cardID){
      case MODE_NORMAL:
        LCD_display(0, 0, "Mode :",ALN_LEFT,0,1);
        LCD_display(0, 1, "NORMAL",ALN_CENTER,0,0);
        break;
      case MODE_DEMO:
        LCD_display(0, 0, "Mode :",ALN_LEFT,0,1);
        LCD_display(0, 1, "DEMO",ALN_CENTER,0,0);
        break;
      case MODE_MAKERFAIRE:
        LCD_display(0, 0, "Mode :",ALN_LEFT,0,1);
        LCD_display(0, 1, "MAKER FAIRE",ALN_CENTER,0,0);
        break;
      case MODE_BASILIC:
        LCD_display(0, 0, "Mode :",ALN_LEFT,0,1);
        LCD_display(0, 1, "BASILIC",ALN_CENTER,0,0);
        break;
      case MODE_TOMATE:
        LCD_display(0, 0, "Mode :",ALN_LEFT,0,1);
        LCD_display(0, 1, "TOMATE",ALN_CENTER,0,0);
        break;
      case MODE_EPILEPTIQUE:
        LCD_display(0, 0, "Mode :",ALN_LEFT,0,1);
        LCD_display(0, 1, "EPILEPTIQUE",ALN_CENTER,0,0);
        break;
      default:
       LCD_display(0, 0, "Mode :",ALN_LEFT,0,1);
       LCD_display(0, 1, "INCONNUE",ALN_CENTER,0,0);
       break;
    }
    delay(3000);
    LCD_display(0, 1, "GO !!",ALN_CENTER,0,1);
    delay(1000);
  }

  if(currentMode == 0){
    LCD_display(0, 0, "Selectionnez un",ALN_CENTER,0,1);
    LCD_display(0, 1, "MODE",ALN_CENTER,0,0);
    if(toneOk == 0){
      tone(PIN_TONE,NOTE_AS4,100);
      delay(200);
      tone(PIN_TONE,NOTE_AS4,100);
      toneOk = 1;
    }
  }
  else{
    switch(currentMode){
      case MODE_NORMAL:
        modeConfig(cmNormal);
        LCD_allSensor(snr_Temp.fValue,snr_Moist.fValue,snr_Light.iValue,snr_Level.iValue);
        break;
      case MODE_DEMO:
        modeDemo();
        break;
      case MODE_MAKERFAIRE:
        modeMakerFaire();
        break;
      case MODE_BASILIC:
        modeConfig(cmBasilic);
        LCD_allSensor(snr_Temp.fValue,snr_Moist.fValue,snr_Light.iValue,snr_Level.iValue);
        break;
      case MODE_TOMATE:
        modeConfig(cmTomate);
        LCD_allSensor(snr_Temp.fValue,snr_Moist.fValue,snr_Light.iValue,snr_Level.iValue);
        break;
      case MODE_EPILEPTIQUE:
        modeEpileptique();
        break;
      default:

       break;
    }
    
  }
  
  delay(500);

/*
  //allumage de la lumiere si inferieur à 50%
  if(light < 60) relay_Led12(RELAY_ON);
  else relay_Led12(RELAY_OFF);

  //reduction de l'humidité de la boite si Humidité > 50%
  if(humi >= 47) {
    relay_FanExt(RELAY_ON);
    relay_FanInj(RELAY_ON);
  }
  //reduction de l'humidité de la boite si Humidité > 50%
  if(humi <= 46.5) {
    relay_FanExt(RELAY_OFF);
    relay_FanInj(RELAY_OFF);
  }
  */
}

/*########################################################
###########    FONCTIONS SMART DU SYSTEME   ##############
########################################################*/
void modeEpileptique(){
  tetris();
  relay_Led12(RELAY_ON);
  tone(PIN_TONE,NOTE_C6,100);
  delay(200);
  relay_Led12(RELAY_OFF);
  relay_FanInj(RELAY_ON);
  delay(200);
  relay_Led12(RELAY_ON);
  tone(PIN_TONE,NOTE_CS6,100);
  delay(200);
  relay_Led12(RELAY_OFF);
  relay_FanInj(RELAY_OFF);
  delay(200);
  relay_Led24(RELAY_ON);
  tone(PIN_TONE,NOTE_D6,100);
  delay(200);
  relay_Led24(RELAY_OFF);
  relay_Pump(RELAY_ON);
  delay(200);
  
  relay_Led12(RELAY_ON);
  tone(PIN_TONE,NOTE_DS6,100);
  delay(200);
  relay_Led12(RELAY_OFF);
  delay(200);
  relay_Led12(RELAY_ON);
  relay_Pump(RELAY_OFF);
  tone(PIN_TONE,NOTE_E6,100);
  delay(200);
  relay_Led12(RELAY_OFF);
  delay(200);
  relay_Led24(RELAY_ON);
  relay_FanInj(RELAY_ON);
  tone(PIN_TONE,NOTE_F6,100);
  delay(200);
  relay_Led24(RELAY_OFF);
  delay(200);
  
  relay_Led12(RELAY_ON);
  tone(PIN_TONE,NOTE_FS6,100);
  delay(200);
  relay_Led12(RELAY_OFF);
  delay(200);
  relay_Led12(RELAY_ON);
  tone(PIN_TONE,NOTE_G6,100);
  delay(200);
  relay_Led12(RELAY_OFF);
  delay(200);
  relay_Led24(RELAY_ON);
  tone(PIN_TONE,NOTE_GS6,100);
  relay_FanInj(RELAY_OFF);
  delay(200);
  relay_Led24(RELAY_OFF);
  delay(200);
  
  relay_Led12(RELAY_ON);
  tone(PIN_TONE,NOTE_A6,100);
  delay(200);
  relay_Led12(RELAY_OFF);
  delay(200);
  relay_Led12(RELAY_ON);
  tone(PIN_TONE,NOTE_AS6,100);
  delay(200);
  relay_Led12(RELAY_OFF);
  delay(200);
}

void modeMakerFaire(){
  char *Dialogue[] = {
    "Bonjour",
    "et bienvenue",
    "sur le stand de",
    "iQgarden",
    "dans ce mode",
    "nous allons voir",
    " comment je ",
    "fonctionne.",
    " ",
    "Je mesure :",
    "- Temperature",
    "- Humidite",
    "- Pression",
    "- Niveau d'eau",
    " ",
    "Avec tous ca",
    "Je controle : ",
    "- Lumiere",
    "- Arrosage",
    "- Ventilation",
    "- Niveau d'eau",
    "",
  };
  
  for(int i = 0;i < 21; i++){
    LCD_display(0, 0,Dialogue[i],ALN_LEFT,0,1);
    LCD_display(0, 1, Dialogue[i+1],ALN_LEFT,0,0);
    delay(1000);
  }
  
  
  LCD_display(0, 0,"DEMO",ALN_CENTER,0,1);
  tone(PIN_TONE,NOTE_AS4,100);
  delay(200);
  tone(PIN_TONE,NOTE_AS4,100);

  //demo temperature
  LCD_display(0, 0,"Il fait",ALN_CENTER,0,1);
  char tempor[3];
  toolToDisplay(snr_Temp.fValue,tempor,1);
  LCD_display(0, 1,tempor,ALN_RIGHT,2,0);
  LCD_display(0, 1," c",ALN_RIGHT,0,0);
  delay(2000);
  LCD_display(0, 0,tempor,ALN_RIGHT,2,1);
  LCD_display(0, 0," c",ALN_RIGHT,0,0);
  LCD_display(0, 1,"Je demarre les",ALN_LEFT,2,0);
  delay(1000);
  LCD_display(0, 0,"Je demarre les",ALN_LEFT,2,1);
  LCD_display(0, 1,"ventilateurs",ALN_LEFT,2,0);
  relay_FanInj(RELAY_ON);
  relay_FanExt(RELAY_ON);
  delay(1000);
  LCD_display(0, 1,"..",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 1,"....",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 1,"......",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 1,"........",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 0,"Et je les coupes",ALN_LEFT,2,1);
  LCD_display(0, 1,"",ALN_LEFT,2,0);
  relay_FanInj(RELAY_OFF);
  relay_FanExt(RELAY_OFF);
  delay(2000);

  //Demo Lumieres
  LCD_display(0, 0,"Maintement la",ALN_LEFT,0,1);
  LCD_display(0, 1,"Lumiere",ALN_CENTER,0,0);
  delay(1000);
  LCD_display(0, 0,"Lumiere",ALN_CENTER,0,1);
  LCD_display(0, 1,"Elle est de : ",ALN_LEFT,0,0);
  delay(1000);
  LCD_display(0, 0,"Elle est de : ",ALN_LEFT,0,1);

  toolToDisplay(snr_Light.iValue,tempor,1);
  LCD_display(0, 1,tempor,ALN_RIGHT,2,0);
  LCD_display(0, 1," %",ALN_RIGHT,0,0);
  delay(2000);
  LCD_display(0, 0,tempor,ALN_RIGHT,2,1);
  LCD_display(0, 0," %",ALN_RIGHT,0,0);
  LCD_display(0, 1,"J'allume !",ALN_LEFT,2,0);
  delay(1000);
  LCD_display(0, 0,"J'allume",ALN_LEFT,2,1);
  LCD_display(0, 1,"les 1000 lumens",ALN_LEFT,2,0);
  relay_Led12(RELAY_ON);
  delay(1000);
  LCD_display(0, 1,"..",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 1,"....",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 1,"......",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 1,"........",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 0,"Et je les coupes",ALN_LEFT,2,1);
  LCD_display(0, 1,"",ALN_LEFT,2,0);
  relay_Led12(RELAY_OFF);
  delay(2000);
  
  //Demo arrosage
  LCD_display(0, 0,"Et si",ALN_LEFT,0,1);
  LCD_display(0, 1,"on arrosait !",ALN_LEFT,0,0);
  delay(1000);
  LCD_display(0, 0,"on arrosait !",ALN_LEFT,0,1);
  LCD_display(0, 1,"Est-ce qu'il",ALN_LEFT,0,0);
  delay(1000);
  LCD_display(0, 0,"Est-ce qu'il",ALN_LEFT,0,1);
  LCD_display(0, 1,"reste de l'eau ?",ALN_LEFT,0,0);
  delay(1000);

  toolToDisplay(snr_Level.iValue,tempor,1);
  LCD_display(0, 0,"Il reste :",ALN_LEFT,0,1);
  LCD_display(0, 1,tempor,ALN_RIGHT,2,0);
  LCD_display(0, 1," %",ALN_RIGHT,0,0);
  delay(1000);
  LCD_display(0, 0,"C'est bon !",ALN_LEFT,0,1);
  LCD_display(0, 1,"J'arrose !",ALN_LEFT,0,0);
  relay_Pump(RELAY_ON);
  delay(1000);
  LCD_display(0, 1,"..",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 1,"....",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 1,"......",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 1,"........",ALN_LEFT,2,1);
  delay(1000);
  LCD_display(0, 0,"Et je stop !",ALN_LEFT,2,1);
  LCD_display(0, 1,"",ALN_LEFT,2,0);
  relay_Pump(RELAY_OFF);
  delay(2000);
  char *Dialogue2[] = {
    "Et vous le saviez",
    " que les couleurs",
    "rouge et bleu",
    "favorisent la ",
    "croissance des ",
    "feuilles et ",
    "fleurs ?",
    " ",
  };
  relay_Growup(RELAY_ON);
  for(int i = 0;i <= 7; i++){
    LCD_display(0, 0,Dialogue2[i],ALN_LEFT,0,1);
    LCD_display(0, 1, Dialogue2[i+1],ALN_LEFT,0,0);
    delay(1000);
  }
  relay_FanRef(RELAY_OFF);
  relay_Led24(RELAY_OFF);
  delay(1000);
  LCD_display(0, 0,"A bientot sur ",ALN_LEFT,0,1);
  LCD_display(0, 1,"www.iQgarden.me",ALN_LEFT,0,0);

  
  delay(4000);
  currentMode = 0;
}

void modeConfig(CONFIGMODE myMode){
  //check de l'heure
  int actualHour = RTC_getHour();
  if(actualHour >= myMode.HourStart && actualHour <= myMode.HourStop){
    //on peu allumer la lumiere
    //allumage de la lumiere si inferieur à 50%
    if(snr_Light.iValue < myMode.Led12Percent){
      relay_Led12(RELAY_ON);
    }
    else{
      relay_Led12(RELAY_OFF);
    }

    //check pour les LEDs rouge et bleu
    //les leds ne sont pas actives
    if(rly_Led24.state == RELAY_OFF && timeNow - rly_Led24 .unix >= (myMode.Led24Every*60)){ //toute les 10 min    relay_Growup(RELAY_ON);
    }
    //les LEDs 24 sont actives
    if(timeNow - rly_Led24.unix >= (myMode.Led24During*60) && rly_Led24.state == RELAY_ON){ //durant 60s
      relay_Growup(RELAY_OFF);
    }
  }

  //check pour l'arrosage
  //la pompe n'est pas active
  if(rly_Pump.state == RELAY_OFF && timeNow - rly_Pump.unix >= (myMode.PumpEvery*60)){
    relay_Pump(RELAY_ON);
  }
  //la pompe est active
  if(timeNow - rly_Pump.unix >= myMode.PumpDuring && rly_Pump.state == RELAY_ON){
    relay_Pump(RELAY_OFF);
  }

  //check pour l'humidité
  if(snr_Moist.fValue >= myMode.MoistChange) {
    relay_FanInj(RELAY_ON);
    relay_FanExt(RELAY_ON);
  }
  else{
    relay_FanInj(RELAY_OFF);
    relay_FanExt(RELAY_OFF);
  }

  //check pour la temperature
  if(snr_Temp.fValue >= myMode.TempChange) {
    relay_FanInj(RELAY_ON);
    relay_FanExt(RELAY_ON);
  }
  else{
    relay_FanInj(RELAY_OFF);
    relay_FanExt(RELAY_OFF);
  }

}

void getAllMesure(){
  //recuperation des infos du DHT22 : Temperature & Humidité
  getTempOrMoist(DHT_ALL);
  //recuperation du pourcentage de lumiere 0 - 100%
  int light = getLight();
  //recuperation de la pression
  getPressur();
  //recuperation du niveau d'eau 
  getLevel();
}

int needCheck(){
  timeNow = RTC_getUnix();
  if( timeNow - lastNow >= 5){
    lastNow = timeNow;
    return 1;
  }
  else{
    return 0;
  }
}

/*########################################################
#############       FONCTIONS RFID        ################
########################################################*/
int checkCardRFID(){
 if(rfid.isCard()){
        if(rfid.readCardSerial()){
            int cardID = rfid.serNum[0]*100000000000+rfid.serNum[1]*100000000+rfid.serNum[2]*100000+rfid.serNum[3]*100+rfid.serNum[4];
            Serial.println(cardID);  
            rfid.halt();
            tone(PIN_TONE,NOTE_DS6,500);
            delay(1000);
            return cardID;
        }              
    }
    return 0;
}

/*########################################################
#############   FONCTIONS AFFICHAGE LCD   ################
########################################################*/
/*
 * Affiche un message sur l'ecran LCD
 * @param col : numero de la colonne où commence le message (0-15)
 * @param row : ligne où sera affichée le message (0-1)
 * @param message : le message de 16 caractere maximum
 * @param align : Alignement du test [left,center,right,cursor]
 * @param addon : offset à la taille de la chaine. exemple si on souhaite ajouté une caractere apres (1)
 * @param clr : true si on efface les messages precedent sinon False, le message s'ajoute
 * @return : 0 en cas d'erreur, sinon la position du curseur après affichage
 */
int LCD_display(int col, int row, char * message,int align,int addon, bool clr){
 if(col > LCD_COLS) return 0;
 if(row > LCD_ROWS) return 0;
 if(clr == true) lcd.clear();
 int chrSize = String(message).length()+addon;
 //quelle position ?
 col = LCD_getCol(col,chrSize,align);
 lcd.setCursor(col,row);
 lcd.print(String(message));
 return col + chrSize - addon; 
}
/*
 * Affiche un charactere spéciale sur l'ecran LCD
 * @param col : numero de la colonne où commence le message (0-15)
 * @param row : ligne où sera affichée le message (0-1)
 * @param chr : id du caractere pré-enregistré
 * @param clr : true si on efface les messages precedent sinon False, le message s'ajoute
 * @return : 0 en cas d'erreur, sinon la position du curseur après affichage
 */
int LCD_displayChar(int col, int row,int chr,bool clr){
 if(col > LCD_COLS) return 0;
 if(row > LCD_ROWS) return 0;
 if(clr == true) lcd.clear();
 lcd.setCursor(col,row);
 lcd.write(byte(chr));
 return col+1; 
}
/*
 * Determine la position du message en fonction de l'alignement demandé
 */
int LCD_getCol(int col,int chrSize,int align){
 if(align == ALN_CURSOR){
  col = col;
 } else if(align == ALN_LEFT){
  col = 0;
 } else if(align == ALN_RIGHT){
  col = int(LCD_COLS) - int(chrSize);
 } else if(align == ALN_CENTER){
  col = int(LCD_COLS/2) - int(chrSize/2)-1;
 } else col = 0;
 return col;
}
/*
 * Affiche les infos des capteurs principaux (Temperature, Humidité, lumiere et eau)
 */
int LCD_allSensor(float temp, float moist, int light, int water){
  LCD_displayChar(int(LCD_COLS/4)-1,0,3,true); //temperature
  LCD_displayChar(int(LCD_COLS/4)*2,0,2,false); //humidité
  LCD_displayChar(int(LCD_COLS/4)*3-1,0,1,false); //lumiere
  LCD_displayChar(int(LCD_COLS/4)*4-2,0,4,false); //eau  
//       (int col, int row, char * message,char * align,int addon, bool clr)
  char temperature[3];
  char humidite[3];
  char lumiere[3];
  char eau[3];
  
  toolToDisplay(temp,temperature,1);
  LCD_display(0,1,temperature,ALN_CURSOR,0,false); //temperature
  toolToDisplay(moist,humidite,1);  
  LCD_display(5,1,humidite,ALN_CURSOR,0,false); //humidité
  LCD_display(10,1,itoa(light,lumiere,10),ALN_CURSOR,0,false); //lumiere
  LCD_display(13,1,itoa(water,eau,10),ALN_CURSOR,0,false); //eau
}
/*
 * Affiche le logo iQgaden.me sur le LCD
 */
int LCD_logo(){
	// Initialisation de l'écran LCD
	lcd.createChar(0, logo_A1);
	lcd.createChar(1, logo_A2);
	lcd.createChar(2, logo_A3);
	lcd.createChar(3, logo_A4);
	lcd.createChar(4, logo_B1);
	lcd.createChar(5, logo_B2);
	lcd.createChar(6, logo_B3);
	lcd.createChar(7, logo_B4);
	
	lcd.begin(16, 2);
	
	LCD_displayChar(0,0,0,true);
	LCD_displayChar(1,0,1,false);
	LCD_displayChar(2,0,2,false);
	LCD_displayChar(3,0,3,false);
	LCD_displayChar(0,1,4,false);
	LCD_displayChar(1,1,5,false);
	LCD_displayChar(2,1,6,false);
	LCD_displayChar(3,1,7,false);
}

int LCD_initChar(){
	lcd.createChar(0, celcius);
	lcd.createChar(1, light);
	lcd.createChar(2, humidity);
	lcd.createChar(3, temp);
	lcd.createChar(4, water);
	lcd.begin(16, 2);
}
/*########################################################
############# FONCTIONS LIES AU CAPTEURS  ################
########################################################*/

/**
 * Recupere la temperature dans la boite
 * les valeurs sont enregistrées dans les variable globales
 * @return 1 si OK
 * @return 0 si KO
 */
float getTempOrMoist(int DHT_data){
  DHT22_ERROR_t errorCode;
  errorCode = sensor_DHT.readData();

  switch(errorCode)
  {
    case DHT_ERROR_TOOQUICK:
      if(DEBUG_MODE) Serial.println("Polled to quick ");
      break;
    default :
      break;
  }

  float sensorReadTemp;
  float sensorReadMoist;
  if(errorCode == DHT_ERROR_NONE){
    sensorReadTemp = sensor_DHT.getTemperatureC();
    sensorReadMoist = sensor_DHT.getHumidity();
  }
  else return 0;
  
  if(snr_Temp.state == 1){
    snr_Temp.fPValue = snr_Temp.fValue;
  }
  else{
    snr_Temp.fPValue = 0;
    snr_Temp.state = 1;
  }
  
  if(snr_Moist.state == 1){
    snr_Moist.fPValue = snr_Moist.fValue;
  }
  else{
    snr_Moist.fPValue = 0;
    snr_Moist.state = 1;
  }
  
  snr_Temp.fValue = sensorReadTemp;
  snr_Temp.lastRead = RTC_getUnix();
  snr_Moist.fValue = sensorReadMoist;
  snr_Moist.lastRead = RTC_getUnix();
  
  if(DHT_data == DHT_ALL) return 1;
  else if(DHT_data == DHT_TEMP) return sensorReadTemp;
  else if(DHT_data == DHT_MOIST) return sensorReadMoist;
  else return 0;
}

/**
 * Recupere la pression dans la boite
 * les valeurs sont enregistrées dans les variable globales
 * @return 1 si OK
 * @return 0 si KO
 */
int getPressur(){
  if(snr_Press.state == 1){
    snr_Press.fPValue = snr_Press.fValue;
  }
  else{
    snr_Press.fPValue = 0;
    snr_Press.state = 1;
  }
  double a,P;
  P = getPressure();
  a = sensor_Pressur.altitude(P,baseline);

  snr_Alt.iValue = P;
  snr_Alt.lastRead = RTC_getUnix();
  
  snr_Press.fValue = P;
  snr_Press.lastRead = RTC_getUnix();

  return P;
}

/**
 * Recupere la luminosité ambiante
 * les valeurs sont enregistrées dans les variable globales
 * @return 1 si OK
 * @return 0 si KO
 */
int getLight(){
  if(snr_Light.state == 1){
    snr_Light.iPValue = snr_Light.iValue;
  }
  else{
    snr_Light.iPValue = 0;
    snr_Light.state = 1;
  }
  int sensorRead = map(analogRead(PIN_LIGHT),0,1023,0,100);
  snr_Light.iValue = sensorRead;
  snr_Light.lastRead = RTC_getUnix();

  return sensorRead;
}

double getPressure(){
  char status;
  double T,P,p0,a;
  status = sensor_Pressur.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = sensor_Pressur.getTemperature(T);
    if (status != 0){
      status = sensor_Pressur.startPressure(3);
      if (status != 0){
        // Wait for the measurement to complete:
        delay(status);
        status = sensor_Pressur.getPressure(P,T);
        if (status != 0){
          return(P);
        }
        else{
          if(DEBUG_MODE){
            Serial.println("error retrieving pressure measurement\n");
          }
        }
      }
      else{
        if(DEBUG_MODE){
          Serial.println("error starting pressure measurement\n");
        }
      }
    }
    else{
      if(DEBUG_MODE){
        Serial.println("error retrieving temperature measurement\n");
      }
    }
  }
  else{
    if(DEBUG_MODE){
      Serial.println("error starting temperature measurement\n");
    }
  }
}

/**
 * Recupere la niveau d'eau
 * les valeurs sont enregistrées dans les variable globales
 * @return 1 si OK
 * @return 0 si KO
 */
int getLevel(){
  if(snr_Level.state == 1){
    snr_Level.iPValue = snr_Level.iValue;
  }
  else{
    snr_Level.iPValue = 0;
    snr_Level.state = 1;
  }
  int SensorValue = analogRead(PIN_WATER);
  int sensorRead = map(SensorValue,480,582,0,100);
  if(sensorRead > 100) sensorRead = 100;
  if(sensorRead < 0) sensorRead = 0;
  snr_Level.iValue = sensorRead;
  snr_Level.lastRead = RTC_getUnix();

  return sensorRead;
}

/*########################################################
###### FONCTION DE GRESTION DES DIFFERENTS RELAIS ########
#####  ATTENTION, HIGH = Désactivé / LOW = Activé ########
########################################################*/
/*
 * Initialise tout les relais à OFF
 */
void relay_AllOff(){
  // Initialisation des Relais à OFF
  relay_Led24(RELAY_OFF);
  relay_Led12(RELAY_OFF);
  relay_FanInj(RELAY_OFF);
  relay_FanExt(RELAY_OFF);
  relay_Pump(RELAY_OFF);
  relay_FanRef(RELAY_OFF);
}
/*
 * Commande groupé de la lumiere Rouge/bleu et du ventilateur de refroidissement pour ce dernier
 * @return 1 si OK
 */
int relay_Growup(bool state){
  relay_Led24(RELAY_ON);
  relay_FanRef(RELAY_ON);
  return 1;
}

/*
 * Commande des Leds Rouges et Bleus
 * Spectre lumineux utile au developpement des feuilles et fleures
 * @return 1 si OK avec changement d'etat
 * @return 2 si OK sans changement d'etat
 */
int relay_Led24(bool  state){
  if(rly_Led24.state == state) return 2;
  digitalWrite(PIN_RLY_LED24, state);
  rly_Led24.state = state;
  rly_Led24.unix = RTC_getUnix();
  return 1;
}
/*
 * Commande des Leds blanches
 * Remplace la lumiere du soleil
 * @return 1 si OK avec changement d'etat
 * @return 2 si OK sans changement d'etat
 */
int relay_Led12(bool  state){
  if(rly_Led12.state == state) return 2;
  digitalWrite(PIN_RLY_LED12, state);
  rly_Led12.state = state;
  rly_Led12.unix = RTC_getUnix();
  
  return 1;
}

/*
 * Commande du ventilateur pour l'injection d'air exterieur dans la boite
 * @return 1 si OK avec changement d'etat
 * @return 2 si OK sans changement d'etat
 */
int relay_FanInj(bool  state){
  if(rly_FanInj.state == state) return 2;
  digitalWrite(PIN_RLY_FANINJ, state);
  rly_FanInj.state = state;
  rly_FanInj.unix = RTC_getUnix();
  
  return 1;
}

/*
 * Commande pour le ventilateur d'extraction d'air dans la boite
 * @return 1 si OK avec changement d'etat
 * @return 2 si OK sans changement d'etat
 */
int relay_FanExt(bool  state){
  if(rly_FanExt.state == state) return 2;
  digitalWrite(PIN_RLY_FANEXT, state);
  rly_FanExt.state = state;
  rly_FanExt.unix = RTC_getUnix();
  
  return 1;
}

/*
 * Commande de la pompe à eau pour l'arrosage
 * @return 1 si OK avec changement d'etat
 * @return 2 si OK sans changement d'etat
 */
int relay_Pump(bool  state){
  if(rly_Pump.state == state) return 2;
  digitalWrite(PIN_RLY_PUMP, state);
  rly_Pump.state = state;
  rly_Pump.unix = RTC_getUnix();
  
  return 1;
}

/*
 * Commande du ventilateur de refroidissement des Led24v
 * @return 1 si OK avec changement d'etat
 * @return 2 si OK sans changement d'etat
 */
int relay_FanRef(bool  state){
  if(rly_FanRef.state == state) return 2;
  digitalWrite(PIN_RLY_FANREF, state);
  rly_FanRef.state = state;
  rly_FanRef.unix = RTC_getUnix();
  
  return 1;
}

/*########################################################
#################  FONCTIONS ETHERNET ####################
########################################################*/

/*
 * Permet d'envoyer les derniers etat des capteurs sur le serveur web
 */
int EthSendData(){
  int temp = 0;
  int humi = 0;
  int lum = 0;
  
  //eth
  if(client.available()) {
    char c = client.read();
    //Serial.print(c);
  }
    
  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    if(DEBUG_MODE){
      Serial.println();
      Serial.println("disconnecting. A");
    }
    client.stop();
   }
  
  if(!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    if (client.connect(server, 80)) {
      if(DEBUG_MODE){
        Serial.println("connecting... A");
      }
      // send the HTTP PUT request:
      client.print("GET /domo/index.php?add-data=y&dev=");
      client.print(DEVICEID);
      client.print("&tem=");
      client.print(temp);
      client.print("&hum=");
      client.print(humi);
      client.print("&lum");
      client.print(lum);
      client.println(" HTTP/1.0");
      client.println("Connection: close");
      client.println("Host: 192.168.0.3");
      client.println("Accept: text/html, text/plain");
      client.println();
      client.flush();
    } 
    else {
      // if you couldn't make a connection:
      if(DEBUG_MODE){
        Serial.println("connection failed");
        Serial.println();
        Serial.println("disconnecting. B");
      }
      client.stop();
    }
    // note the time that the connection was made or attempted:
    lastConnectionTime = millis();
  }
 

  lastConnected = client.connected();
}

/*########################################################
###########      FONCTIONS RTCDS3231          ############
########################################################*/

/*
 * Retourne l'heure courante
 */
char * RTC_getTime(){
struct ts t;
  DS3231_get(&t);
  char buff[BUFF_MAX];
  snprintf(buff, BUFF_MAX, "%d.%02d.%02d %02d:%02d:%02d", t.year,
             t.mon, t.mday, t.hour, t.min, t.sec);
  if(DEBUG_MODE){
    Serial.println(buff);
  }
  return buff;
}

int RTC_getHour(){
  struct ts t;
  DS3231_get(&t);
  return t.hour;
}

unsigned long RTC_getUnix(){
  struct ts t;
  DS3231_get(&t);
  char buff[BUFF_MAX];
  snprintf(buff, BUFF_MAX, "%ld", t.unixtime);
  if(DEBUG_MODE){
    Serial.println(buff);
  }
  return atol(buff);
}

/*########################################################
###########      FONCTIONS UTILITAIRES        ############
########################################################*/

void toolToDisplay(float myValue,char * myString,int commat){
  dtostrf(myValue,0,commat,myString);
}

String doubleToString(double input,int decimalPlaces)
{
  if(decimalPlaces!=0){
  String string = String((int)(input*pow(10,decimalPlaces)));
  if(abs(input)<1)
    {
    if(input>0)
      string = "0"+string;
    else if(input<0)
      string = string.substring(0,1)+"0"+string.substring(1);
    }
    return string.substring(0,string.length()-decimalPlaces)+"."+string.substring(string.length()-decimalPlaces);
  }
  else 
  {
    return String((int)input);
  }
}

void tetris(){
int melody[] = {
  NOTE_E4, NOTE_B3,NOTE_C4, NOTE_D4, NOTE_D4, NOTE_C4, NOTE_A3, NOTE_A3,
  NOTE_C4, NOTE_E4,NOTE_D4, NOTE_C4, NOTE_B3, NOTE_C4, NOTE_D4, NOTE_E4,
  NOTE_C4, NOTE_A3,NOTE_A3,0,NOTE_D4,NOTE_F4, NOTE_A4, NOTE_G4, NOTE_F4,
  NOTE_E4, NOTE_C4,NOTE_E4, NOTE_D4, NOTE_C4, NOTE_B3, NOTE_C4, NOTE_D4,
  NOTE_E4, NOTE_C4,NOTE_A3, NOTE_A3,0,
  NOTE_E3, NOTE_C3,NOTE_D3, NOTE_B2, NOTE_C3, NOTE_A2, NOTE_GS2, NOTE_B2,0,
  NOTE_E3, NOTE_C3,NOTE_D3, NOTE_B2, NOTE_C3, NOTE_E3, NOTE_A3, NOTE_GS3};

int noteDurations[] = {
  4, 8, 8, 4,8,8,4,8,8,4,8,8,4,8,4,4,4,4,4,8,
  4, 8, 4, 8,8,4,8,4,8,8,4,8,4,4,4,4,4,8,
  2, 2, 2, 2,2, 2, 2, 8, 8,2,2,2,2,4,4,2,2};

  for (int thisNote = 0; thisNote < 55; thisNote++) {
    int noteDuration = 1100/noteDurations[thisNote];
    tone(PIN_TONE, melody[thisNote],noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(PIN_TONE);
  }
}
/*########################################################
###########  FONCTIONS NON VITALES AU SYSTEME ############
########################################################*/

/*
 * Mode demo
 * Active pendant 5 secondes chacun des actionneurs
 */
void modeDemo(){
  int myDelay = 3000;
  int cardID = checkCardRFID();
   if( cardID != 0) return;
  relay_Led24(RELAY_ON);
  LCD_display(0,0,rly_Led24.caption,ALN_LEFT,0,1);
  LCD_display(0,0,"ON",ALN_RIGHT,0,0);
  cardID = checkCardRFID();
  if( cardID != 0) return;
  delay(myDelay);
  relay_Led24(RELAY_OFF);
  LCD_display(0,0,rly_Led24.caption,ALN_LEFT,0,1);
  LCD_display(0,0,"OFF",ALN_RIGHT,0,0);
  relay_Led12(RELAY_ON);
  LCD_display(0,1,rly_Led12.caption,ALN_LEFT,0,0);
  LCD_display(0,1,"ON",ALN_RIGHT,0,0);
  cardID = checkCardRFID();
  if( cardID != 0) return;
  delay(myDelay);
  relay_Led12(RELAY_OFF);
  LCD_display(0,0,rly_Led12.caption,ALN_LEFT,0,1);
  LCD_display(0,0,"OFF",ALN_RIGHT,0,0);
  relay_FanInj(RELAY_ON);
  LCD_display(0,1,rly_FanInj.caption,ALN_LEFT,0,0);
  LCD_display(0,1,"ON",ALN_RIGHT,0,0);
  cardID = checkCardRFID();
   if( cardID != 0) return;
  delay(myDelay);
  relay_FanInj(RELAY_OFF);
  LCD_display(0,0,rly_FanInj.caption,ALN_LEFT,0,1);
  LCD_display(0,0,"OFF",ALN_RIGHT,0,0);
  relay_FanExt(RELAY_ON);
  LCD_display(0,1,rly_FanExt.caption,ALN_LEFT,0,0);
  LCD_display(0,1,"ON",ALN_RIGHT,0,0);
  cardID = checkCardRFID();
   if( cardID != 0) return;
  delay(myDelay);
  relay_FanExt(RELAY_OFF);
  LCD_display(0,0,rly_FanExt.caption,ALN_LEFT,0,1);
  LCD_display(0,0,"OFF",ALN_RIGHT,0,0);
  relay_Pump(RELAY_ON);
  LCD_display(0,1,rly_Pump.caption,ALN_LEFT,0,0);
  LCD_display(0,1,"ON",ALN_RIGHT,0,0);
  cardID = checkCardRFID();
   if( cardID != 0) return;
  delay(myDelay);
  relay_Pump(RELAY_OFF);
  LCD_display(0,0,rly_Pump.caption,ALN_LEFT,0,1);
  LCD_display(0,0,"OFF",ALN_RIGHT,0,0);
  relay_FanRef(RELAY_ON);
  LCD_display(0,1,rly_FanRef.caption,ALN_LEFT,0,0);
  LCD_display(0,1,"ON",ALN_RIGHT,0,0);
  cardID = checkCardRFID();
   if( cardID != 0) return;
  delay(myDelay);
  relay_FanRef(RELAY_OFF);
  LCD_display(0,0,rly_FanRef.caption,ALN_LEFT,0,1);
  LCD_display(0,0,"OFF",ALN_RIGHT,0,0);
  cardID = checkCardRFID();
   if( cardID != 0) return;
  delay(myDelay); 
}



