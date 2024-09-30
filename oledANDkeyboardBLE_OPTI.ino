#include <BleKeyboard.h>        //Permet de détecter arduino comme périphérique BLE
#include <Wire.h>               // Chargement de la bibliothèque Wire
#include "SH1106Wire.h"   // chargement de la bibliothèque SH1106

SH1106Wire  display(0x3C, 23, 22);  //Création de l'objet display D2 = SDA D1 = SCK ------- Ecran oled 132*64
//BleKeyboard bleKeyboard ;            //Création de l'objet bleKeyboard --------------------- Périphérique Bluetooth
BleKeyboard* bleKeyboard = new BleKeyboard;


#include <FastLED.h>  //Librairie qui gère les leds
#define NUM_LEDS 11   //Nombre de leds
#define DATA_PIN 17   //Broche data leds
#define LED_TYPE    WS2811    //Type LEDS
#define COLOR_ORDER GRB       // Ordre affichage

CRGB leds[NUM_LEDS];    //Créationde l'objet bandeau de leds --> tableau de NUM_LEDS cases

uint8_t gHue = 0; // rotating "base color" used by many of the patterns


//----------------------------------------------DEFINITIONS BROCHES GPIO BOUTTONS DU CLAVIER-------------------------------------------------------------

#define gp_b1 18
#define gp_b2 19
#define gp_b3 16
#define gp_b4 12

#define gp_b5 14
#define gp_b6 27
#define gp_b7 26
#define gp_b8 25

#define gp_b9 33
#define gp_b10 32
#define gp_b11 35
#define gp_b12 34


#define gp_bCHANGE 5    //Bouton de changement de profil

//----------------------------------------------DECLARATION ET INITIALISATION DES TABLEAUX DE BOUTONS -------------------------------------------------------------

bool keyStates[13] = {false, false, false,false, false, false,false, false, false, false, false, false,false};              //TABLEAU ETAT DES TOUCHES 
int keyPins[13] = {gp_b1, gp_b2, gp_b3, gp_b4, gp_b5 , gp_b6 , gp_b7 , gp_b8 , gp_b9 , gp_b10 , gp_b11 , gp_b12 , gp_bCHANGE};       //TABLEAU BROCHES GPIO DES TOUCHES


//************************************CREATION D'UN PROFIL DE JEU SOUS FORME DE STRUCTURE****************************************************************************

struct profil
{
  String p_name;       // NOM PROFIL
  uint8_t pKey[12];    // TABLEAU DE TOUCHES UTILISE PAR LE PROFIL
};

//*******************************************************************************************************************************************************************



//----------------------------------------------CREATION ET INITIALISATION DE PLUSIEURS PROFILS----------------------------------------------------------------------

int j = 0;    //COMPTEUR POUR INDIQUER LE PROFIL ACTIF

profil valo = {"VALO", 0xB0 , 'z','e','r','t', 'y','u','i','o', 'p','q','s'};
profil lol = {"LOL", 'h', 'j','k','l','m', 'w','x','c', 'v', 'b','n','?'};
profil cs = {"CS:GO", '?', '*','1','2','3', '4','5','6', '7', '8','9','0'};

profil actif;

#define NB_PROFIL 3         //NOMBRE DE PROFIL CHARGE

profil tabProfil[NB_PROFIL] = {valo, lol, cs};  //CREATION D'UN TABLEAU DE PROFIL DE TAILLE NB_PROFIL 



/*String fr2en(String text){
  int i = 0;
  String _en = " =qwertyuiopasdfghjkl;zxcvbnQWERTYUIOPASDFGHJKL:ZXCVBNm,./M<>?1234567890!@#$%^&*()",
         _fr = " =azertyuiopqsdfghjklmwxcvbnAZERTYUIOPQSDFGHJKLMWXCVBN,;:!?./ & \"'(- _  1234567890",
         str = "";
  while (text[i] != 0){
    str = str + (String)_en[_fr.indexOf((String)text[i++])];
  }
  return str;
  }*/



void setup() {

  LEDS.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds,NUM_LEDS);
  LEDS.setBrightness(84);

  display.init(); // initialisation de l'objet display
  display.flipScreenVertically(); //rotation de l'écran
  display.setFont(ArialMT_Plain_10); //Chargement de pilotes de caractere
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  actif = tabProfil[0];                         //INITIALISATION DU PROFIL ACTIF = PREMIERE VALEUR DU TABLEAU
  
  Serial.println(actif.p_name);
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  Serial.println(actif.p_name);
  
  setInputs();

//  bleKeyboard.begin();
    bleKeyboard->begin();
    
  preBLE_beforeConnection(WHITE);


}



void loop() {
  int counter;
  preBLE_beforeConnection(BLACK);
  
  //if(bleKeyboard.isConnected()==false){
  if(bleKeyboard->isConnected()==false){
     display.setColor(BLACK);
     display.fillRect(0,0,132,64);
     actifBLE_beforeConnection(WHITE);
     }
  //if (bleKeyboard.isConnected())
  if (bleKeyboard->isConnected())
  {
    actifBLE_beforeConnection(BLACK);
    Serial.println("ble good");
    for (counter = 0; counter < 12 ; counter ++) {
      Serial.println("for good");
      handleButton(counter);
      Serial.println("handle good");
      changeKey();
      Serial.println("changkey good");


    
    }
      drawtouche(WHITE);
      drawBLE(118,10,2,WHITE);
      
      display.drawString(100,25,"Profil");
      drawActiveKey(actif, WHITE);
      display.display();

      sinelon();
      FastLED.show();
      FastLED.delay(1000/100);
      EVERY_N_MILLISECONDS( 20 ) { gHue++; }

  }
  else
  {
    drawBLE(118,10,2,BLACK);
    display.display();
  }
   
}







void handleButton(int keyIndex) {

  if (!digitalRead(keyPins[keyIndex])) {
    if (!keyStates[keyIndex]) {
      keyStates[keyIndex] = true;
      
     // bleKeyboard.press(actif.pKey[keyIndex]);
        bleKeyboard->press(actif.pKey[keyIndex]);
    }
  }
  else {
    if (keyStates[keyIndex]) {
      keyStates[keyIndex] = false;
      
      //bleKeyboard.release(actif.pKey[keyIndex]);
        bleKeyboard->release(actif.pKey[keyIndex]);
      
      color();
    }
  }
}

//------------------------------------------INITIALISATION DES BOUTONS EN PULLUP-----------------------------------------------------------------------------------------------

void setInputs() 
{
  pinMode(gp_b1, INPUT_PULLUP);
  pinMode(gp_b2, INPUT_PULLUP);
  pinMode(gp_b3, INPUT_PULLUP);
  pinMode(gp_b4, INPUT_PULLUP);
  
  pinMode(gp_b5, INPUT_PULLUP);
  pinMode(gp_b6, INPUT_PULLUP);
  pinMode(gp_b7, INPUT_PULLUP);
  pinMode(gp_b8, INPUT_PULLUP);
  
  pinMode(gp_b9, INPUT_PULLUP);
  pinMode(gp_b10, INPUT_PULLUP);
  pinMode(gp_b11, INPUT_PULLUP);
  pinMode(gp_b12, INPUT_PULLUP);
  
  
  pinMode(gp_bCHANGE, INPUT_PULLUP);

}

//-------------------------------------------------------------------GERE LE CHANGEMENT DE PROFIL --> CHANGEMENT DE TOUCHES------------------------------------------------------

void changeKey()
{
  if (!digitalRead(gp_bCHANGE))       //RESISTANCE PULLUP --> APPUYER = 0 / PAS APPUYER = 1
  {
    if (!keyStates[12])                
    {
      keyStates[12] = true;
    }
  }
  else 
  {
    if (keyStates[12])             //SI LE BOUTON  A L'ETAT TRUE 
    {
      keyStates[12] = false;           //RESET ETAT
      if (j > NB_PROFIL-1) j = 0;       //SI J > TAILLE DU TABLEAU DE PROFIL , RAZ
      
      drawtouche(BLACK);            //? KESKE SA FE LA SA /!\ -----------------------------------------------------????????????????
      drawActiveKey(actif,BLACK);     //EFFACE LES LETTRES DES TOUCHES DU PROFILE ACTIF
      actif = tabProfil[j];         // PROFIL ACTIF = J
      j++;
    }
  }
}

//-------------------------------------------------------------------DESSINNE LES 12 TOUCHES [3*4 CARRES]-----------------------------------------------------------------------

void drawtouche(OLEDDISPLAY_COLOR color)
 {
  display.setColor(WHITE);
  for(int j=0;j<=64-16;j+=24)             // REMPLI L'ECRAN 132*64 DE TOUCHES 
  {
    for(int i=0;i<=128-34.905;i+=24)
    {
      display.drawRect(i,j,16,16);        //RECTANGLE BLANC REPRESENTE TOUCHES
      display.setColor(color);
      
    }
  }     
 }
 
//------------------------------------------------------------------------------DESSINE LOGO BLUETOOTH----------------------------------------------------------------------------

void drawBLE(int x, int y, int s, OLEDDISPLAY_COLOR color)      //AFFICHE LE LOGO BLUETOOTH EN X Y DE TAILLE S  - PARAMETRE COLOR POUR ECRIRE/EFFACER
{
  display.setColor(color);                //CALCUL A PARTIR D'UNE IMAGE DE BLE PIXEL
  display.drawLine(x,y-5*s,x,y+5*s);
  display.drawLine(x,y+5*s,x+3*s,y+2*s);
  display.drawLine(x+3*s,y+2*s,x+1*s,y);
  display.drawLine(x+1*s,y,x+3*s,y-2*s);
  display.drawLine(x+3*s,y-2*s,x,y-5*s);
  display.drawLine(x-2*s,y-3*s,x,y);
  display.drawLine(x,y,x-2*s,y+3*s);
}


//--------------------------------------------------------------------------AFFICHE LES TOUCHES DU PROFIL ACTIF----------------------------------------------------------------------

void drawActiveKey(profil actif, OLEDDISPLAY_COLOR color)  //PREND EN PARAMETRE LE PROFIL ACTIF ET LA COULEUR POUR DESSINNER/EFFACER
{

int iB = 0;      //INDICE DE LA TOUCHE A AFFICHER DANS LE TABLEAU DE TOUCHES  --> PAS DE 1

display.setColor(color);
for( int j=0 ; j<=64-16 ; j+=24 )       //POSITION Y DU CARRE BLANC REPRESENTANT LA TOUCHE
{
 for(int i=0;i<=128-34.905;i+=24)       //POSITION  X DU CARRE BLANC REPRESENTANT LA TOUCHE
  {
   char tmp = (char)actif.pKey[iB];       // CONVERTI UINT8 EN CHAR
   String str(tmp); str.toUpperCase();    // CONVERTI LE CHAR EN STRING ET LE MET EN MAJUSCULE
   display.drawString( i+3, j+2 , str);   // AFFICHE AU MILIEU DU CARRE BLANC 
   iB++;            //PASSE A LA TOUCHE SUIVANTE
  }
}
display.drawString(95, 40,actif.p_name);  // AFFICHE LE NOM DU PROFIL ACTIF
}


//----------------------------------------------------------------------GERE L'AFFICHAGE EN FONCTION DE L'ETAT DE LA CONNECTION BLUETOOTH----------------------------------------------

void preBLE_beforeConnection(OLEDDISPLAY_COLOR color)     //COLOR EN PARAMETRE POUR POUVOIR EFFACER/ECRIRE
{
  display.setColor(color);
  display.drawString(16,16, "Starting Bluetooth...");
}


void actifBLE_beforeConnection(OLEDDISPLAY_COLOR color)   
{
  display.setColor(color);
  display.drawString(16,16, "Connecting Bluetooth...");
}


void color()
{
  leds[11] = CRGB::Black;
  for(int i=0; i < NUM_LEDS;i++)
  {
    leds[i] = CRGB(random(0,255),random(0,255),random(0,255));
    leds[i-1]= CRGB::Black;
    FastLED.delay(1000/40);
    
  }
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 40, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}
