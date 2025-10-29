// mapping.ino - Advanced stationary rotation mapping with precise 18-degree increments
#include "mapping_struct.h"

// ---- EXTERNS (using your existing functions) ----
extern volatile bool ledstate;
extern void motor(int a, int b);
extern void readAllSonars();
extern float front_distance, left_distance, right_distance;

// ---- Mapping public flags ----
bool mappingEnabled = false;
bool mappingDone = false;

// ---- Your precisely measured parameters ----
static const int ROTATE_MS = 250;     // motor(100,0) for 250ms - your exact timing
static const int PAUSE_MS = 1000;     // then pause 1s for stable readings
static const float ANGLE_STEP = -18.0f;  // CCW -18 per step
static const int TARGET_STEPS = 20;   // 20 steps  18 = 360

// ---- Enhanced data structure ----
struct AdvancedMapSample {
  float frontAngle, rightAngle, leftAngle;
  float frontDist, rightDist, leftDist;
  int stepNumber;
};

static AdvancedMapSample samples[25];
static int sampleCount = 0;

// ---- Internal state machine ----
static unsigned long phaseStart = 0;
static float currentRotation = 0.0f;
static int stepIndex = 0;
enum RotationPhase { IDLE, ROTATING, PAUSED_READING };
static RotationPhase phase = IDLE;

// ---- Public interface functions ----
int map_getCount() { 
  return sampleCount; 
}

bool map_isEnabled() { 
  return mappingEnabled; 
}

bool map_isDone() { 
  return mappingDone; 
}

// Convert to old format for web compatibility
const MapSample* map_data() {
  static MapSample compatSamples[25];
  for (int i = 0; i < sampleCount; i++) {
    compatSamples[i].ang = samples[i].frontAngle;
    compatSamples[i].f = samples[i].frontDist;
    compatSamples[i].l = samples[i].leftDist;
    compatSamples[i].r = samples[i].rightDist;
  }
  return compatSamples;
}

// Get advanced sample data
const AdvancedMapSample* map_getAdvancedData() { 
  return samples; 
}

// ---- Control functions ----
void map_startSession() {
  if (ledstate) return;
  mappingEnabled = true;
  mappingDone = false;
  phase = IDLE;
  phaseStart = 0;
  currentRotation = 0.0f;
  stepIndex = 0;
  sampleCount = 0;
  
  Serial.println("Starting Advanced Mapping Session");
  Serial.println("Car will rotate CCW in 18 degree steps while stationary");
}

void map_cancelSession() {
  mappingEnabled = false;
  mappingDone = false;
  motor(0, 0);
  phase = IDLE;
  
  Serial.println("Mapping Session Cancelled");
}

// ---- Main mapping worker function ----
void map_handle() {
  if (!mappingEnabled) return;
  
  if (ledstate) {
    map_cancelSession();
    return;
  }

  unsigned long now = millis();

  switch (phase) {
    case IDLE:
      if (stepIndex >= TARGET_STEPS) {
        motor(0, 0);
        mappingEnabled = false;
        mappingDone = true;
        
        Serial.println("Mapping Complete!");
        Serial.print("Total samples: ");
        Serial.println(sampleCount);
        Serial.println("360 degree rotation finished.");
        return;
      }
      
      Serial.print("Step ");
      Serial.print(stepIndex + 1);
      Serial.print("/");
      Serial.print(TARGET_STEPS);
      Serial.print(" - Rotating to ");
      Serial.print(currentRotation + ANGLE_STEP);
      Serial.println(" degrees");
      
      motor(100, 0);  // YOUR EXACT MOTOR COMMAND: CCW rotation
      phase = ROTATING;
      phaseStart = now;
      break;

    case ROTATING:
      if (now - phaseStart >= ROTATE_MS) {
        motor(0, 0);
        currentRotation += ANGLE_STEP;
        
        Serial.print("Stopped at ");
        Serial.print(currentRotation);
        Serial.println(" degrees - Taking readings...");
        
        phase = PAUSED_READING;
        phaseStart = now;
      }
      break;

    case PAUSED_READING:
      if (now - phaseStart >= PAUSE_MS) {
        readAllSonars();
        
        // Calculate absolute angles for each sonar
        float frontAngle = currentRotation;
        float rightAngle = currentRotation + 90.0f;
        float leftAngle = currentRotation - 90.0f;
        
        // Normalize angles to -180 to +180 range
        while (frontAngle > 180.0f) frontAngle -= 360.0f;
        while (frontAngle < -180.0f) frontAngle += 360.0f;
        while (rightAngle > 180.0f) rightAngle -= 360.0f;
        while (rightAngle < -180.0f) rightAngle += 360.0f;
        while (leftAngle > 180.0f) leftAngle -= 360.0f;
        while (leftAngle < -180.0f) leftAngle += 360.0f;
        
        // Store the sample
        if (sampleCount < (int)(sizeof(samples)/sizeof(samples[0]))) {
          samples[sampleCount].frontAngle = frontAngle;
          samples[sampleCount].rightAngle = rightAngle;
          samples[sampleCount].leftAngle = leftAngle;
          samples[sampleCount].frontDist = front_distance;
          samples[sampleCount].rightDist = right_distance;
          samples[sampleCount].leftDist = left_distance;
          samples[sampleCount].stepNumber = stepIndex;
          
          Serial.print("Sample ");
          Serial.print(sampleCount + 1);
          Serial.print(": F=");
          Serial.print(front_distance);
          Serial.print("cm at ");
          Serial.print(frontAngle);
          Serial.print(" deg, R=");
          Serial.print(right_distance);
          Serial.print("cm at ");
          Serial.print(rightAngle);
          Serial.print(" deg, L=");
          Serial.print(left_distance);
          Serial.print("cm at ");
          Serial.print(leftAngle);
          Serial.println(" deg");
          
          sampleCount++;
        }
        
        stepIndex++;
        phase = IDLE;
      }
      break;
  }
}
