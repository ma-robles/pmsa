#define __DEBUG__
#include<ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include<ESP8266WiFiMulti.h>
#include <SD.h>
#include <strings_en.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
IPAddress ip(192, 168, 0, 200);
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 0, 1);
#include <TinyGPS++.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_PM25AQI.h"
#define _TASK_PRIORITY
#include <TaskScheduler.h>
#include "ESP8266FtpServer.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "RTClib.h"
#include <NTPClient.h>

FtpServer ftpSrv; 
//int Bo_STAR=16;
//int estadoBo_star;
//int sum;
const int numReadings=10;
int readings[numReadings];
int inde=0;//indice actual
int total_PM1=0;//total
int total_PM25=0;
int total_PM10=0;
int total_temperatura=0;
int total_humedad=0;
int total_presion=0;
int total_gas=0;
int average=0;//promedio
int inputPin=0;

/////////////////////////////////////
RTC_DS1307 rtc;
int minuto_anterior=0;
int minuto=0;
int segundo,hora,dia,mes;
long anio;
DateTime HoraFecha;
///////////////////////////7
void  escribirSD_sistema();
int ID_conv;
void RTC();
void COR_GPS();
int boton=A0;
void menu();
void regreso();
void ftp_datos();
void compr_SD();
void leerSD();
void conexion_Internet();
void no_conexion();
void datos_PMSA();
void enviar_datos();
 int estado_boton=1;
int estado_menu=0;
 int id=1;
 int y=0;
void promedio();
String nom_documento;
String fecha;
String Dia;
String Mes;
String Anio;
//Configuracion de pantalla/////////////
#define ANCHO_PANTALLA 128 // ancho pantalla OLED
#define ALTO_PANTALLA 32
//Adafruit_SSD1306 display(ANCHO_PANTALLA, ALTO_PANTALLA, &Wire, -1);
////////////dimencion logotipos
#define LOGO_WIDTH    128
#define LOGO_HEIGHT   32

#define LOGO_WIDTH_M_   128
#define LOGO_HEIGHT_M_WIFI   32

#define LOGO_WIDTH_M_NO_WIFI   128
#define LOGO_HEIGHT_M_NO_WIFI   32

/////Definir Sensor
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
 PM25_AQI_Data data;
//////////////definir sensor temperatura
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C


//const int RXPin = 4, TXPin = 5;
const int RXPin = 2, TXPin = 0;
SoftwareSerial neo6m(RXPin, TXPin);///software serial GPS
const int CS = D8; // Para el NodeMcu
String mensaje="Prueba1";
int referencia=0;
String cadena="";
///varables sensor Pmsa
float PM2_5; 
float PM1;
float PM10;
float temperatura;
float humedad;
float presion;
float gas;
int IN_WIF=0;
String TEMPERATURA_;
String HUMEDAD_;
String PRESION_;
String GAS_;
String FECHA_;
String ID;
WiFiClient client;
File myFile;
TinyGPSPlus gps;
Scheduler runner;
Scheduler runn;


//Task SinInt(25,TASK_FOREVER,&no_conexion);
Task PMSA(1000,TASK_FOREVER, &datos_PMSA );

//date
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {

  // pinMode(Bo_STAR,INPUT);
  Serial.begin(115200);
  Serial.println();
  neo6m.begin(9600);//serial gps
  rtc.begin();
  smartdelay_gps(5000);
////////////añadir tareas////////////////////////////////
 //runner.addTask(me);
 //runner.addTask(regr);
 runner.addTask(PMSA);
//runner.addTask(dat);
 // runner.addTask(SinInt);
 //dat.enable();

     // cSD.enable();
     // regr.enable();
while (!Serial) continue;
Serial.print("Iniciando SD ...");
if (!SD.begin(CS)){
    Serial.println("No se pudo inicializar inserte sd ");
    delay(1000);
    while (1);
}else{
    Serial.println("Iniciando servidor FTP");
    ftpSrv.begin("admin","admin");  
              Serial.println("inicio exitoso");
        //      while (!Serial) delay(1000);
              Serial.println(" PMSA003I Calidad Aire");
            // Wait one second for sensor to boot up!
              delay(1000);
              if (! aqi.begin_I2C()){
                          Serial.println("No se pudo encontrar sensor 2.5 sensor!");
                          while (1) delay(10);
    
                    }
                 Serial.println("PM25 encontrado!");
                 delay(1000);
                   if (!bme.begin()) {
                        Serial.println("No se pudo encontrar BME680");
                        while (1);
                      }else{
                        delay(1000);
                         bme.setTemperatureOversampling(BME680_OS_8X);
                         bme.setHumidityOversampling(BME680_OS_2X);
                         bme.setPressureOversampling(BME680_OS_4X);
                         bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
                         bme.setGasHeater(320, 150);
                        
                          bool useStaticIP = false;
                          if(useStaticIP) WiFi.config(ip, gateway, subnet);
                              WiFiManager wifiManager;
          // wifiManager.resetSettings();

                          if(!wifiManager.autoConnect("PMSA", "12345678"))
                           { 
                            ESP.reset();
                 //delay(1000);
                                     }
          delay(2000);
          Serial.println("....conectado!!!!.......");
         // display.display();
  timeClient.begin();
  timeClient.setTimeOffset(-6*3600);
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  if (ptm->tm_year>100){
    rtc.adjust(DateTime(ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec));
  }
  DateTime now = rtc.now();
  //Serial.println(now);
  Serial.println(now.year());                       
                        }
                }

                  PMSA.enable();

}

void loop() {
   runner.execute();
   
   RTC();                    

    
  ftp_datos();
    
   
    
    } 
    
   // runner.execute();// put your main code here, to run repeatedly:
    //no_conexion();
   
    //conexion_Internet();
   // ftp_datos();
  // put your main code here, to run repeatedly:
    



/////////////////////////////////////////////////
  
//D4AB820C01F8


void ftp_datos(){

    if (WiFi.status() ==WL_CONNECTED){
        bool useStaticIP = false;
        WiFi.config(ip, gateway, subnet);
        ftpSrv.handleFTP();
    
             //PMSA.enable();           //me.disable();
    
    }
  }




void datos_PMSA(){
 
  
 
  // Serial.println(25);
  if (! aqi.read(&data)) {
    Serial.println("no se pudo leer la calidad del aire");
    //delay(500);  // try again in a bit!
    return;
  }
  


 
   if (! bme.performReading()) {
    Serial.println("no se pudo leer Temperatura y humedad :(");
    return;
  }









promedio();

    COR_GPS();
 
  
    //escribirSD_sistema();
    //leerSD();
    
   // enviar_datos();
   
    
   
    
  
 }





void COR_GPS(){
   
    smartdelay_gps(10);
   
  if (gps.location.isValid()) 
  {
    Serial.println("cordenadas recibidas");
  }else{
    
   }

 
    }

/////////////////////////////////////////////////////////7777
   static void smartdelay_gps(unsigned long ms)
{
  //Serial.println("Conectando a GPS");
  unsigned long start = millis();
  do 
  {
    while (neo6m.available())
    //Serial.print(".");
      gps.encode(neo6m.read());
  } while (millis() - start < ms);
}




void escribirSD_sistema()
{

 


 /// "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
    myFile=SD.open("sistema.txt",FILE_WRITE);
  if(myFile)
  {
    Serial.println("Escribiendo en SD: ");
    myFile.println();
    
    myFile.print("{\"mes\":");
    //myFile.print("\"gps\",");
    myFile.print("\"");
    myFile.print(String(Mes));
    myFile.print("\",");
    myFile.print("\"dia\":");
    myFile.print("\"");
    myFile.print(String(Dia));
    myFile.print("\",");
    myFile.print("\"id\":");
    myFile.print("\"");
    myFile.print(String(id));
    myFile.print("\",");
    myFile.print("\"PM1\":");
    myFile.print("\"");
    myFile.print(String(PM1));
    myFile.print("\",");
    myFile.print("\"PM25\":");
    myFile.print("\"");
    myFile.print(String(PM2_5));
    myFile.print("\",");
    myFile.print("\"PM10\":");
    myFile.print("\"");
    myFile.print(String(PM10));
    myFile.print("\",");
    myFile.print("\"Gas\":");
    myFile.print("\"");
    myFile.print(String(gas));
    myFile.print("\",");
      myFile.print("\"humedad\":");
    myFile.print("\"");
    myFile.print(String(humedad));
    myFile.print("\",");
      myFile.print("\"temperatura\":");
    myFile.print("\"");
    myFile.print(String(temperatura));
    myFile.print("\",");
      myFile.print("\"presion\":");
    myFile.print("\"");
    myFile.print(String(presion));
    myFile.print("\",");
   // myFile.print("\"time\":");
    //myFile.print("1351824120");
    //myFile.print("\"data\":");
     myFile.print("\"latitud\":");
    myFile.print("\"");
    myFile.print(String(gps.location.lat(),8));
    myFile.print("\",");
    myFile.print("\"longitud\":");
    myFile.print("\"");
    myFile.print(String(gps.location.lng(),8));
    myFile.print("\",");
   
    myFile.print("\"fecha\":");
     myFile.print("\"");
      myFile.print(String(fecha));
    myFile.print("\"}");
    //myFile.print("[48.756080,2.302038]}");
   

    myFile.close();
  }


}



////////////////////////////////////////////////////////////////////////////////////////////////
void leerSD()
{
 


  myFile = SD.open("sistema.txt",FILE_READ);//abrimos  el archivo 

  if (myFile) 
  {
    bool line=false;
    myFile.seek(myFile.size()-1); //Ubicacion en posicion anterior a ultimo caracter
   
    while (myFile.available()) 
    {
      if(line==false) //Primero leer en reversa para buscar salto de linea
      {
        char caracter=myFile.read();
        //Serial.println(caracter);
        myFile.seek(myFile.position()-2);   
        
          if(caracter=='\n') //Cuando encuentra salto de linea cambia estado
          {
            line=true;
          }   
      }

      if(line==true) //Empieza a leer normalmente de izquierda a derecha
      {
          char caracter=myFile.read();
         //Serial.println(caracter);
          cadena=cadena+caracter;
        
          if(caracter=='}') //La cadena termina en este caracter para formato JSON
          {
            break;
          }      
      }
    }
    
    myFile.close(); //cerramos el archivo
    //delay(300);  
   StaticJsonDocument<500> doc;

 
  char json[cadena.length()+1];
  cadena.toCharArray(json,cadena.length()+1);
  
      //"{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, json);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  // Fetch values.
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do doc["time"].as<long>();
  const char* mes_0 = doc["mes"];
  const char* dia_0 = doc["dia"];
  const char* ID = doc["id"];
  const char* PM1_=doc["PM1"];
  const char* PM25_=doc["PM25"];
  const char* PM10_=doc["PM10"];
  const char* GAS_=doc["Gas"];
  const char* HUMEDAD_=doc["humedad"];
  const char* TEMPERATURA_=doc["temperatura"];
  const char* PRESION_=doc["presion"];
 
 // long time = doc["time"];
  const char* latitud = doc["latitud"];
   const char* longitud = doc["longitud"];
     const char* FECHA_=doc["fecha"];

 // double latitude = doc["data"][0];
  //double longitude = doc["data"][1];

  // Print values.
  //Serial.println(mes_0);
  //Serial.println(dia_0);
   //Serial.println(ID);
  //Serial.println(latitud);
  //Serial.println(longitud);
//delay(5000);
  
/////////////////////////////////////////////////////////////////

if(SD.exists(nom_documento)){
  Serial.println(nom_documento);
   myFile=SD.open(nom_documento,FILE_WRITE);
    if(myFile){
  myFile.println();
 // myFile.print(String(ID));
  //myFile.print(",");
  myFile.print(String(PM1_));
   myFile.print(",");
  myFile.print(String(PM25_));
  myFile.print(",");
  myFile.print(String(PM10));
  myFile.print(",");
  myFile.print(String(TEMPERATURA_));
  myFile.print(",");
  myFile.print(String(HUMEDAD_));
  myFile.print(",");
  myFile.print(String(PRESION_));
  myFile.print(",");
  myFile.print(String(GAS_));
  myFile.print(",");
  myFile.print(String(gps.location.lat(),8));
  myFile.print(",");
  myFile.print(String(gps.location.lng(),8));
   myFile.print(",");
   myFile.print(String(FECHA_));
   myFile.close();
  
 }}

  else{
    myFile=SD.open(nom_documento,FILE_WRITE);

 if(myFile){
      // myFile.print("ID");
  //myFile.print(",");
  myFile.print("PM1");
   myFile.print(",");
  myFile.print("PM2.5");
  myFile.print(",");
  myFile.print("PM10");
  myFile.print(",");
  myFile.print("TEMPERATURA °C");
  myFile.print(",");
  myFile.print("HUMEDAD %");
  myFile.print(",");
  myFile.print("PRESION");
  myFile.print(",");
  myFile.print("GAS");
  myFile.print(",");
  myFile.print("latitud");
  myFile.print(",");
  myFile.print("longitud");
   myFile.print(",");
    myFile.print("Fecha");
    myFile.println();
  //myFile.print(String(ID));
//  myFile.print(",");
  myFile.print(String(PM1_));
   myFile.print(",");
  myFile.print(String(PM25_));
  myFile.print(",");
  myFile.print(String(PM10));
  myFile.print(",");
  myFile.print(String(TEMPERATURA_));
  myFile.print(",");
  myFile.print(String(HUMEDAD_));
  myFile.print(",");
  myFile.print(String(PRESION_));
  myFile.print(",");
  myFile.print(String(GAS_));
  myFile.print(",");
  myFile.print(String(gps.location.lat(),8));
  myFile.print(",");
  myFile.print(String(gps.location.lng(),8));
   myFile.print(",");
   myFile.print(String(FECHA_));
   myFile.close();
  
      
    }
 
 }}
//////////////////////////////////////////////////////////7    

  else 
  {
    Serial.println("Error al abrir el archivo");
  }

  cadena="";
  


}




void enviar_datos(){


if (WiFi.status() ==WL_CONNECTED){
    HTTPClient http; //Creacion de objeto http
    String datos_a_enviar = "id="+ String(id)+"&pm1=" + String(PM1) + "&pm2_5=" + String(PM2_5)  +  "&pm10=" + String(PM10) + "&temperatura="+ String(temperatura)+"&humedad="+String(humedad)+"&presion="+String(presion)+"&gas="+String(gas) +"&longitud=" + String(gps.location.lng(),7) + "&latitud=" + String(gps.location.lat(),7)+"&anio="+String(anio)+"&mes="+String(Mes)+"&dia="+String(Dia)+"&hora="+String(hora)+"&minuto="+String(minuto);
//http://esp8266p.000webhostapp.com/EspPost.php/pm1=55&pm2_5=80&pm10=90&longitud=19.81&latitud=-18.55
    Serial.println(datos_a_enviar);

    String hostname="http://esp8266p.000webhostapp.com/EspPost.php";
    Serial.println(hostname);
    if(client.connect(hostname, 80)) {
      Serial.println("Connected to server");
    } else {
      Serial.println("connection failed");
     client.println("POST ?" + datos_a_enviar + " HTTP/1.1");
     client.println("Host: " + hostname);
     client.println("Connection: close");
     client.println(); // end HTTP header
while(client.available())
{
  // read an incoming byte from the server and print them to serial monitor:
  char c = client.read();
  Serial.println("respuesta nueva:");
  Serial.print(c);
}

if(!client.connected())
{
  // if the server's disconnected, stop the client:
  Serial.println("disconnected");
  client.stop();
}

      // send HTTP body
      client.println(datos_a_enviar);
}
    http.begin(client,"http://esp8266p.000webhostapp.com/EspPost.php");
    //http.begin(client,"http://esp8266p.000webhostapp.com/EspPost.php");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");//texto plano

     int codigo_respuesta = http.POST(datos_a_enviar);
     Serial.print("respuesta:");
     Serial.println(codigo_respuesta);

     if (codigo_respuesta>0){
      Serial.println("codigo HTTP: "+ String(codigo_respuesta));
        if(codigo_respuesta ==200){
           String cuerpo_respuesta = http.getString();
         // delay(60000); //espera 60s
          }
      } else{
        }
        Serial.println("No hay conexión con la Base de datos");

        http.end();
     
    } else{
       Serial.println("Error en la conexion WIFI");
      }
      


}


void RTC(){
   HoraFecha=rtc.now();
  segundo=HoraFecha.second();
  minuto=HoraFecha.minute();
  hora= HoraFecha.hour();
  dia=HoraFecha.day();
  mes=HoraFecha.month();
  anio=HoraFecha.year();
  
  
}
void promedio(){
  
total_PM1+=data.pm10_env;
total_PM25+=data.pm25_env;
total_PM10+=data.pm100_env;
total_temperatura+=bme.temperature;
total_presion+=(bme.pressure/100);
total_humedad+=bme.humidity;
total_gas+=(bme.gas_resistance / 1000.0);
inde++;


   if (inde>=numReadings){
inde=0;
total_PM1=trunc(total_PM1/numReadings);
total_PM25=trunc(total_PM25/numReadings);
total_PM10=trunc(total_PM10/numReadings);
total_temperatura=trunc(total_temperatura/numReadings);
total_humedad=trunc(total_humedad/numReadings);
total_presion=trunc(total_presion/numReadings);
total_gas=trunc(total_gas/numReadings);
//pru:
Serial.println("Promedio:");
Serial.print("total_PM1: ");
Serial.println(total_PM1);
Serial.print("total_PM25:");
Serial.println(total_PM25);
Serial.print("total_PM10:");
Serial.println(total_PM10);
Serial.print("total temperatura:");
Serial.println(total_temperatura);
Serial.print("total HR:");
Serial.println(total_humedad);
Serial.print("total P:");
Serial.println(total_presion);
Serial.print("total gas:");
Serial.println(total_gas);
Serial.println("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
PM1=total_PM1;
PM2_5=total_PM25;
PM10=total_PM10;
temperatura=total_temperatura;
presion=total_presion;
humedad=total_humedad;
gas=total_gas;
//goto pru;
}
RTC();
if(y==0){
  minuto_anterior=minuto;
  y=1;
}
Serial.print(minuto);
if(minuto!=minuto_anterior){
  minuto_anterior=minuto;

  if(mes<10){
    Mes="0"+String(mes);
    }
 else{Mes=String(mes);}
     if(dia<10){
    Dia="0"+String(dia);
    }else{Dia=String(dia);}
    
nom_documento=String(anio)+"-"+String(Mes)+"-"+String(Dia)+".csv";
Serial.println(nom_documento);
fecha=String(anio)+"-"+String(Mes)+"-"+String(Dia)+" "+String(hora)+":"+String(minuto);
Serial.println(fecha);
escribirSD_sistema();
leerSD();
// Serial.println("////////////////////////////////////////////////////////");


enviar_datos();



}





  
}
