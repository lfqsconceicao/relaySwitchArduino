#include <EEPROM.h>
#include <Keypad.h>

#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//definição de variavel global
#define idDisplay 0x27
#define colunas 16
#define linhas 2

#define pinSH_CP A1   //Pino Clock SH_CP
#define pinST_CP A2  //Pino Latch  ST_CP
#define pinDS    A3  //Pino Data   DS
#define qtdeCI   2
//objetos
LiquidCrystal_I2C lcd(idDisplay, colunas, linhas);
const byte rows = 7;
const byte cols = 3;
char keys[rows][cols] = {
{'a','f','k'}, 
{'b','g','l'},
{'c','h','m'},
{'d','i','n'},
{'e','j','o'},
{'p','p','p'},
{'q','q','q'},
};
byte rowPins[rows] = {2,3,4,5,6,12,13};
byte colPins[cols] = {7,8,9};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);
int relayPin[5] = {10,11,12,13,14};
int ledPin[5] = {15,16,17,18,19};

int relayICPin[5] = {0,1,2,3,4};
int relayICPinState[5] = {0,0,0,0,0};
byte midiChannel = 0;
int numBank = 0;
int maxBank = 4;

bool estadoNum = true;
int cursorNum[5] = {12,10,8,6,4};

/******************************************************/
/*
const byte adress[11][8] = {
{0,0,0,0,0,0,1,1}, //0
{1,0,0,1,1,1,1,1}, //1
{0,0,1,0,0,1,0,1}, //2
{0,0,0,0,1,1,0,1}, //3
{1,0,0,1,1,0,0,1}, //4
{0,1,0,0,1,0,0,1}, //5
{0,1,0,0,0,0,0,1}, //6
{0,0,0,1,1,1,1,1}, //7
{0,0,0,0,0,0,0,1}, //8
{0,0,0,0,1,0,0,1}, //9
{1,0,0,1,0,0,0,0} //.
};
*/
/******************************************************/
void setup()
{
  pinMode(pinSH_CP, OUTPUT);
  pinMode(pinST_CP, OUTPUT);
  pinMode(pinDS, OUTPUT);
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0,0);
  scrollInitialMessage(0,"Inicializando...", 100,16);
  /*lcd.print("Inicializando");
  for(int i=0; i<5; i++) 
  {
   // pinMode(relayPin[i], OUTPUT);
    //digitalWrite(relayPin[i], HIGH);
    ci74HC595Write(relayICPin[i], HIGH);
    delay(300);
    //digitalWrite(relayPin[i], LOW);
    ci74HC595Write(relayICPin[i], LOW);
    ci74HC595Write(i+8, HIGH);
    delay(300);
    ci74HC595Write(i+8, LOW);
    
  }*/
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PRESET:");
  lcd.setCursor(10, 0);
  lcd.print("BNK:");
  digitarBanco(0);
readPreset(11, 1, 0); /* initiate  default mode */
Serial.begin(31250); /* for midi communication - pin 1 TX */
/*for (int i = 0; i < 512; i++){
   EEPROM.write(i, 0); 
} // erase eeprom (optional)
  */

   
}
/*********************************************************/
void midiProg(byte status, int data) 
 {
  Serial.write(status);
  Serial.write(data);
 }
 /*********************************************************/
void memory(int addr, int led)
{
  int enderecoRom = 0;
  for(int i=0; i<5; i++)
  {
    //enderecoRom = enderecoRom + (digitalRead(relayPin[i])<<i);
    enderecoRom = enderecoRom + (relayICPinState[i]<<i);
    //digitalWrite(ledPin[i], LOW); // all leds reset
    ci74HC595Write(i+8,LOW);
  }
  Serial.println(enderecoRom);

  EEPROM.update(addr +(numBank*5) , enderecoRom);
  
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(4, 1);
  lcd.print("Salvo!");
  
  delay(100);
  ci74HC595Write(led+8, HIGH);
  delay(100); 
  ci74HC595Write(led+8,  LOW);
  delay(100); 
  ci74HC595Write(led+8,  HIGH);
  delay(100);
  ci74HC595Write(led+8,  LOW);
  delay(100); 
  ci74HC595Write(led+8,  HIGH);
  delay(100);
  ci74HC595Write(led+8,  LOW);
  delay(100); 
  ci74HC595Write(led+8,  HIGH);

  lcd.setCursor(0,1);
  lcd.print("                ");
  for(int i =0;i< 5;i++){
    turnEffect(i+1, relayICPinState[i]);
  }

}

/*********************************************************/
void resetAllRelays()
{
  for(int i=0; i<5; i++)
  {
    digitalWrite(relayPin[i], LOW);
  }
}
/*********************************************************/
void resetAllLeds()
{
  for(int i=0; i<5; i++)
  {
    ci74HC595Write(i+8, LOW);
  }
}
/*********************************************************/
void writeOut(int relay)
{
  //resetAllLeds();
  //digitalWrite(relayPin[relay], !digitalRead(relayPin[relay]));

  relayICPinState[relay] = !relayICPinState[relay];
  ci74HC595Write(relayICPin[relay], relayICPinState[relay]);
  

  //digitalWrite(ledPin[relay], digitalRead(relayPin[relay]));
  ci74HC595Write(relay+8, relayICPinState[relay]);
  turnEffect(relay+1, relayICPinState[relay]);
  /* thanks to  anton.efremoff.1 for this tip */
}
/*********************************************************/
void readPreset(int addr, int pcNum, int led)
{
  int endereco = EEPROM.read(addr+(numBank*5));

  for(int i=0; i<5; i++)
  {
    int estado = (endereco & 1<<i) !=0;
    relayICPinState[i] = estado;
    ci74HC595Write(relayICPin[i], estado);

    //digitalWrite(relayPin[i], estado);
    ci74HC595Write(i+8, LOW);
    ci74HC595Write(led+8, HIGH);

    turnEffect(i+1, relayICPinState[i]);
  }
  digitarPreset(addr%10);

  midiProg(0xC0 | midiChannel , pcNum);  /* send midi change program 1 */
}
/*********************************************************/
void changeBank(bool up){
  if(up){
    numBank++;
    if(numBank > maxBank){
      numBank = 0;
    }

  } else {
    numBank--;
    if(numBank < 0){
      numBank = maxBank;
    }
  }
  digitarBanco(numBank);
  //Serial.println(numBank);
}


/*********************************************************/
void ci74HC595Write(byte pino, bool estado) {
static byte ciBuffer[qtdeCI];

//usado pois o pino 12 do projeto original estava com defeito, apenas remover
if(pino==12){
  pino++;
}

bitWrite(ciBuffer[pino / 8], pino % 8, estado);

digitalWrite(pinST_CP, LOW); //Inicia a Transmissão

digitalWrite(pinDS, LOW);    //Apaga Tudo para Preparar Transmissão
digitalWrite(pinSH_CP, LOW);

for (int nC = qtdeCI-1; nC >= 0; nC--) {
    for (int nB = 7; nB >= 0; nB--) {

        digitalWrite(pinSH_CP, LOW);  //Baixa o Clock      
        
        digitalWrite(pinDS,  bitRead(ciBuffer[nC], nB) );     //Escreve o BIT
        
        digitalWrite(pinSH_CP, HIGH); //Eleva o Clock
        digitalWrite(pinDS, LOW);     //Baixa o Data para Previnir Vazamento      
    }  
}

digitalWrite(pinST_CP, HIGH);  //Finaliza a Transmissão

}

void digitarBanco(int num){
  lcd.setCursor(14, 0);
  lcd.print(num);
  /*
  for(int i=0;i<8;i++){
    ci74HC595Write(i, adress[num][i]);
  }*/
}

void digitarPreset(int num){
  lcd.setCursor(8, 0);
  lcd.print(num);
}



void ligarLed(int numLed, bool estado){
  ci74HC595Write(numLed + 8, estado);
}

void turnEffect(int num, bool estado){
   lcd.setCursor(cursorNum[num-1],1);
   if(estado){
      lcd.print(num);
    }else{
      lcd.print(" ");
    }
}
void scrollMessage(int row, String message, int delayTime, int totalColumns) {
  for (int i=0; i < totalColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int position = 0; position < message.length(); position++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(position, position + totalColumns));
    delay(delayTime);
  }
}


void scrollInitialMessage(int row, String message, int delayTime, int totalColumns) {
  int times = 0;
  int relay = 0;
  for(int i=0;i<16;i++){
    ci74HC595Write(i, 0);
  }

  
  for (int i=0; i < totalColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int position = 0; position < message.length(); position++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(position, position + totalColumns));
    
    if(times == 3 && relay < 5){
      ci74HC595Write(relayICPin[relay], HIGH);
      ci74HC595Write(relay+8, HIGH);
      
    }

    delay(delayTime);
    if(times ==3 && relay < 5){
      ci74HC595Write(relayICPin[relay], LOW);
      ci74HC595Write(relay+8, LOW);
    }
    times++;
    if(times > 3){
      times = 0;
      relay++;
    }
  }
}


/*********************************************************/


void loop()



{
  
  char key = keypad.getKey();
  if(key)  // Check for a valid key.
  {
   switch (key)
      { 
    case 'a':  // a to x 
      writeOut(0); // relay
      break; 
    case 'b': 
      writeOut(1);
      break;
    case 'c': 
      writeOut(2);
      break;
    case 'd': 
      writeOut(3);
      break;
    case 'e': 
      writeOut(4);
      break;  
    /****************************** STORE PRESET MODE */       
    case 'f': 
      memory(11,0);  //addr, led
      break; 
    case 'g': 
      memory(12,1);
      break;
    case 'h': 
      memory(13,2);
      break;
    case 'i': 
      memory(14,3);
      break;
    case 'j': 
      memory(15,4);
      break;
    /****************************** READ PRESET MODE */      
    case 'k':  
      readPreset(11, 1, 0); // addr, pcNum, relay
      break; 
    case 'l':  
      readPreset(12, 2, 1);
      break;   
    case 'm': 
      readPreset(13, 3, 2);
      break;
    case 'n': 
      readPreset(14, 4, 3);
      break;
    case 'o': 
      readPreset(15, 5, 4);
      break;   

    /****************************** CHANGE BANK MODE */ 
    case 'p':
    changeBank(true);
      break;
    case 'q':
    changeBank(false);
      break;
    }
           
   }
}
