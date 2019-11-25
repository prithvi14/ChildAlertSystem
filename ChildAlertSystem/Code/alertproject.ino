
#include "HX711.h"
#include "Queue.h"

// HX711 circuit wiring//Libraries
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


//#define CHILDSEAT_QUEUE_SIZE_ITEMS 20
//#define DRIVERSEAT_QUEUE_SIZE_ITEMS 20

// Queue creation:

//DataQueue<int> childseatsensqueue(CHILDSEAT_QUEUE_SIZE_ITEMS);
// Queue creation:

//DataQueue<int> driverseatsensqueue(DRIVERSEAT_QUEUE_SIZE_ITEMS);

//Time allowance for driver to return to vehicle or remove child before an alert is sent
static int checkTime = 30000;

int status = WL_IDLE_STATUS;
IPAddress server(192,168,1,3);  // Google

//Sensor check variables
struct sysStatus{
  bool driver = false;
  bool child = false;
};

// Initialize the client library
WiFiClient client;




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
/*
  for(int n=1; n<=CHILDSEAT_QUEUE_SIZE_ITEMS; n++) {

    if (!childseatsensqueue.isFull()) {
      childseatsensqueue.enqueue(255);  
    }
  }
  for(int n=1; n<=DRIVERSEAT_QUEUE_SIZE_ITEMS; n++) {

    if (!driverseatsensqueue.isFull()) {
      driverseatsensqueue.enqueue(255); 
    }  

}*/
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

//Check for the presence of the driver if driver is not there for more than 1 min then send status as false
//Inputs:  None
//Outputs:
//  bool driver:
//    true:  Driver is present in vehicle
//    false: Driver is not in vehicle
bool checkDriver(){
  //int driverseatavg=0;
  int driverseatval=255;
  int count=0; /* driver not present count 1ms*60=60 sec */
  if(driverseatsens.is_ready()){
   Serial.print("Driver Scale Reading: ");
   driverseatval=(int)driverseatsens.get_units()*0.453592;
   //driverseatsensqueue.dequeue();
   if(driverseatval == 0)
   {
    count += 1;
   }
   else
   {
    count=0; /*reset timer */
   }
   //driverseatsensqueue.enqueue(driverseatval); 
   Serial.print(driverseatsens.get_units()*0.453592, 3);
   Serial.print(" kg"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
   Serial.println( );
  // for(int n=1; n<=DRIVERSEAT_QUEUE_SIZE_ITEMS; n++) {
   //driverseatavg +=driverseatsensqueue[n];
//}
//driverseatavg = driverseatavg/driverseatsensqueue.item_count;
if(count == 60)
{
  return false; 
}
}
  else{
    Serial.println("Driver Scale Not reading .");
  }
return true;  
}

//Check for the presence of the child
//Inputs:  None
//Outputs:
//  bool child:
//    true:  Child is present in vehicle
//    false: Child is not in vehicle
bool checkChild(){
//  int childseatavg=0;
  int childseatval=255;
int count=0; /* driver not present count 1ms*60=60 sec */
  if(childseatsens.is_ready()){
   Serial.print("Child Scale Reading: ");
   childseatval=(int)childseatsens.get_units()*0.453592;
   //childseatsensqueue.dequeue();
   //childseatsensqueue.enqueue(childseatval);
   Serial.print(childseatsens.get_units()*0.453592, 3);
   Serial.print("kg"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
   Serial.println( );
    if(childseatval == 0)
   {
    count += 1;
   }
   else
   {
    count=0; /*reset timer */
   }
  // for(int n=1; n<=CHILDSEAT_QUEUE_SIZE_ITEMS; n++) {
  // childseatavg +=childseatsensqueue.front();
//}
//childseatavg = childseatavg/childseatsensqueue.item_count;
if(count == 60)
{
  return false; 
}
  }
  else{
    Serial.println("Child scale Not reading .");
  }
  return true;
}

// Function to check the sensor.
//Inputs:
//  delayTime - controls how long to check for driver presence before sending an alert
//Outputs:
//  bool alert - returns the status of the alert check.
//    1 = no alert (driver present or child is not)
//    2= alert because child is alone and driver has not reentered vehicle over 
int checkSensors(int delayTime){
  int alert = 1;
  //Create the structure to keep track of sensor status and initialize to false
  sysStatus sensStatus;
  sensStatus.driver = false;
  sensStatus.child = false;
  bool check = false;
  //Loop to check the status of the system
  while(check == false){
    //if there's no child, immediately stop checking since there's no reason to proceed
    if(checkChild() == false){
      check == true;
      alert == 1;
    }
    //otherwise check for the driver
    else if(checkDriver() == true){
      check == true;
      alert == 1;
    }
    //otherwise, child is alone in the vehicle.  Use delayTime to delay for a set amount of
    //time to give driver time to get back in vehicle or remove child
    else{
      delay(delayTime);
      if(checkChild() == false){
        check == true;
        alert == 1;
      }
      //otherwise check for the driver
      else if(checkDriver() == true){
        check == true;
        alert == 1;
      }
      else{
        check == false;
        alert == 2;
      }
    }
  }
  return alert;
}

//Send an alert that the child is alone
//Inputs:  None
//Outputs:  None
void sendAlert(void){
  Serial.println("Sending alert.");
  if (!client.connect(server, 80)) {
 
        Serial.println("Connection to host failed");
        return;
    }
 
    Serial.println("Connected to server successful!");
 
    client.print("Alert");
    client.stop();
}

void loop() {
  //Check to make sure the connection still exists
  //checkWifi();
  // Make a HTTP request:
  int alertval=0;
  alertval=checkSensors(checkTime);
  //sendAlert(alertval);
  if(alertval == 2)
  {
    sendAlert();
  }
  

  delay(1000);
}
