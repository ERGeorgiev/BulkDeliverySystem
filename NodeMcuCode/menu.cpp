#include <arduino.h>
#include "menu.h"

int stats[MENU_COUNT][MENU_ITEMS] = {{0}};
bool statActive = false;

byte menu_current = MAIN;
byte menu_selection = ITEMA;
