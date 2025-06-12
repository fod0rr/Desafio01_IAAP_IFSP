#include <LiquidCrystal.h>

// --- MAPEAMENTO DOS PINOS ---
const int LCD_RS = 12;
const int LCD_EN = 11;
const int LCD_D4 = 9;
const int LCD_D5 = 4;
const int LCD_D6 = 3;
const int LCD_D7 = 2;

const int GREEN_LED_PIN = 5;
const int YELLOW_LED_PIN = 6;
const int RED_LED_PIN = 7;

const int BUZZER_PIN = 10;

const int LDR_PIN = A2;
const int TEMP_PIN = A0;
const int HUMIDITY_PIN = A1;

// --- CONSTANTES DE LÓGICA DO PROJETO ---
const float TEMP_IDEAL_MIN = 10.0;
const float TEMP_IDEAL_MAX = 15.0;
const int HUMIDITY_IDEAL_MIN = 50;
const int HUMIDITY_IDEAL_MAX = 70;
const int LIGHT_DARK_THRESHOLD = 60;
const int LIGHT_BRIGHT_THRESHOLD = 280;

// --- CONFIGURAÇÕES DE TEMPO E AMOSTRAGEM ---
// Atende à parte de "pelo menos 5 leituras" do Requisito 7
const int NUM_SAMPLES = 5;
// Atende à parte de "apresentados a cada 5 segundos" do Requisito 7
const unsigned long READ_INTERVAL = 5000; // 5 segundos

// --- INICIALIZAÇÃO DOS COMPONENTES E VARIÁVEIS GLOBAIS ---
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.begin(16, 2);
  lcd.print("Vinheria Edupe");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(2000);
}

void loop() {
  unsigned long currentMillis = millis();

  // Garante que o ciclo de leitura e exibição ocorra a cada 5 segundos (Requisito 7)
  if (currentMillis - previousMillis >= READ_INTERVAL) {
    previousMillis = currentMillis;

    // Bloco que calcula a média de 5 leituras dos sensores (Requisito 7)
    long lightSum = 0, tempSum = 0, humiditySum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
      lightSum += analogRead(LDR_PIN);
      tempSum += analogRead(TEMP_PIN);
      humiditySum += analogRead(HUMIDITY_PIN);
      delay(10);
    }
    float avgLight = lightSum / (float)NUM_SAMPLES;
    float avgTempRaw = tempSum / (float)NUM_SAMPLES;
    float avgHumidityRaw = humiditySum / (float)NUM_SAMPLES;

    // Conversão dos valores lidos para as unidades corretas
    float voltage = (avgTempRaw / 1023.0) * 5.0;
    float temperatureC = (voltage - 0.5) * 100.0;
    int humidityPercent = map(avgHumidityRaw, 0, 1023, 20, 100);

    // Verificação de todos os estados de alerta
    bool isTempAlert = !(temperatureC >= TEMP_IDEAL_MIN && temperatureC <= TEMP_IDEAL_MAX);
    bool isHumidityAlert = !(humidityPercent >= HUMIDITY_IDEAL_MIN && humidityPercent <= HUMIDITY_IDEAL_MAX);
    bool isLightAlert = (avgLight >= LIGHT_BRIGHT_THRESHOLD);
    bool isMediumLight = (avgLight >= LIGHT_DARK_THRESHOLD && !isLightAlert);

    // Define se o buzzer deve ser ativado, atendendo aos Requisitos 4, 8 e 11
    bool buzzerActive = isTempAlert || isHumidityAlert || isLightAlert;

    // Atualização dos LEDs com base nos alertas
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);

    // Atende à parte do LED dos Requisitos 3 e 11
    if (isHumidityAlert || isLightAlert) {
      digitalWrite(RED_LED_PIN, HIGH);
    }
    // Atende à parte do LED do Requisito 8
    if (isTempAlert) {
      digitalWrite(YELLOW_LED_PIN, HIGH);
    }

    // Lógica para luz normal (só executa se não houver alertas)
    if (!isTempAlert && !isHumidityAlert && !isLightAlert) {
      if (avgLight < LIGHT_DARK_THRESHOLD) {
        // Atende ao Requisito 1
        digitalWrite(GREEN_LED_PIN, HIGH);
      } else {
        // Atende à parte do LED do Requisito 2
        digitalWrite(YELLOW_LED_PIN, HIGH);
      }
    }

    // Atualização do display LCD
    lcd.clear();

    // Hierarquia de mensagens para o display
    if (isLightAlert) {
      // Atende à parte do display do Requisito 3
      lcd.setCursor(0, 0);
      lcd.print("Ambiente muito");
      lcd.setCursor(0, 1);
      lcd.print("CLARO");
    } else if (isMediumLight) {
      // Atende à parte do display do Requisito 2
      lcd.setCursor(0, 0);
      lcd.print("Ambiente a meia");
      lcd.setCursor(0, 1);
      lcd.print("luz");
    } else if (isTempAlert) {
      // Este bloco atende aos Requisitos 9 e 10
      if (temperatureC > TEMP_IDEAL_MAX) {
        lcd.setCursor(0, 0);
        lcd.print("Temp. ALTA");
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Temp. BAIXA");
      }
      lcd.setCursor(0, 1);
      lcd.print("Temp. = ");
      lcd.print(temperatureC, 1);
      lcd.print("C");
    } else if (isHumidityAlert) {
      // Requisitos 12 e 13
      if (humidityPercent > HUMIDITY_IDEAL_MAX) {
        lcd.setCursor(0, 0);
        lcd.print("Umidade ALTA");
      } else {
        lcd.setCursor(0, 0);
        lcd.print("Umidade BAIXA");
      }
      lcd.setCursor(0, 1);
      lcd.print("Umidade = ");
      lcd.print(humidityPercent);
      lcd.print("%");
    } else {
      // Se não houver alertas, alterna entre os 3 status OK
      static int okState = 0;

      switch (okState) {
        case 0: // Atende ao Requisito 5
          lcd.setCursor(0, 0);
          lcd.print("Temperatura OK");
          lcd.setCursor(0, 1);
          lcd.print("Temp. = ");
          lcd.print(temperatureC, 1);
          lcd.print("C");
          break;
        case 1: // Atende ao Requisito 6
          lcd.setCursor(0, 0);
          lcd.print("Umidade OK");
          lcd.setCursor(0, 1);
          lcd.print("Umidade = ");
          lcd.print(humidityPercent);
          lcd.print("%");
          break;
        case 2: // Mostra status OK para a luz
          lcd.setCursor(0, 0);
          lcd.print("Luminosidade OK");
          lcd.setCursor(0, 1);
          if (avgLight < LIGHT_DARK_THRESHOLD) {
            lcd.print("Condicao: Escuro");
          } else {
            lcd.print("Condicao: Meia Luz");
          }
          break;
      }
      okState = (okState + 1) % 3; 
    }

    // Ativação do buzzer
    // Este bloco finaliza a lógica dos Requisitos 4, 8 e 11
    if (buzzerActive) {
      tone(BUZZER_PIN, 1000);
    } else {
      noTone(BUZZER_PIN);
    }
  }
}