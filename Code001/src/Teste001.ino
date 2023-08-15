//Bibiliotecas
#include <ModbusMaster.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPIFFS.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>

//Definições
#define RS485Serial  Serial2  // Utilize a porta Serial2 para comunicação RS485 (ou outra porta disponível)
#define SLAVE_ADD    1        // Defina o endereço do escravo aqui (pode ser qualquer valor entre 1 e 247) - Definido no proprio Controlador
#define RX_PIN      16        //RX2 
#define TX_PIN      17        //TX2 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
const uint16_t dataTxtTimeInterval = 500;

//Variaveis que tomarão conta da conexão Wifi
const char* ssid = "Dicalab";
const char* password = "dicalab2763";

//Criação de objetos 
ModbusMaster modbus;
Adafruit_SSD1306 display (SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AsyncWebServer server (80);
WebSocketsServer websockets (81);

//........................................Código..................................

void setup() {
  WiFi.begin(ssid, password);

  Serial.begin(115200, SERIAL_8N1);       // Inicializa a comunicação serial para fins de depuração
  RS485Serial.begin(9600, SERIAL_8N2, RX_PIN, TX_PIN);  // Inicializa a comunicação RS485 com a taxa de transmissão desejada
  modbus.begin(SLAVE_ADD, RS485Serial); // Inicializa o ModbusMaster com o endereço do escravo e a porta Serial RS485
 
  //config. da tela
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, true);
  
  //Desenhando o "Iniciando"
  drawIniciando();
  display.display();

//checkando o SPIFFS
  if(!SPIFFS.begin(true)){
      Serial.println();
      Serial.println("Erro ao montar SPIFFS");
      return;
  }
  else{
    Serial.println();
    Serial.println("SPIFFS montado com sucesso!");
  }

  //Conectando o server e criando arquivos index.html e text.html através do SPIFFS
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send(SPIFFS, "/index.html", "text/html"); });
  server.onNotFound(notFound);

  //incializando
  server.begin(); //server assíncrono
  websockets.begin();
  websockets.onEvent(webSocketEvent);
  }


void loop() {
  //Inicializa o websockets
  websockets.loop();

  //Vamos fazer uma checagem para não sobrepor a função, e deixar com uma leitura de 1s (1000ms). 
  //long currentMillis = millis();
  static uint32_t prevMillis = 0;

  //if (currentMillis - lastMillis > 1000){
  if (millis() - prevMillis >= dataTxtTimeInterval){
    //iguala o tempo
    prevMillis = millis();  
    
    Serial.println("Slave " + (String)SLAVE_ADD); //Aqui vai printar o Salve utilizado
    uint8_t result = modbus.readInputRegisters(0x03E8, 1); //uint8_t readInputRegisters(uint16_t address (Registrador Hexadecimal encontrado na tabela), uint16_t quantity, uint16_t *dest);
  
    if (result == modbus.ku8MBSuccess){
      Serial.print("Temperatura: ");
      float temp = modbus.getResponseBuffer(0)/10.0f;
      Serial.println(temp);

      //enviando o dado em Json para o WebSocket
      String data = "{\"temperature\": " + String(temp) + "}";
      websockets.broadcastTXT(data);
      Serial.println();
      Serial.println(data);

        //Printando no display
        drawTemp(temp);
        display.display();
    }
    else{
      getMsgError(&modbus, result);
      drawErro();
      display.display(); 
    }
    //lastMillis = currentMillis;
    Serial.println();
  } 
}

//Função que vai fazer o checkup e caso houver erros, vai identificar qual!
bool getMsgError(ModbusMaster *node, uint8_t result) {
  String tmpstr2 = "\r\n";
  switch (result) {
    case node->ku8MBIllegalFunction:
      tmpstr2 += "Illegal Function";
      break;
    case node->ku8MBIllegalDataAddress:
      tmpstr2 += "Illegal Data Address";
      break;
    case node->ku8MBIllegalDataValue:
      tmpstr2 += "Illegal Data Value";
      break;
     case node-> ku8MBSlaveDeviceFailure:
       tmpstr2 += "Slave Device Failure";
       break;
    case node->ku8MBInvalidSlaveID:
      tmpstr2 += "Invalid Slave ID";
      break;
    case node->ku8MBInvalidFunction:
      tmpstr2 += "Invalid Function";
      break;
    case node->ku8MBResponseTimedOut:
      tmpstr2 += "Response Timed Out";
      break;
    case node->ku8MBInvalidCRC:
      tmpstr2 += "Invalid CRC";
      break;
    default:
      tmpstr2 += "Unknown error: " + String(result);
      break;
  }
  Serial.println(tmpstr2);
  return false;
}

void wifiConnection (){
  WiFi.begin(ssid, password);

  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Conectado ao WiFi com o endereço IP: ");
  Serial.println(WiFi.localIP());
}

void wifiAPmaker(){
  WiFi.softAP("Esp32AP","");
  Serial.println("\nsoftAP");
  Serial.println(WiFi.softAPIP());
}

// void drawWifiSymbol() {
//   display.drawLine(113,28,127,28,WHITE);
//   display.drawLine(115,29,125,29,WHITE);
//   display.drawLine(117,30,123,30,WHITE);
//   display.drawLine(119,31,121,31,WHITE);
// }

void drawTemp(float temp){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Temperatura:");
  display.setTextSize(2);
  display.setCursor(10,10);
  display.print(temp);
  display.print(" C");
}

void drawErro(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,10);
  display.print("Tem algo errado!");
}

void drawIniciando(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("Iniciando...");
}

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		Serial.printf("%02X ", *src);
		src++;
	}
	Serial.printf("\n");
}

//callbacks
void notFound(AsyncWebServerRequest *request){
  request->send(404, "text/plain" , "Página não encontrada!");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = websockets.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
				// send message to client
				websockets.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u\n", num, length);
            hexdump(payload, length);

            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
        case WStype_ERROR:			
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
        Serial.println("Erro nessa caralha");
			break;
    }

}