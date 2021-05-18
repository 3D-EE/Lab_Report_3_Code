#define DEBUG 0

#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);
float initialAngle, angle;

#define PWMa 7  //define PWM pins
#define PWMb 5
#define PWMc 8
#define PWMd 6
#define motorSpeed 200
int leftMotorSpeed, rightMotorSpeed;

//varaible for tracking commands
char command = ' ';
int prevCommand = 0; //if prev command was 'B' or 'F' store incoming message as num contours

//variable for timing forwards and backwards commands
unsigned long previousMillis = 0;
unsigned long period;
int nForwards, nBackwards;

//variables for PID
float lineError = 0;
const int kP = 5, kI = 0.00, kD = 0;
unsigned long currentTime, previousTime;
float  error, cumulativeError, rateError, outputU = 1;
float elapsedTime = 0, lastError = 0;


void setup() {
  Serial.begin(9600);
  Wire.begin(0x09);
  Wire.onReceive(receiveEvent);

  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  mpu6050.update();
  initialAngle = mpu6050.getAngleZ();
  Serial.println("\nInitial angle is:\t");
  Serial.println(initialAngle);
  Serial.println("\n");

  pinMode(4, INPUT);

  pinMode(PWMa, OUTPUT);  //define PWM motor pins
  pinMode(PWMb, OUTPUT);
  pinMode(PWMc, OUTPUT);
  pinMode(PWMd, OUTPUT);
  analogWrite(PWMa, 0); //turn off the motors
  analogWrite(PWMb, 0);
  analogWrite(PWMc, 0);
  analogWrite(PWMd, 0);
}


// Function that executes whenever data is received from master
void receiveEvent(int howMany) {


  while (Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character

    Serial.println("#######");
    Serial.println(c);
    Serial.println("#######\n\n");

    command = c;

    if (prevCommand == 1 || prevCommand == 2)  //if prev command was F or B
    {
      int numContours = int(c);
      Serial.print("Num contours: ");
      Serial.println(numContours);
      Serial.println("\n\n");

      if (prevCommand == 1) {
        nForwards = numContours;
      }

      if (prevCommand == 2) {
        nBackwards = numContours;
      }

      prevCommand = 0;
      numContours = 0;

    }

    if (prevCommand == 3)
    {
      lineError = int(c);
      //Serial.print("Line error: ");
      //Serial.println(lineError);

      prevCommand = 0;
    }

    if (c == 'F') //command for forward
    {
      Serial.println(c);
      prevCommand = 1;
    }

    if (c == 'B') //command for backwards
    {
      Serial.println(c);
      prevCommand = 2;
    }

    if (c == 'S')
    {
      Serial.println(c);
      prevCommand = 3;
    }


  }
}

void loop()
{
  delay(10);

  if (command != ' ')
  {
    switch (command)
    {

      case 'R':
        right();
        break;

      case 'L':
        left();
        break;

      case 'T':
        turn();
        break;

      case 'O':
        full_turn();
        break;

      case 'P':
        pause();
        break;

      default:
        Serial.println("");
    }
    command = ' ';
  }


  if (nForwards > 0) {
    forwards(nForwards);
    nForwards = 0;
  }

  if (nBackwards > 0) {
    backwards(nBackwards);
    nBackwards = 0;
  }

  if (lineError > 0) {
    line_follow(lineError);
    lineError = 0;
  }



}


void left()
{
  Serial.println("Left test");
  mpu6050.update();
  initialAngle = mpu6050.getAngleZ();

  leftMotorSpeed = 255 - motorSpeed;
  rightMotorSpeed = motorSpeed;

  Serial.print("Right: ");
  Serial.println(rightMotorSpeed);
  Serial.print("Left: ");
  Serial.println(leftMotorSpeed);
  analogWrite(PWMa, 0);
  analogWrite(PWMb, rightMotorSpeed);
  analogWrite(PWMc, 0);
  analogWrite(PWMd, 0);

  while (angle < initialAngle + 90)
  {
    mpu6050.update();
    angle = mpu6050.getAngleZ();
    Serial.println("Turning....");
    Serial.println(angle);

  }

  analogWrite(PWMa, 0); //turn motors off
  analogWrite(PWMb, 0);
  analogWrite(PWMc, 0);
  analogWrite(PWMd, 0);
  Serial.println("Turned 90 left");

}


void right()
{
  Serial.println("Right test");
  mpu6050.update();
  initialAngle = mpu6050.getAngleZ();

  leftMotorSpeed = 255 - motorSpeed;
  rightMotorSpeed = motorSpeed;

  Serial.print("Right: ");
  Serial.println(rightMotorSpeed);
  Serial.print("Left: ");
  Serial.println(leftMotorSpeed);
  analogWrite(PWMa, 0);
  analogWrite(PWMb, 0);
  analogWrite(PWMc, 255);
  analogWrite(PWMd, leftMotorSpeed);

  while (angle > initialAngle - 90)
  {
    mpu6050.update();
    angle = mpu6050.getAngleZ();
    Serial.println("Turning....");
    Serial.println(angle);

  }

  analogWrite(PWMa, 0); //turn motors off
  analogWrite(PWMb, 0);
  analogWrite(PWMc, 0);
  analogWrite(PWMd, 0);
  Serial.println("Turned 90 right");
}


void turn()
{
  Serial.println("Turn (180) test");
  mpu6050.update();
  initialAngle = mpu6050.getAngleZ();

  leftMotorSpeed = 255 - motorSpeed;
  rightMotorSpeed = motorSpeed;

  Serial.print("Right: ");
  Serial.println(rightMotorSpeed);
  Serial.print("Left: ");
  Serial.println(leftMotorSpeed);
  analogWrite(PWMa, 0);
  analogWrite(PWMb, rightMotorSpeed);
  analogWrite(PWMc, 0);
  analogWrite(PWMd, 0);

  while (angle < initialAngle + 180)
  {
    mpu6050.update();
    angle = mpu6050.getAngleZ();
    Serial.println("Turning....");
    Serial.println(angle);
  }
  analogWrite(PWMa, 0); //turn motors off
  analogWrite(PWMb, 0);
  analogWrite(PWMc, 0);
  analogWrite(PWMd, 0);
  Serial.println("Turned 180");
}



void full_turn()
{
  Serial.println("Full turn test");
  mpu6050.update();
  initialAngle = mpu6050.getAngleZ();

  leftMotorSpeed = 255 - motorSpeed;
  rightMotorSpeed = motorSpeed;

  Serial.print("Right: ");
  Serial.println(rightMotorSpeed);
  Serial.print("Left: ");
  Serial.println(leftMotorSpeed);
  analogWrite(PWMa, 0);
  analogWrite(PWMb, rightMotorSpeed);
  analogWrite(PWMc, 0);
  analogWrite(PWMd, 0);
  delay(500);   //getting around the fact that initial angle will be similar to final angle
  while (angle < initialAngle + 360)
  {
    mpu6050.update();
    angle = mpu6050.getAngleZ();
    Serial.println("Turning....");
    Serial.println(angle);
  }
  analogWrite(PWMa, 0); //turn motors off
  analogWrite(PWMb, 0);
  analogWrite(PWMc, 0);
  analogWrite(PWMd, 0);
  Serial.println("Success: Turned 360");
}



void forwards(int n)
{
  period = n * 1000; //convert seconds to milliseconds
  Serial.println(period);

  leftMotorSpeed = 255 - motorSpeed;
  rightMotorSpeed = motorSpeed;

  Serial.print("Right: ");
  Serial.println(rightMotorSpeed);
  Serial.print("Left: ");
  Serial.println(leftMotorSpeed);
  analogWrite(PWMa, 0);
  analogWrite(PWMb, rightMotorSpeed);
  analogWrite(PWMc, 255);
  analogWrite(PWMd, leftMotorSpeed);
  delay(100);
  previousMillis = millis();

  while (1)
  {
    unsigned long currentMillis = millis();
    //Serial.println(currentMillis);
    //Serial.println(previousMillis);
    Serial.println(currentMillis - previousMillis);
    //Serial.println("----");

    if (currentMillis - previousMillis >= period)
    {
      previousMillis = currentMillis;
      Serial.println("Breaking");
      break;
    }
    delay(100);  //

  }
  analogWrite(PWMa, 0);
  analogWrite(PWMb, 0);
  analogWrite(PWMc, 0);
  analogWrite(PWMd, 0);

  delay(100);
  //exit(0);
}

void backwards(int n)
{
  period = n * 1000; //convert seconds to milliseconds
  Serial.println(period);

  digitalWrite(PWMa, HIGH);
  digitalWrite(PWMb, LOW);
  digitalWrite(PWMc, LOW);
  digitalWrite(PWMd, HIGH);
  delay(100);
  previousMillis = millis();

  while (1)
  {
    unsigned long currentMillis = millis();
    //Serial.println(currentMillis);
    //Serial.println(previousMillis);
    Serial.println(currentMillis - previousMillis);
    //Serial.println("----");

    if (currentMillis - previousMillis >= period)
    {
      previousMillis = currentMillis;
      Serial.println("Breaking");
      break;
    }
    delay(100);  //

  }
  analogWrite(PWMa, 0);
  analogWrite(PWMb, 0);
  analogWrite(PWMc, 0);
  analogWrite(PWMd, 0);

  delay(100);
}


void line_follow(int imgError)
{
  //error values from pi are 50 for centre, 25 for far left, 75 for far right.

  currentTime = millis();
  elapsedTime = (float)(currentTime - previousTime);

  error = -(imgError - 50);  //correcting value from pi, now centre is zero, left of image is -25, right of image + 25
  cumulativeError += error * elapsedTime; //calculate integral (summing up many rectangles to determine integral)
  rateError = (error - lastError) / elapsedTime;  //calculate derivative, i.e. rate of change of error

  if (isnan(cumulativeError)) {   //check for nan
    cumulativeError = 0;
  }

  outputU = kP * error + kI * cumulativeError + kD * rateError;


  Serial.print("error (mm) = ");
  Serial.println(error);

#if DEBUG == 1
  Serial.print("cumulativeError = ");
  Serial.println(cumulativeError);
  Serial.print("rateError = ");
  Serial.println(rateError);
  Serial.println("");
  Serial.print("OutputU = ");
  Serial.println(outputU);
  Serial.println("");
#endif

  lastError = error;
  previousTime = currentTime;

  rightMotorSpeed = motorSpeed + outputU ;
  leftMotorSpeed =  255 - (motorSpeed - outputU); //fixes the inverted way left control works

  if (rightMotorSpeed > 254) { //prevent values too high that are nonsensical
    rightMotorSpeed = 255;
  }
  if (leftMotorSpeed < 2) {
    leftMotorSpeed = 1;
  }

  analogWrite(PWMa, 0);
  analogWrite(PWMb, rightMotorSpeed);
  analogWrite(PWMc, 255);
  analogWrite(PWMd, leftMotorSpeed);


}


void pause()
{
  analogWrite(PWMa, 0);
  analogWrite(PWMb, 0);
  analogWrite(PWMc, 0);
  analogWrite(PWMd, 0);
  delay(100);
}
