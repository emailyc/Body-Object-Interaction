/* FSR testing sketch. 
 
Connect one end of FSR to 5V, the other end to Analog 0.
Then connect one end of a 10K resistor from Analog 0 to ground
Connect LED from pin 11 through a resistor to ground 
 
For more information see www.ladyada.net/learn/sensors/fsr.html */

#include <arduino.h>

#if defined(USBCON)
USBDevice.attach();
#endif

auto setup() -> void;
auto printCurrentTrial(const byte) -> void;
auto printCurrentStimuli(const byte) -> void;
auto sendSignalToPresentation(const int, const int, const int, const int) -> void;
auto endingConditions(unsigned long) -> void;


static const float  g_MAX_Pressure{2000};
static const float  g_Aref{1.076};   
static const int    g_ADC_Range{1024};
static const int    g_Newton{ 9.80665 }; 
static const float  g_Input_Voltage{3300}; //Connect bread board power to 3300mV power source

static const unsigned long g_OneHourInMilliSeconds{ 1000l * 60l * 60l }; // one hour in milli seconds

static const byte   g_sensorPinLeft{A0};      //Left Pressure Sensor
static const byte   g_sensorPinRight{A1};     //Right Pressure Sensor
static const float  g_leftADCThreshold{0.0 * g_ADC_Range};
static const float  g_rightADCThreshold{0.0 * g_ADC_Range};

static const byte g_bitMaskPin23{ (1 << 3) | (1 << 2) };
static const byte g_bitMaskPin8910{ (1 << 2) | (1 << 1) | (1 << 0)  };

enum class sitmuliType
{
  Low_BOI = 1,     // 0000 0001
  High_BOI,        // 0000 0010
  Abstract,        // 0000 0011
};

enum class trialType
{
  Stimulus = 1,    // 0000 0001
  ITI,             // 0000 0010
  Break,           // 0000 0011
  Instructions,    // 0000 0100
  End,             // 0000 0101
};    


int main() 
{
  init();
  setup();

  int fsrReadingLeft;
  int fsrReadingRight;
  
//  double voltageLeft;
//  double voltageRight;
  
  double pressureLeft;
  double pressureRight;

  unsigned long startClock{0};
    
  Serial.println("Left Reading,Right Reading,Trial,Stimuli,Experiment Time Clock (Micro Sec)");

  while(1)
  {
    endingConditions(startClock);

    if (!startClock )
    {
      if ( !( PIND & g_bitMaskPin23 ) ) 
      {
        continue;
      } else                      // stimuli signal started
      {
        startClock = micros();    //Remember micros() will overflow after approximately 70 minutes!!!
      }
    }


    fsrReadingLeft = analogRead(g_sensorPinLeft);
    fsrReadingRight = analogRead(g_sensorPinRight);

    //    sendSignalToPresentation(fsrReadingLeft, fsrReadingRight, g_leftADCThreshold, g_rightADCThreshold);
    
//    voltageLeft = map(float(fsrReadingLeft), 0, g_ADC_Range, 0, g_Input_Voltage);
//    voltageRight = map(float(fsrReadingRight), 0, g_ADC_Range, 0, g_Input_Voltage);
//    ***Do voltage calculation during data analysis instead of calculating here. Saves performance.***
//  
//    pressureLeft = (float(fsrReadingLeft) / g_ADC_Range) * g_MAX_Pressure;
//    pressureRight = (float(fsrReadingRight) / g_ADC_Range) * g_MAX_Pressure;
   
    Serial.print(fsrReadingLeft);
    Serial.print(",");
    Serial.print(fsrReadingRight);
    Serial.print(",");
    printCurrentTrial(g_bitMaskPin8910);
    Serial.print(",");
    printCurrentStimuli(g_bitMaskPin23);
    Serial.print(",");
    Serial.println((micros() - startClock)); //Minus startClock because Arduino clock starts counting at boot up.
  }

  return 0;
}

void setup(void) 
{
  Serial.begin(9600);

  // connect digital pin 2 to parallal port pin 2 (D0) 
  // connect digital pin 3 to parallal port pin 3 (D1) 
  // connect digital pin 4 to parallal port pin 4 (D2) 
  
  DDRD = (DDRD | 0b0);                                // sets Arduino pins 2 to 7 as inputs.
                                                      // without changing the value of pins 0 & 1, which are RX & TX
                                                      // People online say don't use pin 0 and 1

  // connect digital pin 8 to parallal port pin 5 (D3) 
  // connect digital pin 9 to parallal port pin 6 (D4) 
  // connect digital pin 10 to parallal port pin 7 (D5) 
  DDRB = (DDRB | 0b0);                                // sets Arduino pins 8 to 13 as inputs.                          


  pinMode(g_sensorPinLeft, INPUT);
  pinMode(g_sensorPinRight, INPUT);
}



auto printCurrentTrial(const byte bitMask) -> void
{
  switch( PINB & bitMask ) // read first three bits of PINB. i.e. pin 8, 9, 10                
  {
    case static_cast<uint8_t>(trialType::Stimulus):
      Serial.print("Stimulus");
      break;
    case static_cast<uint8_t>(trialType::ITI):
      Serial.print("ITI");
      break;
    case static_cast<uint8_t>(trialType::Break):
      Serial.print("Break");
      break;
    case static_cast<uint8_t>(trialType::Instructions):
      Serial.print("Instructions_or_others_messages");
      break;
    default:
      Serial.print("No_signal");
      break;
  }

}

auto sendSignalToPresentation(const int fsrReadingLeft, const int fsrReadingRight, const int leftADCThreshold, const int rightADCThreshold) -> void
{
  
  if( (fsrReadingLeft > leftADCThreshold) && (fsrReadingRight > rightADCThreshold) )
  {
    PORTB |= ( (1 << 1) | (1 << 0) );                //set both outputSignal_pinLeft pin and outputSignal_pinRight pin to HIGH
  } else if( (fsrReadingLeft > leftADCThreshold) )
  {
    PORTB &= ~(1 << 0);                              //set outputSignal_pinRight pin to LOW
    PORTB |= (1 << 1);                               //set outputSignal_pinLeft pin to HIGH
  } else if( (fsrReadingRight > rightADCThreshold) )
  {
    PORTB &= ~(1 << 1);                              //set outputSignal_pinLeft pin to LOW
    PORTB |= (1 << 0);                               //set outputSignal_pinRight pin to HIGH
  } else 
  {
    PORTB &= ~( (1 << 1) | (1 << 0) );               //set both outputSignal_pinLeft pin and outputSignal_pinRight pin to LOW
  }
}

auto printCurrentStimuli(const byte bitMask) -> void
{

  
  switch( (PIND & bitMask) >> 2 ) //Read first three bits
  {
    case static_cast<uint8_t>(sitmuliType::Low_BOI):
      Serial.print("Low_BOI");                         
      break;
    case static_cast<uint8_t>(sitmuliType::High_BOI):
      Serial.print("High_BOI");                         
      break;
    case static_cast<uint8_t>(sitmuliType::Abstract):
      Serial.print("Abstract");                         
      break;
    default:
      Serial.print("-1");
      break;         
  }
  return;
}

auto endingConditions(unsigned long startClock) -> void
{
  String endReason;
  
  if( (PIND & (1<<7)) ) //read pin 7
  {
    endReason = "End button pressed";
  } else if (millis() > g_OneHourInMilliSeconds)
  {
    endReason = "Experiment_exceeded_1_hour, terminate_automatically";
  } else if ( (PINB & g_bitMaskPin8910) == static_cast<uint8_t>(trialType::End))
  {
    endReason = "Experiment_complete";
  } else
  {
    return;
  }
  Serial.print("-1,-1,");
  Serial.print(endReason);
  Serial.print(",-1,");
  Serial.println((micros() - startClock));
  delay(100);
  exit(0);
}
