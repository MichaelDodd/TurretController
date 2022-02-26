#ifndef TurretCtrl_h
#define TurretCtrl_h

#include <arduino.h>

class TurretCtrl
{
   public:
      TurretCtrl();
      bool Goto(int PositionDmd);
      void Update();
      bool Calibrate();
      int  TestCase(int Test);
      uint8_t MaxToolPositions();
      // CurrentPosition - returns a validated "good" current position of the turret controller.
      uint8_t CurrentPosition();
      bool EverythingIsGood();      
      bool Lock();
      bool UnLock();
      bool Brake();
      bool UnBrake();
   private:
      // Data Store
      int  Position = 0;
      // Internal Methods
      bool GetSpindleLock();
      bool IsParityOkay();
      bool Good;
};

#endif
