// HEADER FILES: *****************************************************
#include <SerialComm.h>
#include <EEPROM.h>
#include <HX711.h>
#include <HC_SR04.h>
#include <AccelStepper.h>
// END HEADER: *****************************************************

// NODEMCU'S USEFUL SERIAL ENUMS:
enum Item
{
  TITLE = 0,
  ITEMA,
  ITEMB,
  ITEMC,
  ITEMD
};

enum Menu
{
  MAIN = 0,
  PLAN,
  DANNI,
  NASTROIKI,
  VRUZKA
};
// END ************************

// Motor definitions
#define HALFSTEP 8
#define motorPin1  12     // IN1 on the ULN2003 driver 1
#define motorPin2  11     // IN2 on the ULN2003 driver 1
#define motorPin3  10     // IN3 on the ULN2003 driver 1
#define motorPin4  9     // IN4 on the ULN2003 driver 1
#define motorPinEN A5 
#define quarterRotation 500
#define turnsPer10g     3
#define rotationPer10g (quarterRotation * turnsPer10g)
#define rotationUnblock (quarterRotation / 2)

// Scale definitions
#define TARETIMES 3
#define RSCALE 1985.66
#define LSCALE 2196.89
#define BOWLWEIGHT 85

enum Bowls
{
  LBowl = 0,
  RBowl,
  BothBowls
};

enum Memory
{
  LBowlTare = 0,
  LBowlScale,
  RBowlTare,
  RBowlScale  
};

// GLOBALS ***********************
static unsigned foodRequest[2] = {0};
static unsigned int foodTarget[2] = {0};
static float foodWeight[2] = {0};
static float capacity      =  0;
static byte rotate         =  0;

HX711 bowlSensorLeft(A6, A2, A3);
HX711 bowlSensorRight(A6, A0, A1);
HC_SR04 distanceSensor(A7, 8, 7, 5, 0.25, 24);
AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);
// END_GLOBALS ***********************

void setup()
{
  Serial.begin(115200);
  delay(10);

  pinMode(A6, OUTPUT);
  digitalWrite(A6, LOW);
  
  pinMode(motorPinEN, OUTPUT); 
  digitalWrite(motorPinEN, HIGH);
  
  stepper1.setMaxSpeed(250.0);
  stepper1.setAcceleration(200.0);
  stepper1.setSpeed(200); 
  
  bowlSensorLeft.set_scale(LSCALE);  
  bowlSensorRight.set_scale(RSCALE); 
}

void loop()
{
  checkSerial();

  if (stepper1.distanceToGo() != 0)
  {

  digitalWrite(motorPinEN, LOW);
    stepper1.run();
  }
  else
  {
    
  digitalWrite(motorPinEN, HIGH);
    stepper1.run();
  }
  
  //feed_handle();
  //stepper_handle();
  //autoUpdateData();
}

void feed_handle()
{
  if (stepper1.distanceToGo() == 0)
  {
    if (foodRequest[LBowl] > 0)
    {
      foodTarget[LBowl] = foodWeight[LBowl] + (foodRequest[LBowl]*10);
      if (foodTarget[LBowl] >= 250) foodTarget[LBowl] = 250;
      foodRequest[LBowl] = 0;
    }
    else if (foodRequest[RBowl] > 0)
    {
      foodTarget[RBowl] = foodWeight[RBowl] + (foodRequest[RBowl]*10);
      if (foodTarget[RBowl] >= 250) foodTarget[RBowl] = 250;
      foodRequest[RBowl] = 0;
    }
  }
}

void stepper_handle()
{ 
  static bool stepperRotating = false;
  static long stepperStartTime = 0;
  static long backtrackTime = millis();
  static long backtrackPos = stepper1.currentPosition();
  static bool stepperBlocked = false;
  static byte lastBowl = LBowl;

  if (stepper1.distanceToGo() == 0)
  {
    if (stepperRotating == true)
    {      
      digitalWrite(motorPinEN, HIGH);
      stepperRotating = false;
      stepperStartTime = 0;
    }
    
    if (foodTarget[LBowl] > foodWeight[LBowl])
    {
      stepper1.moveTo(stepper1.currentPosition() + rotationPer10g);
      lastBowl = LBowl;
    }
    if (foodTarget[RBowl] > foodWeight[RBowl])
    {
      stepper1.moveTo(stepper1.currentPosition() - rotationPer10g);
      lastBowl = RBowl;
    }
  }
  else
  {
    if (stepperRotating == false)
    {
      digitalWrite(motorPinEN, LOW);
      stepperRotating = true;
      stepperStartTime = millis();
      backtrackPos = stepper1.currentPosition();
      backtrackTime = stepperStartTime;
    }
    else
    {       
      if (millis() - backtrackTime > 1000)
      {
        if (abs(backtrackPos - stepper1.currentPosition()) < rotationUnblock)
        {
          stepperBlocked = true;
        }
        backtrackTime = millis();
        backtrackPos = stepper1.currentPosition();
      }
      
      if (stepperBlocked)
      {
        if (lastBowl == LBowl)
        {
          stepper1.moveTo(stepper1.currentPosition() - rotationUnblock);          
        }
        else if (lastBowl == RBowl)
        {
          stepper1.moveTo(stepper1.currentPosition() + rotationUnblock);             
        }
      }
    }    
    
    if (foodTarget[LBowl] <= foodWeight[LBowl])
    {      
      if (foodTarget[LBowl] != 0)
      {
        foodTarget[LBowl] = 0;
      }
    }
    if (foodTarget[RBowl] <= foodWeight[RBowl])
    {      
      if (foodTarget[RBowl] != 0)
      {
        foodTarget[RBowl] = 0;
      }
    }

    stepper1.run();
  }
  
}

void tareBowl(byte bowl)
{
  double avg = 0;
  
  if (bowl == LBowl)
  {    
    bowlSensorLeft.tare(5);
  }
  else if (bowl == RBowl)
  {
    bowlSensorRight.tare(5);
  }
  else if(bowl == BothBowls)
  {    
    bowlSensorLeft.tare(5);
    bowlSensorRight.tare(5);
  }
  updateData();
}

void autoUpdateData()
{
  static long lastUpdate = 0;
  
  if (millis() - lastUpdate >= 1000)
  {
    lastUpdate = millis();
    updateData();
  }
}

void updateData()
{
  foodWeight[LBowl] = abs(bowlSensorLeft.get_units(5));
  foodWeight[RBowl] = abs(bowlSensorRight.get_units(5));
  //float capacity = distanceSensor.get_capacity(2500, 22.80);
  //if (capacity < 100) capacity = 0;
  
}

void checkSerial()
{
  char* input = processSerial();
  
  if (strcmp(input,"0") != NULL)
  {
    if (strcmp(input,"INIT") == NULL)
    {
      Serial.println("READY");
      tareBowl(BothBowls);
      updateData();
    }
    if (strcmp(input,"STATS") == NULL)
    {
      updateData();
      Serial.print("STATS:");
      Serial.print(DANNI);Serial.print(":");Serial.print(ITEMA);Serial.print(":");
      Serial.println("513");
      
      Serial.print("STATS:");
      Serial.print(DANNI);Serial.print(":");Serial.print(ITEMB);Serial.print(":");
      Serial.println("0");
      
      Serial.print("STATS:");
      Serial.print(DANNI);Serial.print(":");Serial.print(ITEMC);Serial.print(":");
      Serial.println("28");
    }
    else if (strcmp(input,"TARE") == NULL)
    {
      tareBowl(LBowl);
      tareBowl(RBowl);
    }
    else if (strcmp(input,"<TARE") == NULL)
    {
      tareBowl(LBowl);
    }
    else if (strcmp(input,">TARE") == NULL)
    {
      tareBowl(RBowl);
    }
    else if (strstr(input,"FEED") != NULL)
    {      
      stepper1.moveTo(stepper1.currentPosition()+1000);
    }
    else if (strstr(input,"FEED") != NULL)
    {
      char* p = input;
      byte bowl, request, i = 0;
      
      while (*p)
      {
          if (*p++ == ':')
          {
            request = atoi(p);
            if (request > 25) request = 25;
            i++;
          }
          else if (*p++ == 'L')
          {
            bowl = LBowl;
            i++;
          }
          else if (*p++ == 'R')
          {
            bowl = RBowl;
            i++;
          }
          else if (*p++ == 'B')
          {
            bowl = BothBowls;
            i++;
          }
      }

      switch(bowl)
      {
        case LBowl:
        case RBowl:
          foodRequest[bowl] = request;
          break;
        case BothBowls:
          foodRequest[LBowl] = request;
          foodRequest[RBowl] = request;
         break;
      }
    }
  }
}

void EEPROMWritelong(int address, long value)
{
  long addressActual = address*4;
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
  
  //Write the 4 bytes into the eeprom memory.
  EEPROM.write(addressActual, four);
  EEPROM.write(addressActual + 1, three);
  EEPROM.write(addressActual + 2, two);
  EEPROM.write(addressActual + 3, one);
}

long EEPROMReadlong(long address)
{
  long addressActual = address*4;
  //Read the 4 bytes from the eeprom memory.
  long four = EEPROM.read(addressActual);
  long three = EEPROM.read(addressActual + 1);
  long two = EEPROM.read(addressActual + 2);
  long one = EEPROM.read(addressActual + 3);
  
  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
