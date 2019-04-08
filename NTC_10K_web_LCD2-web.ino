// C.Poissonnier 2018  

#include <math.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266HTTPClient.h>

// wifi parameter
#define ssid      "FreeLyric"       // WiFi SSID
#define password  "Eurosport2007obeoes"  // WiFi password
#define ip "192.168.1.20:8080"
#define idx "10"




// temp declaration
float steinhart;

// pin Analogique
#define THERMISTORPIN A0         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 10
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor 
#define SERIESRESISTOR 49050
    
  uint8_t j;
uint16_t samples[NUMSAMPLES];

// objects configuration
LiquidCrystal_I2C lcd(0x3F, 20, 4);
ESP8266WebServer server ( 80 );

// home page
String getPage(){
  String page = "<html lang=fr-FR><head><meta http-equiv='refresh' content='10'/>";
  page += "<title>ESP8266 LCD NTC 10K</title>";
  page += "<style> body { background-color: #fffff; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style>";
  page += "</head><body><h1>ESP8266 LCD NTC 10K</h1>";
  page += "<h3>NTC 10K</h3>";
  page += "<ul><li>Temperature : ";
  page += steinhart;
  page += " &deg;C</li>";
  page += "</ul>";
  page += "<br><br><p><a href='https://www.geekncomics.com'>geekncomics.com</p>";
  page += "</body></html>";
  return page;
}

String getUrl() {
String url="http://";
  url +=ip; 
  url +="/json.htm?type=command&param=udevice&idx=";
  url +=idx;
  url +="&nvalue=0&svalue=";
  url += steinhart;
  return url ;
}


void handleRoot(){
     server.send ( 200, "text/html", getPage() );
}

 
void setup(void) {
   Serial.begin(115200);

   lcd.begin(20,4);
   lcd.init();
   lcd.noBacklight();

   WiFi.begin ( ssid, password );
   
   lcd.setCursor(2, 0);
   lcd.print("Connecting Wifi"); 
   
   // Attente de la connexion au réseau WiFi / Wait for connection
   while ( WiFi.status() != WL_CONNECTED ) {
   delay ( 500 ); Serial.print ( "." );
   }
   
   // Connexion WiFi établie / WiFi connexion is OK
   Serial.println ( "" );
   Serial.print ( "Connected to " ); Serial.println ( ssid );
   Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );

   // ip address on LCD display
   lcd.setCursor(2, 0);
   lcd.print("IP address:     "); 
   lcd.setCursor(2, 1);      
   lcd.print(WiFi.localIP());
 
  // link to the function that manage launch page
  server.on ( "/", handleRoot );
  
  server.begin();
  Serial.println ( "HTTP server started" );

  j=0; 

  }
 
void loop(void) {
  uint8_t i;
  float average;
  
  server.handleClient();
  
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;

  // print to serial ( debug ) 
  //Serial.print("Average analog reading "); 
  //Serial.println(average);
 
  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;

  // print to serial ( debug ) 
  //Serial.print("Thermistor resistance "); 
  //Serial.println(average);
 
 // float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
 
  // print to serial ( debug ) 
  Serial.print("Temperature "); 
  Serial.print(steinhart);
  Serial.println(" *C");
  
  String temp = "Temp: ";
  temp += steinhart ; 
  lcd.setCursor(2,2);      
  lcd.print(temp);
  
  delay(5000);

  if (j==1 ) { 
   // if(WiFi.status()== WL_CONNECTED){

      HTTPClient http;    //Declare object of class HTTPClient

 
   Serial.println(getUrl());
   http.begin(getUrl());      //Specify request destination
   http.addHeader("Content-Type", "text/plain");  //Specify content-type header
 
   int httpCode = http.GET();   //Send the request
   String payload = http.getString();                  //Get the response payload
 
   Serial.println(httpCode);   //Print HTTP return code
   Serial.println(payload);    //Print request response payload
 
   http.end();  //Close connection


  } 
  //} 
  if (j>100) { j=0 ; }
  
  j++;  
  //Serial.println(j);
}
