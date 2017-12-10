#ifndef keypad_h
#define keypad_h

#define pin_keypad A0
#define BUTTONS 9

byte button_get();

enum Buttons
{
  NOINPUT = 0,
  WHITETOP,
  BLUETOP,
  WHITEMID,
  BLUELEFT,
  GREENMID,
  BLUERIGHT,
  WHITEBOT,
  BLUEBOT
};

const unsigned short btn_values[BUTTONS] =
{
  0,    //NOINPUT
  1022, //WHITETOP
  854,  //BLUETOP
  676,  //WHITEMID
  631,  //BLUELEFT
  594,  //GREENMID
  561,  //BLUERIGHT
  501,  //WHITEBOT
  456   //BLUEBOT
};
// KEYPAD buttons analog out values

#endif