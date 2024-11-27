# Situa-o-Problema-2
Repositório para Situação Problema 2  utilizando ESP32, sensores de temperatura e MQTT.
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>

// Configuração do Wi-Fi
const char* ssid = "SEU_SSID";
const char* password = "SUA_SENHA";

// Configuração MQT
const char* mqtt_server = "broker.mqtt-dashboard.com";
WiFiClient espClient;
PubSubClient client(espClient);

// Configuração do DHT
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Configuração do OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pino do relé
#define RELAY_PIN 5

void setup() {
  Serial.begin(115200);
  
  // Inicializar Wi-Fi
  setup_wifi();

  // Inicializar MQTT
  client.setServer(mqtt_server, 1883);
  
  // Inicializar DHT
  dht.begin();
  
  // Inicializar Display
  if (!display.begin(SSD1306_I2C_ADDRESS, 0x3C)) {
    Serial.println(F("Falha ao iniciar o display!"));
    for (;;);
  }
  display.clearDisplay();

  // Configurar Relé
  pinMode(RELAY_PIN, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Ler temperatura e umidade
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Atualizar Display
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temp);
  display.print(" C");
  display.display();

  // Publicar dados MQTT
  String payload = String("{\"temperature\":") + temp + ",\"humidity\":" + hum + "}";
  client.publish("home/temperature", payload.c_str());

  // Lógica de controle
  if (temp > 25) {
    digitalWrite(RELAY_PIN, HIGH); // Liga o sistema de resfriamento
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }

  delay(2000);
}

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado!");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      client.subscribe("home/control");
    } else {
      delay(5000);
    }
  }
}
