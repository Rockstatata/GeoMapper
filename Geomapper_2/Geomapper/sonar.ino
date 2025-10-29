float front_distance = 0, left_distance = 0, right_distance = 0;

// float get_distance(int trigPin, int echoPin){  
//   digitalWrite(trigPin, LOW);
//   delayMicroseconds(2);
 
//   digitalWrite(trigPin, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(trigPin, LOW);
 
//   duration = pulseIn(echoPin, HIGH);
  
//   distance = duration * 0.034 / 2;

//   // Serial.print("Distance: ");
//   // Serial.println(distance);

//   return distance;
// }

float get_distance(int trigPin, int echoPin) {
  float temperatureC = 25.0;
  const int samples = 5;          // number of measurements to average
  float totalDistance = 0;

  // Calculate speed of sound (in cm/µs) adjusted for temperature
  // Speed of sound ≈ 331.4 + 0.6 * T (m/s)
  float speedOfSound = (331.4 + 0.6 * temperatureC) * 0.0001; // convert to cm/µs

  for (int i = 0; i < samples; i++) {
    // Trigger the sensor
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Measure echo time (with 25ms timeout)
    long duration = pulseIn(echoPin, HIGH, 25000); // returns 0 if no echo

    if (duration > 0) {
      // Distance = (time × speed) / 2
      float distance = (duration * speedOfSound) / 2.0;
      totalDistance += distance;
    }

    delay(10); // small pause between readings
  }

  // Average out the valid readings
  float averageDistance = totalDistance / samples;
  
  // Optional debugging
  // Serial.print("Avg Distance: ");
  // Serial.print(averageDistance);
  // Serial.println(" cm");

  return averageDistance;
}

//demo for ghuraghuri mapping
// for(int i = 0; i < 5; i++) {
//   int cor = sorarReading();
//   send;
//   motor(100, 50);
//   delay(5);
// }


void readAllSonars() {
  front_distance = get_distance(32, 39);
  left_distance  = get_distance(25, 34);
  right_distance = get_distance(33, 36);
}
