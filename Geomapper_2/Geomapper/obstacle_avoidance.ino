// --- ADD THESE LINES AT THE VERY TOP ---
extern float distanceForward;
extern float distanceLeft;
extern float distanceRight;
extern float get_distance(int trigPin, int echoPin);
unsigned long seconds;
void obstacle_avoidance(){
  //bluetooth();
  seconds = millis();
  spd=80;
  
  distanceForward = get_distance(TRIG_F, ECHO_F);
  if (distanceForward>30){
    motor(spd,spd);
    //SerialBT.println('F');
  }
  else{
    if(seconds%2==0){
      motor(-spd,-spd);
      delay(600);
    }else{
      motor(-spd,-spd);
      delay(750);
    }

    distanceRight = get_distance(TRIG_R, ECHO_R);
    distanceLeft = get_distance(TRIG_L, ECHO_L);
    if(distanceRight > distanceLeft){
      motor(spd, -spd);
      //SerialBT.println('R');
      delay(500);
    }
    else{
      motor(-spd,spd);
      //SerialBT.println('L');
      delay(500);
    }
    motor(0,0); 
    delay(1000);
  }
}