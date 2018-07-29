#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include "math.h"

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Wire.h>
#include <ESP8266WiFi.h>


/*--------------------------MPU 6050---------------------------------*/
// La dirección del MPU6050 puede ser 0x68 o 0x69, dependiendo 
// del estado de AD0. Si no se especifica, 0x68 estará implicito
MPU6050 sensor_ang;
// Valores RAW (sin procesar) del acelerometro y giroscopio en los ejes x,y,z
int16_t ax, ay, az;
int16_t gx, gy, gz;
double ax_d, ay_d, az_d;
double  ang_x, ang_y;
float ang_x_prev, ang_y_prev;
long tiempo_prev;
float dt;
int d_max;
float accel_ang_y,accel_ang_x; 

// Pass ::Scarecrow101@
//comment the next line in if you like to publish voltage instead of light. It's an command line option.
//#define PUBLISH_VOLTAGE

#define WLAN_SSID   "shepar_N7"
#define WLAN_PASS   "ghost101"

//MQTT broker settings https://proyectoinfo.mybluemix.net
#define HOST        "192.168.0.104"
//#define HOST        "https://proyectoinfo.mybluemix.net/red"
#define PORT        1883
#define USERNAME    "linux"
#define PASSWORD    "linux"

//time out loop count
const int timeout = 200;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, HOST, PORT, USERNAME, PASSWORD);
Adafruit_MQTT_Publish acelera_x = Adafruit_MQTT_Publish(&mqtt, "acelerometro/ax");
Adafruit_MQTT_Publish acelera_y = Adafruit_MQTT_Publish(&mqtt, "acelerometro/ay");
Adafruit_MQTT_Publish acelera_z = Adafruit_MQTT_Publish(&mqtt, "acelerometro/az");

void MQTT_connect();

void setup() { 
  WiFi.forceSleepWake();
  delay(1);
  WiFi.mode(WIFI_STA);  

  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  int i = 0;
  for (; i < timeout; i++)
  {
    if(WiFi.status() == WL_CONNECTED) break;
    delay(100);
    Serial.print(".");
  }
  if(i == timeout)
    Serial.println("No Conectado");

  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  MQTT_connect();
  init_MPU_sensor();
}


void loop() 
{
  delay(100);

  acelera_x.publish(ax_d);
  acelera_y.publish(ay_d);
  acelera_z.publish(az_d);

  Serial.println("Send Data");
  MPU_sensor();

}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 10;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 1 second...");
       mqtt.disconnect();
       delay(1000);
       retries--;
       if (retries == 0)
         Serial.println("No Conectado");
  }
  Serial.println("MQTT Connected!");
}

void init_MPU_sensor(){
  Wire.begin();           //Iniciando I2C  
  sensor_ang.initialize();    //Iniciando el sensor_ang
  if (sensor_ang.testConnection()) Serial.println("sensor_ang iniciado correctamente");
  else Serial.println("Error al iniciar el sensor_ang");
}

void MPU_sensor(){
  // Leer las aceleraciones y velocidades angulares
  sensor_ang.getAcceleration(&ax, &ay, &az);
  sensor_ang.getRotation(&gx, &gy, &gz);
  /*// Correxion de Offsets
  ax=ax-1045;
  ay=ay+13;
  az=az+1950;
  gx=gx+102;
  gy=gy-18;
  gz=gz-52;*/
  dt = (millis()-tiempo_prev)/1000.0;
  tiempo_prev=millis();
  
  //Calcular los ángulos con acelerometro
  accel_ang_x=atan2(ay,sqrt(pow(ax,2) + pow(az,2)))*(180.0/3.14);
  accel_ang_y=atan2(-ax,sqrt(pow(ay,2) + pow(az,2)))*(180.0/3.14);
  
  //Calcular angulo de rotación con giroscopio y filtro complementario  
  ang_x = 0.98*(ang_x_prev+(gx/127)*dt) + 0.02*accel_ang_x;
  ang_y = 0.98*(ang_y_prev+(gy/160)*dt) + 0.02*accel_ang_y;
   
  ang_x_prev=ang_x;
  ang_y_prev=ang_y;

  az_d = az * (9.81/16384.0);
  ay_d = ay * (9.81/16384.0);
  ax_d = ax * (9.81/16384.0);
}
