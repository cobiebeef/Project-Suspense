
// Includes for DotStar Matrix Wing
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_DotStarMatrix.h>
#include <Adafruit_DotStar.h>
#include <Fonts/TomThumb.h>

// Includes for OLED Wing
#include <Adafruit_SH110X.h>

// Includes for Joy Wing
#include <Adafruit_seesaw.h>


// Definitions for DotStar Matrix Wing
#define DATAPIN    11
#define CLOCKPIN   13
#define SHIFTDELAY 200
#define BRIGHTNESS 1

// Definitions for Joy Wing 
#define BUTTON_A  9
#define BUTTON_B  8
#define BUTTON_C  7
#define BUTTON_RIGHT 6
#define BUTTON_DOWN  7
#define BUTTON_LEFT  9
#define BUTTON_UP    10
#define BUTTON_SEL   14
#define IRQ_PIN   10


uint32_t button_mask = (1 << BUTTON_RIGHT) | (1 << BUTTON_DOWN) | 
                (1 << BUTTON_LEFT) | (1 << BUTTON_UP) | (1 << BUTTON_SEL);

Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire); //create new display

Adafruit_seesaw ss(&Wire); //create input device

Adafruit_DotStarMatrix matrix = Adafruit_DotStarMatrix(
                                  12, 6, DATAPIN, CLOCKPIN,
                                  DS_MATRIX_BOTTOM     + DS_MATRIX_LEFT +
                                  DS_MATRIX_ROWS + DS_MATRIX_PROGRESSIVE,
                                  DOTSTAR_BGR);

enum Options {
  TOP, MID, BOT
} selection;

enum Menus {
  MAIN_MENU, PROGRAMS, CONTROLLER, SETTINGS,
  DEMO, SCHEDULED, RANDOM
} currentMenu;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // what am I using this pin for?
  pinMode(25, INPUT_PULLUP);
 
  // uncomment to have wait
  // while (!Serial) delay(500); 

  // Setup Dotstar Matrix
  initDotstarMatrix();

  // Setup OLED
  initOLED();

  // Setup Joy Wing
  initJoyWing();
}

int x = matrix.width();

int last_x = 0, last_y = 0;


void loop() {

  bool aPressed = false,
       bPressed = false,
       yPressed = false,
       xPressed = false,
       selPressed = false;

  int vertical = ss.analogRead(2);

  if(!digitalRead(IRQ_PIN)) {

    uint32_t buttons = ss.digitalReadBulk(button_mask);

    if (! (buttons & (1 << BUTTON_RIGHT))) {
      aPressed = true;
    }
    if (! (buttons & (1 << BUTTON_DOWN))) {
      bPressed = true;
    }
    if (! (buttons & (1 << BUTTON_LEFT))) {
      yPressed = true;
    }
    if (!(buttons & (1 << BUTTON_UP))) {
      xPressed = true;
    }
    if (!(buttons & (1 << BUTTON_SEL))) {
      selPressed = true;
    }
  }

  switch (selection) { // I want to move the delays out so the screen updates instantly
    case TOP:
      if (vertical > 600) {
        selection = MID;
      }
      break;
    case MID:
      if (vertical > 600) {
        selection = BOT;
      }
      if (vertical < 400) {
        selection = TOP;
      }
      break;
    case BOT:
      if (vertical < 400) {
        selection = MID;
      }
      break;
  }

  // if (!digitalRead(BUTTON_A)) {
  //   selection = TOP;
  // }
  // if (!digitalRead(BUTTON_B)) {
  //   selection = MID;
  // }
  // if (!digitalRead(BUTTON_C)) {
  //   selection = BOT;
  // }

  yield();

  switch(currentMenu) {
    case MAIN_MENU:
      mainMenu(selection, aPressed);
      break;
    case PROGRAMS:
      programsMenu(selection, aPressed);
      if(bPressed) currentMenu = MAIN_MENU;
      break;
    case CONTROLLER:
      controlMode(selection, aPressed);
      if(bPressed) currentMenu = MAIN_MENU;
      break;
    case SETTINGS:
      if(bPressed) currentMenu = MAIN_MENU;
      break;
    case DEMO:
      if(bPressed) currentMenu = PROGRAMS;
      break;
    case SCHEDULED:
      if(bPressed) currentMenu = PROGRAMS;
      break;
    case RANDOM:
      if(bPressed) currentMenu = PROGRAMS;
      break;
  }
  
  char text[] = "Version One!";

  matrixSign(text);

  // int xx = ss.analogRead(2);
  // int y = ss.analogRead(3);
  
  // if ( (abs(xx - last_x) > 3)  ||  (abs(y - last_y) > 3)) {
  //   Serial.print(xx); Serial.print(", "); Serial.println(y);
  //   last_x = xx;
  //   last_y = y;
  // }
}

void initDotstarMatrix(){
  Serial.println("\nDotstar Matrix Wing");
  matrix.begin();
  matrix.clear();
  matrix.show();
  matrix.setFont(&TomThumb);
  matrix.setTextWrap(true);
  matrix.setBrightness(BRIGHTNESS);
}

void initOLED(){
  Serial.println("128x64 OLED FeatherWing");
  delay(250); // wait for the OLED to power up
  display.begin(0x3C, true); // Address 0x3C default

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();
  display.setRotation(1);

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
}

void initJoyWing(){
  Serial.println("Joy FeatherWing");
  if(!ss.begin(0x49)) {
    Serial.println("ERROR! seesaw not found");
    while(1) delay(1);
  } else {
    Serial.println("seesaw started");
    Serial.print("version: ");
    Serial.println(ss.getVersion(), HEX);
  }
  ss.pinModeBulk(button_mask, INPUT_PULLUP);
  ss.setGPIOInterrupts(button_mask, 1);

  pinMode(IRQ_PIN, INPUT);
}

void matrixSign(char *text) {
  matrix.fillScreen(0);
  matrix.setCursor(x, 5);
  matrix.setTextColor(matrix.Color(255,255,255));
  for (byte i = 0; i < strlen(text); i++) matrix.print(text[i]);
  if (--x < -50) {
    x = matrix.width();
  }
  matrix.show();
  delay(SHIFTDELAY);
}

void mainMenu(Options selection, bool select){
  displayMenu("Main Menu", "Programs", "Control Mode", "Settings");

  switch(selection) {
    case TOP:
      display.setCursor(0, 9);
      display.print(">");
      if(select) currentMenu = PROGRAMS;
      break;
    case MID:
      display.setCursor(0, 28);
      display.print(">");
      if(select) currentMenu = CONTROLLER;
      break;
    case BOT:
      if(select) currentMenu = SETTINGS;
      display.setCursor(0, 47);
      display.print(">");
      break;
  }

  display.display();
}
   
void programsMenu(Options selection, bool select) {
  displayMenu("Programs", "Demo", "Random", "Scheduled");
  
  switch(selection) {
    case TOP:
      display.setCursor(0, 9);
      display.print(">");
      if(select) currentMenu = DEMO;
      break;
    case MID:
      display.setCursor(0, 28);
      display.print(">");
      if(select) currentMenu = SCHEDULED;
      break;
    case BOT:
      if(select) currentMenu = RANDOM;
      display.setCursor(0, 47);
      display.print(">");
      break;
  }
  
  display.display();
}

void controlMode(Options selection, bool select) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(24, 1);
  display.println("Control Mode");
  // display.print("  Demo");
  // display.setCursor(0, 28);
  // display.print("  Random");
  // display.setCursor(0, 47);
  // display.print("  Scheduled");
  
  // switch(selection) {
  //   case TOP:
  //     display.setCursor(0, 9);
  //     display.print(">");
  //     // if(select) currentMenu = PROGRAMS;
  //     break;
  //   case MID:
  //     display.setCursor(0, 28);
  //     display.print(">");
  //     // if(select) currentMenu = CONTROLLER;
  //     break;
  //   case BOT:
  //     // if(select) currentMenu = SETTINGS;
  //     display.setCursor(0, 47);
  //     display.print(">");
  //     break;
  // }
  
  display.display();
}

void displayMenu(const char *menuName, const char *top, const char *mid, const char *bot) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(36, 1);
  display.println(menuName);
  display.setCursor(12, 9);
  display.print(top);
  display.setCursor(12, 28);
  display.print(mid);
  display.setCursor(12, 47);
  display.print(bot);
}

// bool menuCursor() {
//   switch (selection) { // I want to move the delays out so the screen updates instantly
//     case TOP:
//       if (vertical > 600) {
//         selection = MID;
//         return true;
//       }
//       break;
//     case MID:
//       if (vertical > 600) {
//         selection = BOT;
//         return true;
//       }
//       if (vertical < 400) {
//         selection = TOP;
//       }
//       break;
//     case BOT:
//       if (vertical < 400) {
//         selection = MID;
//         return true;
//       }
//       break;
//       return false;
//   }
// }