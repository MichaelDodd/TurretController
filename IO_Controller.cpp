#include "IO_Controller.h"
#include "Globals.h"
#include <arduino.h>
#include <Wire.h>
#include "src/MCP23017.h"

/*
 * IO_Controller()
 * 
 * Constructor code to initialise the MCP23017 interface
 */
void IO_Controller::Init()
{
  // Stepper Driver outputs
  pinMode (11, OUTPUT);
  pinMode (12, OUTPUT);
  // Error output
  pinMode (13, OUTPUT);
  // Relay outputs
  pinMode (A0, OUTPUT);
  pinMode (A1, OUTPUT);
  pinMode (A2, OUTPUT);
  pinMode (A3, OUTPUT);

  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);

  // IIC I/O
  mcp.init();
  mcp.portMode(MCP23017Port::A, 0b11111111);   // Port A as output
  mcp.portMode(MCP23017Port::B, 0b11111110);   // Bit 0 = output, "Busy"; rest is input

  mcp.writeRegister(MCP23017Register::GPIO_A, 0x00); // Reset port A
  mcp.writeRegister(MCP23017Register::GPIO_B, 0x00); // Reset port B

  mcp.writeRegister(MCP23017Register::IPOL_B, 0x00);
  mcp.writeRegister(MCP23017Register::IPOL_B, 0x00);

  JogBck = 0;
  JogFwd = 0;
  NewCmd = 0;
  PosnCmd = 0;

  RelayLock = 0;
  RelayBrake = 0;
  RelayJ3 = 0;
  RelayJ4 = 0;
  
  Busy = 0;

  // Set the inputs and outputs
  Update();

  // store the current "NewCmd" value into OldCmd, to allow edge-detection on first instance.
  OldCmd = NewCmd;
}


/*
 * Update()
 * 
 * Perform all IIC activity to refresh I/O on all IO_Controller state data
 * 
 * Complete operation takes about 300us
 */
void IO_Controller::Update()
{
  uint8_t ip, white, yellow, green, violet, black, pink;

  // ******* PORT A INPUTS ******
  ip = mcp.readPort(MCP23017Port::A);

  white   = ((ip & 0b00100000) ? 1 : 0);
  yellow  = ((ip & 0b00010000) ? 1 : 0);
  green   = ((ip & 0b00001000) ? 1 : 0);
  violet  = ((ip & 0b00000100) ? 1 : 0);
  black   = ((ip & 0b00000010) ? 1 : 0);
  pink    = ((ip & 0b00000001) ? 1 : 0);

  TurretStrobe     = black;
  TurretParity     = pink;
  if (TurretStrobe)
  {
     TurretPosn       = (violet << 3) | (green << 2) | (yellow << 1) | (white << 0);    
  }
  
  LockInp  = ((ip & 0b10000000) ? 1 : 0);  
  IndexInp = ((ip & 0b01000000) ? 1 : 0);
  
  // ******* PORT B INPUTS ******  
  ip = mcp.readPort(MCP23017Port::B);
  JogFwd = ((ip & 0b10000000) ? 1 : 0);
  JogBck = ((ip & 0b01000000) ? 1 : 0);

  NewCmd = ((ip & 0b00000010) ? 1 : 0);
  PosnCmd = ((ip & 0b00111100) >> 2);

  // ******* PORT B OUTPUTS *****
  ip = ((Busy == 0) ? 0b00000000 : 0b00000001);
  mcp.writePort(MCP23017Port::B, ip);  

  // ******* Relay Outputs ******
  UpdateRelays();

  // ******* Stepper Motor Outputs ********
//  digitalWrite(11, (StepperDir == 0) ? 0 : 1);
//  digitalWrite(12, (StepperStep == 0) ? 0 : 1);
}

void IO_Controller::Error(uint8_t Error)
{
  digitalWrite(13, LOW);

  while (1)
  {
    for (uint8_t l = 0; l < Error; l++)
    {
      digitalWrite(13, HIGH);
      delay(200);
      digitalWrite(13, LOW);
      delay(200);
    }
    delay(1000);
  }

}

void IO_Controller::UpdateRelays()
{
  // ******* Relay Outputs ******
  digitalWrite(A0, (RelayBrake==0) ? 0 : 1);
  digitalWrite(A1, (RelayLock==0) ? 0 : 1);
  digitalWrite(A2, (RelayJ3==0) ? 0 : 1);
  digitalWrite(A3, (RelayJ4==0) ? 0 : 1);
}

void IO_Controller::BrakeOn()
{
  RelayBrake = 1;
  UpdateRelays();
}
void IO_Controller::BrakeOff()
{
  RelayBrake = 0;
  UpdateRelays();
}

void IO_Controller::LockOn()
{
  RelayLock = 1;
  UpdateRelays();
}
void IO_Controller::LockOff()
{
  RelayLock = 0;
  UpdateRelays();
}
void IO_Controller::J3On()
{
  RelayJ3 = 1;
  UpdateRelays();
}
void IO_Controller::J3Off()
{
  RelayJ3 = 0;
  UpdateRelays();
}
void IO_Controller::J4On()
{
  RelayJ4 = 1;
  UpdateRelays();
}
void IO_Controller::J4Off()
{
  RelayJ4 = 0;
  UpdateRelays();
}
void IO_Controller::StepFwd()
{
  switch (LastDirection)
  {
    case UNDEFINED:
    case BACKWARD:
       digitalWrite(DIR_PIN, DIR_BCK);
       delayMicroseconds(DIR_SETUP_TIME);
       break;
    case FORWARD:
       break;
  }

  digitalWrite(STEP_PIN,STEP_ON);
  delayMicroseconds(STEP_PULSE_WIDTH);
  digitalWrite(STEP_PIN,STEP_OFF);
  delayMicroseconds(STEP_PULSE_WIDTH);

  LastDirection = FORWARD;
}

void IO_Controller::StepBkd()
{
  switch (LastDirection)
  {
    case UNDEFINED:
    case FORWARD:
       digitalWrite(DIR_PIN, DIR_FWD);   
       delayMicroseconds(DIR_SETUP_TIME);
       break;
    case BACKWARD:
       break;
  }
  digitalWrite(STEP_PIN,STEP_ON);
  delayMicroseconds(STEP_PULSE_WIDTH);
  digitalWrite(STEP_PIN,STEP_OFF);
  delayMicroseconds(STEP_PULSE_WIDTH);

  LastDirection = BACKWARD;
}

//
// JogFwd - Button Press must be between 50 and 2000 milliseconds, otherwise rejected
//
bool IO_Controller::JogFwdSelected()
{
  static uint8_t LastSelc;
  static unsigned long PressedTimer = 0;

  if ((LastSelc == 0) && (JogFwd != 0))  
  {
     // Button pressed
     PressedTimer = millis();
     LastSelc = JogFwd;
  }
  else if ((LastSelc == 1) && (JogFwd == 0))
  {
     LastSelc = JogFwd;
     // Button released
     unsigned long ButtonPressedTime = millis() - PressedTimer;
     if (ButtonPressedTime > 2000)
        return false;
     if (ButtonPressedTime > 300)
     {
      Serial.println("Fwd Pressed");
        return true;
     }
  } 
  else
  {
     LastSelc = JogFwd;
  }
  
  return false;
}

//
// JogBck - Button Press must be between 50 and 2000 milliseconds, otherwise rejected
//
bool IO_Controller::JogBckSelected()
{
  static uint8_t LastSelc;
  static unsigned long PressedTimer = 0;

  if ((LastSelc == 0) && (JogBck != 0))  
  {
     // Button pressed
     PressedTimer = millis();
     LastSelc = JogBck;
  }
  else if ((LastSelc == 1) && (JogBck == 0))
  {
     LastSelc = JogBck;
     // Button released
     unsigned long ButtonPressedTime = millis() - PressedTimer;
     if (ButtonPressedTime > 2000)
        return false;
     if (ButtonPressedTime > 300)
     {
      Serial.println("Bck Pressed");
        return true;
     }
  } 
  else
  {
     LastSelc = JogBck;
  }
  
  return false;
}

//
// NewCmd - Command Strobe must be between 50 and 2000 milliseconds, otherwise rejected
//
bool IO_Controller::NewCmdSelected()
{
  static uint8_t LastSelc;
  static unsigned long PressedTimer = 0;

  if ((LastSelc == 0) && (NewCmd != 0))  
  {
     // Button pressed
     PressedTimer = millis();
     LastSelc = JogBck;
  }
  else if ((LastSelc == 1) && (NewCmd == 0))
  {
     LastSelc = NewCmd;
     // Button released
     unsigned long ButtonPressedTime = millis() - PressedTimer;
     if (ButtonPressedTime > 2000)
        return false;
     if (ButtonPressedTime > 50)        
     {
      Serial.println("Bck Pressed");
        return true;
     }
  } 
  else
  {
     LastSelc = JogBck;
  }
  
  return false;
}
