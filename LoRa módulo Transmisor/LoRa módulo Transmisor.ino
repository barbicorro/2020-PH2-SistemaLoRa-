
#include "heltec.h" //librería para poder usa la placa
#include "images.h" //librería para poder mostrar gráficos en el display de pixeles


#define BAND    915E6  //Banda en la que se van a transmitir los datos, el      número corresponde a la banda europea

unsigned int counter = 0;
String rssi = "RSSI --";
String packSize = "--";
String packet ;

void logo()
{
  /*
  * Configuración del logo de LoRa en el display durante el arranque de la placa
  */
  Heltec.display->clear();
  Heltec.display->drawXbm(0,5,logo_width,logo_height,logo_bits);
  Heltec.display->display();
}

void setup()
{
  //Se configura la transmisión LoRa
  Heltec.begin(true /*DisplayEnable Enable*/, 
			   true /*Heltec.Heltec.Heltec.LoRa Disable*/,
			   true /*Serial Enable*/, 
			   true /*PABOOST Enable*/, 
			   BAND /*long BAND*/);
 
 /* Configuración del display y muestra del mensaje de inicio*/ 
 Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
  Heltec.display->clear();
  
  /* Mensaje de fin de inicialización y de espera a una conexión entrante */
  Heltec.display->drawString(0, 0, "Heltec.LoRa Initial success!");
  Heltec.display->display();
  delay(1000);
}

void loop()
{
 /*
  * Configuración inicial del display
  */
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  
  Heltec.display->drawString(0, 0, "Sending packet: ");//Se muestra en pantalla que se va a mandar un nuevo dato
  Heltec.display->drawString(90, 0, String(counter));//se muestra el valor del contador que será enviado
  Heltec.display->display();

  // send packet
  LoRa.beginPacket();//Se carga un nuevo paquete para transmitirlo
  
  LoRa.setTxPower(15,RF_PACONFIG_PASELECT_PABOOST);//Se configura la transmisión con un Tx Power de 15.
 /*
  * se realiza el envío del dato
  */
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();//se indica el final de la transmisión

  counter++;//se incrementa el contador
  /*
  * Se muesta un feedback visual en la placa para indicar que se produjo un nuevo envío
  */
  digitalWrite(LED, HIGH);   // enciende el LED
  delay(1000);               // espera un segundo
  digitalWrite(LED, LOW);    // apaga el LED
  delay(1000);               // espera un segundo
}
