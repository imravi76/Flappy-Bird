/**
   bitty bird - a flappy bird clone for arduino & oled screen
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// pin the flap button is attached to
#define FLAP_BUTTON  2

#define SPRITE_HEIGHT   16
#define SPRITE_WIDTH    16

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


int gamestate = 1, score = 0, highScore = 0, bird_x = (int)display.width() / 4, bird_y, vy = 0, wx[2], wy[2] ;
int gap = 30;
int block_width = 10;
void setup() {

  // used for debugging
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // setup the flap button pin for input
  pinMode(FLAP_BUTTON, INPUT_PULLUP);

  // initialise the random number generator
  randomSeed(analogRead(0));

  display.setTextColor(WHITE);
  display.clearDisplay();

}

void loop() {

  if (gamestate == 0) {

    display.clearDisplay();

    if (digitalRead(FLAP_BUTTON) == LOW) {
      vy = -4;
    }

    vy += 1;
    bird_y += vy;

    if (bird_y < 0 ) {
      bird_y - 0;
    }

    if (bird_y > display.height() - SPRITE_HEIGHT) {
      bird_y = display.height() - SPRITE_HEIGHT;
      vy = -2;
    }

    if (vy < 0) {

      if (random(2) == 0) {

        display.drawBitmap(bird_x, bird_y, bird1_bmp, 16, 16, WHITE);
      }
      else {

        display.drawBitmap(bird_x, bird_y, bird2_bmp, 16, 16, WHITE);
      }

    }
    else {
      display.drawBitmap(bird_x, bird_y, bird2_bmp, 16, 16, WHITE);
    }

    for (int i = 0 ; i < 2; i++) {

      display.fillRect(wx[i], 0, block_width, wy[i], WHITE);
      display.fillRect(wx[i], wy[i] + gap, block_width, display.height() - wy[i] + gap, WHITE);

      if (wx[i] < 0) {
        wy[i] = random(0, display.height() - gap);
        wx[i] = display.width();
      }

      if (wx[i] == bird_x) {
        highScore = max(++score, highScore);
      }

      //display.fillRect(0, 0, abs(bird_y - wy[i]), 5, WHITE);
      if (bird_y > display.height() || bird_y < 0 || ((bird_x + SPRITE_WIDTH > wx[i] && bird_x < wx[i] + block_width)) && (bird_y < wy[i] || bird_y + SPRITE_HEIGHT > wy[i] + gap)) {
        gamestate = 1;
        display.display();
        delay(500);

      }
      wx[i] -= 4;
    }

    textAt(display.width() / 2, 0, (String)score);  
    display.display();
    delay(50);

  }
  else {

    screenWipe(10);

    textAt(display.width() / 2 - 22, display.height() / 2, "GAME OVER");
    textAt(display.width() / 2 - 2.5, display.height() / 2 + 16, (String)highScore);
    display.display();

    while (digitalRead(FLAP_BUTTON) == LOW);

    bird_y = display.height() / 2;
    vy = -4;
    wx[0] = display.width() ;
    wy[0] = display.height() / 2 - gap / 2;
    wx[1] = display.width() + display.width() / 2;
    wy[1] = display.height() / 2 - gap / 1;
    gamestate = score = 0;

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
