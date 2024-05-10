/**
Written by Drew King using code from the Makeability lab for drawing shapes and handling collisions
https://github.com/makeabilitylab/arduino/blob/master/MakeabilityLab_Arduino_Library/src/Shape.hpp
**/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Shape.hpp>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins for controller
const int SOUND_PIN = 9;
const int BUTTON_PIN_ONE = 5;
const int BUTTON_PIN_TWO = 6;
const int BUTTON_PIN_THREE = 7;
const int LED_PIN_R = 10;
const int LED_PIN_Y = 11;
const int LED_PIN_G = 12;
const int VIBRO_PIN = 13;

// Game mode.
int MODE = 0;

// Bouncy ball global variables.
int BALL_RADIUS = 5;
int CURR_X = display.width()/2;
int CURR_Y = display.height()/2;
bool moveLeft;
bool moveUp;

// Main game global variables.
const int RADIUS = 10;
const int BALL_R = RADIUS - 2;
const int X1 = SCREEN_WIDTH / 4;
const int X2 = SCREEN_WIDTH / 2;
const int X3 = SCREEN_WIDTH / 4 * 3;
const int FLOOR_Y = SCREEN_HEIGHT - RADIUS - 2;
int HEALTH = 3;
bool invincible = false;

int initialBallPositions[] = {-30, -60, -90};
int ballMappings[] = {X1, X3, X2};
const int numBalls = 3;
Ball *balls[numBalls];

// Music notes.
Ball note1 = Ball(X1, -30, BALL_R);
Ball note2 = Ball(X2, -90, BALL_R);
Ball note3 = Ball(X3, -60, BALL_R);

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  // display.display();
  // delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  
  // Prepare the main game
  for(int i = 0; i < numBalls; i++){
    int mapping = ballMappings[i];
    int position = initialBallPositions[i];
    balls[i] = &Ball(mapping, position, BALL_R);
    balls[i]->setSpeed(0, 1);
  }

  // Declare the pinmodes
  pinMode(SOUND_PIN, OUTPUT);
  pinMode(BUTTON_PIN_ONE, INPUT_PULLUP);
  pinMode(BUTTON_PIN_TWO, INPUT_PULLUP);
  pinMode(BUTTON_PIN_THREE, INPUT_PULLUP);
  pinMode(LED_PIN_R, OUTPUT);
  pinMode(LED_PIN_Y, OUTPUT);
  pinMode(LED_PIN_G, OUTPUT);

  // Default for bouncy ball
  moveLeft = true;
  moveUp = false;

  // Default for mainGame
  note1.setSpeed(0, 1);
  note2.setSpeed(0, 1);
  note3.setSpeed(0, 1);
}

void loop() {
  int buttonR = digitalRead(BUTTON_PIN_ONE);
  int buttonC = digitalRead(BUTTON_PIN_TWO);
  int buttonL = digitalRead(BUTTON_PIN_THREE);
  Serial.print("Right: ");
  Serial.print(buttonR);
  Serial.print(", Center: ");
  Serial.print(buttonC);
  Serial.print(", Left: ");
  Serial.print(buttonL);
  Serial.println();

  if(MODE == 0) {
    selectGame(buttonR, buttonC, buttonL);
  } else if(MODE == 1){
    bouncyBall(buttonR, buttonC, buttonL);
  } else if(MODE == 2) {
    mainGame(buttonR, buttonC, buttonL);
  } else {
    randomGame(buttonR, buttonC, buttonL);
  }
  
}

void selectGame(int button1, int button2, int button3) {
  display.clearDisplay();
  if(button1 == 0){
    MODE = 1;
  } else if(button2 == 0){
    MODE = 2;
  } else if(button3 == 0) {
    MODE = 3;
  }
  display.setTextSize(2);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);
  display.println("Pick game:");
  display.setTextSize(1);
  display.println("1) Bouncy Ball");
  display.println("2) Music Mash");
  display.println("3) Random Song");
  display.display();
}

void mainGame(int button1, int button2, int button3) {
  display.clearDisplay();
  for(int i = 0; i < numBalls; i++){
    balls[i]->update();
    balls[i]->draw(display);
    musicNoteCollisionCheck(button1, button2, button3, *balls[i], 440); // TODO Update with music.
  }

  display.display();

  // Return to title screen if health runs out.
  checkHealth();

  // note1.update();
  // note2.update();
  // note3.update();
  // note1.draw(display);
  // note2.draw(display);
  // note3.draw(display);
  // musicNoteCollisionCheck(button1, button2, button3, note1);
  // musicNoteCollisionCheck(button1, button2, button3, note2);
  // musicNoteCollisionCheck(button1, button2, button3, note3);
}

void musicNoteCollisionCheck(int button1, int button2, int button3, Ball &note, int buzzerSound) {
  if(button1 == 0){
    Circle goal1 = Circle(X1, FLOOR_Y, RADIUS, true);
    goal1.draw(display);
    if(goal1.overlaps(note)) {
      note.reverseYSpeed();
    }
  } else if (button1 == 1) {
    Circle goal1 = Circle(X1, FLOOR_Y, RADIUS, false);
    goal1.draw(display);
    if(note.contains(X1, SCREEN_HEIGHT + RADIUS)) {
      HEALTH--;
      note.reverseYSpeed();
    }
  }
  if(button2 == 0){
    Circle goal2 = Circle(X2, FLOOR_Y, RADIUS, true);
    goal2.draw(display);
    if(goal2.overlaps(note)) {
      note.reverseYSpeed();
    }
  } else if (button2 == 1) {
    Circle goal2 = Circle(X2, FLOOR_Y, RADIUS, false);
    goal2.draw(display);
    if(note.contains(X2, SCREEN_HEIGHT + RADIUS)) {
      HEALTH--;
      note.reverseYSpeed();
    }
  }
  if(button3 == 0){
    Circle goal3 = Circle(X3, FLOOR_Y, RADIUS, true);
    goal3.draw(display);
    if(goal3.overlaps(note)) {
      note.reverseYSpeed();
    }
  } else if (button3 == 1) {
    Circle goal3 = Circle(X3, FLOOR_Y, RADIUS, false);
    goal3.draw(display);
    if(note.contains(X3, SCREEN_HEIGHT + RADIUS)) {
      HEALTH--;
      note.reverseYSpeed();
    }
  }
}

void checkHealth(){
  if (HEALTH == 3){
    digitalWrite(LED_PIN_R, HIGH);
    digitalWrite(LED_PIN_Y, HIGH);
    digitalWrite(LED_PIN_G, HIGH);
  } else if (HEALTH == 2){
    digitalWrite(LED_PIN_R, HIGH);
    digitalWrite(LED_PIN_Y, HIGH);
    digitalWrite(LED_PIN_G, LOW);
  } else if (HEALTH == 1){
    digitalWrite(LED_PIN_R, HIGH);
    digitalWrite(LED_PIN_Y, LOW);
    digitalWrite(LED_PIN_G, LOW);
  } else {
    digitalWrite(LED_PIN_R, LOW);
    digitalWrite(LED_PIN_Y, LOW);
    digitalWrite(LED_PIN_G, LOW);
    MODE = 0;
    HEALTH = 3;
    delay(20);
  }
}

void randomGame(int button1, int button2, int button3) {
  
}

void bouncyBall(int button1, int button2, int button3) {
  if(button1 == 0 || button2 == 0 || button3 == 0){
    MODE = 0;
    delay(20);
  }
  display.clearDisplay();
  int vx = random() % 12;
  int vy = random() % 12;
  bounceX(vx);
  bounceY(vy);
  Circle circle = Circle(CURR_X, CURR_Y, BALL_RADIUS);
  circle.draw(display);
  display.display();
  //Serial.println(vx);
  delay(20);
}

// Fixed by ChatGPT.
void bounceX(int velocity) {
  if (CURR_X + velocity <= display.width() && moveLeft) {
    CURR_X = CURR_X + velocity;
  } else if (CURR_X + velocity >= BALL_RADIUS && !moveLeft) {
    velocity = -velocity; // Reverse the velocity
    CURR_X = CURR_X + velocity;
  } else {
    moveLeft = !moveLeft; // change the direction.
    int freq = abs(velocity * 440) / 32;
    tone(SOUND_PIN, freq, 100);
    //Serial.print(freq);
  }
}

void bounceY(int velocity) {
  if (CURR_Y + velocity <= display.height() && moveUp) {
    CURR_Y = CURR_Y + velocity;
  } else if (CURR_Y + velocity >= BALL_RADIUS && !moveUp) {
    velocity = -velocity; // Reverse the velocity
    CURR_Y = CURR_Y + velocity;
  } else {
    moveUp = !moveUp; // change the direction.
    int freq = abs(velocity * 440) / 12;
    tone(SOUND_PIN, freq, 100);
    //Serial.print(freq);
  }
}
