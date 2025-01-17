#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// Definir el tipo de sensor y el pin de datos del DHT11
#define DHTPIN 4  // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Tipo de sensor (DHT11)
#define RELAY_PIN 2 // Pin del relé

#define START_BUTTON 17 // Pulsador para iniciar el modo de ajuste
#define DECREASE_BUTTON 23 // Pulsador para disminuir el rango
#define INCREASE_BUTTON 15 // Pulsador para aumentar el rango

int blueMin = 10;
int blueMax = 20;
int greenMin = 20;
int greenMax = 24;
int redMin = 24;
int redMax = 40;

volatile bool isAdjustingModeFlag = false;
volatile bool decreaseFlag = false;
volatile bool increaseFlag = false;

bool isAdjustingMode = false; // Variable para indicar si estamos en modo de ajuste

DHT dht(DHTPIN, DHTTYPE);

// Inicializar la pantalla LCD con la dirección I2C (normalmente es 0x27 o 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Asegúrate de usar una librería compatible con ESP32

// Pines del LED RGB
#define RED_PIN 19
#define GREEN_PIN 18
#define BLUE_PIN 5

void setup() {
  // Iniciar la comunicación serie para depuración
  Serial.begin(115200);
  
  // Inicializar el sensor DHT
  dht.begin();

  // Inicializar la pantalla LCD
  lcd.init();
  lcd.backlight();
  
  // Configurar los pines del LED RGB como salida
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Configurar los pines de los pulsadores como entrada
  pinMode(START_BUTTON, INPUT_PULLUP);
  pinMode(DECREASE_BUTTON, INPUT_PULLUP);
  pinMode(INCREASE_BUTTON, INPUT_PULLUP);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Configurar interrupciones
  attachInterrupt(digitalPinToInterrupt(START_BUTTON), startAdjusting, FALLING);
  attachInterrupt(digitalPinToInterrupt(DECREASE_BUTTON), decreaseRange, FALLING);
  attachInterrupt(digitalPinToInterrupt(INCREASE_BUTTON), increaseRange, FALLING);
  
  // Mostrar un mensaje inicial en la pantalla
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Manejar el estado de ajuste
  if (isAdjustingModeFlag) {
    isAdjustingModeFlag = false;
    handleAdjustingMode(); // Llamar a la función para manejar el modo de ajuste
  }

  // Manejar la disminución de rangos
  if (decreaseFlag) {
    decreaseFlag = false;
    if (isAdjustingMode) {
      decreaseRanges(); // Función para disminuir rangos
    }
  }

  // Manejar el aumento de rangos
  if (increaseFlag) {
    increaseFlag = false;
    if (isAdjustingMode) {
      increaseRanges(); // Función para aumentar rangos
    }
  }

  // Leer temperatura y humedad del sensor
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  // Comprobar si la lectura es válida
  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("Error al leer del sensor DHT11");
    lcd.setCursor(0, 0);
    lcd.print("Error en sensor");
  } else {
    // Mostrar los valores en el monitor serie
    Serial.print("Temperatura leída: ");
    Serial.println(temperatura);
    Serial.print("Humedad: ");
    Serial.println(humedad);

    // Mostrar los valores en la pantalla LCD
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperatura);
    lcd.print(" C");
    
    lcd.setCursor(0, 1);
    lcd.print("Hum: ");
    lcd.print(humedad);
    lcd.print(" %");

    // Controlar el color del LED RGB según la temperatura
    controlarLedRGB(temperatura);
  }

  // Esperar 2 segundos antes de la siguiente lectura
  delay(2000);
}

// Controlar el LED RGB según la temperatura
void controlarLedRGB(float temperatura) {
  if (temperatura >= blueMin && temperatura <= blueMax) {
    digitalWrite(BLUE_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(RELAY_PIN, HIGH);
  } else if (temperatura >= greenMin && temperatura <= greenMax) {
    digitalWrite(BLUE_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(RELAY_PIN, HIGH);
  } else if (temperatura >= redMin) {
    digitalWrite(BLUE_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(RED_PIN, LOW);
    digitalWrite(RELAY_PIN,LOW );
  } else {
    digitalWrite(BLUE_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(RELAY_PIN, LOW);
  }
}

// Función para cambiar el modo de ajuste
void handleAdjustingMode() {
  isAdjustingMode = !isAdjustingMode;
  Serial.println(isAdjustingMode ? "Ajuste Activado" : "Ajuste Desactivado");
  lcd.clear();
  lcd.print(isAdjustingMode ? "Ajuste Activado" : "Ajuste Desactivado");
  delay(2000);
  lcd.clear();
}

// Función para disminuir los rangos
void decreaseRanges() {
  blueMin--;
  blueMax--;
  greenMin--;
  greenMax--;
  redMin--;
  redMax--;
  mostrarRangos();
  delay(100); // Añadir un retraso para evitar rebotes
}

// Función para aumentar los rangos
void increaseRanges() {
  blueMin++;
  blueMax++;
  greenMin++;
  greenMax++;
  redMin++;
  redMax++;
  mostrarRangos();
  delay(100); // Añadir un retraso para evitar rebotes
}

// Mostrar los nuevos rangos en el monitor serie
void mostrarRangos() {
  Serial.print("Rango Azul: ");
  Serial.print(blueMin);
  Serial.print(" - ");
  Serial.println(blueMax);
  Serial.print("Rango Verde: ");
  Serial.print(greenMin);
  Serial.print(" - ");
  Serial.println(greenMax);
  Serial.print("Rango Rojo: ");
  Serial.print(redMin);
  Serial.print(" - ");
  Serial.println(redMax);
}

// Funciones de interrupción (simplificadas)
void startAdjusting() {
  isAdjustingModeFlag = true;
}

void decreaseRange() {
  decreaseFlag = true;
}

void increaseRange() {
  increaseFlag = true;
}