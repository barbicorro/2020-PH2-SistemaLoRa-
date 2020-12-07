/*
	Este es el código del módulo que recibe datos por medio
	de una comunicación LoRa, levanta un wifi con AccesPoint
	y un servidor HTTP donde muestra los datos recibidos cuando
	el usuario lo solicita a través de la página.
*/
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include "heltec.h"
#include "images.h"

#define BAND    915E6 //Banda en la que se van a transmitor los datos, el número corresponde a la banda europea 
/*información que se va a mostrar en el display*/
String rssi = "RSSI --";
String packSize = "--";
String packet ; //variable para guardar el dato recibido
String header; // Variable para guardar el HTTP request

//---------------------Credenciales de WiFi-----------------------
const char *ssid = "LoRaNetwork";//  Identificador de la red
const char *password = "70217021";// Contraseña de la red

//------------------Servidor Web en puerto 80---------------------
WiFiServer server(80);

//------------------------Codigo HTML------------------------------
String HttpWebPageHeader =
  "<!DOCTYPE html>"
  "<html>"
  "<head><title>Sistema LoRa</title>"
  "<meta http-equiv=\"refresh\" content=\"15\">"
  "</head>"
  "<p style=\"text-align: center;\">&nbsp;</p>"
  "<p style=\"text-align: center;\"><span style=\"color: #800000;\"><strong>Sistema LoRa</strong></span></p>"
  "<h1 style=\"text-align: center;\"><span style=\"color: #800000;\"><strong> Bienvenido al Servidor Web HTTP<br />Proyecto LoRa</strong></span></h1>"
  "<h4 style=\"text-align: center;\"><strong><em>En esta pagina web se muestran datos enviados por el Modulo LoRa Transmisor,</em></strong><br /><strong><em> cuando se presiona el boton de DATOS.</em></strong></h4>"
  "<center>"
  "<p><a href='/D'><button style='height:50px;width:100px'>DATOS</button></a></p>"
  "</center>"
  "<p style=\"text-align: center;\">&nbsp;</p>"
  "<p style=\"text-align: center;\">&nbsp;</p>"
  ;

String HttpWebPageEnd =
  "<p style=\"text-align: center;\">&nbsp;</p>"
  "<p style=\"text-align: center;\">&nbsp;</p>"
  "<hr />"
  "<p style=\"text-align: center;\"><em>Nordio Mauricio -&nbsp;Ibanez Barbara -&nbsp;</em><em>Corro Barbara</em></p>"
  "<p style=\"text-align: center;\">TALLER DE PROYECTO II 2020</p>"
  "</html>";

String HttpWebPageBody = "El dato recibido es: ";

//----------------------------Funciones----------------------------------
void logo() {
  /*
    Configuración del logo de LoRa en el display durante el arranque de la placa
  */
  Heltec.display->clear();
  Heltec.display->drawXbm(0, 5, logo_width, logo_height, logo_bits);
  Heltec.display->display();
}

void cbk(int packetSize) {
  /*se construye la cadena de caracteres que se va a mostrar en el display*/
  packet = "";
  packSize = String(packetSize, DEC);
  for (int i = 0; i < packetSize; i++) {
    packet += (char) LoRa.read();
  }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;//se lee carácter por carácter el dato recibido
  LoRaData();
}

void LoRaData() {
  /*
    Configuración de la comunicación LoRa con feedback en el display
  */
  Heltec.display->clear();//borra la información del display
  /*configura tipo de fuente y alineación del texto*/
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  /*se imprime el dato recibido*/
  Heltec.display->drawString(0 , 15 , "Received " + packSize + " bytes");
  Heltec.display->drawStringMaxWidth(0 , 26 , 128, packet);
  Heltec.display->drawString(0, 0, rssi);
  Heltec.display->display();
}

//---------------------------SETUP--------------------------------
void setup() {
  Serial.begin(115200);      
  Serial.println();
  Serial.println("Configurando el access point...");

  // Configura el Acces Point
  Serial.println(WiFi.softAP(ssid, password) ? "Funcionó el softAP" : "Falló el softAP");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server iniciado");
  // escribe como conectarse a la página
  Serial.print("Para ver la pagina del servidor, escriba en la barra del navegador http://");
  Serial.println(myIP);

  //Se configura la transmisión LoRa
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
  /* Configuración del display y muestra del mensaje de iniciio*/
  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
  Heltec.display->clear();

  /* Mensaje de fin de inicializaciín y de espera a una conexión entrante */
  Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  Heltec.display->drawString(0, 10, "Wait for incoming data...");
  Heltec.display->display();
  delay(1000);
  //LoRa.onReceive(cbk);
  LoRa.receive(); //Instrucción para que node comience a recibir información

}

//----------------------------LOOP----------------------------------
void loop() {

  int packetSize = LoRa.parsePacket(); //Recibe el tamaño del dato transmitido
  if (packetSize) {
    cbk(packetSize); //si el tamaño del paquete es mayor disque cero hace la lectura desde el buffer de entrada
  }

  WiFiClient client = server.available();   // Escucha a los clientes entrantes

  if (client) {                             // Si se conecta un nuevo cliente
    Serial.println("New client");
    String currentLine = "";                // crea un String para guardar datos entrantes del cliente
    while (client.connected()) {            // loop mientras el cliente está conectado
      if (client.available()) {             // si hay bytes para leer desde el cliente
        char c = client.read();             // lee un byte
        Serial.write(c);                    // imprime ese byte en el monitor serial
        header += c;
        if (c == '\n') {                    // si el byte es un caracter de salto de linea

          // Si la nueva linea está en blanco significa que es el fin del
          // HTTP request del cliente, entonces respondemos:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            // client.println("Connection: close");
            client.println();

            // Revisa si la solicitud del cliente fue "GET /D":
            if (header.indexOf("GET /D") >= 0) {
              //sprintf(HttpWebPageBody,"%s%s","El dato recibido es:","hola soy un dato");
              client.print(HttpWebPageHeader + "<center>" + HttpWebPageBody + packet + "</center>" + HttpWebPageEnd);
            } else {
              // envia la pagina
              //sprintf(textoHttp,"%s%s",HttpWebPageHeader,HttpWebPageEnd);
              client.print(HttpWebPageHeader + HttpWebPageEnd);
            }

            // la respuesta HTTP temina con una linea en blanco
            client.println();
            break;
          } else {
            currentLine = "";// si tenemos una nueva linea limpiamos currentLine
          }
        } else if (c != '\r') {  // si C es distinto al caracter de retorno de carro
          currentLine += c;      // lo agrega al final de currentLine
        }
      }
    }
    // Limpiamos la variable header
    header = "";
    // cierra la conexión:
    client.stop();
    Serial.println("cliente desconectado");
  }
}
