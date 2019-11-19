#include <HX711.h>

#define RDPIN 4
#define DT 21
#define SCK 20

HX711 scale;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial)
  pinMode(DT, INPUT);
  pinMode(SCK, OUTPUT);
  Serial.println("Starting Program.");
  scale.begin(DT, SCK, 128);
  scale.tare();
}

void loop() {
  if(scale.is_ready()){
    Serial.println("Output Ready:");
    //Serial.println( scale.get_units(10));
    Serial.println( scale.read_average());
    Serial.println( scale.read());
  }
  else{
    Serial.println("Output Not Ready.");
  }
  Serial.println(scale.get_scale());
  delay(1000);
}
