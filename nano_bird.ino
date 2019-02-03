/**
   Nano Bird - a flappy bird clone for arduino nano, oled screen & push on switch

   @author  Richard Allsebrook <richardathome@gmail.com>
*/

// Include the libraries we need to communicate with the display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Initialise the display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// pin the flap button is attached to
#define FLAP_BUTTON  2

// Initialise 'sprites'
#define SPRITE_HEIGHT   16
#define SPRITE_WIDTH    16

// Two frames of animation
static const unsigned char PROGMEM bird1_bmp[] =
{ B00000000, B00000000,
  B00000000, B00000000,
  B00000011, B11000000,
  B00011111, B11110000,
  B00111111, B00111000,
  B01111111, B11111110,
  B11111111, B11000001,
  B11011111, B01111110,
  B11011111, B01111000,
  B11011111, B01111000,
  B11001110, B01111000,
  B11110001, B11110000,
  B01111111, B11100000,
  B00111111, B11000000,
  B00000111, B00000000,
  B00000000, B00000000,
};

static const unsigned char PROGMEM bird2_bmp[] =
{ B00000000, B00000000,
  B00000000, B00000000,
  B00000011, B11000000,
  B00011111, B11110000,
  B00111111, B00111000,
  B01110001, B11111110,
  B11101110, B11000001,
  B11011111, B01111110,
  B11011111, B01111000,
  B11111111, B11111000,
  B11111111, B11111000,
  B11111111, B11110000,
  B01111111, B11100000,
  B00111111, B11000000,
  B00000111, B00000000,
  B00000000, B00000000,
};

// Game variables
#define GAME_SPEED 50

int game_state = 1; // 0 = game over screen, 1 = in game
int score = 0; // current game score
int high_score = 0; // highest score since the nano was reset
int bird_x = (int)display.width() / 4; // birds x position (along) - initialised to 1/4 the way along the screen
int bird_y; // birds y position (down)
int vy = 0; // how much force is pulling the bird down
int wx[2]; // an array to hold the walls x positions
int wy[2]; // an array to hold the walls y positions
int wall_gap = 30; // size of the wall wall_gap in pixels
int wall_width = 10; // width of the wall in pixels

void setup() {

  // used for debugging output
  Serial.begin(9600);

  // initialise the display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.setTextColor(WHITE);
  display.clearDisplay();

  // Setup the flap button pin for input
  // We use INPUT_PULLUP so we don't need a resitor on the switch
  pinMode(FLAP_BUTTON, INPUT_PULLUP);

  // Initialise the random number generator
  randomSeed(analogRead(0));

}

void loop() {

  if (game_state == 0) {
    // in game
    display.clearDisplay();

    // If the flap button is currently pressed, reduce the downward force on the bird a bit.
    // Once this foce goes negative the bird goes up, otherwise it falls towards the ground
    // gaining speed
    if (digitalRead(FLAP_BUTTON) == LOW) {
      vy = -4;
    }

    // increase the downward force on the bird
    vy += 1;

    // add the downward force to the bird position to determine it's new position
    bird_y += vy;

    // make sure the bird doesn't fly off the top of the screen
    if (bird_y < 0 ) {
      bird_y = 0;
    }

    // make sure the bird doesn't fall off the bottom of the screen
    // give it a slight positive lift so it 'waddles' along the ground.
    if (bird_y > display.height() - SPRITE_HEIGHT) {
      bird_y = display.height() - SPRITE_HEIGHT;
      vy = -2;
    }

    // if the downward force on the bird is negative the bird is going up!
    if (vy < 0) {

      // display the bird using a randomly picked flap animation frame
      if (random(2) == 0) {

        // wind down
        display.drawBitmap(bird_x, bird_y, bird1_bmp, 16, 16, WHITE);
      }
      else {

        // wing up
        display.drawBitmap(bird_x, bird_y, bird2_bmp, 16, 16, WHITE);

      }

    }
    else {

      // bird is currently falling, use wing up frame
      display.drawBitmap(bird_x, bird_y, bird2_bmp, 16, 16, WHITE);

    }

    // now we move the walls and see if the player has hit anything
    for (int i = 0 ; i < 2; i++) {

      // draw the top half of the wall
      display.fillRect(wx[i], 0, wall_width, wy[i], WHITE);

      // draw the bottom half of the wall
      display.fillRect(wx[i], wy[i] + wall_gap, wall_width, display.height() - wy[i] + wall_gap, WHITE);

      // if the wall has hit the edge of the screen
      // reset it back to the other side with a new gap position
      if (wx[i] < 0) {
        wy[i] = random(0, display.height() - wall_gap);
        wx[i] = display.width();
      }

      // if the bird has passed the wall, update the score
      if (wx[i] == bird_x) {
        score++;

        // highscore is whichever is bigger, the current high score or the current score
        high_score = max(score, high_score);
      }

      // if the bird is level with the wall and not level with the gap - game over!
      if (
        (bird_x + SPRITE_WIDTH > wx[i] && bird_x < wx[i] + wall_width) // level with wall
        &&
        (bird_y < wy[i] || bird_y + SPRITE_HEIGHT > wy[i] + wall_gap) // not level with the gap
      ) {
        game_state = 1;

        // display the crash and pause 1/2 a second
        display.display();
        delay(500);

      }

      // move the wall left 4 pixels
      wx[i] -= 4;
    }

    // display the current score
    boldTextAtCenter(0, (String)score);

    // now display everything to the user and wait
    display.display();
    delay(GAME_SPEED);

  }
  else {

    // game over screen

    screenWipe(10);

    outlineTextAtCenter(1, "NANO BIRD");

    textAtCenter(display.height() / 2 - 8, "GAME OVER");
    textAtCenter(display.height() / 2, String(score));

    boldTextAtCenter(display.height() - 16, "HIGH SCORE");
    boldTextAtCenter(display.height()  - 8, String(high_score));

    display.display();

    while (digitalRead(FLAP_BUTTON) == LOW);

    bird_y = display.height() / 2;
    vy = -4;
    wx[0] = display.width() ;
    wy[0] = display.height() / 2 - wall_gap / 2;
    wx[1] = display.width() + display.width() / 2;
    wy[1] = display.height() / 2 - wall_gap / 1;
    game_state = score = 0;

    while (digitalRead(FLAP_BUTTON) == HIGH);

    screenWipe(10);
  }


}

void screenWipe(int speed) {

  for (int i = 0; i < display.height(); i += speed) {
    display.fillRect(0, i, display.width(), speed, WHITE);
    display.display();
  }

  for (int i = 0; i < display.height(); i += speed) {
    display.fillRect(0, i, display.width(), speed, BLACK);
    display.display();
  }

  display.clearDisplay();

}

void textAt(int x, int y, String txt) {
  display.setCursor(x, y);
  display.print(txt);
}

void textAtCenter(int y, String txt) {
  textAt(display.width() / 2 - txt.length() * 3, y, txt);
}

void outlineTextAtCenter(int y, String txt) {
  int x = display.width() / 2 - txt.length() * 3;

  display.setTextColor(WHITE);
  textAt(x - 1, y, txt);
  textAt(x + 1, y, txt);
  textAt(x, y - 1, txt);
  textAt(x, y + 1, txt);

  display.setTextColor(BLACK);
  textAt(x, y, txt);
  display.setTextColor(WHITE);

}

void boldTextAtCenter(int y, String txt) {
  int x = display.width() / 2 - txt.length() * 3;

  textAt(x, y, txt);
  textAt(x + 1, y, txt);

}
