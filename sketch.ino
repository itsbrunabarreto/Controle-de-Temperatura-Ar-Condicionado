#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Inicializar o display I2C no endereço 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pinos I2C no ESP32
#define SDA_PIN 21
#define SCL_PIN 22

// Mapear GPIOs
#define POWER 12
#define TempS 13
#define UpTemp 14
#define DownTemp 27
#define UpVel 26
#define DownVel 25
#define Mode 33
#define Control 32
#define FAN 19
#define COMPRESSOR 18
#define LED_ON 5
#define LED_PID 23

// Sensor de temperatura
OneWire onewire (TempS);
DallasTemperature sensors(&onewire);


enum EstadoLigaDesliga
{
  Desligado = 0, Ligado = 1
} estadoLigaDesliga;

enum EstadoControle
{
  On_Off = 0, PID = 1
} estadoControle;

enum EstadoClimatizacaoOnOff
{
  Ventilando = 0, 
  Refrigerando = 1, 
  IncrTempI = 2, 
  DecrTempI = 3, 
  IncrSpeedR = 4, 
  DecrSpeedR = 5, 
  DecrSpeedV = 6, 
  IncrSpeedV = 7
} estadoClimOnOff;

enum EstadoClimatizacaoPID
{
  Ventilando_PID = 0, 
  Refrigerando_PID = 1, 
  IncrTempI_PID = 2, 
  DecrTempI_PID = 3, 
  IncrSpeedR_PID = 4, 
  DecrSpeedR_PID = 5, 
  DecrSpeedV_PID = 6, 
  IncrSpeedV_PID = 7
} estadoClimPID;


unsigned long previousMillis = 0;  // Armazena o tempo anterior
const long interval = 1000;  // Intervalo de 1 segundo para alternar
bool mostrarTemp = true;  // Flag para alternar entre TempI e Speed

int TempI = 23, Speed = 5, TempT = 2, Att = 5;

void setup() 
{
  Serial.begin(115200);

  // Inicializar o barramento I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Inicializar o display
  lcd.begin(16, 2);
  lcd.backlight();
  

  pinMode(POWER, INPUT_PULLDOWN);
  pinMode(UpTemp, INPUT_PULLDOWN);
  pinMode(DownTemp, INPUT_PULLDOWN);
  pinMode(UpVel, INPUT_PULLDOWN);
  pinMode(DownVel, INPUT_PULLDOWN);
  pinMode(Mode, INPUT_PULLDOWN);
  pinMode(Control, INPUT_PULLDOWN);
  pinMode(FAN, OUTPUT);
  pinMode(COMPRESSOR, OUTPUT);
  pinMode(LED_ON, OUTPUT);
  pinMode(LED_PID, OUTPUT);

  sensors.begin();

  estadoLigaDesliga = Desligado;
  estadoControle = On_Off;
  estadoClimOnOff = Ventilando;
  
}

//ESTADOS DIAGRAMA LIGA/DESLIGA

void processamentoDesligado() 
{
  if(digitalRead(POWER))
  {
    delay(300);
    estadoLigaDesliga = Ligado;
  }

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  digitalWrite(LED_ON, LOW);

  lcd.clear();
  Serial.println("Estado Desligado");
}

void processamentoLigado()
{
  if(digitalRead(POWER))
  {
    delay(300);
    estadoLigaDesliga = Desligado;
  }

  digitalWrite(LED_ON, HIGH);

  Serial.println("Estado Ligado");

}

//ESTADOS DIAGRAMA CONTROLE

void processamentoOn_Off()
{
  if(digitalRead(Control))
  {
    delay(300);
    estadoControle = PID;
  }

  digitalWrite(LED_PID, LOW);

  lcd.setCursor(0, 0);
  lcd.print("On/Off");
}

void processamentoPID()
{
  if(digitalRead(Control))
  {
    delay(300);
    estadoControle = On_Off;
  }

  digitalWrite(LED_PID, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PID");
}

//ESTADOS DIAGRAMA CLIMATIZAÇÃO ON OFF

void processamentoVenti()
{
  sensors.requestTemperatures();

  float temperaturaS = sensors.getTempCByIndex(0);

  if(temperaturaS > (TempI + TempT))
  {
    estadoClimOnOff = Refrigerando;
  }

  if(digitalRead(UpVel))
  {
    delay(300);
    estadoClimOnOff = IncrSpeedV;
  }

  if(digitalRead(DownVel))
  {
    delay(300);
    estadoClimOnOff = DecrSpeedV;
  }

  digitalWrite(FAN, HIGH);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado: Ventilando - Temperatura: ");
  Serial.println(temperaturaS);
  
  lcd.setCursor(0, 1);
  lcd.print("Speed: ");
  lcd.print(Speed);
}

void processamentoRefri()
{ 
  sensors.requestTemperatures();

  float temperaturaS = sensors.getTempCByIndex(0);
  
  if(temperaturaS < (TempI + TempT))
  {
    estadoClimOnOff = Ventilando;
  }

  if(digitalRead(UpTemp))
  {
    delay(300);
    estadoClimOnOff = IncrTempI;
  }

  if(digitalRead(DownTemp))
  {
    delay(300);
    estadoClimOnOff = DecrTempI;
  }

  if(digitalRead(UpVel))
  {
    delay(300);
    estadoClimOnOff = IncrSpeedR;
  }

  if(digitalRead(DownVel))
  {
    delay(300);
    estadoClimOnOff = DecrSpeedR;
  }

  digitalWrite(FAN, HIGH);
  digitalWrite(COMPRESSOR, HIGH);
  Serial.print("Estado: Refrigerando - Temperatura: ");
  Serial.println(temperaturaS);

  unsigned long currentMillis = millis();  
  
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;  
    
    // Alterna entre mostrar TempI e Speed
    lcd.setCursor(0, 1);
    if (mostrarTemp) {
      lcd.print("Speed: ");  
      lcd.print(Speed); 
    } else {
      lcd.print("Temp: "); 
      lcd.print(TempI);  
    }
    
    mostrarTemp = !mostrarTemp;
  }
}

void processamentoIncrTempI()
{

  if(TempI < 29)
  {
    TempI++;
  }

  estadoClimOnOff = Refrigerando;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado IncrTempI - Temperatura Alvo: ");
  Serial.println(TempI);
}

void processamentoDecrTempI()
{

  if(TempI > 16)
  {
    TempI--;
  }

  estadoClimOnOff = Refrigerando;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado DecrTempI - Temperatura Alvo: ");
  Serial.println(TempI);
}

void processamentoIncrSpeedV()
{
  
  if(Speed < 10)
  {
    Speed++;
  }

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado IncrSpeedV - Speed: ");
  Serial.println(Speed);

  estadoClimOnOff = Ventilando;
}

void processamentoDecrSpeedV()
{

  if(Speed > 1)
  {
    Speed--;
  }

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado DecrSpeedV - Speed: ");
  Serial.println(Speed);

  estadoClimOnOff = Ventilando;
}

void processamentoDecrSpeedR()
{
  
  if(Speed > 1)
  {
    Speed--;
  }

  estadoClimOnOff = Refrigerando;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado DecrSpeedR - Speed: ");
  Serial.println(Speed);
}

void processamentoIncrSpeedR()
{
  
  if(Speed < 10)
  {
    Speed++;
  }

  estadoClimOnOff = Refrigerando;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado IncrSpeedR - Speed: ");
  Serial.println(Speed);
}

//ESTADOS DIAGRAMA CLIMATIZAÇÃO PID

void processamentoVenti_PID()
{
  if(digitalRead(Mode))
  {
    estadoClimPID = Refrigerando_PID;
  }

  if(digitalRead(UpVel))
  {
    delay(300);
    estadoClimPID = IncrSpeedV_PID;
  }

  if(digitalRead(DownVel))
  {
    delay(300);
    estadoClimPID = DecrSpeedV_PID;
  }

  digitalWrite(FAN, HIGH);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado PID: Ventilando - Speed: ");
  Serial.println(Speed);

  lcd.setCursor(0, 1);
  lcd.print("Speed: ");
  lcd.print(Speed);
}

void processamentoRefri_PID()
{ 
  sensors.requestTemperatures();

  float temperaturaS = sensors.getTempCByIndex(0);
  
  if(digitalRead(Mode))
  {
    estadoClimPID = Ventilando_PID;
  }

  if(digitalRead(UpTemp))
  {
    delay(300);
    estadoClimPID = IncrTempI_PID;
  }

  if(digitalRead(DownTemp))
  {
    delay(300);
    estadoClimPID = DecrTempI_PID;
  }

  if(digitalRead(UpVel))
  {
    delay(300);
    estadoClimPID = IncrSpeedR_PID;
  }

  if(digitalRead(DownVel))
  {
    delay(300);
    estadoClimPID = DecrSpeedR_PID;
  }


  digitalWrite(FAN, HIGH);
  digitalWrite(COMPRESSOR, HIGH);
  Serial.print("Estado PID: Refrigerando - Temperatura: ");
  Serial.println(temperaturaS);

  unsigned long currentMillis = millis();  
  
  if (currentMillis - previousMillis >= interval) 
  {
    previousMillis = currentMillis;  
    
    // Alterna entre mostrar TempI e Speed
    lcd.setCursor(0, 1);
    if (mostrarTemp) {
      lcd.print("Speed: ");  
      lcd.print(Speed); 
    } else {
      lcd.print("Temp: "); 
      lcd.print(TempI);  
    }
    
    mostrarTemp = !mostrarTemp;
  }
}

void processamentoIncrTempI_PID()
{

  if(TempI < 29)
  {
    TempI++;
  }

  estadoClimPID = Refrigerando_PID;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado IncrTempI - Temperatura Alvo: ");
  Serial.println(TempI);
}

void processamentoDecrTempI_PID()
{

  if(TempI > 16)
  {
    TempI--;
  }

  estadoClimPID = Refrigerando_PID;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado DecrTempI - Temperatura Alvo: ");
  Serial.println(TempI);
}

void processamentoIncrSpeedV_PID()
{
  
  if(Speed < 10)
  {
    Speed++;
  }

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado IncrSpeedV - Speed: ");
  Serial.println(Speed);

  estadoClimPID = Ventilando_PID;
}

void processamentoDecrSpeedV_PID()
{

  if(Speed > 1)
  {
    Speed--;
  }

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado DecrSpeedV - Speed: ");
  Serial.println(Speed);

  estadoClimPID = Ventilando_PID;
}

void processamentoDecrSpeedR_PID()
{
  
  if(Speed > 1)
  {
    Speed--;
  }

  estadoClimPID = Refrigerando_PID;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado DecrSpeedR - Speed: ");
  Serial.println(Speed);
}

void processamentoIncrSpeedR_PID()
{
  
  if(Speed < 10)
  {
    Speed++;
  }

  estadoClimPID = Refrigerando_PID;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado IncrSpeedR - Speed: ");
  Serial.println(Speed);
}


void loop()
 {

  switch (estadoLigaDesliga) 
  {
    case Desligado:
      processamentoDesligado();
      break;
    case Ligado:
      processamentoLigado();
      break;
  }

  if(estadoLigaDesliga == Ligado)
  {
    switch (estadoControle) 
    {
      case On_Off:
        processamentoOn_Off();
        break;
      case PID:
        processamentoPID();
        break;
    }
  }
  

  if(estadoControle == On_Off && estadoLigaDesliga == Ligado)
  {
    switch(estadoClimOnOff)
    {
      case Ventilando:
        processamentoVenti();
        break;
      case Refrigerando:
        processamentoRefri();
        break;
      case IncrTempI:
        processamentoIncrTempI();
        break;
      case DecrTempI:
        processamentoDecrTempI();
        break;
      case IncrSpeedR:
        processamentoIncrSpeedR();
        break;
      case DecrSpeedR:
        processamentoDecrSpeedR();
        break;
      case DecrSpeedV:
        processamentoDecrSpeedV();
        break;
      case IncrSpeedV:
        processamentoIncrSpeedV();
        break;
    }
  }


  if(estadoControle == PID && estadoLigaDesliga == Ligado)
  {
    switch(estadoClimPID)
    {
      case Ventilando_PID:
        processamentoVenti_PID();
        break;
      case Refrigerando_PID:
        processamentoRefri_PID();
        break;
      case IncrTempI_PID:
        processamentoIncrTempI_PID();
        break;
      case DecrTempI_PID:
        processamentoDecrTempI_PID();
        break;
      case IncrSpeedR_PID:
        processamentoIncrSpeedR_PID();
        break;
      case DecrSpeedR_PID:
        processamentoDecrSpeedR_PID();
        break;
      case DecrSpeedV_PID:
        processamentoDecrSpeedV_PID();
        break;
      case IncrSpeedV_PID:
        processamentoIncrSpeedV_PID();
        break;
    }
  }

  delay(500); 
}
