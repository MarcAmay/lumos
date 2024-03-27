#include<ESP8266WiFi.h>
#include<ESP8266mDNS.h>
#include<ESP8266WebServerSecure.h>
#include<rgb_lcd.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/**********************************************************************************
 *               DEFINES
 **********************************************************************************/
#define BUTTONPIN     15 
#define LEDPIN        15
#define ssid          "Guapito"                           // hotspot SSID or name
#define password      "smartnet"                          // hotspot password
#define domainname    "maraudersmap"                      // domain name of the https
#define deviceid      "001"                               // unique identifier
#define ADASERVER     "io.adafruit.com"                   // do not change this
#define ADAPORT       1883
#define ADAUSERNAME   "MarcAmay"                          // Adafruit username
#define ADAKEY        "aio_HyNy45eWuaXeSeTjavme8fI3jPPA"  // Adafruit IO key

/**********************************************************************************
 *               PROTOTYPES
 **********************************************************************************/
void MQTTconnect(void);
void task1(void);                                         //polls and counts footfall
void task2(void);                                         //transfers data to HTTPS and MQTT servers
void task3(void);
void resetfootfall(void);                                 //resets footfall counter
int buttonpress(void);                                    //counts the low-to-high transition of button state by comparing its last and current values

/**********************************************************************************
 *               GLOBAL INSTANCES VARIABLES
 **********************************************************************************/
WiFiClient client; 
rgb_lcd LCD;
Adafruit_MQTT_Client MQTT(&client, ADASERVER, ADAPORT, ADAUSERNAME, ADAKEY);
BearSSL::ESP8266WebServerSecure SERVER(443);

const long    task1L = 5, task2L = 60000, task3L = 1000; //required period per task respectively
unsigned long task1LC = 0, task2LC = 0, task3LC = 0;
int red=255, green=255, blue=255;                        //LCD screen color of choice
int footfall_total = 0;
int footfall_ave = 0;
int current = 0;
int last = 0;
int seconds = 0;
int minutes = 0;

/**********************************************************************************
 *               SUBSCRIPTIONS & PUBLISHINGS
 **********************************************************************************/
Adafruit_MQTT_Subscribe RESET = Adafruit_MQTT_Subscribe(&MQTT, ADAUSERNAME "/feeds/reset");
Adafruit_MQTT_Publish FOOTFALLTOTAL = Adafruit_MQTT_Publish(&MQTT, ADAUSERNAME "/feeds/footfalltotal");
Adafruit_MQTT_Publish FOOTFALLAVERAGE = Adafruit_MQTT_Publish(&MQTT, ADAUSERNAME "/feeds/footfallaverage");

static const char cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIC8jCCAlugAwIBAgIUYOIOrtubcFsmeN3TrSfpvaJoMHAwDQYJKoZIhvcNAQEL
BQAwfjELMAkGA1UEBhMCVUsxFjAUBgNVBAgMDVdFU1QgTUlETEFORFMxEzARBgNV
BAcMCkJJUk1JTkdIQU0xGjAYBgNVBAoMEUFzdG9uIFVuaXZlcnNpdHkgMQ0wCwYD
VQQLDARFRVBFMRcwFQYDVQQDDA51c2VybmFtZS5sb2NhbDAeFw0yMDAyMTUxOTQ5
MzZaFw00NzA3MDMxOTQ5MzZaMH4xCzAJBgNVBAYTAlVLMRYwFAYDVQQIDA1XRVNU
IE1JRExBTkRTMRMwEQYDVQQHDApCSVJNSU5HSEFNMRowGAYDVQQKDBFBc3RvbiBV
bml2ZXJzaXR5IDENMAsGA1UECwwERUVQRTEXMBUGA1UEAwwOdXNlcm5hbWUubG9j
YWwwgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBAKZPMudaME7JDhWSUhH+ZfOQ
dRZ6Pa4I1Xt0RJCb9F9PUX4vU7cyVWufoYpBlyOxzPyn1kMAzBmEQ0vAwXG0m7PU
urft8ZB/ZZallIfscgEI8HDcOVdwt/uEbfsONp7R4BAGKGhijx+niMgWLL3aoIF1
Tj7AqzjU85e/v2kPrdR5AgMBAAGjbTBrMB0GA1UdDgQWBBTAz79RjP3Mbs4Y32Ga
9p44Vycp+zAfBgNVHSMEGDAWgBTAz79RjP3Mbs4Y32Ga9p44Vycp+zAPBgNVHRMB
Af8EBTADAQH/MBgGA1UdEQQRMA+CDXVzZXJuYW1lLmxvY2EwDQYJKoZIhvcNAQEL
BQADgYEABmkelyDzfZRcqVGM8edRJM5dAQvBlHnjnsMPhvhgkG6MUisBa+vtVhKC
cOE4gAFvv8/hCaWIQwtFdWNfyle5meRX/Tj4L9N1aHASyPfTBcqKTGpjq6GiqtmL
rTINar7hi15E+adMVRRg/1VNfnBXKxh6IYch5hgeHj5TRgGwgAs=
-----END CERTIFICATE-----
)EOF";

static const char key[] PROGMEM =  R"EOF(
-----BEGIN PRIVATE KEY-----
MIICdwIBADANBgkqhkiG9w0BAQEFAASCAmEwggJdAgEAAoGBAKZPMudaME7JDhWS
UhH+ZfOQdRZ6Pa4I1Xt0RJCb9F9PUX4vU7cyVWufoYpBlyOxzPyn1kMAzBmEQ0vA
wXG0m7PUurft8ZB/ZZallIfscgEI8HDcOVdwt/uEbfsONp7R4BAGKGhijx+niMgW
LL3aoIF1Tj7AqzjU85e/v2kPrdR5AgMBAAECgYBgNFpW+JYPTUDne6AcJpSlY8BH
w2jgvt13r9dl68FeTQzwOMJtrCE7w7j3uF+M13KkCRbp5ZErhZZEQPnmI7sZSkal
3TIWmLQq9g9mIscdIjc6rsfWJ7DAdpVgdsClS3sHPxv3D3RTy6Z40vzPgb2977/y
26pUHHJVEG27dqthbQJBANJPu7fm8EOUcvg1/wwnp3p+dCHEmZ7snQlUerMGbw3+
fSJTqFhrwEXYhfzj2VQUqZyaDBM+tZ9vUQU+BreM+fcCQQDKcFUVeUh6PzJJ1uSJ
F3gabOc1IoIPyotQ9RWAU70vaj/aAuIXJd6PsUt+0DucYfcg1G5YteUqJGl/5Mpv
CokPAkADNLHw2LVa4l1qSTBtGAGmjVzp0txgnsy6Aq6oIfX5aaKwrkPHrUTOC8Hn
G/YJIROAzpxWgsMz/fdnNA3YKG77AkEAiA7NsJwOMVNuKiCLAvTKHQCauKSTw6c+
0U+XfuNJIKgJeC495I7oMa1Yb0fm+KkDHoaID4lZF2TXn0SXJeBv0wJBAKhRejgb
R/KkoohDnWQCNkI9TOMPtTohGjoqMakVcaFHB+SOYhKi7iI0+coxNv/1dMydaE5X
xkjlush/Q6zlliA=
-----END PRIVATE KEY-----
)EOF";

/**********************************************************************************
 *               SETUP FUNCTION
 **********************************************************************************/
void setup()
{
  Serial.begin(115200);
  LCD.begin(16,2);
  LCD.setRGB(red, green, blue);

  Serial.print("\n");
  Serial.print("Connecting to WiFi network: ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  while(WiFi.status()!= WL_CONNECTED)
  {
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, HIGH);
    delay(500);
    Serial.print(".");

  }
  
  Serial.print("\n");
  Serial.println("1. Wifi successfully connected!");
  Serial.print("    MAC Address is: ");
  Serial.print(WiFi.macAddress());
  Serial.print("\n");
  Serial.print("    Local IP is: ");
  Serial.print(WiFi.localIP());
  Serial.print("\n");
  Serial.print("    Subnet Mask is: ");
  Serial.print(WiFi.subnetMask());
  Serial.print("\n");
  Serial.print("    Gateway IP is: ");
  Serial.print(WiFi.gatewayIP());
  Serial.print("\n");
  Serial.print("    DNS IP is: ");
  Serial.print(WiFi.dnsIP());
  Serial.print("\n");

  SERVER.getServer().setRSACert(new BearSSL::X509List(cert), new BearSSL::PrivateKey(key));
  SERVER.on("/", task2);
  SERVER.begin();
  Serial.println("2. Server is now running");
  
  if (MDNS.begin(domainname, WiFi.localIP()))             //starts the mDNS process
  {
    Serial.print("    Access your server at https://");
    Serial.print(domainname);
    Serial.println(".local");
  }
  
  MQTT.subscribe(&RESET);                                //subscribe to MQTT feed
}

/**********************************************************************************
 *               LOOP FUNCTION
 **********************************************************************************/
void loop()
{
  MQTTconnect();                                            //checks MQTT connections
  
  Adafruit_MQTT_Subscribe *subscription;                    //create a subscriber object instance
  subscription = MQTT.readSubscription();                   //Read a subscription and wait for max of 5 seconds.
                                                            //   will return 1 on a subscription being read.
  if (subscription == &RESET)                               //if the subscription we have receieved matches the one we are after
  {                                                         //   will return 1 on a subscription being read.
    if(strcmp ((char *)RESET.lastread,"1")==0)
    {
      resetfootfall();
      Serial.print("\n");
      Serial.print("Reset total footfall data via MQTT successful!");
    }
  }

  if (SERVER.hasArg("reset"))                               //checks for argument on HTTPS request string 
  {                                                         //   and if it would satify condition
    resetfootfall();
    Serial.print("\n");
    Serial.print("Reset total footfall data via HTTPS successful!");
  }
  
  unsigned long current_millis = millis();                 //time-driven scheduler
  if ((current_millis - task1LC) >= task1L)
  {
    task1();
    task1LC = current_millis;
  }

  if ((current_millis - task2LC) >= task2L)
  {
    task2();
    task2LC = current_millis;
  }

  if ((current_millis - task3LC)>= task3L)
  {
    task3();
    task3LC = current_millis;
  }

  if (WiFi.status())                                    //checks WiFi connection status and 
  {                                                     //   and if it would satify condition
    ;
  }

  else
  {
    pinMode(LEDPIN, OUTPUT);
    digitalWrite(LEDPIN, HIGH);
  }
  SERVER.handleClient();                                //continuously updates server and MDNS
  MDNS.update();  
}


/**********************************************************************************
 *               POLL AND COUNT FOOTFALL
 **********************************************************************************/
void task1(void)
{
  int footfall = buttonpress();
  if(footfall == 1)
  {
    footfall_total++;
    Serial.print("o");                                 //serial manifestion of the button press
  }
}

  
/**********************************************************************************
 *               TRANSFER DATA TO HTTPS AND MQTT
 **********************************************************************************/
void task2 (void)
{
  minutes++;  
  footfall_ave=footfall_total/minutes;  
  String reply;
  
  reply += "<!DOCTYPE HTML>";
  reply += "<html>";
  reply += "<head>";
  reply += "<title>Marc's sensor</title>";
  reply += "</head>";
  reply += "<body>";
  reply += "<h1>Footfall Monitor</h1>";
  reply += "Number of footfall: "+String(footfall_total)+"<br>";
  reply += "Number of footafall per minute:"+String(footfall_ave)+"<br>";
  reply += deviceid;
  
  reply += "\n";
  reply += "</body>";
  reply += "</html>";

  SERVER.send(200, "text/html", reply);
  FOOTFALLTOTAL.publish(footfall_total);
  FOOTFALLAVERAGE.publish(footfall_ave);
}

/**********************************************************************************
 *               LCD DISPLAY IP ADDRESS AND FOOTFALL AVERAGE
 **********************************************************************************/
void task3 (void)
{
  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("IP: ");
  LCD.print(WiFi.localIP());
  
  LCD.setCursor(0, 1);
  LCD.print("Footfall Ave: ");
  LCD.print(footfall_ave);
}

/**********************************************************************************
 *               MQTT CONNECTION CHECKER
 **********************************************************************************/
void MQTTconnect ( void ) 
{
  unsigned char tries = 0;
  if ( MQTT.connected() ) 
  {
    return;
  }

  while ( MQTT.connect() != 0 )                                        // while we are
  {
    Serial.println("Will try to connect again in five seconds");   // inform user
    MQTT.disconnect();                                             // disconnect
    delay(5000);                                                   // wait 5 seconds
    tries++;
    if (tries == 3)
    {
      Serial.println("problem with communication, forcing WDT for reset");
      while (1)
      {
        ;   // forever do nothing
      }
    }
  }
  Serial.println("3. MQTT succesfully connected!");
}

/**********************************************************************************
 *               BUTTONPRESS CHECKER
 **********************************************************************************/
int buttonpress (void)
{
  pinMode(BUTTONPIN, INPUT);
  current = digitalRead(BUTTONPIN);
  int ret = 0; 
  if(current!=last)
  {
    if(current==HIGH) 
    {
      ret = 1;  
    }
  }
  last = current;
  return ret;
}

/**********************************************************************************
 *               RESET FOOTFALL COUNTER
 **********************************************************************************/
void resetfootfall (void)
{
  footfall_total=0;
  footfall_ave=0;
}
