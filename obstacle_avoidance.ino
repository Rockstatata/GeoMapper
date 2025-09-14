void obstacle_avoidance(){
  //bluetooth();
  spd=100;
  
  distanceForward = get_distance(TRIG_F, ECHO_F);
  if (distanceForward>30){
    motor(spd,spd);
    SerialBT.println('F');
  }
  else{
    motor(0,0);
    delay(1000);
    distanceRight = get_distance(TRIG_R, ECHO_R);
    distanceLeft = get_distance(TRIG_L, ECHO_L);
    if(distanceRight > distanceLeft){
      motor(spd, -spd);
      SerialBT.println('R');
      delay(400);
    }
    else{
      motor(-spd,spd);
      SerialBT.println('L');
      delay(400);
    }
    motor(0,0); 
    delay(1000);
  }
}