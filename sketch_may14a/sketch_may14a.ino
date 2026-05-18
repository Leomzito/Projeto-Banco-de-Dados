#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Coloque aqui o nome e a senha do Roteador Wi-Fi do seu celular
const char* ssid = "NOME_DO_WIFI_DO_CELULAR";
const char* password = "SENHA_DO_WIFI_DO_CELULAR";

// IMPORTANTE: Insira o IP do seu computador na rede do celular (veja o passo 2)
const String serverName = "http://192.168.X.X/sensor.php"; 

// Definição do pino do sensor Hall
#define HALL_SENSOR_PIN 4

volatile unsigned long lastPulseTime = 0;
volatile unsigned long pulseInterval = 0;
volatile bool firstPulse = true;

float wheelDiameter = 0.60; // Diâmetro da roda em metros
float wheelCircumference;
float speedKmh = 0;

// Controle de tempo para envio dos dados
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 2000; 

void IRAM_ATTR detectMagnet() {
    unsigned long currentTime = micros();
    
    // Debounce de 5ms
    if (currentTime - lastPulseTime > 5000) {
        if (!firstPulse) {
            pulseInterval = currentTime - lastPulseTime;
        } else {
            firstPulse = false;
        }
        lastPulseTime = currentTime;
    }
}

void setup() {
    Serial.begin(115200);
    
    // Conectando à rede Wi-Fi do Celular
    Serial.print("Conectando-se a rede: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("");
    Serial.println("Wi-Fi Conectado!");
    Serial.print("IP do ESP32: ");
    Serial.println(WiFi.localIP());
    
    pinMode(HALL_SENSOR_PIN, INPUT_PULLUP);
    
    wheelCircumference = wheelDiameter * 3.141592;
    
    attachInterrupt(digitalPinToInterrupt(HALL_SENSOR_PIN), detectMagnet, FALLING);
    
    Serial.println("Sistema de Velocidade Iniciado");
}

void loop() {
    noInterrupts();
    unsigned long interval = pulseInterval;
    unsigned long timeSinceLastPulse = micros() - lastPulseTime;
    interrupts();
    
    if (timeSinceLastPulse > 2000000 || firstPulse) {
        speedKmh = 0;
        noInterrupts();
        pulseInterval = 0;
        firstPulse = true; 
        interrupts();
    }
    else if (interval > 0) {
        float pulsesPerSecond = 1000000.0 / interval;
        speedKmh = (wheelCircumference * pulsesPerSecond) * 3.6;
    }
    
    // Monitor Serial (via Cabo)
    Serial.print("Velocidade: ");
    Serial.print(speedKmh, 1);
    Serial.println(" km/h");
    
    // Envia para o servidor PHP local via Wi-Fi se estiver conectado
    if (millis() - lastSendTime > sendInterval) {
        lastSendTime = millis();
        
        if (WiFi.status() == WL_CONNECTED) {
            HTTPClient http;
            
            http.begin(serverName);
            http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            
            String httpRequestData = "velocidade=" + String(speedKmh, 2);
            
            int httpResponseCode = http.POST(httpRequestData);
            
            if (httpResponseCode > 0) {
                String response = http.getString();
                Serial.print("[HTTP] Código: ");
                Serial.print(httpResponseCode);
                Serial.print(" | Resposta: ");
                Serial.println(response);
            } else {
                Serial.print("[HTTP] Erro ao enviar POST: ");
                Serial.println(http.errorToString(httpResponseCode).c_str());
            }
            http.end();
        } else {
            Serial.println("[Wi-Fi] Desconectado da rede do celular...");
        }
    }
    
    delay(100);
}