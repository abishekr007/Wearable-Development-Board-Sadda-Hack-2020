#include <ESP8266WiFi.h>
#include <WifiUDP.h>
#include <String.h>
#include <Wire.h>
#include <SSD1306.h>
#include <SSD1306Wire.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

#define BLYNK_PRINT Serial
#include <BlynkSimpleEsp8266.h>

const int DataDisplayButton = 14;
int RelayButtonPin1 = 12;
int RelayButtonPin2 = 13;

int Relay1Pin = 2; 
int Relay2Pin = 0; 

int Relay1State = HIGH;         
int Relay2State = HIGH;         

String RlSt = String(Relay1State, HEX);

int Relay1ButtonState;             
int Relay2ButtonState;             

int lastButtonState1 = LOW;   
int lastButtonState2 = LOW;   
unsigned long lastDebounceTime1 = 0;  
unsigned long lastDebounceTime2 = 0;  

unsigned long debounceDelay1 = 50;    
unsigned long debounceDelay2 = 50;    

char auth[] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"; 
#define NTP_OFFSET   60 * 60      
#define NTP_INTERVAL 60 * 1000    
#define NTP_ADDRESS  "ir.pool.ntp.org"  
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
SSD1306  display(0x3C, 4, 5); 

const char* ssid = "XXXXXXXXXXXXX";   
const char* password = "XXXXXXXXXX";              
String date;
String t;
String tempC;
const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

const char hostname[] = "query.yahooapis.com";
const String url = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"; 
const int port = 80;

unsigned long timeout = 10000; 

WiFiClient client;

BlynkTimer timer;

WidgetBridge bridge1(V1);  


BLYNK_CONNECTED() {
  
  bridge1.setAuthToken("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"); 
}

void setup ()
{
  Serial.begin(115200); 
  timeClient.begin();   

  Wire.pins(4, 5);  
  Wire.begin(4, 5); 
  display.init();
  display.flipScreenVertically();

  Blynk.begin(auth, ssid, password, "blynk-cloud.com", 8080);
  pinMode(DataDisplayButton, INPUT);

  
  Serial.println("");
  display.drawString(0, 0, "Connected to WiFi.");
  Serial.print(WiFi.localIP());
  Serial.println("");
  display.drawString(0, 24, "Team Helios");
  display.display();
  delay(1000);
}

void loop()
{
  int buttonState = digitalRead(DataDisplayButton);
  if (buttonState == LOW) {
    Serial.print("Button pressed");
    GetWeatherData();
    tellTime();
    delay(6000);
  }
  else {
    display.clear();
  }
  
  Blynk.run();
  timer.run();
  ControlRelays();
  
  display.display();
}

void ControlRelays(){
  
  
  int reading1 = digitalRead(RelayButtonPin1);
  int reading2 = digitalRead(RelayButtonPin2);
   
  if(reading1 == LOW || reading2 == LOW){ 
    display.drawRect(0, 20, 60, 40);
    display.drawRect(61, 20, 60, 40);
    display.setFont(ArialMT_Plain_10);
    display.drawString(17, 3, "Lights");
    display.drawString(84, 3, "A/C");        
    
    if(Relay1State == HIGH){
    display.setFont(ArialMT_Plain_16);
    display.drawString(18, 30, "ON");
    }
    else if(Relay1State == LOW){
    display.setFont(ArialMT_Plain_16);
    display.drawString(15, 30, "OFF");
    } 
    if(Relay2State == HIGH){
    display.setFont(ArialMT_Plain_16);
    display.drawString(78, 30, "ON");
    }
    else if(Relay2State == LOW){
    display.setFont(ArialMT_Plain_16);
    display.drawString(76, 30, "OFF");
    }   
    
}


 
  if (reading1 != lastButtonState1) {
    
    lastDebounceTime1 = millis();
  }
  if (reading2 != lastButtonState2) {
    
    lastDebounceTime2 = millis();
  }


  if ((millis() - lastDebounceTime1) > debounceDelay1) {
    

    if (reading1 != Relay1ButtonState) {
      Relay1ButtonState = reading1;

    if (Relay1ButtonState == HIGH) {
        Relay1State = !Relay1State;
      }
    }
  }
  if ((millis() - lastDebounceTime2) > debounceDelay2) {


    if (reading2 != Relay2ButtonState) {
      Relay2ButtonState = reading2;

      if (Relay2ButtonState == HIGH) {
        Relay2State = !Relay2State;
      }
    }
  }
  
  bridge1.digitalWrite(Relay1Pin, Relay1State);
  bridge1.digitalWrite(Relay2Pin, Relay2State);
  lastButtonState1 = reading1;
  lastButtonState2 = reading2;
}

void tellTime() {
  if (WiFi.status() == WL_CONNECTED) 
  {
    date = "";  
    t = "";

    
    timeClient.update();
    unsigned long epochTime =  timeClient.getEpochTime();
    time_t local, utc;
    utc = epochTime;

    
    TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, +150};  
    TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, +150};   
    Timezone usEastern(usEDT, usEST);
    local = usEastern.toLocal(utc);

    
    date += days[weekday(local) - 1];
    date += ", ";
    date += months[month(local) - 1];
    date += " ";
    date += day(local);
    date += ", ";
    date += year(local);

    
    t += hourFormat12(local);
    t += ":";
    if (minute(local) < 10) 
      t += "0";
    t += minute(local);
    t += " ";
    t += ampm[isPM(local)];

    
    Serial.println("");
    Serial.print("Local date: ");
    Serial.print(date);
    Serial.println("");
    Serial.print("Local time: ");
    Serial.print(t);

    
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_24);
    display.drawStringMaxWidth(64, 14, 128, t); 
    display.setFont(ArialMT_Plain_10);
    display.drawStringMaxWidth(64, 42, 128, date); 
    
    display.drawString(70, 0, "Temp:"); 
    display.drawString(100, 0, tempC);  
    display.drawString(113, 0, "C");
    display.display();
  }
  else 
  {
    display.clear();
    display.drawString(0, 18, "Connecting to Wifi...");
    display.display();
    WiFi.begin(ssid, password);
    display.drawString(0, 32, "Connected.");
    display.display();
  }
}
  void GetWeatherData(){
  
  unsigned long timestamp;
  int temp;
  Serial.print("Connecting to ");
  Serial.println(hostname);
  if ( !client.connect(hostname, port) ) {
    Serial.println("Connection failed");
  }

  
  String req = "GET " + url + " HTTP/1.1\r\n" + 
                "Host: " + hostname + "\r\n" +
                "Connection: close\r\n" +
                "\r\n";
  client.print(req);

  
  delay(500);
  timestamp = millis();
  while ( !client.available() && (millis() < timestamp + timeout) ) {
    delay(1);
  }

  
  if ( client.find("temp\":") ) {
    temp = client.parseInt();
    tempC = (temp - 32) * 5/9 ;
    Serial.print("Local temperature: ");
    Serial.print(tempC);
    Serial.println("°C");
  }

  
  while ( client.available() ) {
    client.readStringUntil('\r');
  }

  
  client.stop();
  Serial.println();
  Serial.println("Connection closed");
  }
