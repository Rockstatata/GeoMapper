#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BT_SPP_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it
#endif

#define SWITCH_PIN 17  
#define LED_PIN 2 

#define right_motor_forward 27
#define right_motor_backward 14
#define right_motor_speed 26

#define left_motor_forward 16
#define left_motor_backward 4
#define left_motor_speed 13

const int TRIG_F = 32;   // Forward sensor
const int ECHO_F = 39;
const int TRIG_L = 25;   // Left sensor
const int ECHO_L = 34;
const int TRIG_R = 33;  // Right sensor
const int ECHO_R = 36;


BluetoothSerial SerialBT;

char x;
int spd;
float distance, duration;

const int freq = 500;
const int resolution = 8;
float distanceForward, distanceLeft, distanceRight;

volatile bool ledstate = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceTime = 500;

void motor(int a, int b);
void bluetooth();
float get_distance(int trigPin, int echoPin);
void obstacle_avoidance();

void IRAM_ATTR handleInterrupt() {

  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > debounceTime) {
    ledstate = !ledstate;  
    lastInterruptTime = currentTime;
  }

}

void setup() {
  
  pinMode(right_motor_forward, OUTPUT);
  pinMode(right_motor_backward, OUTPUT);
  pinMode(right_motor_speed, OUTPUT);
  pinMode(left_motor_forward, OUTPUT);
  pinMode(left_motor_backward, OUTPUT);
  pinMode(left_motor_speed, OUTPUT); 
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_F, OUTPUT);
  pinMode(ECHO_F, INPUT);
  pinMode(TRIG_L, OUTPUT);
  pinMode(ECHO_L, INPUT);
  pinMode(TRIG_R, OUTPUT);
  pinMode(ECHO_R, INPUT);

  pinMode(SWITCH_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), handleInterrupt, FALLING);

 
  ledcAttach(left_motor_speed, freq, resolution);
  ledcAttach(right_motor_speed, freq, resolution);

  Serial.begin(115200);
  SerialBT.begin();
}

void loop() {
  digitalWrite(LED_PIN, ledstate);
  bluetooth();
  while(ledstate){
    digitalWrite(LED_PIN, ledstate);
    obstacle_avoidance();
  }
  // distanceForward = get_distance(TRIG_F, ECHO_F);
  // distanceRight = get_distance(TRIG_R, ECHO_R);
  // distanceLeft = get_distance(TRIG_L, ECHO_L);
  // Serial.print("Forward distance: ");
  // Serial.print(distanceForward);
  // Serial.print(" || ");
  // Serial.print("Right distance: ");
  // Serial.print(distanceRight);
  // Serial.print(" || ");
  // Serial.print("Left distance: ");
  // Serial.println(distanceLeft);
}
