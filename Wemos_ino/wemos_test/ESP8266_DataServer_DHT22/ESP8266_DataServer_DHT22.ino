/*
Autor: David Castillo Alvarado
Documento: Código de ejecución Wemos mini D1
Proyecto: Registro de temperatura (DHT22) en un Servidor local SQL, con interfaz gráfica.
Descripción: Las temperaturas medidas por el sensor DHT22 son registradas en el servidos SQL
             el cual tiene un alojamiento local.
             El acceso a los datos se realiza por medio de una interfaz gráfica de usuario 
             a través del buscador web (Chrome).
FUNCIONAMIENTO::::::::::::::::::::
  El sistema realiza lecturas independientemente si está o no conectado al servidor SQL
  El Sistema hace parpadear 2 veces por segundo el led D4 cuando no hay conexión WIFI
  EL sistema hace parpadear 1 vez cada 9 segundos cuando no hay conexión con el HOST
  El sistema se conecta/reconecta automáticamente cuando encuentra a la red WIFI o al HOST configurado.
CONEXIÓN::::::::::::::::::::::::::
  PIN D2 ->Pin2 DHT22 
  PIN D1 -> Conexión al sistema de enfriamiento - LED Azul
  PIN D3 -> Conexión al sistema de calefacción  - LED Rojo 
  DHT22         - 5.00V
  Wemos mini D1 - 5.00V
Fuentes:
  https://ioticos.com/
  https://www.highcharts.com/
  https://www.apachefriends.org/es/index.html
  http://php.net/
  https://colorlib.com/wp/free-bootstrap-admin-dashboard-templates/
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
bool calienta=1;
bool enfria=1;
float h;
float t;
float hic;
bool control_temp=1;

//Introduce los datos de PIN y tipo de sensor dentro de la clase DHT      

int DIRECTA = D1;                      // Pines para controlar el aire acondicionado
int INVERSA = D3;                      // Pines para controlar el aire acondicionado
double SP_temp=20;                     // Valor predeterminado del setpoint de temperatura
double *SP_t=&SP_temp;                 // definiendo puntero para el SetPoint de la temperatura

String serie="1005";                   // Personaliza a cada Dispositivo
const char* ssid     = "shepar_N7";   // Nombre de la red WIFI
const char* password = "ghost101";     // Clave de la red wifi

const char* host = "192.168.0.103";   // IP de la PC en donde esta la base de datos
const int   baud=115200;
const int httpPort = 80;               // Puerto de comunicación por defecto

WiFiClient client;                     // Inicia el cliente para poder enviar codigo

void conection_wifi(){
// Inicia la conexión con la red WIFI
      Serial.println();
      Serial.print("Connecting to ");
      Serial.println(ssid);

      WiFi.begin(ssid, password);

     // Espera a que se establesca la conexión con la red ...
      while (WiFi.status() != WL_CONNECTED) { 
        delay(250);
        digitalWrite(D4,LOW);
        delay(250);
        digitalWrite(D4,HIGH);
        Serial.print(".");
      }

     // Imprime cuando se tiene un conexión exitosa
      Serial.println("");
      Serial.println("WiFi connected");  
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
}


void setup() {
  // Inicia la transferencia de datos a travez del monitor serial
      Serial.begin(baud);
      delay(10);
   
  //Setup del DHT22
      pinMode(DIRECTA, OUTPUT);         // Directa aumenta la temperatura dentro de la casa
      pinMode(INVERSA, OUTPUT);         // Inversa reduce la temperatura dentro de la casa
      pinMode(D4, OUTPUT);              // Indicador de Estado de conexión Wifi
      digitalWrite(D4,LOW);
      digitalWrite(DIRECTA,LOW);
      digitalWrite(INVERSA,LOW);
      delay(80);
  //Inicia la conexion con La red WiFI
  conection_wifi();
}

void loop() {

  hic = 20;

//////////////////////////////////ENVIO DE DATOS AL SERVIDOR /////////////////////////////
  Serial.print("Conectando al Servidor : ");
  Serial.println(host);

  if (!client.connect(host, httpPort) || WiFi.status() != WL_CONNECTED) {
    Serial.println("Conexioon Fallida Con el Servidor");
        digitalWrite(D4,LOW);
        delay(1000);
        digitalWrite(D4,HIGH);
    if (WiFi.status() != WL_CONNECTED){
        Serial.println("Conexioon Fallida con el WIFI");
        //Reinicia la conexion con La red WiFI
        conection_wifi();
    }
    return;
  }

  // Creamos la dirección en formato de tipo String, donde se encuentra alojado nuestro servidor SQL
    String url = "http://" + String(host) + "/registro_temp/in_data.php";
  // creo el formato de tipo String de los datos que se van a enviar
    String data = "serie=" + serie + "&temp=" + String(hic);
    Serial.println(data);

  //Imprime la dirección en donde se aloja el servidor SQL
    Serial.print("Requesting URL: ");
    Serial.println(url);

  // Solicitud del tipoPOST enviado al servidor para registrar las lecturas del sensor
    client.print(String("POST ") + url + " HTTP/1.0\r\n" +
                 "Host: " + host + "\r\n" +
                 "Accept: *" + "/" + "*\r\n" +
                 "Content-Length: " + data.length() + "\r\n" +
                 "Content-Type: application/x-www-form-urlencoded\r\n" +
                 "\r\n" + data);
    delay(20);

  // Imprime las respuesta del servidor
    Serial.println("Respond:");
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
  }

  // se cierra la conexión
    Serial.println(" ... closing connection\n");
}

