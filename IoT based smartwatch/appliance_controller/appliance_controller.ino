#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
char auth[] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXX";
char ssid[] = "XXXXXXXXXXX";           
char pass[] = "XXXXXXXXXXXXX";      

void setup()
{
  
  Serial.begin(115200); 
  Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8080);
}
void loop()
{
  Blynk.run();
}
