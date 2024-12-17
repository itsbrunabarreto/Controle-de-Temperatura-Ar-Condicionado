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
#define POWER_PIN 12
#define TEMP_SENSOR_PIN 13
#define FAN_PIN 19
#define COMPRESSOR_PIN 18
#define LED_ON_PIN 5
// Sensor de temperatura
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
// Estados do sistema
enum Estado {DESLIGADO, LIGADO};
Estado estadoAtual;
// Evitar múltiplas leituras durante um único clique no botão
bool powerPressed = false;

void setup()
{
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
  estadoAtual = DESLIGADO;
}
void estadoDesligado()
{
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(COMPRESSOR_PIN, LOW);
  digitalWrite(LED_ON_PIN, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistema: OFF");
}

void estadoLigado()
{
  digitalWrite(LED_ON_PIN, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistema: ON");
  // Atualizar temperatura
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temperatureC);
  lcd.print((char)223);
  lcd.print("C");
}
void loop()
{
  // Verificar o botão Power
  if (digitalRead(POWER_PIN) && !powerPressed)
  {
    powerPressed = true;
    delay(50);
    while (digitalRead(POWER_PIN));
    if (estadoAtual == DESLIGADO)
    {
      estadoAtual = LIGADO;
    } 
    else
    {
      estadoAtual = DESLIGADO;
    }
    }
      powerPressed = false;
      switch (estadoAtual)
      {
        case DESLIGADO:
          estadoDesligado();
          break;
        case LIGADO:
          estadoLigado();
          break;
      }
  delay(500);
}