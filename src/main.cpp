#include "FS.h"
#include <DHT.h>
#include <WiFi.h>
#include <Wire.h>
#include <DHT_U.h>
#include <iostream>
#include <Arduino.h>
#include <PubSubClient.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_BusIO_Register.h>

// Definicion de pines
#define pin1 2
#define BMP_SCK (22)
#define BMP_MOSI (21)
#define SensorDeLluvia (32)
float temperature2, pressure2, altitude2;

// Configuracion del servidor Mqtt//
const char *mqttUser = "Tu_Usuario";
const char *mqttPassword = "Tu_Contraseña";
const char* mqtt_server = "mqtt.rosariot.com";
const int mqtt_port = 1880;
const char *mqttServer2 = "broker.hivemq.com";
const int mqttPort2 = 1883;

// Wifi//
const char *ssid = "BA Escuela";
const char *password = "";
unsigned long channelID = 1332310;
const char *WriteAPIKey = "5SJCDAUGILT6TYMW";
WiFiClient cliente;
PubSubClient client(cliente);

// Sensores de temperatura ,y humedad, y de presion atmosferica//
DHT dht(pin1, DHT11);
Adafruit_BMP085 bmp;

// Topics MQTT //
const char *humidityTopic = "/ET28/65/RBM/Humidity";
const char *temperatureTopic = "/ET28/65/RBM/Temperature";
const char *pressureTopic = "/ET28/65/RBM/Pressure";
const char *windTopic = "/ET28/65/RBM/Wind";
const char *rainTopic = "/ET28/65/RBM/Rain";

// Funcion que permite leer el BMP (sensor de presion y altitud)//
void leerBmp()
{
  float seaLevelPressure = 1013.25;
  float pressure = bmp.readPressure() / 100.0;
  float temp = bmp.readTemperature();
  Serial.print("Temperatura bmp: ");
  Serial.print(temp);
  Serial.println("°C");
  Serial.print("Presion bmp: ");
  Serial.print(pressure);
  Serial.println("hPa");
  Serial.println("-------------------------");
}

// Funcion que permite leer el DHT11 (sensor de temperatura y humedad)//
void leerDht11()
{
  float t1 = dht.readTemperature();
  float h1 = dht.readHumidity();
  Serial.print("temperatura DHT11: ");
  Serial.print(t1);
  Serial.println("°C");
  Serial.print("humedad DHT11: ");
  Serial.print(h1);
  Serial.println("%");
  Serial.println("-------------------------");
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword))
    {
      Serial.println("conectado");
    }
    else
    {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando nuevamente en 5 segundos");
      delay(5000);
    }
  }
}

String howMuchRain()
{
  int valorSensorLluvia = analogRead(SensorDeLluvia);
  int PocaLluvia = 3000;
  int MuchaLluvia = 2300;
  Serial.println(valorSensorLluvia);
  if (valorSensorLluvia > PocaLluvia)
  {
    return "No está lloviendo.";
  }
  else if (valorSensorLluvia > MuchaLluvia && valorSensorLluvia < PocaLluvia)
  {
    return "Está lloviendo un poco.";
  }
  else if (isnan(valorSensorLluvia))
  {
    return "Error al leer sensor de lluvia";
  }
  else if (valorSensorLluvia < MuchaLluvia)
  {
    return "Esta Lloviendo mucho";
  }

  return "No hay datos disponibles, revise el sensor";
}

void enviarDatosMqtt()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  String humidity = String(dht.readHumidity()); 
  String temperature = String(dht.readTemperature());
  String pressure = String(bmp.readPressure() / 100.0);
  String rain = howMuchRain();

  client.publish(rainTopic, rain.c_str());
  client.publish(temperatureTopic, temperature.c_str());
  client.publish(humidityTopic, humidity.c_str());
  client.publish(pressureTopic, pressure.c_str());
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Conectando a WIFI");
  client.setServer(mqtt_server, mqtt_port);
  while(!client.connected()){
    client.setServer(mqttServer2,mqttPort2);
  }
  WiFi.begin(ssid, password);
  Wire.begin(21, 22);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi conectado");

  dht.begin();
  bmp.begin(0x33);
}

void loop()
{
  delay(1000);
  leerDht11();
  delay(1000);
  leerBmp();
  delay(1000);
  String lluvia = howMuchRain();
  Serial.print(lluvia);
  enviarDatosMqtt();
  Serial.println("datos enviados");
  delay(8000);
}