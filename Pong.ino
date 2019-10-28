#include "ssd1306.h"
    // 70-72 up
    // 67-69 Up/Right
    // 61-63 right
    // 59-61 Up/Down
    // 55-57 Down/Right
    // 50-52 down
    // 47-49 Up/Left
    // 41-43 Left/Right
    // 38-40 Down/Left
    // 35-37 left
    // 0  No button
#define FRAMETIME 10 //Display a frame a minimum of this many ms
#define HEIGHT 32 //display height
#define WIDTH 128 //display width
#define BALLDIAMETER 8  //sizeof(ball);
#define PADDLELENGTH 8  //also change paddle[x], up to 8px long down to 2px
#define PADDLEWIDTH 2 //how thick the paddle is
#define SCORETOWIN 10 //score needed to win, must be between 1-15.
#define AIDIFFICULTY 65 //1-100, percentage of how well the AI follows the ball
#define SPIKESPEED 6 //multiplier for speed
#define SPIKECOOLDOWNLENGTH 1500 //1500 frames
#define LASERSPEED 3 //3px per frame
#define SHOOTCOOLDOWNLENGTH 300 //300 frames
//#define GetAnalogStrength //Debug

float ballSpeed = 1;
bool spikeBall = false;
bool shoot = false;
short spikeCooldown = 0;
short shootCooldown = 0;
char integerOutput[2]; //supports 1-2 digits
uint8_t score = 0; //0bPPPPCCCC (max 15 points each)
const PROGMEM uint8_t projectile[2] = {0B00000011,0B00000011}; //2x2 square
const PROGMEM uint8_t paddle[2] = {0B11111111,0B11111111}; //(solid line 2px wide), 126 for 6px long
const PROGMEM uint8_t ball[BALLDIAMETER] = {
  0B00001110,
  0B00011111,
  0B00111111,
  0B01111110,
  0B01111110,
  0B00111101,
  0B00011001,
  0B00001110
};

SPRITE player; //player
SPRITE cpu; //cpu paddle
SPRITE ballSprite; //ball
SPRITE laser; //laser

void setup() {
  //Define button
  pinMode(A2,INPUT);
  //Define LED's
  pinMode(0,OUTPUT); //Spike
  pinMode(1,OUTPUT); //Laser
  //init OLED
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_128x32_i2c_init();
  showTitle();
  player = ssd1306_createSprite(0,(HEIGHT-PADDLELENGTH)/2, sizeof(paddle),paddle);
  cpu = ssd1306_createSprite(WIDTH-2, (HEIGHT-PADDLELENGTH)/2, sizeof(paddle),paddle);
  ballSprite = ssd1306_createSprite((HEIGHT-BALLDIAMETER)/2,(WIDTH-BALLDIAMETER)/2,BALLDIAMETER,ball); //create at center of screen
  laser = ssd1306_createSprite(0,(HEIGHT-PADDLELENGTH)/2-1 ,sizeof(projectile), projectile);
}

void startGame(){
  float ballxf = (WIDTH-BALLDIAMETER)/2; float ballDirX;
  float ballyf = (HEIGHT-BALLDIAMETER)/2; float ballDirY;
  byte ledFlash = 0;
  //reset positions
  player.y = (HEIGHT-PADDLELENGTH)/2;
  cpu.y = (HEIGHT-PADDLELENGTH)/2;
  ballSprite.x = (WIDTH-BALLDIAMETER)/2;
  ballSprite.y = (HEIGHT-BALLDIAMETER)/2;
  //show score, clear screen, then generate random direction
  updateScore();
  ssd1306_clearScreen();
  generateRandomDirection(ballDirX, ballDirY, false);
  for(;;ballSpeed+=0.0005){
    //move ball first to avoid pixel overlap bugs with ball size <8
    moveBall(ballSprite, ballxf, ballyf, ballDirX, ballDirY);

    //Check if winner
    if(score%16==SCORETOWIN || (score>>4)==SCORETOWIN){
      player.draw();
      cpu.draw();
      if(score%16==10) ssd1306_printFixed(30,0,"Player Wins",STYLE_BOLD);
      else ssd1306_printFixed(42,0,"CPU Wins",STYLE_BOLD);
      delay(5000); break;
    }
    
    //move player if input is valid
    switch(analogRead(A1)){
      case 35 ... 37:
        startSpike();break;
      case 38 ... 40:
        if(player.y+PADDLELENGTH<HEIGHT) player.y++;
        startSpike();break;
      case 41 ... 43:
        startSpike(); startShoot(); break;
      case 47 ... 49:
        if(player.y > 0) player.y--;
        startSpike(); break;
      case 50 ... 52:
        if(player.y+PADDLELENGTH<HEIGHT) player.y++;break;
      case 55 ... 57:
        if(player.y+PADDLELENGTH<HEIGHT) player.y++;
        startShoot(); break;
      case 61 ... 63:
        startShoot(); break; 
      case 67 ... 69:
        if(player.y > 0) player.y--;
        startShoot(); break;
      case 70 ... 72:
        if(player.y > 0) player.y--;break;
    }
    player.eraseTrace();
    player.draw();
    
    //move projectile
    if(shoot){
      if(laser.x+2 >= ballSprite.x && 
         laser.x<= ballSprite.x+BALLDIAMETER && 
         laser.y<=ballSprite.y+BALLDIAMETER && 
         laser.y+2>=ballSprite.y){
            ballDirX = abs(ballDirX);
            shoot=false;
            laser.x=0;
            laser.eraseTrace();
      }else if (laser.x< WIDTH-2){
        laser.x += LASERSPEED;
        laser.eraseTrace();
        laser.draw();
      }else{
        shoot = false;
        laser.x=0;
        laser.eraseTrace();
      }
    }
    
    //move AI
    moveAI(cpu,ballSprite,ballDirX, ballDirY);
    cpu.eraseTrace();
    cpu.draw();

    //Increment cooldowns
    spikeCooldown++;
    shootCooldown++;

    //Show LEDS
    if(spikeCooldown>=SPIKECOOLDOWNLENGTH){
      if (spikeBall){
        ledFlash++;
        if(ledFlash%50<25) digitalWrite(0,HIGH);
        else digitalWrite(0,LOW);
        if (ledFlash>50) ledFlash=0;
      }else{
        digitalWrite(0,HIGH);
      }
    }else digitalWrite(0,LOW);
    (shootCooldown>=SHOOTCOOLDOWNLENGTH) ? digitalWrite(1,HIGH) : digitalWrite(1,LOW);

    //delay frame
    delay(FRAMETIME);
  }
}

void startShoot(){
  if(shootCooldown > SHOOTCOOLDOWNLENGTH){
    laser.y = player.y+PADDLELENGTH/2-1;
    shootCooldown = 0;
    shoot = true;
  }
}

void startSpike(){
  if(spikeCooldown > SPIKECOOLDOWNLENGTH) spikeBall = true;  
}

void moveAI(SPRITE &ai, SPRITE &ball, float deltaX, float deltaY){
  //basic following AI;
  if(random(100)< AIDIFFICULTY){ //% chance that cpu will move
    if(ball.y > ai.y+PADDLEWIDTH/2){
      if(ai.y+PADDLEWIDTH<HEIGHT) ai.y++;
    }else{
      if(ai.y>0) ai.y--;
    }
  }
}

void moveBall(SPRITE &sprite, float &x, float &y, float &deltaX, float &deltaY){
  //Reset ball when ball hits the side
  if(sprite.x<=1 || sprite.x>=WIDTH-BALLDIAMETER-1){
    if(sprite.x<=1) score+=16; //0b00010000, cpu score
    if(sprite.x>=WIDTH-BALLDIAMETER-1) score++; //0b00000001, player score
    //reset ball and cpu position
    sprite.x=((WIDTH-BALLDIAMETER)/2); x=sprite.x;
    sprite.y=((HEIGHT-BALLDIAMETER)/2); y=sprite.y;
    cpu.y = (HEIGHT+PADDLELENGTH)/2; 
    ballSpeed = 1;
    generateRandomDirection(deltaX, deltaY, false);
    //reset old sprite locations
    laser.x=0;laser.eraseTrace();shoot=false;
    sprite.eraseTrace();
    //reset cooldowns to 0
    shootCooldown = 0; shoot = false;
    spikeCooldown = 0; spikeBall = false;
    //Display score before new round;
    updateScore();
    ssd1306_clearScreen();
    sprite.draw();
  }
  //Bounce off the paddles
  if(deltaX<0 && sprite.x <= PADDLEWIDTH+1){ //Player
    if(sprite.y+BALLDIAMETER>=player.y && sprite.y<=player.y+PADDLELENGTH){ //bounce off player
      generateRandomDirection(deltaX,deltaY,true);
      deltaX = abs(deltaX); //turn x positive 
      if (spikeBall){
        deltaX *= (SPIKESPEED/2)*2; //less ball breaking through the paddle
        deltaY *= SPIKESPEED/10;
        spikeBall = false; 
        spikeCooldown = 0;
      }
    }
  }else if(deltaX>0 && sprite.x >= WIDTH-BALLDIAMETER-PADDLEWIDTH-1){ //CPU
    if(sprite.y+BALLDIAMETER>=cpu.y && sprite.y<=cpu.y+PADDLELENGTH){ //bounce off cpu
      generateRandomDirection(deltaX,deltaY,true);
      deltaX = -abs(deltaX); //turn x negative 
    }
  }
  //bounce off top and bottom walls
  if(deltaY>0 && y+deltaY+BALLDIAMETER>HEIGHT) deltaY=-deltaY;
  if(deltaY<0 && y+deltaY<0) deltaY=-deltaY;
  //Move ball
  x+=deltaX; sprite.x=round(x);
  y+=deltaY; sprite.y=round(y);
  sprite.eraseTrace();
  sprite.draw();
}

void updateScore(){
  cpu.eraseTrace();
  player.draw();
  cpu.draw();
  if(score%16<10) ssd1306_printFixed(54,0,itoa((score%16),integerOutput,10), STYLE_NORMAL);
  else ssd1306_printFixed(48,0,itoa((score%16),integerOutput,10), STYLE_NORMAL);
  ssd1306_printFixed(61,0,":", STYLE_BOLD);
  ssd1306_printFixed(68,0,itoa((score>>4),integerOutput,10), STYLE_NORMAL);
  delay(1000);
}

void generateRandomDirection(float &deltaX, float &deltaY, bool conserveDirection){
  deltaX = ballSpeed*(random(260)/1000.0 + 0.7); //generate random x between 0.0.7-0.96
  if(random(2)==1) deltaX=-deltaX; //random direction to head towards player or cpu
  bool yNeg = (deltaY<0) ? true : false;
  deltaY = sqrt(ballSpeed*ballSpeed-deltaX*deltaX); //use pythag to get deltaY
  if(yNeg) deltaY=-deltaY;
  if(!conserveDirection && random(2)==1) deltaY=-deltaY; //random direction to head up or down
}

void loop() {
  if(analogRead(A1)>10){ //if button pressed
    randomSeed(millis()); //seed random based on when the user starts the game
    ssd1306_clearScreen();
    #ifdef GetAnalogStrength
      ssd1306_printFixed(50,0,itoa(analogRead(A1),integerOutput,10), STYLE_NORMAL);
    #else
      ssd1306_clearScreen();
      startGame();
      score=0; //reset score for next round
      showTitle();
    #endif
  }
  delay(50);
}

void showTitle(){
  ssd1306_clearScreen();
  ssd1306_printFixed(36,10,"Laser Pong", STYLE_ITALIC);
  ssd1306_negativeMode();
  ssd1306_printFixed(4,30,"Push Button To Start",STYLE_NORMAL);
  ssd1306_positiveMode();
}

//ssd1306_clearScreen();
//ssd1306_printFixed(y,x,"text", STYLE_NORMAL / STYLE_BOLD / STYLE_ITALIC);
//ssd1306_negativeMode();
//ssd1306_positiveMode();
//ssd1306_drawLine(y,x,xEnd,yEnd);
//ssd1306_displayWidth();
//ssd1306_displayHeight()
//ssd1306_drawBitmap(y,x,width,height,uint8_t[]);
//ssd1306_createSprite(y,x,width,uint8_t[]); //scan from bottom to up, left to right
  //spriteName.x;
  //spriteName.y;
  //spriteName.eraseTrace(); //delete previous drawing of sprite
  //spriteName.draw();  //draw new sprite
