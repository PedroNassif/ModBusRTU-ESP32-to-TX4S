//Bibiliotecas
#include <ModbusMaster.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


//Definições
#define RS485Serial  Serial2  // Utilize a porta Serial2 para comunicação RS485 (ou outra porta disponível)
#define SLAVE_ADD    1        // Defina o endereço do escravo aqui (pode ser qualquer valor entre 1 e 247) - Definido no proprio Controlador
#define RX_PIN      16        //RX2 
#define TX_PIN      17        //TX2 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

//Variaveis que tomarão conta da conexão Wifi
const char* ssid = "Dicalab";
const char* password = "dicalab2763";

//Criação de instâncias 
ModbusMaster modbus;
Adafruit_SSD1306 display (SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


/////////////////////////////////////////////////////////Código//////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200, SERIAL_8N1);       // Inicializa a comunicação serial para fins de depuração
  RS485Serial.begin(9600, SERIAL_8N2, RX_PIN, TX_PIN);  // Inicializa a comunicação RS485 com a taxa de transmissão desejada
  modbus.begin(SLAVE_ADD, RS485Serial); // Inicializa o ModbusMaster com o endereço do escravo e a porta Serial RS485
  wifiConnection();

  //config. da tela
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS, true);
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("Iniciando...");
  display.display();
}

//variavel que vai armazenar o tempo para dar um delay - millis()= Retorna o número de milissegundos passados desde que a placa Arduino começou a executar o programa atual. Esse número irá sofrer overflow (chegar ao maior número possível e então voltar pra zero), após aproximadamente 50 dias.
unsigned long lastMillis = 0;


void loop() {
  //Vamos fazer uma checagem para não sobrepor a função, e deixar com uma leitura de 1s (1000ms). 
  long currentMillis = millis();

  if (currentMillis - lastMillis > 1000){
    Serial.println("Slave " + (String)SLAVE_ADD); //Aqui vai printar o Salve utilizado
    uint8_t result = modbus.readInputRegisters(0x03E8, 1); //uint8_t readInputRegisters(uint16_t address (Registrador Hexadecimal encontrado na tabela), uint16_t quantity, uint16_t *dest);
  
    if (result == modbus.ku8MBSuccess){
      Serial.print("Temperatura: ");
      float temp = modbus.getResponseBuffer(0)/10.0f;
      Serial.println(temp);

        //Printando no display
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,0);
        display.println("Temperatura:");
        display.setTextSize(2);
        display.setCursor(10,10);
        display.print(temp);
        display.print(" C");
        display.write(9);
        drawWifiSymbol();
        display.display();
    }
    else{
      getMsgError(&modbus, result);
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0,10);
        display.print("Tem algo errado!");
        display.display();
      
    }
    lastMillis = currentMillis;
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

void drawWifiSymbol() {

  display.drawLine(113,28,127,28,WHITE);
  display.drawLine(115,29,125,29,WHITE);
  display.drawLine(117,30,123,30,WHITE);
  display.drawLine(119,31,121,31,WHITE);

}