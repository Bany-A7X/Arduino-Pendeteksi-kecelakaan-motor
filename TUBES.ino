// SendSMS
#include <SoftwareSerial.h>
SoftwareSerial SIM800L(11, 10);
// ----------------------------

// Accelerometer
#include <Wire.h>
#include <Adafruit_Sensor.h> 
#include <Adafruit_ADXL345_U.h>
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
// -----------------------------

// Ultrasonic
const int TRIG_PIN = 3;
const int ECHO_PIN = 4;
const int buzzer = 7;
int led1 = 8;
int led2 = 6;
int led3 = 9;
boolean Ya = true;
unsigned long duration;
unsigned long distance;
// ---------------------------------------------------

// THREAD
#include <Thread.h>
#include <ThreadController.h>

// ThreadController that will controll all threads
ThreadController controll = ThreadController();

//My Thread (as a pointer)
Thread* myThread = new Thread();
//His Thread (not pointer)
Thread* hisThread = new Thread();
// ---------------------------------------------------

void jarak(){
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration/58.2;

  if(distance > 20){
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(buzzer, LOW);
    Serial.println((String) distance+ " cm");
    delay(100);
  }else if(distance<20 && distance > 5){
    digitalWrite(led1, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led2, HIGH);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    digitalWrite(led2, LOW);
    Serial.println((String) distance+ " cm");
  }else if(distance<5){
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);
    digitalWrite(buzzer, HIGH);
    Serial.println((String) distance+ " cm");
    sendSMS();
    delay(5);
  }else{
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(buzzer, LOW);
  }
  Serial.println((String) distance+ " cm");
  delay(500);
}

void sendSMS(){
  if(Ya == true){  
    SIM800L.println("AT+CMGF=1");   
    delay(1000);  
    SIM800L.println("AT+CMGS=\"082114616319\"\r");
    delay(1000);
    SIM800L.println("Motor anda mengalami kecelakaan");
    delay(100);
    SIM800L.println((char)26);
    delay(1000);
    
    Serial.println("SMS telah terkirimkan");
     Ya = false;
     delay(1000);
  }
}

void accelerometer(){
  sensors_event_t event; 
  accel.getEvent(&event);

  float x = event.acceleration.x, y= event.acceleration.y, z = event.acceleration.z;

  Serial.print("X: "); Serial.print(x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(z); Serial.print("  ");
  Serial.println("m/s^2 ");

  if(x < -8 && y < 0.15 && z < 7.00){ //x < -7 && y > -0.20 && z < -4.00 kanan
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);
    digitalWrite(buzzer, HIGH);
    Serial.println("Motor jatoh ke kanan");
    Serial.print("  X= "); Serial.print(x); Serial.print("  ");
    Serial.print("  Y= "); Serial.print(y); Serial.print("  ");
    Serial.println("  Z= "); Serial.print(z); Serial.print("  ");
    sendSMS();
  }else if(x > 7.15 && y >0.01 && z > 7.00){
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, HIGH);
    digitalWrite(buzzer, HIGH);
    Serial.println("Motor jatoh ke kiri");
    Serial.print("  X= "); Serial.print(x); Serial.print("  ");
    Serial.print("  Y= "); Serial.print(y); Serial.print("  ");
    Serial.println("  Z= "); Serial.print(z); Serial.print("  ");
    sendSMS();
  }else{
    digitalWrite(buzzer, LOW);
  }
}

void setup() {
  // Jarak
  pinMode(TRIG_PIN,OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  Serial.begin(9600);
  //------------------------

  // SendSMS
  SIM800L.begin(9600);  
  SIM800L.println("AT+CMGF=1");
  Serial.println("SIM800L started at 9600");
  delay(1000);
  Serial.println("Setup Complete! SIM800L is Ready!");
  SIM800L.println("AT+CNMI=2,2,0,0,0");
  //---------------------------------------------------

  // Accelerometer
  if(!accel.begin()){
    Serial.println("No valid sensor found");
    while(1);
  }
  
  //------------------------ THREAD ----------------------------------
  // Configure myThread
  myThread->onRun(jarak);

  // Configure myThread
  hisThread->onRun(accelerometer);

  // Adds both threads to the controller
  controll.add(myThread);
  controll.add(hisThread); // & to pass the pointer to it
}

void loop() {
  // run ThreadController
  // this will check every thread inside ThreadController,
  // if it should run. If yes, he will run it;
  controll.run();
}
