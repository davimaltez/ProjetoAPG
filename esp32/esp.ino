#include <WiFi.h>
#include <HTTPClient.h>

// ----------------------------
// CONFIG Wi-Fi
// ----------------------------
const char* ssid = "uaifai-tiradentes";
const char* password = "bemvindoaocesar";

// ----------------------------
// GOOGLE SHEETS WEB APP
// ----------------------------
const char* scriptURL = "https://script.google.com/macros/s/AKfycbzQWJ39LfO4rySVE2p-AouBnPO77gtcrWHw38OZ4yCGGf4jlmdhHthsW9vTKXon3z95/exec";

// ----------------------------
// WHATSAPP VIA CALLMEBOT
// ----------------------------
const char* phoneNumber = "+558193142442";
const char* apiKey = "7722631";

// ----------------------------
// SERIAL COM ARDUINO
// ----------------------------
#define RX2_PIN 16
#define TX2_PIN 17

bool alertaEnviado = false;

// Controle do envio da planilha
unsigned long lastSheetsSend = 0;
const unsigned long sheetsInterval = 1000; // Enviar para Sheets a cada 1s

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RX2_PIN, TX2_PIN);

  // Limpa o buffer inicial do Arduino
  delay(200);
  while (Serial2.available()) Serial2.readStringUntil('\n');

  // ----------------------------
  // CONECTANDO AO WIFI
  // ----------------------------
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// --------------------------------------------------------------
// ENVIO PARA GOOGLE SHEETS (OTIMIZADO — NÃO TRAVA O LOOP)
// --------------------------------------------------------------
void enviarParaPlanilha(float V1, float C1, float V2, float C2) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sheets: Wi-Fi OFF");
    return;
  }

  HTTPClient http;
  http.setTimeout(500); // timeout reduzido (0.5s)

  String url = String(scriptURL) +
               "?voltagem1=" + String(V1, 3) +
               "&corrente1=" + String(C1, 3) +
               "&voltagem2=" + String(V2, 3) +
               "&corrente2=" + String(C2, 3);

  http.begin(url);
  http.GET();  // Envio rápido, sem leitura de resposta
  http.end();
}

// --------------------------------------------------------------
// ENVIO WHATSAPP
// --------------------------------------------------------------
void sendMessage(String message) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi não conectado!");
    return;
  }

  HTTPClient http;
  http.setTimeout(600); // evita travamento

  message.replace(" ", "%20");

  String url = "https://api.callmebot.com/whatsapp.php?phone=" +
               String(phoneNumber) +
               "&text=" + message +
               "&apikey=" + String(apiKey);

  Serial.println("Enviando mensagem...");
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("HTTP: %d\n", httpCode);
  } else {
    Serial.printf("Erro ao enviar mensagem: %d\n", httpCode);
  }

  http.end();
}

// --------------------------------------------------------------
// LOOP PRINCIPAL
// --------------------------------------------------------------
void loop() {
  if (Serial2.available()) {
    String linha = Serial2.readStringUntil('\n');
    linha.trim();

    if (linha.length() == 0) return;

    Serial.print("Recebido: ");
    Serial.println(linha);

    float V1, C1, V2, C2;

    int matched = sscanf(linha.c_str(), "%f,%f,%f,%f", &V1, &C1, &V2, &C2);

    if (matched != 4) {
      Serial.println("Linha inválida, ignorada.");
      return;
    }

    // ----------------------------
    // ENVIO PARA A PLANILHA (1x/segundo)
    // ----------------------------
    unsigned long now = millis();
    if (now - lastSheetsSend > sheetsInterval) {
      enviarParaPlanilha(V1, C1, V2, C2);
      lastSheetsSend = now;
    }

    // ----------------------------
    // DETECÇÃO DO GATO
    // ----------------------------
    if (C1 > 0.300 && !alertaEnviado) {
      Serial.println("⚠️ ALERTA: POSSÍVEL DESVIO DE ENERGIA!");
      sendMessage("ALERTA: Possivel gato de energia detectado na area do CESAR!");
      alertaEnviado = true;
    }

    if (C1 < 0.300) {
      alertaEnviado = false;
    }
  }

  delay(10);
}
