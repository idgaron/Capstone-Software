#include <cmath>

int i = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

float V = 0;
unsigned long start_time = 0;
unsigned long previous_time;
unsigned long duration;

void loop() {
  // put your main code here, to run repeatedly:
  //duration = micros() - previous_time;
  
  Serial.print("time: ");
  Serial.print(i);
  Serial.print("  (x100us), voltage: ");
  Serial.print(V);
  Serial.print(" V, acceleration: ");
  Serial.print(0);
  Serial.print(" m/s^2 ");
  Serial.println(micros() - previous_time);
  previous_time = micros();
  V = sin(2*3.1415*(i)*2000);
  i++;

  if (V > 3.3){
    V = 0;
  }
  
  delayMicroseconds(100);

}
