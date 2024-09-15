/*
   Test and validation of the SI4735 Arduino Library.
   It is a FM, MW and SW (1700kHz to 30000kHz)
   
   ATTENTION:  Please, avoid using the computer connected to the mains during testing.

   The main advantages of using this sketch are: 
    1) It is a easy way to check if your circuit is working;
    2) You do not need to connect any display device to make your radio works;
    3) You do not need connect any push buttons or encoders to change volume and frequency;
    4) The Arduino IDE is all you need to control the radio.  
   
   This sketch has been successfully tested on:
    1) Pro Mini 3.3V; 
    2) UNO (by using a voltage converter); 
    3) Arduino Yún;
    4) Arduino Mega (by using a voltage converter); and 
    5) ESP32 (LOLIN32 WEMOS)


    The table below shows the Si4735 and Arduino Pro Mini pin connections 
    
    | Si4735 pin      |  Arduino Pin  |
    | ----------------| ------------  |
    | RESET (pin 15)  |     12        |
    | SDIO (pin 18)   |     A4        |
    | CLK (pin 17)   |     A5        |


  I strongly recommend starting with this sketch.

  Schematic: https://github.com/pu2clr/SI4735/blob/master/extras/images/basic_schematic.png

  Prototype documentation : https://pu2clr.github.io/SI4735/
  PU2CLR Si47XX API documentation: https://pu2clr.github.io/SI4735/extras/apidoc/html/

   By Ricardo Lima Caratti, Nov 2019.
*/

//#include <Arduino.h>

#include "SI4735.h"
#include "Serial.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define PICO_SUPPLIES_CLOCK 1
#if PICO_SUPPLIES_CLOCK
#define RADIO_CLOCK_PIN 9
#define PWM_CHANNEL 4
#define SYSTEM_CLOCK 125000000
#define PWM_FREQUENCY 131000
#define PWM_DIVIDER (SYSTEM_CLOCK / PWM_FREQUENCY)
#endif

#define RESET_PIN 6

#define AM_FUNCTION 1
#define FM_FUNCTION 0

void showHelp();
void showStatus();
void showFrequency( uint16_t freq );

uint16_t currentFrequency;
uint16_t previousFrequency;
uint8_t bandwidthIdx = 0;
const char *bandwitdth[] = {"6", "4", "3", "2", "1", "1.8", "2.5"};

class PicoSerial * PicoSerial::instance = nullptr;

class PicoSerial& Serial=PicoSerial::get_instance();

class SI4735 * SI4735::instance = nullptr;



void setup()
{
  SI4735& rx= SI4735::get_instance();

  Serial.begin(9600);
  //while(!Serial);

  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);

  printf("System clock frequency: %d Hz\n",clock_get_hz(clk_sys));
  
 
  
  Serial.println("AM and FM station tuning test.");

  showHelp();

  // Look for the Si47XX I2C bus address
  //rx.setI2CStandardMode();
    rx.setI2CLowSpeedMode();
  int16_t si4735Addr = rx.getDeviceI2CAddress(RESET_PIN);
  if ( si4735Addr == 0 ) {
    Serial.println("Si473X not found!");
    Serial.flush();
    while (1);
  } else {
    printf("Si47XX I2C bus address is %X\n", si4735Addr);
    //Serial.print("The Si473X I2C address is 0x");
    //Serial.println(si4735Addr, HEX);
  }

  //delay(500);

 #if PICO_SUPPLIES_CLOCK
  gpio_set_function(RADIO_CLOCK_PIN, GPIO_FUNC_PWM);
  // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
  uint slice_num = pwm_gpio_to_slice_num(9);

  // pwm_set_clkdiv_mode(slice_num, PWM_DIV_FREE_RUNNING);

  // Set period of 650 clock cycles (100KHz with a 130MHZ clock)
  pwm_set_wrap(slice_num, PWM_DIVIDER);
  // Set channel A output high for half e before dropping
  pwm_set_chan_level(slice_num, PWM_CHAN_B, PWM_DIVIDER/2);
  pwm_set_enabled(slice_num, true);
  
  rx.setup(RESET_PIN, -1, POWER_UP_FM, SI473X_ANALOG_AUDIO, XOSCEN_RCLK);
  rx.setRefClock(32723); // 130889/4
  rx.setRefClockPrescaler(4,0);
#else
  rx.setup(RESET_PIN, FM_FUNCTION);
  #endif

  // Starts defaul radio function and band (FM; from 84 to 108 MHz; 103.9 MHz; step 100kHz)
  rx.setFM(8400, 10800, 995, 10);
  delay(500);
  currentFrequency = previousFrequency = rx.getFrequency();
  rx.setVolume(45);
  showStatus();
}

void showHelp()
{

  Serial.println("Type F to FM; A to MW; 1 to All Band (100kHz to 30MHz)");
  Serial.println("Type U to increase and D to decrease the frequency");
  Serial.println("Type S or s to seek station Up or Down");
  Serial.println("Type + or - to volume Up or Down");
  Serial.println("Type 0 to show current status");
  Serial.println("Type B to change Bandwidth filter");
  Serial.println("Type 4 to 8 (4 to step 1; 5 to step 5kHz; 6 to 10kHz; 7 to 100kHz; 8 to 1000kHz)");
  Serial.println("Type ? to this help.");
  Serial.println("==================================================");
  delay(1000);
}

// Show current frequency
void showStatus()
{
  SI4735& rx= SI4735::get_instance();
  // rx.getStatus();
  previousFrequency = currentFrequency = rx.getFrequency();
  rx.getCurrentReceivedSignalQuality();
  Serial.print("You are tuned on ");
  if (rx.isCurrentTuneFM())
  {
    printf("%3.1f MHz %s\n", currentFrequency, (rx.getCurrentPilot()) ? "STEREO" : "MONO");
    //Serial.print(String(currentFrequency / 100.0, 2));
    //Serial.print("MHz ");
    //Serial.print((rx.getCurrentPilot()) ? "STEREO" : "MONO");
  }
  else
  {
    Serial.print(currentFrequency);
    Serial.print("kHz");
  }
  Serial.print(" [SNR:");
  Serial.print(rx.getCurrentSNR());
  Serial.print("dB");

  Serial.print(" Signal:");
  Serial.print(rx.getCurrentRSSI());
  Serial.println("dBuV]");
}

void showFrequency( uint16_t freq ) {

  SI4735& rx= SI4735::get_instance();
  if (rx.isCurrentTuneFM())
  {
    printf("%.2f MHz\n", freq / 100.0);
    //Serial.print(String(freq / 100.0, 2));
    //Serial.println("MHz ");
  }
  else
  {
    Serial.print(freq);
    Serial.println("kHz");
  }
  
}


// Main
void loop()
{
  SI4735& rx= SI4735::get_instance();
  while (Serial.available() > 0)
  {
    char key = Serial.read();
    switch (key)
    {
    case '+':
      rx.volumeUp();
      break;
    case '-':
      rx.volumeDown();
      break;
    case 'a':
    case 'A':
      rx.setAM(520, 1750, 810, 10);
      rx.setSeekAmLimits(520, 1750);
      rx.setSeekAmSpacing(10); // spacing 10kHz
      break;
    case 'f':
    case 'F':
      rx.setFM(8600, 10800, 10390, 50);
      rx.setSeekAmRssiThreshold(0);
      rx.setSeekAmSrnThreshold(10);
      break;
    case '1':
      rx.setAM(100, 30000, 7200, 5);
      rx.setSeekAmLimits(100, 30000);   // Range for seeking.
      rx.setSeekAmSpacing(1); // spacing 1kHz
      Serial.println("\nALL - LW/MW/SW");
      break;
    case 'U':
    case 'u':
      rx.frequencyUp();
      break;
    case 'D':
    case 'd':
      rx.frequencyDown();
      break;
    case 'b':
    case 'B':
      if (rx.isCurrentTuneFM())
      {
        Serial.println("Not valid for FM");
      }
      else
      {
        if (bandwidthIdx > 6)
          bandwidthIdx = 0;
        rx.setBandwidth(bandwidthIdx, 1);
        printf("Filter - Bandwidth: %s Khz\n", bandwitdth[bandwidthIdx]);
        //Serial.print("Filter - Bandwidth: ");
        //Serial.print(String(bandwitdth[bandwidthIdx]));
        //Serial.println(" kHz");
        bandwidthIdx++;
      }
      break;
    case 'S':
      rx.seekStationProgress(showFrequency,1);
      // rx.seekStationUp();
      break;
    case 's':
      rx.seekStationProgress(showFrequency,0);
      // rx.seekStationDown();
      break;
    case '0':
      showStatus();
      break;
    case '4':
      rx.setFrequencyStep(1);
      Serial.println("\nStep 1");
      break;  
    case '5':
      rx.setFrequencyStep(5);
      Serial.println("\nStep 5");
      break;    
    case '6':
      rx.setFrequencyStep(10);
      Serial.println("\nStep 10");
      break;
    case '7':
      rx.setFrequencyStep(100);
      Serial.println("\nStep 100");      
      break;
    case '8':
      rx.setFrequencyStep(1000);
      Serial.println("\nStep 1000");    
      break;
    case '?':
      showHelp();
      break;
    default:
      break;
    }
  }
  delay(100);
  currentFrequency = rx.getCurrentFrequency();
  if (currentFrequency != previousFrequency)
  {
    previousFrequency = currentFrequency;
    showStatus();
    delay(300);
  }
}

int main() {
  setup();
  loop();
}
