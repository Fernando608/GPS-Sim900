//Programa de Rastreo GPS
/*Indicaciones:


  Se envia un mensaje al telefono movil del Automovil y este realiza la accion determinada*/
// El Sistema siempre va a estar monitoreando los SMS recibidos

//**********************Librerias*************************//

#include <TinyGPS++.h>//Librería para el manejo del Módulo gps
#include <SoftwareSerial.h>//Librería para crear puertos virtuales en arduino

// *****************************************************************************/////



//*******************Declaracion de Metodos*************************//

SoftwareSerial GPSSerial(7, 8); //Pines Tx y Rx para modulo GPS
TinyGPSPlus gps; //Metodo para usar libreria GPS
//*******************************************************************/////



//*******************Declaracion de Variables Globales *************************//

float latitud, longitud, velocidad;                 //Almacena las variables del sistema de rastreo
int satelites;
//Almacena numeros de satelites
boolean gps_status = 0;                           //Estado del GPS para salir de bucle de reoger muestras

// ***********************************************************************************************************/////////////

//Variables para indentificar SMS////////////////////////////////////////////////////////////////////////////////////////////

char incoming_char = 0;                        //Variable que guarda los caracteres que envia el
String mensaje = "";                          //Variable que almacena los mensajes recibidos
char movil_propietario[] = "3013844644";     //Número de Teléfono del Propietario   Num cel FARCO
//*****************************************************************************************************************************


//*******************Método de Configuracion Inicial*************************//

void setup() {
  power(); //Metodo de Encendido del Sistema
  iniciar(); //Metodo que Inicia todas las configuraciones iniciales
}


//******************* Método Principal Repetitivo *************************//

void loop() {

  //Condicion para leer el puerto serie del Módulo Sim900*** ACTIVO PARA UTILIZAR ENCANSULA EN UNA VARIABLE MENSAJE Y REVISA EL COMANDO CAPTURADO

  if (Serial.available() > 0) {
    incoming_char = Serial.read(); //Guarda los carácter enviados de GPRS
    mensaje = mensaje + incoming_char ; // Guarda los caracteres en un String
    identifica_SMS (); //Llama al metodo identifica SMS
  }
}


//*******************Métodos del Programa*************************//
//***************************************************************//

//****************Metodo de Identificar SMS*************************//
//Analiza el SMS recibido y procede a realizar la accion correspondiente//

void identifica_SMS()
{
  //Claves del Sistema
  int Rastreo = mensaje.indexOf("Sent from your Twilio trial account - 3218915053rastreo"); //Sent from your Twilio trial account - 3218915053rastreo
  int Posicion = mensaje.indexOf("Sent from your Twilio trial account - 3218915053posicion"); //Sent from your Twilio trial account - 3218915053posicionint

  //Si se cumple la condicion del SMS procede a realizar las acciones
  if (Rastreo >= 0)
  {
    rastreogps(); //Método de Envio de datos a Servidor
    mensaje = "" ;
    lcd_sistemalisto();
  }



  if (Posicion >= 0)
  {
    envioposicion();//Metodo de Envio de Datos por SMS
    mensaje = "" ;
    lcd_sistemalisto();
  }
}

//****************Metodo de SMS Posicion*************************//
//Envia un SMS con los datos del GPS//

void envioposicion()
{
  lcd_envioSMS();
enviarsms:
  if (enviarAT("AT+CREG?", "+CREG: 0,1", 1000) == 1)                            //comprueba la conexion a la red GSM
  {
    char aux_str[50];//Variable para concatenar el SMS
    enviarAT("AT+CMGF=1\r", "OK", 1000);                                         //Selecciona Formato para enviar SMS
    getgps(); //Obtiene los datos del GPS
    sprintf(aux_str, "AT+CMGS=\"%s\"", movil_propietario);                       //Numero al que vamos a enviar el mensaje
    if (enviarAT(aux_str, ">", 10000) == 1)
    {
      //Texto del mensaje

      Serial.print(F("Ubicacion:\rhttps://maps.google.com/maps?q="));              //Colocamos la url de google maps
      Serial.print(latitud, 6);                                 //Obtemos los datos de latitud del módulo gps y se lo enviamos al módulo gsm
      Serial.print(F("+"));
      Serial.println(longitud, 6); //Obtemos los datos de longitud del módulo gps y se lo enviamos al módulo gsm
      Serial.print(F("Velocidad:\r"));
      Serial.print(velocidad);
      Serial.print(F("Km/h:\r"));
      Serial.print(F("\r"));
      delay(1000);
      Serial.write(0x1A); //Ctrl+Z para enviar SMS
    }
    //Colocar las variables del GPS en 0 para usar la proxima vez
    latitud = 0;
    longitud = 0;
    velocidad = 0;
  }
  else
  {
    goto enviarsms; //Sino hay señal se envia repite todo el proceso
  }
}

//***************************************************************
//****************METODO DE RASTREO *************************//
//********Envia Datos del GPS a Servidor************************* //
//*****************************************************************//

void rastreogps() {
  envioposicion();

  // volver:
  int i = 0;
  int j = 6; //Numero de muestras a enviar a servidor
  int moto = 123456; //dato de la moto para poderla identificar cuando se envia dato al servidor

  //Ciclo de envio de muestras
  while (1)
  {
    Serial.print("AT+CGDCONT=1,\"IP\",\"internet.comcel.com.co\"");// Establece parametros PDP
    Serial.print((char)13);
    delay(1000);
    Serial.print("AT+CSTT=\"internet.comcel.com.co\",\"comcel\",\"comcel\""); //Seteo de APN usuario y password
    Serial.print((char)13);
    delay(2000);
    Serial.print("AT+CIICR");// Inicia la conexión
    Serial.print((char)13);
    delay(5000);
    Serial.print("AT+CIFSR");
    Serial.print((char)13);
    delay(2000);
    Serial.print("AT+CIPSTART=\"TCP\",\"pruebalicomovil.000webhostapp.com\",\"80\"");// Inicia conexion TCP o UDP
    Serial.print((char)13);
    delay(5000);
    Serial.print("AT+CIPSEND");// Envias Datos TCP o UDP
    Serial.print((char)13);
    delay(2000);
    getgps(); //Metodo para obtener datos del GPS
    Serial.print(F("GET /AgregarArduino/"));// //Archivo donde se enviara el dato y el parametro a enviar
    Serial.print(latitud,6);

      Serial.print(F("/"));
      Serial.print(longitud,6);
      Serial.print(F("/"));
      Serial.print(velocidad);
      Serial.print(F("/"));
      Serial.print(satelites);
      Serial.print(F("/"));
      Serial.print(5);
    
    Serial.print(F(" HTTP/1.1"));
    
    Serial.print((char)13);
    Serial.print((char)10);
    Serial.print("Host: pruebalicomovil.000webhostapp.com");//
    Serial.print((char)13);
    Serial.print((char)10);
    Serial.print((char)10);
    Serial.print((char)13);
    Serial.print((char)10);
    Serial.print((char)26);
    delay(2000);
    latitud = 0;
    longitud = 0;
    velocidad = 0;
    Serial.print("AT+CIPCLOSE");// Cierra la conexión TCP o UDP
    Serial.print((char)13);
    delay(2000);
    //Serial.print("AT+CIPSHUT"); //Cierra el contesto PDP de GPRS
    Serial.print((char)13);

    delay(2000);



    //fin Btrex

  }


} //fin rstreo

//****************Metodo de GPS*************************//
//Obtiene los datos enviados por el GPS, los pasa por la libreria TinyGPS++//
// y los convierte datos legibles para ser ocupados por los diversos metodos//

void getgps() {
  while (1) { //Ingresa a un ciclo infinito hasta que obtenega nuevos datos
    while (GPSSerial.available() > 0) { //Lee los datos seriales de los pines 7 y 8 del modulo GPS
      gps.encode(GPSSerial.read()); //envia los tramas NMEA a la libreria para procesarlas
      if (gps.location.isUpdated()) {
        gps_status = 1;
        //Obtiene las coordenadas en forma legible
        satelites = (gps.satellites.value(), 5);
        latitud = (gps.location.lat());
        longitud = (gps.location.lng());
        velocidad = (gps.speed.kmph());
      }
    }
    if (latitud != 0 || longitud != 0) { //Condicion para romper el Ciclo infinito cuando halle datos GPS
      gps_status = 1;
    } else {
      gps_status = 0;
    }
    if (gps_status)
      break;
  }
}

//****************Método de Encendido del Sistema*************************//
//Enciende el Sistema y sus componentes e inicializa variables//

void power(void)
{
  //************************************************************
  // CONFIGURACIONES DE PINES  O INICIALIZACIO DE SIM900 Y SERIAL DE ARDUINO
  //************************************************************

  Serial.begin(9600); //Configura velocidad de modulo GPRS a 9600 baudios
  GPSSerial.begin(9600); //Configura velocidad de modulo GPS a 9600 baudios

  // Comprueba que el modulo SIM900 esta arrancado
  int respuesta = 0;
  if (enviarAT("AT", "OK", 2000) == 0)
  {
    //Descomentar si se trabaja con shield para arduino uno,
    //Donde el se enciende por medio de pin9

    pinMode(9, OUTPUT);
    digitalWrite(9, LOW);
    delay(1000);
    digitalWrite(9, HIGH);
    delay(2000);
    digitalWrite(9, LOW);
    delay(3000);

    while (respuesta == 0) { //Ciclio que se ejecuta hasta que se encienda el modulo
      respuesta = enviarAT("AT", "OK", 2000);
    }
  }
}

//****************Método de Configarción de SIM900*************************//
//Despues de encenderse el sistema se configura con comandos AT para que funcione//

void iniciar()
{
  //espera hasta estar conectado a la red movil
  while ( enviarAT("AT+CREG?", "+CREG: 0,1", 1000) == 0 )
  {
  }
  enviarAT("AT+CLIP=1\r", "OK", 1000); // Activamos la identificacion de llamadas
  enviarAT("AT+CMGF=1\r", "OK", 1000); //Configura el modo texto para enviar o recibir mensajes
  enviarAT("AT+CNMI=2,2,0,0,0\r", "OK", 1000); //Configuramos el modulo para que nos muestre los SMS recibidos por comunicacion serie
  lcd_sistemalisto();
}

//****************Método para enviar comandos AT*************************//
//Envia Comandos AT de manera sencilla esperando su respuesta a Sim900//
//Se tuliza para esperar la respuesta del modulo SIM900 y realizar un control mejor//


int enviarAT(String ATcommand, char* resp_correcta, unsigned int tiempo)
{
  int x = 0;
  bool correcto = 0;
  char respuesta[100];
  unsigned long anterior;
  memset(respuesta, '\0', 100); // Inicializa el string
  delay(100);
  while ( Serial.available() > 0) Serial.read(); // Limpia el buffer de entrada
  Serial.println(ATcommand); // Envia el comando AT
  x = 0;
  anterior = millis();
  // Espera una respuesta
  do {
    // si hay datos el buffer de entrada del UART lee y comprueba la respuesta
    if (Serial.available() != 0)
    {
      respuesta[x] = Serial.read();
      x++;
      // Comprueba si la respuesta es correcta
      if (strstr(respuesta, resp_correcta) != NULL)
      {
        correcto = 1;
      }
    }
  }
  // Espera hasta tener una respuesta
  while ((correcto == 0) && ((millis() - anterior) < tiempo));
  return correcto;
}
/***********************************************************************/
/*********Metodos para Gráficos de LCD 128x32************************/
/********************************************************************/
/*************Logos************************************************/

/************Texto Estatico LCD ***********************************/

void lcd_sistemalisto() {
  Serial.print("SISTEMA LISTO ESPERANDO MENSAJE MIJO \n");
  delay(100);
}


/************Animaciones de Cargando ...*********/


void lcd_envioSMS() {
  Serial.print("ENVIANDO MENSAJE CON POSICION  \n");
  delay(100);
}



