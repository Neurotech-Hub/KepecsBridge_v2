#include "A4988.h"

// BOARD
#define RX 38
#define TX 39

// GPIO
#define GPIO_1 13
#define GPIO_2 11
#define GPIO_3 12
#define GPIO_4 13

// MOTORS
#define MOTOR0_EN 8
#define MOTOR0_STEP 14
#define MOTOR0_DIR 15
#define MOTOR0_MS1 16
#define MOTOR0_MS2 17
#define MOTOR0_MS3 18

#define MOTOR1_EN 5
#define MOTOR1_STEP 6
#define MOTOR1_DIR 9
#define MOTOR1_MS1 10
#define MOTOR1_MS2 11
#define MOTOR1_MS3 12

// #define MOTOR_RESET 3
#define MAG_SENSOR 3
#define MOTOR_SLEEP 4

#define MOTOR_STEPS 200 // Most steppers are 200 steps/rev (1.8 degrees/step)
#define MOTOR0_RPM 800
#define MOTOR1_RPM 120
#define MOTOR0_MICROSTEPS 1
#define MOTOR1_MICROSTEPS 1

// Calibration parameters
#define CAL_STEP_SIZE 200 // Step size for calibration sequence

// Global position tracker
int currentPosition = 0;
// !! may need to change  these fixed numbers depending on microstepping
// !! see also: handleCalibration()
const int M0_TOP_STEP_LIMIT = 18000;
bool isCalibrated = false; // Track calibration status

// Function to handle rotations with position tracking
void rotateWithTracking(A4988 &stepper, int steps)
{
  // Calculate new position
  int newPosition = currentPosition + steps;

  // Check if this would exceed limits
  if (newPosition < 0)
  {
    Serial.println("Warning: Movement would go below 0. Movement cancelled.");
    return;
  }

  if (newPosition > M0_TOP_STEP_LIMIT)
  {
    Serial.println("Warning: Movement would exceed top limit. Movement cancelled.");
    return;
  }

  // If within limits, execute movement
  stepper.rotate(steps);
  currentPosition = newPosition;
}

// Initialize both stepper motors with enable pins
A4988 stepper0(MOTOR_STEPS, MOTOR0_DIR, MOTOR0_STEP, MOTOR0_EN, MOTOR0_MS1, MOTOR0_MS2, MOTOR0_MS3);
A4988 stepper1(MOTOR_STEPS, MOTOR1_DIR, MOTOR1_STEP, MOTOR1_EN, MOTOR1_MS1, MOTOR1_MS2, MOTOR1_MS3);

// Function declarations
void handleCalibration();
void handleMagneticSensorMonitoring();
void handleMotorCommand(String command);
void parseAndExecuteCommand(String command);
void moveToPercentage(int percentage);
void displayHelp();

void displayHelp()
{
  Serial.println("\n=== Kepecs Bridge v2 Commands ===");
  Serial.println("cal     - Run calibration routine");
  Serial.println("mag     - Monitor magnetic sensor state");
  Serial.println("p0-100  - Move to percentage of range (e.g., p50)");
  Serial.println("0:steps - Move motor 0 by steps (e.g., 0:1000)");
  Serial.println("1:steps - Move motor 1 by steps (e.g., 1:1000)");
  Serial.println("help    - Display this help menu");
  Serial.println("==============================\n");
}

void handleCalibration()
{
  Serial.println("Starting calibration routine...");

  // Check initial state
  bool initialState = digitalRead(MAG_SENSOR);
  Serial.print("Initial MAG_SENSOR state: ");
  Serial.println(initialState);

  // Enable motor
  stepper0.enable();

  if (initialState == 0)
  {
    // Need to move up until we find MAG_SENSOR=1
    Serial.println("Moving up to find sensor...");
    int totalSteps = 0;

    while (totalSteps < 3000)
    {
      stepper0.rotate(CAL_STEP_SIZE); // Direct rotation for calibration
      totalSteps += CAL_STEP_SIZE;
      bool currentState = digitalRead(MAG_SENSOR);

      if (currentState == 1)
      {
        Serial.println("Found sensor, moving down to calibrate...");
        break;
      }

      if (totalSteps >= 3000)
      {
        Serial.println("Calibration failed: No sensor detected after 3000 steps");
        stepper0.disable();
        return;
      }
    }
  }

  // Now move down until we find MAG_SENSOR=0
  Serial.println("Moving down to find calibration point...");
  int stepsDown = 0;
  while (stepsDown < M0_TOP_STEP_LIMIT)
  {
    stepper0.rotate(-CAL_STEP_SIZE); // Direct rotation for calibration
    stepsDown += CAL_STEP_SIZE;
    bool currentState = digitalRead(MAG_SENSOR);

    if (currentState == 0)
    {
      Serial.println("Calibration successful!");
      Serial.print("Final position: ");
      Serial.println(stepsDown);
      currentPosition = 0; // Reset position tracker on successful calibration
      isCalibrated = true; // Set calibration flag
      stepper0.disable();
      return;
    }
  }

  Serial.println("Calibration failed: Could not find calibration point");
  stepper0.disable();
}

void handleMagneticSensorMonitoring()
{
  Serial.println("Monitoring magnetic sensor. Send any command to exit.");
  while (!Serial.available())
  {
    bool magState = digitalRead(MAG_SENSOR);
    Serial.print("MAG_SENSOR state: ");
    Serial.println(magState);
    delay(200);
  }
  // Clear any received input
  while (Serial.available())
  {
    Serial.read();
  }
}

void handleMotorCommand(int motor, int steps)
{
  if (!isCalibrated)
  {
    Serial.println("Error: System not calibrated. Please run 'cal' first.");
    return;
  }

  if (motor == 0)
  {
    Serial.print("Moving Motor 0: ");
    Serial.print(steps);
    Serial.println(" steps");
    digitalWrite(GPIO_4, HIGH);
    stepper0.enable();
    rotateWithTracking(stepper0, steps);
    stepper0.disable();
    digitalWrite(GPIO_4, LOW);
    Serial.print("Motor 0 movement complete and disabled. Current position: ");
    Serial.println(currentPosition);
  }
  else if (motor == 1)
  {
    Serial.print("Moving Motor 1: ");
    Serial.print(steps);
    Serial.println(" steps");
    digitalWrite(GPIO_4, HIGH);
    stepper1.enable();
    rotateWithTracking(stepper1, steps);
    stepper1.disable();
    digitalWrite(GPIO_4, LOW);
    Serial.print("Motor 1 movement complete and disabled. Current position: ");
    Serial.println(currentPosition);
  }
  else
  {
    Serial.println("Invalid motor number. Use 0 or 1.");
  }
}

void parseAndExecuteCommand(String command)
{
  int separatorIndex = command.indexOf(':');
  if (separatorIndex != -1)
  {
    int motor = command.substring(0, separatorIndex).toInt();
    int steps = command.substring(separatorIndex + 1).toInt();
    handleMotorCommand(motor, steps);
  }
  else
  {
    Serial.println("Invalid command format. Use 'motor:steps' (eg, 0:5000)");
  }
}

void moveToPercentage(int percentage)
{
  if (!isCalibrated)
  {
    Serial.println("Error: System not calibrated. Please run 'cal' first.");
    return;
  }

  // Validate input
  if (percentage < 0 || percentage > 100)
  {
    Serial.println("Error: Percentage must be between 0 and 100");
    return;
  }

  // Calculate target position
  int targetPosition = (M0_TOP_STEP_LIMIT * percentage) / 100;

  // Calculate steps needed to reach target
  int stepsNeeded = targetPosition - currentPosition;

  Serial.print("Moving to ");
  Serial.print(percentage);
  Serial.print("% (");
  Serial.print(targetPosition);
  Serial.println(" steps)");

  // Enable motor and move
  digitalWrite(GPIO_4, HIGH);
  stepper0.enable();
  rotateWithTracking(stepper0, stepsNeeded);
  stepper0.disable();
  digitalWrite(GPIO_4, LOW);

  Serial.print("Movement complete. Current position: ");
  Serial.println(currentPosition);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting dual A4988 test");
  displayHelp(); // Display help menu at startup

  // Configure GPIO pins
  pinMode(GPIO_1, INPUT);
  pinMode(GPIO_2, INPUT);
  pinMode(GPIO_3, INPUT);
  pinMode(GPIO_4, OUTPUT);

  // Configure all motor control pins for Motor 0
  pinMode(MOTOR0_DIR, OUTPUT);
  pinMode(MOTOR0_STEP, OUTPUT);
  pinMode(MOTOR0_EN, OUTPUT);
  pinMode(MOTOR0_MS1, OUTPUT);
  pinMode(MOTOR0_MS2, OUTPUT);
  pinMode(MOTOR0_MS3, OUTPUT);

  // Configure all motor control pins for Motor 1
  pinMode(MOTOR1_DIR, OUTPUT);
  pinMode(MOTOR1_STEP, OUTPUT);
  pinMode(MOTOR1_EN, OUTPUT);
  pinMode(MOTOR1_MS1, OUTPUT);
  pinMode(MOTOR1_MS2, OUTPUT);
  pinMode(MOTOR1_MS3, OUTPUT);

  pinMode(MAG_SENSOR, INPUT_PULLUP);
  pinMode(MOTOR_SLEEP, OUTPUT);
  digitalWrite(MOTOR_SLEEP, HIGH); // enable=HIGH

  // Initialize both steppers
  stepper0.begin(MOTOR0_RPM, MOTOR0_MICROSTEPS);
  stepper1.begin(MOTOR1_RPM, MOTOR1_MICROSTEPS);

  // Configure speed profile for faster acceleration
  // Using LINEAR_SPEED mode with higher acceleration/deceleration in steps/sec^2
  stepper0.setSpeedProfile(BasicStepperDriver::LINEAR_SPEED, 5000, 5000);
  stepper1.setSpeedProfile(BasicStepperDriver::LINEAR_SPEED, 5000, 5000);

  // Set enable pin active state (active LOW)
  stepper0.setEnableActiveState(LOW);
  stepper1.setEnableActiveState(LOW);

  // Start with motors disabled
  stepper0.disable();
  stepper1.disable();

  Serial.println("Setup complete - motors disabled");
}

void loop()
{
  if (Serial.available() > 0)
  {
    String command = Serial.readStringUntil('\n');
    command.trim(); // Remove any whitespace/newlines

    if (command == "cal")
    {
      handleCalibration();
    }
    else if (command == "mag")
    {
      handleMagneticSensorMonitoring();
    }
    else if (command == "help")
    {
      displayHelp();
    }
    else if (command.startsWith("p"))
    {
      // Handle percentage command (e.g., "p50" moves to 50%)
      String percentageStr = command.substring(1);
      int percentage = percentageStr.toInt();
      moveToPercentage(percentage);
    }
    else
    {
      parseAndExecuteCommand(command);
    }
  }
}
