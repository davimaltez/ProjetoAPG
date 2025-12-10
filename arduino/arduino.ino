const int ACS1_PIN = A0;   // Sensor 1 (ex: Casa 1)
const int ACS2_PIN = A2;   // Sensor 2 (ex: Casa 2)

const int SAMPLES_CAL = 1000;   // amostras para calibração (sem carga)
const int SAMPLES_READ = 500;   // amostras por leitura (média)

const float ADC_TO_V = 5.0 / 1023.0; // Conversão ADC->Volts (UNO)
const float SENSITIVITY = 0.185;     // Sensibilidade ACS712-5A (V/A)

float offset1 = 2.5; // inicial (será calibrado)
float offset2 = 2.5;

void setup() { //Calibração Inicial
  Serial.begin(9600);
  delay(200);
  Serial.println("Arduino: iniciando calibração. REMOVA CARGA dos sensores!");
  
  offset1 = calibrarOffset(ACS1_PIN, SAMPLES_CAL);
  offset2 = calibrarOffset(ACS2_PIN, SAMPLES_CAL);

  Serial.print("Offset1 calibrado (V): ");
  Serial.println(offset1, 4);
  Serial.print("Offset2 calibrado (V): ");
  Serial.println(offset2, 4);
  Serial.println("Calibracao concluida. Enviando leituras...");
}

void loop() {
  // Leitura sensor 1
  float somaV1 = 0.0;

  for (int i = 0; i < SAMPLES_READ; i++) {

    int raw = analogRead(ACS1_PIN);

    somaV1 += (raw * ADC_TO_V);
    delay(1);
  }

  float voltage1 = somaV1 / SAMPLES_READ;

  float current1 = (voltage1 - offset1) / SENSITIVITY; //Fórmula pra calcular corrente

  // Leitura sensor 2
  float somaV2 = 0.0;

  for (int i = 0; i < SAMPLES_READ; i++) {

    int raw = analogRead(ACS2_PIN);

    somaV2 += (raw * ADC_TO_V);
    delay(1);
  }

  float voltage2 = somaV2 / SAMPLES_READ;

  float current2 = (voltage2 - offset2) / SENSITIVITY;

  // Envia para o ESP32: V1,I1,V2,I2
  // Ex: 2.499,0.012,2.503,0.003

  Serial.print(voltage1, 3);
  Serial.print(",");
  Serial.print(current1, 3);
  Serial.print(",");
  Serial.print(voltage2, 3);
  Serial.print(",");
  Serial.println(current2, 3);

  delay(500);
}

// Função de calibração (sem carga conectada ao sensor!)
float calibrarOffset(int pin, int amostras) {
  float soma = 0.0;
  for (int i = 0; i < amostras; i++) {
    int raw = analogRead(pin);
    soma += (raw * ADC_TO_V);
    delay(1);
  }
  return soma / amostras;
}
