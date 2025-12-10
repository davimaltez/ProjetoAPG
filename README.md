# ProjetoAPG

## O que é?

Detector de possíveis furtos de energia ("gato"), utilizando Arduino UNO + ESP32.

O Arduino faz leituras dos sensores (ACS712) e envia linhas CSV pela Serial para o ESP32. O ESP32 grava as medições numa Google Sheets e envia alerta via WhatsApp (CallMeBot) quando detecta um desvio.

---

## Estrutura do projeto

```
ProjetoAPG/
│
├── esquema_circuito/
│   └── circuito.png
│
├── arduino/
│   └── arduino.ino
│
└── esp32/
    └── esp32.ino 

```

## Requisitos

* Arduino UNO
* ESP32 com Wi‑Fi
* Sensores ACS712 5A (ou similar)
* Conta Google (Sheets + Apps Script)
* Conexão com Internet
* Conta no CallMeBot (opcional, para WhatsApp)

---

## Configurações iniciais

1. Abra a planilha do Google Sheets e coloque na primeira linha os cabeçalhos:

   * `timestamp | voltagem1 | corrente1 | voltagem2 | corrente2`
2. Renomeie a aba para `dados` (ou ajuste o nome no Apps Script).
3. Copie o ID da planilha (trecho entre `/d/` e `/edit` na URL) — você usará no Apps Script.

---

## Apps Script (Google Sheets)

1. Na planilha: **Extensões → Apps Script**.
2. Cole o seguinte código (substitua `SPREADSHEET_ID` pelo ID copiado):

```javascript
function doGet(e) {
  return handleRequest(e);
}

function doPost(e) {
  return handleRequest(e);
}

function handleRequest(e) {
  if (!e || !e.parameter) return ContentService.createTextOutput("Servidor ativo. Nenhum parâmetro recebido.");

  const SPREADSHEET_ID = "COLE_AQUI_O_ID_DA_PLANILHA"; // coloque o ID
  const sheetName = "dados"; // ajuste se tiver outro nome

  const ss = SpreadsheetApp.openById(SPREADSHEET_ID);
  const sheet = ss.getSheetByName(sheetName);
  if (!sheet) return ContentService.createTextOutput("Erro: aba não encontrada.");

  const tempo = new Date();
  const voltagem1 = e.parameter.voltagem1;
  const corrente1  = e.parameter.corrente1;
  const voltagem2 = e.parameter.voltagem2;
  const corrente2  = e.parameter.corrente2;

  if (!voltagem1 || !corrente1 || !voltagem2 || !corrente2) {
    return ContentService.createTextOutput("Erro: parâmetros faltando.");
  }

  sheet.appendRow([tempo, voltagem1, corrente1, voltagem2, corrente2]);
  return ContentService.createTextOutput("OK");
}
```

3. **Deploy → New deployment → Web App**

   * Execute as: **Me**
   * Who has access: **Anyone**
4. Copie a URL que termina em `/exec` — essa será `scriptURL` no código do ESP32.

---
## Integração com CallMeBot (opcional)
1. Adicione o número *+34 623 78 95 95* ao seu WhatsApp;
2. Envie a seguinte mensagem:

   ```
   I allow callmebot to send me messages
   ```
4. Após isso, o bot enviará sua API_KEY.
   
Acesse o site [CallMeBot](https://www.callmebot.com/blog/free-api-whatsapp-messages/) para mais informações.

## Código do ESP32(variáveis a ajustar)

No `esp32/esp32.ino` atualize:

```cpp
const char* ssid = "SUA_REDE_WIFI";
const char* password = "SUA_SENHA_WIFI";
const char* scriptURL = "https://script.google.com/macros/s/SEU_ID/exec";
const char* phoneNumber = "+55SEUNUMERO"; // se for usar CallMeBot
const char* apiKey = "SUA_API_KEY";         // se for usar CallMeBot
const float GATO_LIMITE = 0.300; // A
```

O Arduino deve enviar por Serial (para o ESP) linhas no formato CSV:

```
V1,C1,V2,C2\n
```

---

## Testes rápidos

* Teste WebApp no navegador:

```
https://script.google.com/macros/s/SEU_ID/exec?voltagem1=2.5&corrente1=0.12&voltagem2=2.6&corrente2=0.05
```

Deve retornar `OK` e inserir uma linha na planilha.

* Suba o sketch no ESP32 e monitore o Serial (115200) para ver envios e alertas.

---

## Criando o gráfico no Google Sheets 

1. Selecione a coluna `timestamp` (A) e `corrente1` (C) — ou selecione `A:E` para várias séries.
2. Menu: **Inserir → Gráfico**.
3. No Editor de gráfico, escolha **Gráfico de linhas**.
4. Ajuste o intervalo para `dados!A2:E` (para excluir cabeçalho). O gráfico atualizará automaticamente conforme novas linhas forem adicionadas.

---
