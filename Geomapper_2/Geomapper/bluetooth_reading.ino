extern BluetoothSerial SerialBT;
extern volatile bool ledstate;
extern int spd;
extern char x;
extern bool mappingEnabled;
extern void map_startSession();
extern void map_cancelSession();
extern bool map_isEnabled();

extern void motor(int a, int b);

void bluetooth() {
  if (SerialBT.available() > 0) {
    x = SerialBT.read();

    //Serial.println(x);

    // 🔸 Priority check: Handle 'X' for mapping toggle first
    if (x == 'X') {
      if (map_isEnabled()) {
        map_cancelSession();
        //Serial.println("Mapping cancelled via Bluetooth");
      } else {
        map_startSession();
       // Serial.println("Mapping started via Bluetooth");
      }
      return; // Exit after handling X
    }

    // 🔸 If mapping is active, ignore ALL other commands
    if (map_isEnabled()) {
     // Serial.println("Mapping active - ignoring command: " + String(x));
      return;
    }

    // 🔸 Normal Bluetooth control (only when mapping is NOT active)
    if (x == 'F' && !ledstate) motor(spd, spd);
    else if (x == 'B' && !ledstate) motor(-spd, -spd);
    else if (x == 'R' && !ledstate) motor(-spd, spd);
    else if (x == 'L' && !ledstate) motor(spd, -spd);
    else if (x == 'I' && !ledstate) motor(0, spd);
    else if (x == 'G' && !ledstate) motor(spd, 0);
    else if (x == 'J' && !ledstate) motor(0, -spd);
    else if (x == 'H' && !ledstate) motor(-spd, 0);
    else if (x == 'S' && !ledstate) motor(0, 0); // Stop command
    else if (x == '0') spd = 0;
    else if (x == '1') spd = 25;
    else if (x == '2') spd = 50;
    else if (x == '3') spd = 75;
    else if (x == '4') spd = 100;
    else if (x == '5') spd = 125;
    else if (x == '6') spd = 150;
    else if (x == '7') spd = 175;
    else if (x == '8') spd = 200;
    else if (x == '9') spd = 225;
    else if (x == 'q') spd = 255;
  }
}