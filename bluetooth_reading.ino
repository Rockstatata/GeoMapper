void bluetooth(){
  if(SerialBT.available()  > 0){
    x = SerialBT.read();
    Serial.println(x);
    if(x == 'F' && !ledstate) motor(spd, spd);
    else if(x == 'B' && !ledstate) motor(-spd, -spd);
    else if(x == 'L' && !ledstate) motor(-spd, spd);
    else if(x == 'R' && !ledstate) motor(spd, -spd);
    else if(x == 'G' && !ledstate) motor(0, spd);
    else if(x == 'I' && !ledstate) motor(spd, 0);
    else if(x == 'H' && !ledstate) motor(0, -spd);
    else if(x == 'J' && !ledstate) motor(-spd, 0);
    else if(x == 'S' && !ledstate) motor(0, 0);
    else if(x == '0') spd = 0;
    else if(x == '1') spd = 25;
    else if(x == '2') spd = 50;
    else if(x == '3') spd = 75;
    else if(x == '4') spd = 100;
    else if(x == '5') spd = 125;
    else if(x == '6') spd = 150;
    else if(x == '7') spd = 175;
    else if(x == '8') spd = 200;
    else if(x == '9') spd = 225;
    else if(x == 'q') spd = 255;
  }
}
