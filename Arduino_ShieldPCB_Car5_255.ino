//#include <Servo.h> //uses this library diables pins 9 and 10


int carSpeed = 255;
char carState = 'S';
int maxSpeed = 255;
int minSpeed = 150;
int accSpeed = 20;
int decSpeed = 20;
int turnDiff = 50;
int lMTRF = 11; //left motor forward
int lMTRR = 10; //left motor reverse
int rMTRF = 3; //right motor forward
int rMTRR = 9; //right motor reverse

int lLED = 4; //signal when turned left
int rLED = 5; //signal when turned right

int triggerPin = 8; // Trigger pin of the ultrasonic sensor
int echoPin = 7; // Trigger pin of the ultrasonic sensor
float minDistance = 15; // minimum allowable distance to an obstacle
float distance = 100;

// Servo myServo;
// int servoPin = 6; // servo pin
// int servoAngle = 120;
// int lServoAngle = 75;
// int rServoAngle = 165;
// int currentServoAngle;

int speedPin = 2; // pin for the speed encoder
unsigned int rpm;
volatile byte pulses;
unsigned long TIME;
unsigned int pulse_per_turn = 20; //Encoder Disc Resolution = 20 slots!



void setup() {
  // put your setup code here, to run once:
  pinMode(lMTRF, OUTPUT);
  pinMode(lMTRR, OUTPUT);
  pinMode(rMTRF, OUTPUT);
  pinMode(rMTRR, OUTPUT);
  Serial.begin(9600);
  Serial.println("Ready to Go");

  pinMode(lLED, OUTPUT);
  pinMode(rLED, OUTPUT);

  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //myServo.attach(servoPin, 700, 2400);
  //myServo.write(servoAngle);
  //currentServoAngle = servoAngle;

  rpm = 0;
  pulses = 0;
  TIME = 0;
  pinMode(speedPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(speedPin), count, FALLING);

  stopRobot();
}


void loop() {
  controlRobot();
  setLEDs();
  //getDistance();
  getRPM();
  checkDistance();
  logData();
  
  //setPWMpins();
  //Serial.println(carSpeed);
  delay(100);

  

}
//-------------------Control Robot------------------------------

void controlRobot() {
  if (Serial.available() > 0) {
    char command = Serial.read(); // Read the incoming character
    
    // Check the received command and perform corresponding action
    switch (command) { 
      case 'F': 
        goForward();
        carState = 'F';
        break;
      case 'B': 
        goBackward();
        carState = 'B';
        break;
      case 'L': 
        turnLeft();
        carState = 'L';
        break;
      case 'R': 
        turnRight();
        carState = 'R';
        break;
      case 'l':
        reverseLeft();
        carState = 'l';
        break;
      case 'r': 
        reverseRight();
        carState = 'r';
        break;
      case 'S':
        stopRobot();
        carState = 'S';
        break;
      case '0':
        resetSpeed(minSpeed);
        break;
      case '1':
        resetSpeed(maxSpeed);
        break;
      case '+':
        accelerate(accSpeed);
        break;
      case '-':
        decelerate(decSpeed);
        break;
    }
  }
}
void goForward() {
  
  analogWrite(lMTRF, carSpeed);
  analogWrite(lMTRR, 0 );
  analogWrite(rMTRF, carSpeed);
  analogWrite(rMTRR, 0);
  // delay(2000);
  // stopRobot();
}

void goBackward() {
  

  analogWrite(lMTRF, 0);
  analogWrite(lMTRR, carSpeed);
  analogWrite(rMTRF,  0);
  analogWrite(rMTRR, carSpeed);
  // delay(2000);
  // stopRobot();
  
  
  
}
void turnRight() {
  
  analogWrite(lMTRF, carSpeed);
  analogWrite(lMTRR, 0);
  analogWrite(rMTRF, carSpeed-turnDiff);
  analogWrite(rMTRR, 0);
  // delay(2000);
  // stopRobot();
}

void turnLeft() {
  
  analogWrite(lMTRF, carSpeed-turnDiff);
  analogWrite(lMTRR, 0);
  analogWrite(rMTRF, carSpeed);
  analogWrite(rMTRR, 0);
  // delay(2000);
  // stopRobot();
}

void reverseRight() {
  
  analogWrite(lMTRF, 0);
  analogWrite(lMTRR, carSpeed);
  analogWrite(rMTRF, 0);
  analogWrite(rMTRR, carSpeed-turnDiff);
  // delay(2000);
  // stopRobot();
}

void reverseLeft() {
  
  analogWrite(lMTRF, 0);
  analogWrite(lMTRR, carSpeed-turnDiff);
  analogWrite(rMTRF, 0);
  analogWrite(rMTRR, carSpeed);
  // delay(2000);
  // stopRobot();
}

void stopRobot() {
  analogWrite(lMTRF, 0);
  analogWrite(lMTRR, 0);
  analogWrite(rMTRF,  0);
  analogWrite(rMTRR, 0);
  
}

void resetSpeed(int speed) {
  carSpeed = speed;
  switch (carState) {
    case 'F':
      goForward();
      break;
    case 'B':
      goBackward();
      break;
    case 'R':
      turnRight();
      break;
    case 'L':
      turnLeft();
      break;
    case 'r':
      reverseRight();
      break;
    case 'l':
      reverseLeft();
      break;
  
  }
}

void accelerate(int acceleration) {
  if (carSpeed >= maxSpeed)
    Serial.println("Mamimum speed has been reached ");
  else {
    carSpeed += acceleration;
    if (carSpeed > maxSpeed) {
      carSpeed = maxSpeed;
      Serial.println("Running at maximum speed");
    }
  }
  resetSpeed(carSpeed);
}

void decelerate(int deceleration) {
  if (carSpeed <= minSpeed)
    Serial.println("Minimum speed has been reached ");
  else {
    carSpeed -= deceleration;
    if (carSpeed < minSpeed) {
      carSpeed = minSpeed;
      Serial.println("Running at minimum speed");
    }
  }
  resetSpeed(carSpeed);
}


//-------------------Control Robot------------------------------


//-------------------Log Data ------------------------------------
void logData() {
  Serial.print("Car State: ");
  Serial.print(carState);
  Serial.print(" Car Speed: ");
  Serial.print(carSpeed);
  Serial.print(" Distance: ");
  Serial.print(distance);
  //Serial.print(" ServoAngle: ");
  //Serial.println(currentServoAngle);
  Serial.print(" RPM: ");
  Serial.println(rpm);
  
}
//---------------------Log Data----------------------------------------

//----------------------Set LEDs -------------------------------------------

void setLEDs() {
  switch (carState) {
    case 'F':
      digitalWrite(lLED, HIGH);
      digitalWrite(rLED, HIGH);
      break;
    case 'B':
      digitalWrite(lLED, HIGH);
      digitalWrite(rLED, HIGH);
      delay(200);
      digitalWrite(lLED, LOW);
      digitalWrite(rLED, LOW);
      delay(200);
      break;
    case 'L':
      digitalWrite(lLED, HIGH);
      digitalWrite(rLED, HIGH);
      delay(200);
      digitalWrite(lLED, LOW);
      //digitalWrite(rLED, HIGH);
      delay(200);
      break;
    case 'R':
      digitalWrite(lLED, HIGH);
      digitalWrite(rLED, HIGH);
      delay(200);
      //digitalWrite(lLED, HIGH);
      digitalWrite(rLED, LOW);
      delay(200);
      break;
    case 'l':
      digitalWrite(lLED, HIGH);
      digitalWrite(rLED, LOW);
      delay(200);
      digitalWrite(lLED, LOW);
      //digitalWrite(rLED, HIGH);
      delay(200);
      break;
    case 'r':
      digitalWrite(lLED, LOW);
      digitalWrite(rLED, HIGH);
      delay(200);
      //digitalWrite(lLED, HIGH);
      digitalWrite(rLED, LOW);
      delay(200);
      break;
    case 'S':
      digitalWrite(lLED, LOW);
      digitalWrite(rLED, LOW);
      break;
  }
}

//----------------------Set LEDs -------------------------------------------


//----------------------Ultranoic Transducer-------------------------------------------

float getDistance()
{
  digitalWrite(triggerPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(triggerPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  distance = pulseIn(echoPin, HIGH) / 58.00;
  // Serial.print("distance = ");
  // Serial.println(distance);
  //
  return distance;
  /* Nice code to get average if put in the loop
  //declaration in the beginning
  const int numReadings = 5;
  long Duration = 0;

  int readings[numReadings];
  int index = 0;
  int total = 0;
  int average = 0;
  // code to put in the loop()
  total= total - readings[index];
  readings[index] = analogRead(EchoPin);
  total = total + readings[index];
  index = index + 1;

  if (index >= numReadings)
    index = 0;

  average = total / numReadings;
  */
}

//----------------------Ultranoic Transducer-------------------------------------------

//----------------------Servo----------------------------------------------------------

void checkDistance() {
  getDistance();
  if (distance < minDistance && (carState == 'F' | carState =='L' | carState == 'R')) {
    stopRobot();
    goBackward();
    delay(1000);
    turnLeft();
    delay(1000);
    stopRobot();
    getDistance();
    if (distance > minDistance) {
      Serial.println("Safe to turn left");
      goForward();
    } else {
      goBackward();
      delay(1000);
      turnRight();
      delay(1000);
      stopRobot();
      getDistance();
      if (distance > minDistance) {
        Serial.println("Safe to turn right");
        goForward();
      } else {
        stopRobot();
        carState = 'S';
        Serial.println("Can't go further");
      }
      
    }
    
      
    
  }
}

//-------------------------------Servo---------------------------------

//-------------------------------Encoder-------------------------------
void count() {
  pulses++;
}

void getRPM() {
  if (millis() - TIME >= 100) {
    detachInterrupt(digitalPinToInterrupt(speedPin));
    rpm = (60 * 1000 / pulse_per_turn) / (millis() - TIME) * pulses;
    TIME = millis();
    pulses = 0;
    //Serial.print("RPM: ");
    //Serial.println(rpm);
    attachInterrupt(digitalPinToInterrupt(speedPin), count, FALLING);
  }
}
//-------------------------------Encoder-------------------------------

void writeFPWMPins () {
  analogWrite(lMTRF, carSpeed);
  analogWrite(lMTRR, 0);
  analogWrite(rMTRF, carSpeed);
  analogWrite(rMTRR, 0);
}

void writeBPWMPins () {
  analogWrite(lMTRR, carSpeed);
  analogWrite(rMTRR, carSpeed);
}
void setPWMpins() {
  if (Serial.available() > 0) {
    char command = Serial.read(); // Read the incoming character
    
    // Check the received command and perform corresponding action
    switch (command) { 
      case 'F':
        writeFPWMPins();
        break;
      case 'B':
        writeBPWMPins();
        break;

    //   case '0':
    //     carSpeed = 0;
    //     stopRobot();
    //   case '+':
    //     carSpeed += 20;
    //     resetSpeed();
        
    //     break;
    //   case '-':
    //     carSpeed -= 20;
    //     writeBPWMPins();
    //     break;
      }
  }
}
