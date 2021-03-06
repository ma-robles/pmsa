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
#include <TinyGPS++.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_PM25AQI.h"
#define _TASK_PRIORITY
#include <TaskScheduler.h>
#include "ESP8266FtpServer.h"
//#include <Adafruit_Sensor.h>
#include "Adafruit_BME280.h"
#include "RTClib.h"
#include <NTPClient.h>

FtpServer ftpSrv; 
int ndata=0;//cantidad de datos para el promedio
float total_PM1=0;
float total_PM25=0;
float total_PM10=0;
float total_temperatura=0;
float total_humedad=0;
float total_presion=0;
float total_gas=0;

/////////////////////////////////////
RTC_DS1307 rtc;
//int SQpin= 2;
int minuto_anterior;
int minuto;
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
char const *id="1";
char const *APname=strcat("MPBU_", id);
void promedio();
String nom_documento;
String fecha;
String Dia;
String Mes;
String Anio;
/////Definir Sensor
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
 PM25_AQI_Data data;
//////////////definir sensor temperatura
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C
const int RXPin = 16, TXPin = 0;
//SoftwareSerial neo6m(RXPin, TXPin);///software serial GPS
const int CS = D8; // Para el NodeMcu
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
Task PMSA(5000,TASK_FOREVER, &datos_PMSA );

//date
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//status
struct status{
    boolean msd;
    boolean pm;
    boolean pth;
    boolean rtc;
    boolean gps = false;
    boolean wf = false;
};

struct status stat_all;
uint8_t* structPtr = (uint8_t*) &stat_all;

const int button_pin = 2;

boolean wifi_setup = false;

WiFiManager wm;

void setup() {
    std::vector<const char *> menu = {"wifi","exit"};
    wm.setMenu(menu);

    Serial.begin(115200);
    Serial.println();
    //neo6m.begin(9600);//serial gps
    //smartdelay_gps(5000);
    runner.addTask(PMSA);
    while (!Serial) continue;
    Serial.print("Iniciando SD ...");
    stat_all.msd = SD.begin(CS);
    if (!stat_all.msd){
        Serial.println("No se pudo inicializar inserte sd ");
        //ESP.restart();
    }
    Serial.print("Iniciando servidor FTP...");
    ftpSrv.begin("admin","admin");  
    Serial.println("inicio exitoso");
    Serial.print(" PMSA003I Calidad Aire...");
    delay(1000);
    stat_all.pm = aqi.begin_I2C();
    if (! stat_all.pm){
        Serial.println("No se pudo encontrar sensor PM!");
        }
    else Serial.println("encontrado!");
    Serial.print("Sensor BME...");
    stat_all.pth = bme.begin(BME280_ADDRESS_ALTERNATE);
    //stat_all.pth = bme.begin(BME280_ADDRESS);
    if (!stat_all.pth) {
        Serial.println("No se pudo encontrar BME");
    }
    else Serial.print("encontrado!");
    //rtc.writeSqwPinMode( DS1307_OFF );
    //pinMode(SQpin, INPUT);
    Serial.println("input");
    pinMode(button_pin, INPUT);
    Serial.println("begin");
    stat_all.rtc=rtc.begin();
    //WiFi
    Serial.print("Activando WiFi...");
    WiFi.mode(WIFI_STA);
    //wm.setConfigPortalBlocking(false);
    if (wm.autoConnect(APname, "mpbuCfg2022")) Serial.println("OK");
    else Serial.println("Fall??");
    for (byte i=0; i< sizeof(stat_all); i++){
        Serial.println(*structPtr++);
    }
    Serial.println("Obteniendo fecha");
    timeClient.begin();
    timeClient.setTimeOffset(-6*3600);
    timeClient.update();
    String formattedTime = timeClient.getFormattedTime();
    Serial.print("pool.ntp.org Time(-6): ");
    Serial.println(formattedTime);  
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    Serial.println(ptm->tm_year);
    Serial.print("Sincronizaci??n de hora ");
    if (ptm->tm_year>100 && ptm->tm_year<199){
        Serial.println("correcta");
        rtc.adjust(DateTime(ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec));
    }
    else{
        Serial.println("fallida");
        }
    minuto=ptm->tm_min;
    minuto_anterior=minuto;
    DateTime HoraFecha= rtc.now();
    Serial.print("hora: ");
    Serial.println(HoraFecha.timestamp());
    PMSA.enable();
    attachInterrupt(digitalPinToInterrupt(button_pin), inter0 , FALLING);
}

void loop() {
    if (wifi_setup){
        config_wf(60);
        wifi_setup = false;
        interrupts();
    }
    runner.execute();
    ftp_datos();
} 

void config_wf(int timeout){

    wm.setConfigPortalTimeout(timeout);
    //wm.setConfigPortalBlocking(false);
    if (!wm.startConfigPortal(APname, "mpbuCfg2022")){
        Serial.println("Fall?? la configuraci??n WiFi");
    }
    else Serial.println("Configuraci??n WiFi exitosa");
    //wm.process();
}

ICACHE_RAM_ATTR void inter0(){
    noInterrupts();
    Serial.println("Inter!!!");
    wifi_setup = true;
    
}

void ftp_datos(){
    if (WiFi.status() ==WL_CONNECTED){
        bool useStaticIP = false;
        //WiFi.config(ip, gateway, subnet);
        ftpSrv.handleFTP();
    }
}

void datos_PMSA(){
    if (! aqi.read(&data)) {
        Serial.println("no se pudo leer la calidad del aire");
        //delay(500);  // try again in a bit!
        return;
    }
    promedio();
    COR_GPS();
}

void COR_GPS(){
    //smartdelay_gps(10);
    if (gps.location.isValid()) 
    {
        Serial.println("cordenadas recibidas");
    }else{
    }
}

/////////////////////////////////////////////////////////7777
//static void smartdelay_gps(unsigned long ms)
//{
    ////Serial.println("Conectando a GPS");
    //unsigned long start = millis();
    //do 
    //{
        //while (neo6m.available())
            ////Serial.print(".");
            //gps.encode(neo6m.read());
    //} while (millis() - start < ms);
//}




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
                myFile.print("TEMPERATURA ??C");
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
    HTTPClient http; //Creacion de objeto http
    String req= "id="+ String(id)+
        "&pm1=" + String(PM1) +
        "&pm2_5=" + String(PM2_5) +
        "&pm10=" + String(PM10) +
        "&temperatura="+ String(temperatura)+
        "&humedad="+String(humedad)+
        "&presion="+String(presion)+
        "&gas="+String(gas) +
        "&longitud=" + String(gps.location.lng(),7) +
        "&latitud=" + String(gps.location.lat(),7)+
        "&anio="+String(anio)+
        "&mes="+String(Mes)+
        "&dia="+String(Dia)+
        "&hora="+String(hora)+
        "&minuto="+String(minuto);
    if (WiFi.status() ==WL_CONNECTED){
        IPAddress IP_local = WiFi.localIP();
        req += "&ip1=" + String(IP_local[0]) +
            "&ip2=" + String(IP_local[1]) +
            "&ip3=" + String(IP_local[2]) +
            "&ip4=" + String(IP_local[3]); 
        Serial.println(req);
        http.begin(client,"http://esp8266p.000webhostapp.com/EspPost.php");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");//texto plano
        int codigo_respuesta = http.POST(req);
        if (codigo_respuesta>0){
            Serial.println("codigo HTTP: "+ String(codigo_respuesta));
            if(codigo_respuesta ==200){
                String cuerpo_respuesta = http.getString();
                Serial.println(cuerpo_respuesta);
            }
        }
        else{
            Serial.println("No hay conexi??n con la Base de datos");
        }
        http.end();
    }
    else{
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
    total_temperatura+=bme.readTemperature();
    total_presion+=(bme.readPressure()/100.0);
    total_humedad+=bme.readHumidity();
    ndata++;
    RTC();
    Serial.print('*');
    if(minuto!=minuto_anterior){
        Serial.println();
        Serial.println(ndata);
        minuto_anterior=minuto;
        PM1=total_PM1/ndata;
        PM2_5=total_PM25/ndata;
        PM10=total_PM10/ndata;
        temperatura=total_temperatura/ndata;
        humedad=total_humedad/ndata;
        presion=total_presion/ndata;
        gas=total_gas/ndata;
        total_PM1=0;
        total_PM25=0;
        total_PM10=0;
        total_temperatura=0;
        total_humedad=0;
        total_presion=0;
        total_gas=0;
        ndata=0;
        Serial.print("Promedio: ");
        Serial.print("total_PM1: ");
        Serial.print(PM1);
        Serial.print(" total_PM25:");
        Serial.print(PM2_5);
        Serial.print(" total_PM10:");
        Serial.print(PM10);
        Serial.print(" total temperatura:");
        Serial.print(temperatura);
        Serial.print(" total HR:");
        Serial.print(humedad);
        Serial.print(" total P:");
        Serial.print(presion);
        Serial.print(" total gas:");
        Serial.println(gas);
        if(mes<10){
            Mes="0"+String(mes);
        }
        else{Mes=String(mes);}
        if(dia<10){
            Dia="0"+String(dia);
        }
        else{Dia=String(dia);}

        nom_documento=String(anio)+"-"+String(Mes)+"-"+String(Dia)+".csv";
        Serial.println(nom_documento);
        fecha=String(anio)+"-"+String(Mes)+"-"+String(Dia)+" "+String(hora)+":"+String(minuto);
        Serial.println(fecha);
        escribirSD_sistema();
        leerSD();
        enviar_datos();
    }
}
