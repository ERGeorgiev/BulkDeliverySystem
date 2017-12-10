#include <arduino.h>
#include "keypad.h"

byte button_get()
{
  static short lastPressed = 0;
  int sensorValue = analogRead(pin_keypad);
  int difference_topBtn = BUTTONS;

  if (sensorValue > (btn_values[BUTTONS - 1] - 30))
  {
    int difference_top = 30;
    int difference = 0;

    for (int i = 1; i < BUTTONS; i++)
    {
      difference = abs(btn_values[i] - sensorValue);
      if (difference < difference_top)
      {
        difference_top = difference;
        difference_topBtn = i;
      }
    }

    if ( lastPressed == difference_topBtn )
    {
      difference_topBtn = 0;
    }
    else
    {
      lastPressed = difference_topBtn;
    }
  }
  else
  {
	lastPressed = 0;	  
	difference_topBtn = 0;
  }
  
  return difference_topBtn;
}
