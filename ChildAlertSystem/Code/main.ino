//Libraries
#include <SPI.h>
#include <WiFiNINA.h>
#include <HX711.h>

#define DT 21
#define SCK 20
float calibration_factor = 2125;

HX711 scale;

//char networkSsid[] = "ECE574Project";
//char networkPass[] = "ECE574";


//Time allowance for driver to return to vehicle or remove child before an alert is sent
static int checkTime = 1000;

int status = WL_IDLE_STATUS;

//Sensor check variables
struct sysStatus{
  bool driver = false;
  bool child = false;
};

WiFiClient client;

const long LOADCELL_OFFSET = 50682624;
const long LOADCELL_DIVIDER = 5895655;

void setup() {
  // initialize LED
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial)
  Serial.println("Starting Program.");

  //Connect to the server using WiFi
  connectWifi();

  scale.begin(DT, SCK);
  //scale.set_scale();
  //scale.set_scale(LOADCELL_DIVIDER);
  //scale.set_offset(LOADCELL_OFFSET);
  //scale.tare(); //Reset the scale to 0
  //Serial.print(scale.get_units(10));
  //scale.set_scale(calibration_factor);

  //Set analog read resolution
  analogReadResolution(16);

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
  }
  //if connected, blink the LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
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

//Check for the presence of the driver
//Inputs:  None
//Outputs:
//  bool driver:
//    true:  Driver is present in vehicle
//    false: Driver is not in vehicle
bool checkDriver(){
  
}

//Check for the presence of the child
//Inputs:  None
//Outputs:
//  bool child:
//    true:  Child is present in vehicle
//    false: Child is not in vehicle
bool checkChild(){
  
}

// Function to check the sensor.
//Inputs:
//  delayTime - controls how long to check for driver presence before sending an alert
//Outputs:
//  bool alert - returns the status of the alert check.
//    false = no alert (driver present or child is not)
//    true = alert because child is alone and driver has not reentered vehicle over 
bool checkSensors(int delayTime){
  bool alert = false;
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
      alert == false;
    }
    //otherwise check for the driver
    else if(checkDriver() == true){
      check == true;
      alert == false;
    }
    //otherwise, child is alone in the vehicle.  Use delayTime to delay for a set amount of
    //time to give driver time to get back in vehicle or remove child
    else{
      delay(delayTime);
      if(checkChild() == false){
        check == true;
        alert == false;
      }
      //otherwise check for the driver
      else if(checkDriver() == true){
        check == true;
        alert == false;
      }
      else{
        check == false;
        alert == true;
      }
    }
  }
  return alert;
}

//Send an alert that the child is alone
//Inputs:  None
//Outputs:  None
void sendAlert(){
  Serial.println("Child alone in car.  Sending alert.");
}

void loop() {
  //Check to make sure the connection still exists
  //checkWifi();
  //Check the sensors
  //scale.set_scale(calibration_factor); //Adjust to this calibration factor
  //Serial.println(" calibration_factor: ");
  //Serial.println(calibration_factor);
  if(scale.is_ready()){
    Serial.println("Output Ready:");
    //Serial.println( scale.get_units(10));
    Serial.println( scale.read_average());
    Serial.println( scale.read());
  }
  else{
    Serial.println("Output Not Ready.");
  }

  //calibration_factor += 5000;
   /*if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 1;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 1;
  }*/
  
  //Serial.println("Read analog pins.");
  //Serial.println(analogRead(A1));
  //Serial.println(analogRead(A2));
  /*bool alert = checkSensors(checkTime);

  //If the child has been left alone, send an alert to the server
  if(alert){
    sendAlert();
  }
  //if there's no reason for an alert, delay for a time
  else{
    //delay(100);
  }*/
  delay(1000);
  
  
}
