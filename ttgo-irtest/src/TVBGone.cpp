#include "TVBGone.h"

TVBGone::TVBGone(uint8_t irLed)
    : m_irSend(irLed)
{
  m_irSend.begin();
}

void TVBGone::setRegion(Region region)
{
  m_region = region;
}

// we cant read more than 8 bits at a time so dont try!
uint8_t TVBGone::read_bits(uint8_t count)
{
  uint8_t i;
  uint8_t tmp = 0;

  // we need to read back count bytes
  for (i = 0; i < count; i++)
  {
    // check if the 8-bit buffer we have has run out
    if (bitsleft_r == 0)
    {
      // in which case we read a new byte in
      bits_r = powerCode->codes[code_ptr++];
      Log::debugf("Get byte: %x", bits_r);
      // and reset the buffer size (8 bites in a byte)
      bitsleft_r = 8;
    }
    // remove one bit
    bitsleft_r--;
    // and shift it off of the end of 'bits_r'
    tmp |= (((bits_r >> (bitsleft_r)) & 1) << (count - 1 - i));
  }
  // return the selected bits in the LSB part of tmp
  return tmp;
}

void TVBGone::sendAllCodes()
{
  bool endingEarly = false; //will be set to true if the user presses the button during code-sending
  uint16_t rawData[300];
  if (m_region == NA)
  {
    num_codes = num_NAcodes;
  }
  else
  {
    num_codes = num_EUcodes;
  }

  // for every POWER code in our collection
  for (unsigned int i = 0; i < num_codes; i++)
  {

    // print out the code # we are about to transmit
    Log::debugf("Code #: %d", i);

    // point to next POWER code, from the right database
    if (m_region == NA)
    {
      powerCode = NApowerCodes[i];
    }
    else
    {
      powerCode = EUpowerCodes[i];
    }

    // Read the carrier frequency from the first byte of code structure
    const uint8_t freq = powerCode->timer_val;
    // set OCR for Timer1 to output this POWER code's carrier frequency

    // Print out the frequency of the carrier and the PWM settings
    Log::debugf("\n\rFrequency: %d", freq);

    uint16_t x = (freq + 1) * 2;
    Log::debugf("Freq: %d", (F_CPU / x));

    // Get the number of pairs, the second byte from the code struct
    const uint8_t numpairs = powerCode->numpairs;
    Log::debugf("On/off pairs: %d", numpairs);

    // Get the number of bits we use to index into the timer table
    // This is the third byte of the structure
    const uint8_t bitcompression = powerCode->bitcompression;
    Log::debugf("Compression: %d", bitcompression);

    // For EACH pair in this code....
    code_ptr = 0;
    // TODO: don't use member variable but pass index
    for (uint8_t k = 0; k < numpairs; k++)
    {
      uint16_t ti;

      // Read the next 'n' bits as indicated by the compression variable
      // The multiply by 4 because there are 2 timing numbers per pair
      // and each timing number is one word long, so 4 bytes total!
      ti = (read_bits(bitcompression)) * 2;

      // read the onTime and offTime from the program memory
      ontime = powerCode->times[ti];      // read word 1 - ontime
      offtime = powerCode->times[ti + 1]; // read word 2 - offtime

      Log::debugf("ti = %d", ti >> 1);
      Log::debugf("Pair = %d", ontime);
      Log::debugf("%d", offtime);

      rawData[k * 2] = ontime * 10;
      rawData[(k * 2) + 1] = offtime * 10;
      // TODO: find out what yield is used for here
      //yield();
      //delay(10);
    }

    // Send Code with library
    m_irSend.sendRaw(rawData, (numpairs * 2), freq);

    //Flush remaining bits, so that next code starts
    //with a fresh set of 8 bits.
    bitsleft_r = 0;

    // visible indication that a code has been output.
    quickflashLED();

    // delay 205 milliseconds before transmitting next POWER code
    delay(205);

    // if user is pushing (holding down) TRIGGER button, stop transmission early
    if (TTGOClass::getWatch()->touched())
    {
      while (TTGOClass::getWatch()->touched())
      {
        delay(10);
      }
      endingEarly = true;
      delay(500); //500ms delay
      quickflashLEDx(4);
      //pause for ~1.5 sec to give the user time to release the button so that the code sequence won't immediately start again.
      delay(1500);
      break; //exit the POWER code "for" loop
    }

  } //end of POWER code for loop

  if (endingEarly == false)
  {
    //pause for ~1.5 sec, then flash the visible LED 8 times to indicate that we're done
    delay(1500);
    quickflashLEDx(8);
  }

} //end of sendAllCodes

// This function quickly pulses the visible LED (connected to PB0, pin 5)
// This will indicate to the user that a code is being transmitted
void TVBGone::quickflashLED(void)
{
  //digitalWrite(LED, LOW);
  //lv_led_on(m_imuPoint);
  //lv_task_handler();
  delay(30); // 30 ms ON-time delay
  //digitalWrite(LED, HIGH);
  //lv_led_off(m_imuPoint);
  lv_task_handler();
}

// This function just flashes the visible LED a couple times, used to
// tell the user what region is selected
void TVBGone::quickflashLEDx(uint8_t x)
{
  quickflashLED();
  while (--x)
  {
    delay(250); // 250 ms OFF-time delay between flashes
    quickflashLED();
  }
}