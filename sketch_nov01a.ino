
#include <M5StickC.h>
#include <EEPROM.h>
 
#define widthscreen 80 // bildschirm breite
#define heightscreen 160 // bildschirm hoehe
#define widthscreen2 40 // haelfte des bildschirm
#define heightscreen2 80 // haelfte des bildschirm
// spiel konstanten
#define SPEED 0.5
#define GRAVITY 10
#define JUMP_FORCE 2.5
#define SKIP_TICKS 20.0 
#define MAX_FRAMESKIP 5
// vogel groesse
#define BIRDW 10 // vogel hoehe
#define BIRDH 10 // vogel bre
#define BIRDW2 5 // half width
#define BIRDH2 5 // half height
// pipe size
#define PIPEW 20 // pipe breite
#define GAPHEIGHT 40 // pipe zwischenabstand groesse
// floor size
#define FLOORH 20 // boden hoehe 
// grass size
#define GRASSH 4 // grass hoehe 
 
int address = 0;
int maxScore = EEPROM.readInt(address);
const int buttonPin = 2; 
// hintergund
const unsigned int BCKGRDCOL = M5.Lcd.color565(random(0, 255),random(0, 255),random(0, 255));
// vogel
const unsigned int BIRDCOL = M5.Lcd.color565(random(0, 255),random(0, 255),random(0, 255));
// pipe
const unsigned int PIPECOL = M5.Lcd.color565(random(0, 255),random(0, 255),random(0, 255));
const unsigned int PIPEHIGHCOL = M5.Lcd.color565(random(0, 255),random(0, 255),random(0, 255));
const unsigned int PIPESEAMCOL = M5.Lcd.color565(0,0,0);

//Boden & Grass
const unsigned int FLOORCOL = M5.Lcd.color565(random(0, 255),random(0, 255),random(0, 255));
const unsigned int GRASSCOL = M5.Lcd.color565(random(0, 255),random(0, 255),random(0, 255));
const unsigned int GRASSCOL2 = M5.Lcd.color565(random(0, 255),random(0, 255),random(0, 255));
 

#define C0 BCKGRDCOL
#define C1 M5.Lcd.color565(random(0, 255),random(0, 255),random(0, 255))
#define C2 BIRDCOL
#define C3 TFT_WHITE
#define C4 TFT_RED
#define C5 M5.Lcd.color565(random(0, 255),random(0, 255),random(0, 255))
 
static unsigned int birdcol[] =
{ C0, C0, C1, C1, C1, C1, C1, C0, C0, C0, C1, C1, C1, C1, C1, C0,
C0, C1, C2, C2, C2, C1, C3, C1, C0, C1, C2, C2, C2, C1, C3, C1,
C0, C2, C2, C2, C2, C1, C3, C1, C0, C2, C2, C2, C2, C1, C3, C1,
C1, C1, C1, C2, C2, C3, C1, C1, C1, C1, C1, C2, C2, C3, C1, C1,
C1, C2, C2, C2, C2, C2, C4, C4, C1, C2, C2, C2, C2, C2, C4, C4,
C1, C2, C2, C2, C1, C5, C4, C0, C1, C2, C2, C2, C1, C5, C4, C0,
C0, C1, C2, C1, C5, C5, C5, C0, C0, C1, C2, C1, C5, C5, C5, C0,
C0, C0, C1, C5, C5, C5, C0, C0, C0, C0, C1, C5, C5, C5, C0, C0};
 
// vogel structure
static struct BIRD {
long x, y, old_y;
long col;
float vel_y;
} bird;
 
// pipe 
static struct PIPES {
long x, gap_y;
long col;
} pipes;
 
// score
int score;
// temporary x and y var
static short tmpx, tmpy;
 

#define drawPixel(a, b, c) M5.Lcd.setAddrWindow(a, b, a, b); M5.Lcd.pushColor(c)

void game_loop() {
unsigned char GAMEH = heightscreen - FLOORH;
// draw the floor once, we will not overwrite on this area in-game
//Gras
M5.Lcd.fillRect(0, GAMEH+1, widthscreen2, GRASSH, GRASSCOL);
M5.Lcd.fillRect(widthscreen2, GAMEH+1, widthscreen2, GRASSH, GRASSCOL2);
M5.Lcd.fillRect(0, GAMEH+GRASSH+1, widthscreen, FLOORH-GRASSH, FLOORCOL);
// grass x position (for stripe animation)
long grassx = widthscreen;
double delta, old_time, next_game_tick, current_time;
next_game_tick = current_time = millis();
int loops;

bool passed_pipe = false;
unsigned char px;
 
while (1) {
loops = 0;
while( millis() > next_game_tick && loops < MAX_FRAMESKIP) {
if(digitalRead(M5_BUTTON_HOME) == LOW){
//while(digitalRead(M5_BUTTON_HOME) == LOW);
if (bird.y > BIRDH2*0.5) bird.vel_y = -JUMP_FORCE;

else bird.vel_y = 0;
}
 

old_time = current_time;
current_time = millis();
delta = (current_time-old_time)/1000;

bird.vel_y += GRAVITY * delta;
bird.y += bird.vel_y;

 
pipes.x -= SPEED;
if (pipes.x < -PIPEW) {
pipes.x = widthscreen;
pipes.gap_y = random(10, GAMEH-(10+GAPHEIGHT));
}
 
// ---------------
next_game_tick += SKIP_TICKS;
loops++;
}
 
 
if (pipes.x >= 0 && pipes.x < widthscreen) {
// pipe color
M5.Lcd.drawFastVLine(pipes.x+3, 0, pipes.gap_y, PIPECOL);
M5.Lcd.drawFastVLine(pipes.x+3, pipes.gap_y+GAPHEIGHT+1, GAMEH-(pipes.gap_y+GAPHEIGHT+1), PIPECOL);
// bottom and top border of pipe
drawPixel(pipes.x, pipes.gap_y, PIPESEAMCOL);
drawPixel(pipes.x, pipes.gap_y+GAPHEIGHT, PIPESEAMCOL);
// pipe 
drawPixel(pipes.x, pipes.gap_y-6, PIPESEAMCOL);
drawPixel(pipes.x, pipes.gap_y+GAPHEIGHT+6, PIPESEAMCOL);
drawPixel(pipes.x+3, pipes.gap_y-6, PIPESEAMCOL);
drawPixel(pipes.x+3, pipes.gap_y+GAPHEIGHT+6, PIPESEAMCOL);
}
#if 1
// erase behind pipe
if (pipes.x <= widthscreen)
M5.Lcd.drawFastVLine(pipes.x+PIPEW, 0, GAMEH, BCKGRDCOL);
#endif
tmpx = BIRDW-1;
do {
px = bird.x+tmpx+BIRDW;
tmpy = BIRDH - 1;
do {
drawPixel(px, bird.old_y + tmpy, BCKGRDCOL);
} while (tmpy--);
tmpy = BIRDH - 1;
do {
drawPixel(px, bird.y + tmpy, birdcol[tmpx + (tmpy * BIRDW)]);
} while (tmpy--);
} while (tmpx--);
bird.old_y = bird.y;
grassx -= SPEED;
if (grassx < 0) grassx = widthscreen;
M5.Lcd.drawFastVLine( grassx %widthscreen, GAMEH+1, GRASSH-1, GRASSCOL);
M5.Lcd.drawFastVLine((grassx+64)%widthscreen, GAMEH+1, GRASSH-1, GRASSCOL2);
//auf Collision ueberpruefen
if (bird.y > GAMEH-BIRDH) break;
if (bird.x+BIRDW >= pipes.x-BIRDW2 && bird.x <= pipes.x+PIPEW-BIRDW) {
if (bird.y < pipes.gap_y || bird.y+BIRDH > pipes.gap_y+GAPHEIGHT) break;
else passed_pipe = true;
}

else if (bird.x > pipes.x+PIPEW-BIRDW && passed_pipe) {
passed_pipe = false;

M5.Lcd.setTextColor(BCKGRDCOL);
M5.Lcd.setCursor( widthscreen2, 4);
M5.Lcd.print(score);
// set text color back to white for new score
M5.Lcd.setTextColor(TFT_WHITE);

score++;
}
 
// update score
// ---------------
M5.Lcd.setCursor( 2, 4);
M5.Lcd.print(score);
}
 
// add a small delay to show how the player lost
delay(1500);
}
 
 

void game_start() {
while (1) {
// wait for push button
if(digitalRead(M5_BUTTON_HOME) == LOW){
while(digitalRead(M5_BUTTON_HOME) == LOW);
break;
}
 
}
// settings
game_init();
}
 
void game_init() {

M5.Lcd.fillScreen(BCKGRDCOL);

score = 0;
// init bird
bird.x = 50;
bird.y = bird.old_y = heightscreen2 - BIRDH;
bird.vel_y = -JUMP_FORCE;
tmpx = tmpy = 0;

randomSeed(analogRead(0));

pipes.x = 0;
pipes.gap_y = random(20, heightscreen-60);
}
 
 
 
void resetScore()
{
EEPROM.writeInt(address, 0);
EEPROM.commit();
}
 
void setup() {
M5.begin();
EEPROM.begin(1000);
pinMode(M5_BUTTON_HOME, INPUT);
resetScore();

 
}
 
void loop() {
// put your main code here, to run repeatedly:
game_start();
game_loop();

}
