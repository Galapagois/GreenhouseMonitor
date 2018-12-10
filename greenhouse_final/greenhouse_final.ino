/*
LCD setup - https://www.arduino.cc/en/Tutorial/HelloWorld
*/
// This is the code to run a greenhouse sensor - an Arduino Uno connected to a temperature & humidity, and soil moisture, sensor, displays their readings on an OLED screen, and uploads an average every minute to a ThingSpeak channel which triggers an email notification if readings are outside of desired parameters.

#include <dht.h> //needed for temp & humid sensor
#include <SPI.h> //needed for wifi
#include <WiFi.h> //needed for wifi
#include "U8glib.h" //needed for display

//display stuff
#define dht_apin A1
U8GLIB_SH1106_128X64 u8g(13, 11, 10, 9, 8); // CLK=13, MOSI=11, CS=10, DC=9, RES=8
/
/temp & humid stuff
dht DHT;
#define DHT11_PIN A1 //pin used by temp & humid sensor

//moisture stuff
//PIN setup - red to 3.3V, black to GND, white to A0
int moistVal = 0; //value for storing moisture  
int soilPin = A0; //variable for the soil moisture data 
int moistMax = 550; //value to calculate moisture % - 550 is max reading in pure water
int moistPer = 0; //variable for moisture percentage calculation for display

//totals & averages stuff
int readings = 0;//number of readings to allow for averaging
int moistTot = 0;//total value of soil readings within a minute
int moistAv = 0;//average calculated over a minute
int tempTot = 0;
int tempAv = 0;
int humidTot = 0;
int humidAv = 0;

//wifi stuff
char ssid[] = "BTBHub6-SW7Q";        // your network SSID (name)
char pass[] = "P6XA73RnWg6r";    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(80);

// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String APIKey = "Z5RTIOIB9DB04VD7";             // channel's Write API Key
const int updateThingSpeakInterval = 20 * 1000; // 20 second interval at which to update ThingSpeak

// Variable Setup
long lastConnectionTime = 0;
boolean lastConnected = false;

// Initialize Arduino Ethernet Client
WiFiClient client;

void setup() {
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    // don't continue:
    while (true);
  }
  
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
 
}

void loop() {
  int chk = DHT.read11(DHT11_PIN); //read the data from the temp & humidity sensor
  if (DHT.temperature > -50) { //DHT sensor's first readings tend to be -999; this if statement ensures that no impossible temperatures skew the readings!
    moistVal = analogRead(soilPin); //take reading
    moistTot = moistTot + moistVal; //add reading to total
    moistPer = analogRead(soilPin) * 100UL / moistMax; //calculate moisture as percentage
    int tempVal = DHT.temperature; //assign temp data to variable
    int humidVal = DHT.humidity; //assign humidty data to variable
    tempTot = tempTot + tempVal;
    humidTot = humidTot + humidVal;
    do {
      draw(); //write to the display
    } while( u8g.nextPage() );
    readings++;
  }
  delay(2000); //take a reading every second

  if (readings == 30) { //after 60 seconds (30 readings), do an average of each reading over that time, and send to thingspeak
    moistAv = moistTot / readings;
    tempAv = tempTot / readings;
    humidAv = humidTot / readings;
    String moistStr = String(moistAv); //convert average to string, as thingspeak only accepts string values
    String tempStr = String(tempAv);
    String humidStr = String(humidAv);

    //check wifi availability
    if (client.available()) {
      char c = client.read();
    }
    // Disconnect from ThingSpeak if needed
    if (!client.connected() && lastConnected) {
      client.stop();
    }
    // Update ThingSpeak
    if (!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval)) {
      updateThingSpeak("field1=" + moistStr + "&field2=" + tempStr + "&field3=" + humidStr);
    }
    lastConnected = client.connected();
    readings = 0; //reset the totals to 0 so new totals can be calculated after the next minute of readings
    moistTot = 0;
    tempTot = 0;
    humidTot = 0; 
    delay(5000); //time delay to settle power draw
  }
}

void draw() {
  u8g.setFont(u8g_font_unifont);  // select font for display
  u8g.drawStr(0, 20, "Moisture: "); //print to display at set pixel (left side, near top)
  u8g.setPrintPos(98, 20); //select next print location (same horizontal location but further along line)
  u8g.print(moistPer); //print the value recorded
  u8g.drawStr(120, 20, "% ");
  u8g.drawStr(0, 40, "Temperature: ");
  u8g.setPrintPos(100, 40);
  u8g.print(tempVal);
  u8g.drawStr(120, 40, "c ");
  u8g.drawStr(0, 60, "Humidity: ");
  u8g.setPrintPos(100, 60);
  u8g.print(humidVal);
  u8g.drawStr(120, 60, "% ");
}

void updateThingSpeak(String tsData) {
  if (client.connect(thingSpeakAddress, 80)) {
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + APIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");
    client.print(tsData);
    lastConnectionTime = millis();

    if (client.connected()) {
    }
  }
}
