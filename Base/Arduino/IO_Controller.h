#include <Arduino.h>

#ifndef IO_Controller_h
#define IO_Controller_h

#include "src/MCP23017.h"

typedef enum { UNDEFINED, FORWARD, BACKWARD } LastDirection_t;

class IO_Controller
{
   public:
      void Init();
      void Update();
      void Error(uint8_t Error);

      void BrakeOn();
      void BrakeOff();
      void LockOn();
      void LockOff();
      void J3On();
      void J3Off();
      void J4On();
      void J4Off();
      void StepFwd();
      void StepBkd();
      bool JogFwdSelected();
      bool JogBckSelected();
      bool NewCmdSelected();


      // LockInp    : Turret Input - Lock signal asserted
      uint8_t LockInp;
      // IndexInp   : Turret Input - Index signal asserted
      uint8_t IndexInp;
      // TurretPosn : Turret Position 
      uint8_t TurretPosn;
      // TurretStrobe : Turret Strobe signal
      uint8_t TurretStrobe;
      // Turret_Parity : Turret Parity signal
      uint8_t TurretParity;
      // RelayBrake    : Demanded Brake Signal
      uint8_t RelayBrake;
      // RelayLock     : Demanded Lock Signal      
      uint8_t RelayLock;
      // RelayJ3    : Demanded J3 Signal
      uint8_t RelayJ3;
      // RelayJ4     : Demanded J4 Signal      
      uint8_t RelayJ4;
      
      // JogBck  : Bob input - Hardware commanded Jog-Tool-Backwords
      uint8_t JogBck;
      // JogFwd  : Bob input - Hardware commanded Jog-Tool-Forwards
      uint8_t JogFwd;
      // Busy    : BoB output - Turret is busy, ignoring inputs
      uint8_t Busy;
      // NewCmd  : BoB input - New commanded position, sampled on leading edge of PosnCmd
      uint8_t NewCmd;
      // PosnCmd : BoB input - New commanded position signal
      uint8_t PosnCmd;
   private:
      void UpdateRelays();
      LastDirection_t LastDirection = UNDEFINED;
      
      MCP23017 mcp = MCP23017(0x20);
      uint8_t OldCmd;
      
};


#endif
