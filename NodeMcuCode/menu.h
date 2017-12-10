#ifndef menu_h
#define menu_h

#define MENU_COUNT 5
#define MENU_ITEMS 5

extern int stats[MENU_COUNT][MENU_ITEMS];
extern bool statActive;

extern byte menu_current;
extern byte menu_selection;

static const char* menu_text[MENU_COUNT][MENU_ITEMS]
{
  {
    "men\\:",
    "sedmi`en plan",
    "danni",
    "nastrojki",
    "wryzka"
  },
  {
    "sedmi`en plan",
    "den",
    "nomer na hrana",
    "koli`estwo",
    "`as"
  },
  {
    "danni",
    "hrana: ",
    "<kupa: ",
    ">kupa: ",
    ""
  },
  {
    "nastrojki",
    "data",
    "byrza hrana",
    "zanuli l-kupa",
    "zanuli d-kupa"
  },
  {
	"",
	"",
	"USER: ADMIN",
	"PASS: ADMIN"
  }
};

enum Item
{
  TITLE = 0,
  ITEMA,
  ITEMB,
  ITEMC,
  ITEMD
};

enum Menu
{
  MAIN = 0,
  PLAN,
  DANNI,
  NASTROIKI,
  VRUZKA
};

#endif