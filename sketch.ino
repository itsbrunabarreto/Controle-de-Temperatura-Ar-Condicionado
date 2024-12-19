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

enum EstadoClimatizacao
{
  Ventilando = 0, 
  Refrigerando = 1, 
  IncrTempI = 2, 
  DecrTempI = 3, 
  IncrSpeedR = 4, 
  DecrSpeedR = 5, 
  DecrSpeedV = 6, 
  IncrSpeedV = 7
} estadoClim;


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
  estadoClim = Ventilando;
  
}

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

void processamentoOn_Off()
{
  if(digitalRead(Control))
  {
    delay(300);
    estadoControle = PID;
  }

  digitalWrite(LED_PID, LOW);

  if (estadoClim != Ventilando) 
  {
    estadoClim = Ventilando;
  }

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

void processamentoVenti()
{
  sensors.requestTemperatures();

  float temperaturaS = sensors.getTempCByIndex(0);

  if(temperaturaS > (TempI + TempT))
  {
    estadoClim = Refrigerando;
  }

  if(digitalRead(UpVel))
  {
    delay(300);
    estadoClim = IncrSpeedV;
  }

  if(digitalRead(DownVel))
  {
    delay(300);
    estadoClim = DecrSpeedV;
  }

  digitalWrite(FAN, HIGH);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Estado: Ventilando - Temperatura: ");
  Serial.println(temperaturaS);
}

void processamentoRefri()
{ 
  sensors.requestTemperatures();

  float temperaturaS = sensors.getTempCByIndex(0);
  
  if(temperaturaS < (TempI + TempT))
  {
    estadoClim = Ventilando;
  }

  if(digitalRead(UpTemp))
  {
    delay(300);
    estadoClim = IncrTempI;
  }

  if(digitalRead(DownTemp))
  {
    delay(300);
    estadoClim = DecrTempI;
  }

  if(digitalRead(UpVel))
  {
    delay(300);
    estadoClim = IncrSpeedR;
  }

  if(digitalRead(DownVel))
  {
    delay(300);
    estadoClim = IncrSpeedR;
  }


  digitalWrite(FAN, HIGH);
  digitalWrite(COMPRESSOR, HIGH);
  Serial.println("Estado Refrigerando");

}

void processamentoIncrTempI()
{

  if(TempI < 29)
  {
    TempI++;
  }

  estadoClim = Refrigerando;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Speed: ");
  Serial.print(Speed);

  Serial.println("Estado IncrTempI");
}

void processamentoDecrTempI()
{

  if(TempI > 5)
  {
    TempI--;
  }

  estadoClim = Refrigerando;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Speed: ");
  Serial.print(Speed);

  Serial.println("Estado DecrTempI");
}

void processamentoIncrSpeedV()
{
  
  if(Speed < 10)
  {
    Speed++;
  }

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Speed: ");
  Serial.println(Speed);

  Serial.println("Estado IncrSpeedV");

  estadoClim = Ventilando;
}

void processamentoDecrSpeedV()
{

  if(Speed > 1)
  {
    Speed--;
  }

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Speed: ");
  Serial.println(Speed);

  Serial.println("Estado DecrSpeedV");

  estadoClim = Ventilando;
}

void processamentoDecrSpeedR()
{
  
  if(Speed > 1)
  {
    Speed--;
  }

  estadoClim = Ventilando;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Speed: ");
  Serial.println(Speed);

  Serial.println("Estado DecrSpeedR");
}

void processamentoIncrSpeedR()
{
  
  if(Speed < 10)
  {
    Speed++;
  }

  estadoClim = Ventilando;

  digitalWrite(FAN, LOW);
  digitalWrite(COMPRESSOR, LOW);
  Serial.print("Speed: ");
  Serial.println(Speed);

  Serial.println("Estado IncrSpeedR");
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
    switch(estadoClim)
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

  delay(500); 
}
