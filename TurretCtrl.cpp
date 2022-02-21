#include "TurretCtrl.h"
#include "Globals.h"
#include <arduino.h>
#include "IO_Controller.h"

extern IO_Controller IO;

void Internals()
{

  Serial.print("LockInp:");
  Serial.print(IO.LockInp);
  Serial.print("  IndexInp:");
  Serial.print(IO.IndexInp);
  Serial.println();
}

TurretCtrl::TurretCtrl()
{
}

uint8_t TurretCtrl::CurrentPosition()
{
  uint8_t Posn = 0;
  

  Posn = IO.TurretPosn;
  switch (Posn)
  {
    case 1:
    case 2:
    case 4:
    case 7:
    case 8:
      if (IO.TurretParity == 1)
        return Posn;
      else
        return 0;

    case 3:
    case 5:
    case 6:
      if (IO.TurretParity == 0)
        return Posn;
      else
        return 0;

    default:
       return 0;  
  }
  
  return Posn;
  
}



bool TurretCtrl::UnLock()
{
  IO.LockOff();
  /*
  digitalWrite(OUT_STEPPER_DIR, DIR_CCW);

  delay(50);   // 50ms

  for (int l = 0; l < 1000; l++)
  {
    Step();
  }  
  */
  return(true);
}

unsigned int LockSteps = 1000;
unsigned int UnLockSteps = 1000;

bool TurretCtrl::Lock()
{
  unsigned int StepCount = 0;
  
  IO.LockOn();

  delay(50);   // 50ms
 
/*
  while ( (StepCount < LockSteps) && GetSpindleLock() == false )
  {
    Step();
    StepCount++;     
  }
 */ 
  return(true);
}

bool TurretCtrl::Brake()
{
  IO.BrakeOn();
  return(true);
}

bool TurretCtrl::UnBrake()
{
  IO.BrakeOff();
  return(true);
}


void TurretCtrl::Update()
{
//  GetPosition();
}


bool TurretCtrl::Goto(int PositionDmd)
{
  
  LastDirection_t Direction = UNDEFINED;
  Position = CurrentPosition();

  Serial.println();
    Serial.print("Turret.Goto : ");
  Serial.print(PositionDmd);
  Serial.print("    from : ");
  Serial.println(Position);
  
  if ((PositionDmd >= 1) && (PositionDmd <= 8))
  {
    if (PositionDmd != Position)
    {
      // Are we going forwards (1->2->3) or backwards (1->8->7)?
      uint8_t distance = (PositionDmd - Position);
      if (distance <= 0) 
         distance = distance + MAX_TOOL_POSITIONS;

      if (distance > ( MAX_TOOL_POSITIONS / 2))
      {
        Direction = BACKWARD;
        Serial.println("Backward");
      }
      else
      {
        Direction = FORWARD;
        Serial.println("Forward");
      }
      ////////////////////////////////////////////////
      // Start Cycle
      ////////////////////////////////////////////////
  // Don't forget - you must call IO.Update within any looped operation within here!!!
      //
      // Unlock the Turret
Serial.println("Testing which way to unlock the turret");
Internals();
      UnLock();
      IO.Update();
      bool Unlocked = false;
      LastDirection_t   UnlockDirection = UNDEFINED;

      if (IO.LockInp == 0)
      {
         Unlocked = true;
         Serial.println("Turret is already unlocked");
         UnlockDirection = FORWARD;
      }
Serial.println("Testing forwards");
Internals();
      for (int l = 0; ((l < 1000) && (Unlocked == false)); l++)
      {
         IO.Update();
         IO.StepFwd();
         if (IO.LockInp == 0)
         {
           // Successfully unlock in the forward direction
           Serial.println("Forward Unlocking");
           Unlocked = true;
           UnlockDirection = FORWARD;
         }
      }
      if (UnlockDirection == UNDEFINED)
      {
  Serial.println("Testing backwards");
Internals();
        for (int l = 0; ((l < 1000) && (Unlocked == false)); l++)
        {
           IO.Update();
           IO.StepBkd();
           if (IO.LockInp == 0)
           {
              Serial.println("Backward Unlocking");
              Unlocked = true;
              UnlockDirection = BACKWARD;
           }
        }
      }

      // Carry on moving until the Strobe signal is de-asserted
Serial.println("Continuing to unlock until Lock is de-asserted");
      while (IO.LockInp == 1)
      {
         IO.Update();
         if (UnlockDirection == FORWARD)
            IO.StepFwd();
         else
            IO.StepBkd();
      }
Serial.println("We're not unlocked and the turret has moved");
      delay(1000);
bool AtPosition = false;

Serial.println("Trying to go to position");
      if (CurrentPosition() == PositionDmd)
      {
        AtPosition = true;
      }
      // Move until we get to the new position
      while (!AtPosition)
      {
        IO.Update();

        if (CurrentPosition() == PositionDmd)
        {
          AtPosition = true;
        }

        if (Direction == FORWARD)
        {
          IO.StepFwd();
        }
        else
        {
          IO.StepBkd();
        }
        
      }
      // Here, we're at position, need to set the lock
      Lock();         // Lock the turret
//      delay(500);
      Serial.println("Strike Index Pin");
      while (IO.IndexInp == 0)
      {
        IO.Update();
        if (Direction == FORWARD)
        {
          IO.StepFwd();
        }
        else
        {
          IO.StepBkd();
        }        
      }
      Serial.println("Strike Index Pin in lock position");
      delay(500);
      Serial.println("Continuing in opposite direction until Lock is asserted");
      while (IO.LockInp == 0)
      {
        IO.Update();
        if (Direction == FORWARD)
        {
          IO.StepBkd();
        }
        else
        {
          IO.StepFwd();
        }        
      }
      Serial.println("Contrinue to overdrive the lock");
      for (int l = 0; l<3000; l++)
      {
        IO.Update();
        if (Direction == FORWARD)
        {
          IO.StepBkd();
        }
        else
        {
          IO.StepFwd();
        }        
      }
      Serial.println("Finished");

      UnLock();
    }
    return true;
  }
  else
  {
    return false;
    // Invalid position demanded
  }
}


/*
 * TestCase(int Test)
 * 
 * Provides access to a series of unit-test cases for the methods provided
 * by the TurretCtrl class
 */

int TurretCtrl::TestCase(int Test)
{
  int retval = 0;
  return (retval);
}


uint8_t TurretCtrl::MaxToolPositions()
{
  return MAX_TOOL_POSITIONS;
}

bool TurretCtrl::EverythingIsGood()
{
  return true;
}
