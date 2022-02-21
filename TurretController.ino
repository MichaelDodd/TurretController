#include "Globals.h"
#include "TurretCtrl.h"
#include "IO_Controller.h"
#include <arduino.h>

TurretCtrl    Turret;
IO_Controller IO;


void setup() 
{
  Serial.begin(9600);   

  IO.Init();

 // IO.LockOn();
//  IO.Update();
}




/*
 * loop()
 * 
 * cyclic operation - deals with all normal run-time moding.
 *
 */
void loop() 
{
  bool COMMISSIONING = false;
  
  // Update all discrete signals
  IO.Update();

  if (COMMISSIONING)
  {
    IO.Update();
    if (IO.LockInp == 0)
      IO.J3Off();
    else
      IO.J3On();

    if (IO.IndexInp == 0)
      IO.J4Off();
    else
      IO.J4On();
        
    if (IO.JogBck)
      IO.StepBkd();
    if (IO.JogFwd)       
    {
      IO.StepFwd();
      if (IO.LockInp == 1)
         IO.LockOff();
    } 
//    if (IO.JogFwdSelected())
//    {
//       IO.LockOn();
//       delay(10000);
//       IO.LockOff();
//    }
  }
  else
  {
    if (Turret.EverythingIsGood())
    {
      if (IO.JogFwdSelected())
      {
        uint8_t NewPosn = 0, CurrentPosn = Turret.CurrentPosition();
    
        if ((CurrentPosn > 0) && (CurrentPosn <= Turret.MaxToolPositions()))
        {
           if (CurrentPosn < Turret.MaxToolPositions())
           {
              NewPosn = CurrentPosn + 1;
           }
           else
           {
              NewPosn = 1;
           }
           
          Turret.Goto(NewPosn);
        }    
        else
          Turret.Goto(1);
        
      }
    
      if (IO.JogBckSelected())
      {
        uint8_t NewPosn = 0, CurrentPosn = Turret.CurrentPosition();
    
        if ((CurrentPosn > 0) && (CurrentPosn <= Turret.MaxToolPositions()))
        {
          if (CurrentPosn > 1)
          {
            NewPosn = CurrentPosn - 1;
          }
          else
          {
            NewPosn = Turret.MaxToolPositions();
          }
    
          Turret.Goto(NewPosn);
        }
        else
          Turret.Goto(1);
      }
    
      if (IO.NewCmdSelected())
      {
        uint8_t NewPosn = IO.PosnCmd;
    
        if ((NewPosn >= 1) && (NewPosn <= Turret.MaxToolPositions()))
        {
          Turret.Goto(NewPosn);
        }
      }
    }
    else
    {
      Serial.println("Everything is NOT okay");
    } 
  }
}
