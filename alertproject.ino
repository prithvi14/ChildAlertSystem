#include <SPI.h>
#include <WiFiNINA.h>
#include <HX711.h>

#define DT 2
#define SCK 3

#define DT1 4
#define SCK1 5


float childsenscalibration_factor = -7050;

float driversenscalibration_factor = 7050;

HX711 childseatsens;
HX711 driverseatsens;
char networkSsid[] = "My Home";
char networkPass[] = "trishok4";



int status = WL_IDLE_STATUS;
IPAddress server(192,168,1,3);  // server addr

//Sensor check variables
struct sysStatus{
  bool driver = false;
  bool child = false;
};

// Initialize the client library
WiFiClient client;

int driverabsenttimer=0; /* driver not present count 1ms*60=60 sec */

void setup() {
  // initialize LED
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial)
  Serial.println("Starting Program.");


  //Connect to the server using WiFi
  connectWifi();

  childseatsens.begin(DT, SCK);
  driverseatsens.begin(DT1, SCK1);
  
  childseatsens.set_scale();
  childseatsens.tare(); //Reset the scale to 0
  childseatsens.set_scale(childsenscalibration_factor);

  driverseatsens.set_scale();
  driverseatsens.tare(); //Reset the scale to 0
  driverseatsens.set_scale(driversenscalibration_factor);
}


void connectWifi(){
  //Connect to the network and server
  while (status != WL_CONNECTED) {
    Serial.print("Attempting connection to network: ");
    Serial.println(networkSsid);
    status = WiFi.begin(networkSsid, networkPass);
    // Delay to establish connection
    delay(10000);
    IPAddress ardAddr = WiFi.localIP();
    Serial.println(ardAddr);
  //if connected, blink the LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
}

void dispWifiStatus() {
  // display network information
  Serial.print("Network: ");
  Serial.println(WiFi.SSID());

  // display Arduino's IP
  IPAddress ipAddr = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ipAddr);
}

void checkWifi(){
  if(!client.connected()){
    Serial.println("Connection Lost.  Trying to Reestablish Connection.");
    connectWifi();
  }
}


/*********************************************************************************************************/
//Check for the presence of the driver if driver is not there for more than 1 min then send status as false
//Inputs:  None
//Outputs:
//  bool driver:
//    true:  Driver is present in vehicle
//    false: Driver is not in vehicle
/********************************************************************************************************/
bool checkDriver(){
  int driverseatval=255;
  bool driversts = true;
  if(driverseatsens.is_ready()){
   Serial.print("Driver Scale Reading: ");
   driverseatval=(int)driverseatsens.get_units()*0.453592;
   if(driverseatval == 0)
   {
    driverabsenttimer += 1;
   }
   else
   {
    driverabsenttimer = 0; /*reset timer */
    driversts = true;
   } 
   Serial.print(driverseatval);
   Serial.print(" kg"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
   Serial.println( );
}
  else{
    Serial.println("Driver Scale Not reading .");
  }
Serial.print(driverabsenttimer);
if(driverabsenttimer >= 60)
{
  driversts = false; 
}
return driversts;  
}


/***********************************************************************/
//Check for the presence of the child
//Inputs:  None
//Outputs:
//  bool child:
//    true:  Child is present in vehicle
//    false: Child is not in vehicle
/***********************************************************************/
bool checkChild(){
  int childseatval=255;
  bool childsts=false;
  if(childseatsens.is_ready()){
   Serial.print("Child Scale Reading: ");
   childseatval=(int)childseatsens.get_units()*0.453592;
   Serial.print(childseatval);
   Serial.print("kg"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
   Serial.println();
    if(childseatval == 0)
   {
    childsts=false;
   }
   else
   {
    childsts = true; 
   }
 }
  else{
    Serial.println("Child scale Not reading .");
  }
  return childsts;
}

/********************************************************************************/
//Send an alert that the child is alone
//Inputs:  None
//Outputs:  None
/*******************************************************************************/
void sendAlert(int alertval){
  Serial.println("Sending alert.");
  if (!client.connect(server, 80)) {
 
        Serial.println("Connection to host failed");
        return;
    }
 
    Serial.println("Connected to server successful!");
    client.print(alertval);
    client.stop();
}

void loop() {
  //Check to make sure the connection still exists
  //checkWifi();
  int alertval=0;
  bool childsts = false;
  bool driversts = false;
  childsts=checkChild();
  driversts=checkDriver();
  Serial.println(" Driver stst,Child Sts");
  Serial.print(driversts,DEC);
  Serial.print(childsts,DEC);
  if(childsts == true)
  {
    if(driversts == false)
    {
      alertval = 2; /* sending alert */
      sendAlert(alertval);
      delay(120000); /* delay the next alert */
    }  
    else
    {
      alertval=1; /* child and driver both present */
    }
   }
  delay(1000);
}
