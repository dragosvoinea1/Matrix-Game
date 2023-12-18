#include <LedControl.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>

const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
const byte dinPin = 12;           // DIN pin connected to MAX7219
const byte clockPin = 11;         // CLK pin connected to MAX7219
const byte loadPin = 10;          // LOAD pin connected to MAX7219
const byte matrixSize = 8;        // Size of LED matrix
const byte backlightPin = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); // MAX7219 initialization
byte matrixBrightness = 2;        // LED matrix brightness inital
int storedBrightnessLevel;

// Joystick configuration and movement variables for LCD
const int pinSW = 2;
const int pinX = A0;
const int pinY = A1;
byte swState = LOW;
byte lastSwState = LOW;
unsigned long blinkDelay = 500;
unsigned long lastUpdateTime = 0;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 40;
unsigned long debounceDelayButton = 10;
unsigned long lastButtonPressTime = 0;
int centerXValue = 512;
int incrementThreshold = 50;
int incrementState = 0;

enum State {
  STARTING,
  MAIN_MENU,
  SETTINGS_MENU,
  BRIGHTNESS_SUBMENU,
  DIFFICULTY_SUBMENU,
  ABOUT,
  RULES,
  GAME  
};

unsigned long startTime;
State currentState = STARTING;

// Main menu elements
String menuElements[] = {"START", "RULES", "HIGHSCORE", "SETTINGS", "ABOUT"};
int currentElement = 0;

// Settings menu elements
String settingsOptions[] = {"DIFFICULTY", "BRIGHTNESS", "RESET HIGHSCORE", "EXIT"};
int currentSettingElement = 0;

// Submenu variables
int brightnessLevel = 5;
int difficultyLevel = 1;
int global_difficulty = 0;
int incrementThresholdDifficulty = 50;



// ABOUT & RULES message
String aboutMessage = "Game name: Snake;   Game creator: Voinea Dragos;    Github: https://github.com/dragosvoinea1";
String rulesMessage = "Diagonal movement and jumping from one wall to another";
int scrollPosition = 0;
unsigned long lastScrollTime = 0;
const unsigned long scrollInterval = 200; // Adjust this value for the scrolling speed
const int screen_text = 16;


// Player and joystick position variables
int playerRow = 0;
int playerCol = 0;
bool playerBlinkState = HIGH;     // Initial state for player LED blinking


// Joystick movement and LED matrix update variables
int xValue = 0;
int yValue = 0;
bool joyMoved = false;
int minThreshold = 400;
int maxThreshold = 600;
const byte moveInterval = 200;    // Timing variable to control the speed of LED movement
unsigned long long lastMoved = 0; // Tracks the last time the LED moved
bool matrixChanged = true;        // Flag to track if the matrix display needs updating

unsigned long joystickHoldStartTime = 0;
const unsigned long joystickHoldDuration = 3000;  
bool joystickHeld = false;


// Player and random LED positions
int row_pos = 0;
int col_pos = 0;
int start_pos = 0;
int row_LastPos = 0;
int col_LastPos = 0;

int row_rand[10] = {0};
int col_rand[10] = {0};

const int ON = 1;
const int OFF = 0;

const int total_random = 10;
int remainingLEDs = 10;

bool gameRunning = false;
unsigned long gameStartTime = 0;


// LED matrix representation
byte matrix[matrixSize][matrixSize] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};


void setup() {
  lc.shutdown(0, false);          // Turn off power saving, enables display
  //lc.setIntensity(0, matrixBrightness); // Sets brightness (0~15 possible values)
  lc.clearDisplay(0);              // Clear the LED matrix display
  Serial.begin(9600);
  displaySmileyFace();
  lcd.begin(16, 2);
  lcd.print("STARTING GAME..");
  pinMode(pinSW, INPUT_PULLUP);
  pinMode(backlightPin, OUTPUT);
  analogWrite(backlightPin, 255);

  randomSeed(analogRead(2));
  for (int i = 0; i < total_random; i++) {
    do {
      // Generate random positions for 10 LEDs and saving them in 2 vectors (need these later)
      row_rand[i] = random(0, matrixSize);
      col_rand[i] = random(0, matrixSize); 
    } while (matrix[row_rand[i]][col_rand[i]] == ON); // Check if the position is already occupied
    matrix[row_rand[i]][col_rand[i]] = ON; // Mark the generated position as occupied
  }
  matrix[row_pos][col_pos] = ON; // Initialize the starting position of the LED

  storedBrightnessLevel = EEPROM.read(0);

  lc.setIntensity(0, storedBrightnessLevel);

  startTime = millis();
}

void loop() {
  swState = digitalRead(pinSW);
  Serial.println(remainingLEDs);

  // State machine
  switch (currentState) {
    case STARTING:
      // Display "MAIN MENU" after 2 seconds
      if (millis() - startTime >= 2000) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MAIN MENU");
        lcd.setCursor(0, 1);
        lcd.print(menuElements[currentElement]);
        currentState = MAIN_MENU;
      }
      break;

    case MAIN_MENU:
      // Handle the main menu logic
      handleMainMenu();
      break;

    case SETTINGS_MENU:
      // Handle the settings menu logic
      handleSettingsMenu();
      break;

    case BRIGHTNESS_SUBMENU:
      // Handle the brightness submenu logic
      handleBrightnessSubmenu();
      break;

    case DIFFICULTY_SUBMENU:
      // Handle the difficulty submenu logic
      handleDifficultySubmenu();
      break;

    case ABOUT:
      // Handle the ABOUT state logic
      handleAbout();
      break;

    case RULES:
      handleRules();
      break;

    case GAME:
      handleGame();
      break;

  }
  if (swState == LOW && swState != lastSwState) {
    matrix[row_pos][col_pos] = OFF;
    matrixChanged = true;
    joystickHoldStartTime = millis();
  }

  lastSwState = swState;

  if(swState == LOW && lastSwState == LOW && (millis() - joystickHoldStartTime > joystickHoldDuration)){
    // Game over, return to the main menu
    resetGame();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MAIN MENU");
    lcd.setCursor(0, 1);
    lcd.print(menuElements[currentElement]);
    currentState = MAIN_MENU;
    gameRunning = false;
  }

  if (remainingLEDs == 0) {
    // Game over, return to the main menu
    resetGame();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MAIN MENU");
    lcd.setCursor(0, 1);
    lcd.print(menuElements[currentElement]);
    currentState = MAIN_MENU;
    gameRunning = false;
  }
  if (currentState == GAME) {
    static int previousLEDs = -1;  

    if (remainingLEDs != previousLEDs) { //updating the LCD only if the 'remainingLEDs' value has changed
      lcd.setCursor(0, 1);
      lcd.print("          ");  
      lcd.setCursor(0, 1);
      lcd.print(remainingLEDs);

      previousLEDs = remainingLEDs;
    }
  }

}


void handleGame() {
  swState = digitalRead(pinSW);
  // Check for switch press and update matrix accordingly
  if (swState == LOW && swState != lastSwState) {
    if(matrix[row_pos][col_pos] == ON)
      remainingLEDs = remainingLEDs - 1; 
    matrix[row_pos][col_pos] = OFF;
    matrixChanged = true;
    joystickHoldStartTime = millis();
  }

  lastSwState = swState;

  // Debounce switch input and update player LED blinking state
  if (millis() - lastDebounceTime > blinkDelay) {
    lastDebounceTime = millis();
    playerBlinkState = !playerBlinkState;
    lc.setLed(0, row_pos, col_pos, playerBlinkState);
  }

  // Check if it's time to move the LED and update positions
  if (millis() - lastMoved > moveInterval) {
    updatePositions();
    lastMoved = millis();
  }

  // Check if the matrix display needs updating
  if (matrixChanged == true) {
    updateMatrix();
    matrixChanged = false;
  }
}

// Function to handle the main menu logic
void handleMainMenu() {
  int xValue = analogRead(pinX);
  swState = digitalRead(pinSW);

  // Check for joystick movement and debounce button press
  if ((millis() - lastDebounceTime) > debounceDelay) {
    int xDifference = xValue - centerXValue;

    // Increment to the next menu element
    if (xDifference > incrementThreshold) {
      if (incrementState == 0) {
        incrementState = 1;
        if (currentElement < sizeof(menuElements) / sizeof(menuElements[0]) - 1) {
          currentElement++;
          updateLCD();
        }
      }
    }
    // Decrement to the previous menu element
    else if (xDifference < -incrementThreshold) {
      if (incrementState == 0) {
        incrementState = -1;
        if (currentElement > 0) {
          currentElement--;
          updateLCD();
        }
      }
    } else {
      incrementState = 0;
    }

    lastDebounceTime = millis();
  }

  // Check for button press and navigate to ABOUT or SETTINGS
    if (swState == LOW && lastSwState == HIGH && (millis() - lastButtonPressTime) > debounceDelayButton) {
      if (menuElements[currentElement] == "ABOUT") {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ABOUT");
        currentState = ABOUT;
        lastButtonPressTime = millis();
      } else
      if (menuElements[currentElement] == "RULES") {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("RULES");
        currentState = RULES;
        lastButtonPressTime = millis();
      }else if (menuElements[currentElement] == "SETTINGS") {
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("SETTINGS MENU");
              lcd.setCursor(0, 1);
              lcd.print(settingsOptions[currentSettingElement]);
              currentState = SETTINGS_MENU;
              lastButtonPressTime = millis();
      }
      else if (swState == LOW && lastSwState == HIGH && (millis() - lastButtonPressTime) > debounceDelayButton) {
      if (menuElements[currentElement] == "START") {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Remaining:");
        currentState = GAME;
        gameRunning = true;
        gameStartTime = millis();
      }
    }
  }
}


// Function to handle the ABOUT state logic
void handleAbout() {
  if (millis() - lastScrollTime >= scrollInterval) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ABOUT");

    String displayText = aboutMessage.substring(scrollPosition, scrollPosition + screen_text);
    lcd.setCursor(0, 1);
    lcd.print(displayText);

    scrollPosition = (scrollPosition + 1) % (aboutMessage.length() + screen_text);
    
    lastScrollTime = millis();
  }

  // Check for button press to return to MAIN MENU
  swState = digitalRead(pinSW);
  if (swState == LOW && lastSwState == HIGH && (millis() - lastButtonPressTime) > debounceDelayButton) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MAIN MENU");
    lcd.setCursor(0, 1);
    lcd.print(menuElements[currentElement]);
    currentState = MAIN_MENU;
    lastButtonPressTime = millis();
  }

  lastSwState = swState;
}

// Function to handle the RULES state logic
void handleRules() {
  if (millis() - lastScrollTime >= scrollInterval) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Rules");

    String displayText = rulesMessage.substring(scrollPosition, scrollPosition + screen_text);
    lcd.setCursor(0, 1);
    lcd.print(displayText);

    scrollPosition = (scrollPosition + 1) % (rulesMessage.length() + screen_text);
    
    lastScrollTime = millis();
  }

  // Check for button press to return to MAIN MENU
  swState = digitalRead(pinSW);
  if (swState == LOW && lastSwState == HIGH && (millis() - lastButtonPressTime) > debounceDelayButton) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("MAIN MENU");
    lcd.setCursor(0, 1);
    lcd.print(menuElements[currentElement]);
    currentState = MAIN_MENU;
    lastButtonPressTime = millis();
  }

  lastSwState = swState;
}

// Function to handle the settings menu logic
void handleSettingsMenu() {
  int xValue = analogRead(pinX);
  swState = digitalRead(pinSW);

  // Check for joystick movement and debounce button press
  if ((millis() - lastDebounceTime) > debounceDelay) {
    int xDifference = xValue - centerXValue;

    // Increment to the next setting element
    if (xDifference > incrementThreshold) {
      if (incrementState == 0) {
        incrementState = 1;
        if (currentSettingElement < sizeof(settingsOptions) / sizeof(settingsOptions[0]) - 1) {
          currentSettingElement++;
          updateLCD();
        }
      }
    }
    // Decrement to the previous setting element
    else if (xDifference < -incrementThreshold) {
      if (incrementState == 0) {
        incrementState = -1;
        if (currentSettingElement > 0) {
          currentSettingElement--;
          updateLCD();
        }
      }
    } else {
      incrementState = 0;
    }

    lastDebounceTime = millis();
  }

  // Check for button press and navigate to MAIN MENU, BRIGHTNESS, or DIFFICULTY
  if (swState == LOW && swState != lastSwState && (millis() - lastButtonPressTime) > debounceDelayButton) {
    if (settingsOptions[currentSettingElement] == "EXIT") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MAIN MENU");
      lcd.setCursor(0, 1);
      lcd.print(menuElements[currentElement]);
      currentState = MAIN_MENU;
    } else if (settingsOptions[currentSettingElement] == "BRIGHTNESS") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("BRIGHTNESS LEVEL");
      lcd.setCursor(0, 1);
      lcd.print("Level: " + String(brightnessLevel));
      currentState = BRIGHTNESS_SUBMENU;
    } else if (settingsOptions[currentSettingElement] == "DIFFICULTY") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("DIFFICULTY LEVEL");
      lcd.setCursor(0, 1);
      lcd.print("Level: " + String(difficultyLevel));
      currentState = DIFFICULTY_SUBMENU;
    }
    lastButtonPressTime = millis();
  }
}

// Function to handle the brightness submenu logic
void handleBrightnessSubmenu() {
  int xValue = analogRead(pinX);
  swState = digitalRead(pinSW);

  // Check for joystick movement and debounce button press
  if ((millis() - lastDebounceTime) > debounceDelay) {
    int xDifference = xValue - centerXValue;

    // Increment or decrement brightness level
    if (xDifference > incrementThreshold) {
      if (incrementState == 0) {
        incrementState = 1;
        brightnessLevel = min(brightnessLevel + 1, 5); // Max 5 levels
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("BRIGHTNESS LEVEL");
        lcd.setCursor(0, 1);
        lcd.print("Level: " + String(brightnessLevel));
      }
    } else if (xDifference < -incrementThreshold) {
      if (incrementState == 0) {
        incrementState = 1;
        brightnessLevel = max(brightnessLevel - 1, 1); // Don't go below 1
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("BRIGHTNESS LEVEL");
        lcd.setCursor(0, 1);
        lcd.print("Level: " + String(brightnessLevel));
      }
    } else {
      incrementState = 0;
    }

    lastDebounceTime = millis();
  }

  // Update LED brightness and save to EEPROM if changed
  if (millis() - lastUpdateTime >= 100) {
    int intensity = brightnessLevel;
    lc.setIntensity(0, intensity);

    // Save brightness level to EEPROM only if it has changed
    if (storedBrightnessLevel != brightnessLevel) {
      EEPROM.update(0, brightnessLevel); // Assuming 0 is the EEPROM address to store brightness
      storedBrightnessLevel = brightnessLevel; // Update stored value
    }

    lastUpdateTime = millis();
  }

  // Check for button press to return to SETTINGS MENU
  if (swState == LOW && swState != lastSwState && (millis() - lastButtonPressTime) > debounceDelayButton) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SETTINGS MENU");
    lcd.setCursor(0, 1);
    lcd.print(settingsOptions[currentSettingElement]);
    currentState = SETTINGS_MENU;
    lastButtonPressTime = millis();
  }
}

// Function to handle the difficulty submenu logic
void handleDifficultySubmenu() {
  int xValue = analogRead(pinX);
  swState = digitalRead(pinSW);

  // Check for joystick movement and debounce button press
  if ((millis() - lastDebounceTime) > debounceDelay) {
    int xDifference = xValue - centerXValue;

    // Increment or decrement difficulty level
    if (xDifference > incrementThreshold) {
      if (incrementState == 0) {
        incrementState = 1;
        difficultyLevel = min(difficultyLevel + 1, 2); //Max 2 levels
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("DIFFICULTY LEVEL");
        lcd.setCursor(0, 1);
        lcd.print("Level: " + String(difficultyLevel));
      }
    } else if (xDifference < -incrementThreshold) {
      if (incrementState == 0) {
        incrementState = 1;
        difficultyLevel = max(difficultyLevel - 1, 1); //Don't go below 1
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("DIFFICULTY LEVEL");
        lcd.setCursor(0, 1);
        lcd.print("Level: " + String(difficultyLevel));
      }
    } else {
      incrementState = 0;
    }

    lastDebounceTime = millis();
  }

  // Check for button press to return to SETTINGS MENU
  if (swState == LOW && swState != lastSwState && (millis() - lastButtonPressTime) > debounceDelayButton) {
    global_difficulty = difficultyLevel;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SETTINGS MENU");
    lcd.setCursor(0, 1);
    lcd.print(settingsOptions[currentSettingElement]);
    currentState = SETTINGS_MENU;
    lastButtonPressTime = millis();
  }
}

// Function to update LCD display
void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (currentState == SETTINGS_MENU) {
    lcd.print("SETTINGS MENU");
  } else {
    lcd.print("MAIN MENU");
  }
  lcd.setCursor(0, 1);
  if (currentState == SETTINGS_MENU) {
    lcd.print(settingsOptions[currentSettingElement]);
  } else {
    lcd.print(menuElements[currentElement]);
  }
}


// Update LED matrix display
void updateMatrix() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, matrix[row][col]);  // Set each LED individually
    }
  }
}

// Update player and random LED positions based on joystick input
void updatePositions() {
  // Read joystick values
  int xValue = analogRead(pinX);
  int yValue = analogRead(pinY);

  // Store the last positions of the LED
  row_LastPos = row_pos;
  col_LastPos = col_pos;

  static int lastXValue = xValue;
  static int lastYValue = yValue;
  static unsigned long lastUpdateTime = millis();
  const int sensibilityThreshold = 50;
  if (millis() - lastUpdateTime >= sensibilityThreshold) {
    // Update row_pos based on joystick movement (X-axis)
    if (yValue < minThreshold) {
      if (row_pos > 0) {
        row_pos--;
      } 
      else {
        row_pos = matrixSize - 1;
      }
    }
    if (yValue > maxThreshold) {
      if (row_pos < matrixSize - 1) {
        row_pos++;
      } 
      else {
        row_pos = 0;
      }
    }

    // Update col_pos based on joystick movement (Y-axis)
    if (xValue > maxThreshold) {
      if (col_pos < matrixSize - 1) {
        col_pos++;
      } 
      else {
        col_pos = 0;
      }
    }
    if (xValue < minThreshold) {
      if (col_pos > 0) {
        col_pos--;
      }
      else {
        col_pos = matrixSize - 1;
      }
    }

    lastXValue = xValue;
    lastYValue = yValue;
    lastUpdateTime = millis();
  }


  // Check if the current position is a random LED
  bool isRandomLED = false;
  for (int i = 0; i < total_random; i++) {
    int randomRow = row_rand[i];
    int randomCol = col_rand[i];
    if (row_pos == randomRow && col_pos == randomCol && matrix[row_pos][col_pos] == ON) {
      isRandomLED = true;
      break;
    }
  }

  // Check if the last position was a random LED
  bool last_isRandomLED = false;
  for (int i = 0; i < total_random; i++) {
    int randomRow = row_rand[i];
    int randomCol = col_rand[i];
    if (row_LastPos == randomRow && col_LastPos == randomCol && matrix[row_LastPos][col_LastPos] == ON) {
      last_isRandomLED = true;
      break;
    }
  }

  // If the position has changed, update the matrix
  if (row_pos != row_LastPos || col_pos != col_LastPos) {
    matrixChanged = true;
    //These conditions are used to not light up / turn off unwanted LEDS
    // If the actual position is on a "random" LED and the previous one is a normal LED
    if (isRandomLED == true && last_isRandomLED == false) {
      matrix[row_LastPos][col_LastPos] = OFF;
      matrix[row_pos][col_pos] = ON;
    }
    // If the actual position is on a normal LED and the previous one is a "random" LED
    if (isRandomLED == false && last_isRandomLED == true) {
      matrix[row_LastPos][col_LastPos] = ON;
      matrix[row_pos][col_pos] = OFF;
    }
    // If 2 random leds are lit up next to eachother
    if (isRandomLED == true && last_isRandomLED == true) {
      matrix[row_LastPos][col_LastPos] = ON;
      matrix[row_pos][col_pos] = ON;
    }
  }
}


void displaySmileyFace() {
  lc.clearDisplay(0);

  lc.setLed(0, 2, 0, 1);
  lc.setLed(0, 3, 0, 1);
  lc.setLed(0, 4, 0, 1);
  lc.setLed(0, 5, 0, 1);
  lc.setLed(0, 6, 1, 1);
  lc.setLed(0, 7, 2, 1);
  lc.setLed(0, 7, 3, 1);
  lc.setLed(0, 7, 4, 1);
  lc.setLed(0, 7, 5, 1);
  lc.setLed(0, 6, 6, 1);
  lc.setLed(0, 2, 7, 1);
  lc.setLed(0, 3, 7, 1);
  lc.setLed(0, 4, 7, 1);
  lc.setLed(0, 5, 7, 1);
  lc.setLed(0, 1, 6, 1);
  lc.setLed(0, 0, 2, 1);
  lc.setLed(0, 0, 3, 1);
  lc.setLed(0, 0, 4, 1);
  lc.setLed(0, 0, 5, 1);
  lc.setLed(0, 1, 1, 1);
  lc.setLed(0, 2, 2, 1);
  lc.setLed(0, 2, 5, 1);
  lc.setLed(0, 4, 2, 1);
  lc.setLed(0, 4, 5, 1);
  lc.setLed(0, 5, 3, 1);
  lc.setLed(0, 5, 4, 1);
}

void resetGame() {
  // Reset game-related variables and matrix
  gameRunning = false;
  gameStartTime = 0;
  remainingLEDs = total_random;
  row_pos = 0;
  col_pos = 0;
  for (int i = 0; i < matrixSize; i++) {
    for (int j = 0; j < matrixSize; j++) {
      matrix[i][j] = OFF;
    }
  }

  for (int i = 0; i < total_random; i++) {
    do {
      row_rand[i] = random(0, matrixSize);
      col_rand[i] = random(0, matrixSize);
    } while (matrix[row_rand[i]][col_rand[i]] == ON);

    matrix[row_rand[i]][col_rand[i]] = ON;
  }
  lc.clearDisplay(0);
  displaySmileyFace();
}
