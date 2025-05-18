#include "DHTesp.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHT_PIN 15
#define POT_PIN 34
#define OUTPUT_PIN 2 
#define ADC_RESOLUTION 4095


LiquidCrystal_I2C lcd(0x27, 16, 2);


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DHTesp dhtSensor;
float lastSetpoint = -1;
float lastTemp = -1;
bool outputState = false;

void setup() {
  Serial.begin(115200);
  
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  
  lcd.init();
  lcd.backlight();
  
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Fallo en el OLED"));
    while (1);
  }
  oled.clearDisplay();
  
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, LOW);

  analogReadResolution(12);
}

void loop() {
  int raw = analogRead(POT_PIN);
  float setpoint = (raw / (float)ADC_RESOLUTION) * 100.0;
  TempAndHumidity data = dhtSensor.getTempAndHumidity();
  float temp = data.temperature;
  
  if (!isnan(temp)) {
    if (temp > setpoint) {
      digitalWrite(OUTPUT_PIN, HIGH);
      outputState = true;
    } else {
      digitalWrite(OUTPUT_PIN, LOW);
      outputState = false;
    }
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Setpoint:");
  lcd.setCursor(10, 0);
  lcd.print(setpoint, 1);
  lcd.print("C");
  
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 0);
  oled.print("Temp: ");
  oled.print(temp, 1);
  oled.println(" C");

  oled.setCursor(0, 16);
  oled.print("Salida: ");
  oled.println(outputState ? "ON" : "OFF");
  oled.display();

  delay(1000);
}
