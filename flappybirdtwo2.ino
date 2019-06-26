// For a test Flippy Bird
// If using the Arduino shield, use the tftpaint_shield.pde sketch instead!
// DOES NOT CURRENTLY WORK ON ARDUINO LEONARDO

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4
#define LED 13
#define BAUDRATE 57600
#define DEBUGOUTPUT 0

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define SKYLEN 240
#define SKYHEI 40
#define GRASSBGEIN 280
#define BOXSIZE 40
#define PENRADIUS 3
#define MINPRESSURE 10
#define MAXPRESSURE 1000
int oldcolor, currentcolor;

//原来bar.h中的定义
#define bar_len 20
#define movespeed 2 //板块的移动速度
int  bar_blank = 70;//挡板之间的间隔
int  count1 = 0;
int  score = 0;
int  mark = 0;//标志位，如果为1时小鸟死亡，游戏结束
bool first_try =true;
int  start_mark = 0;
int  frame=10;
//byte badge=30;

typedef struct{
  int x = 240;//目前障碍的横坐标,x要小于负二十才会消亡
  int y = 0;//目前上柱子的纵坐标；
  int i = 0;//标志位，柱子在运行当中为1，消失后设置为0，重新创建后设置为y，途中最多一次出现3柱子
  }bar1;

bar1 bar[2];
////////////////////////////////////////////////////////////////////////////

typedef struct{
  int x = 120;
  int y = 160;
  int i = 0;//标志位，标志小鸟此时的运动状态，上升时为1，下降时为0；
  int j = 0;//标志位，在之后的几帧中小鸟网上飞；
  int birdspeed = 1;
}bird1;

bird1 bird;
////////////////////////////////////////////////////////////////////////////////////////

void setup(void) {
  Serial.begin(57600);
  Serial.println(F("Paint!"));
  
  restart_screen();
  uint16_t identifier = tft.readID();
  tft.begin(identifier);
  
  game_start();//游戏开始界面
  
  start_mark = 2;
  pinMode(13, OUTPUT);
}
//////////////////////////////////////////////////////////////////////////////

void loop()
{ 

  
  if(start_mark){
    tft.fillScreen(BLACK);
    start_mark--;
  }
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
 
 
  // 触屏部分
  bird.i = 0;
//  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
//    bird.i = 1;
//    }
    if(Serial.available()>0&&ReadOneByte()){
      frame=1;
    }  
    if(frame>0){
      bird.i=1;
    }      
    frame--;
    
  build_bar(0);
  delete_bar(0);
  move_bar(0);
  
  if(count1 >= (120/movespeed)){
    build_bar(1);
    delete_bar(1);
    move_bar(1);
    }
  else{
    count1 += 1;
  }
  
  //tft.fillCircle(bird.x,bird.y,10,BLACK);
  deletebird(bird.x,bird.y);//删除小鸟
  move_bird();
  //tft.fillCircle(bird.x,bird.y,10,BLUE);
  drawbird(bird.x,bird.y);//画小鸟

  scoreprint();
  
  collision_test();//死亡判决
  
  if(mark == 1){
    game_result();
    game_fail();
    initiateGame();
  }
  
}
/////////////////////////////////////////////////////////////////////////////

void build_bar(int num)
{ 
  if(bar[num].i == 0){
    bar[num].y = random(90,140); //若重新创造柱子则重新随机高度,否则纵坐标不变
    bar[num].x = 240;
    bar[num].i = 1;
    }
 
  int downbar = bar[num].y + bar_blank;//下柱子的纵坐标
  int downbar_h = 320 - downbar;//下柱子的高度，都是由上柱子纵坐标计算得到

  if(bar[num].x <= 220 && (bar[num].x + bar_len) > 0){
    tft.fillRect(bar[num].x,0,bar_len,bar[num].y,RED);//画上柱子
    tft.drawFastHLine(bar[num].x,bar[num].y-10, 20, BLACK);
    tft.drawFastHLine(bar[num].x+1,bar[num].y-9, 20, BLACK);
    tft.drawFastVLine(bar[num].x, 0, bar[num].y-10, BLACK);
    tft.drawFastVLine(bar[num].x+17, 0, bar[num].y-10, BLACK);
    
    tft.fillRect(bar[num].x,downbar,bar_len,downbar_h,RED);//画下柱子
    tft.drawFastHLine(bar[num].x,downbar+10, 20, BLACK);
    tft.drawFastHLine(bar[num].x+1,downbar+9, 20, BLACK);
    tft.drawFastVLine(bar[num].x,downbar+10, 180, BLACK);
    tft.drawFastVLine(bar[num].x+17,downbar+10, 180, BLACK);
    }
  else if(bar[num].x > 220){
    int newbar_len = 240 - bar[num].x;
    tft.fillRect(bar[num].x,0,newbar_len,bar[num].y,RED);//画上柱子
    tft.drawFastHLine(bar[num].x,bar[num].y-10, 20, BLACK);
    tft.drawFastHLine(bar[num].x+1,bar[num].y-9, 20, BLACK);
    tft.drawFastVLine(bar[num].x, 0, bar[num].y-10, BLACK);
    tft.drawFastVLine(bar[num].x+17, 0, bar[num].y-10, BLACK);
    
    tft.fillRect(bar[num].x,downbar,newbar_len,downbar_h,RED);//画下柱子
    tft.drawFastHLine(bar[num].x,downbar+10, 20, BLACK);
    tft.drawFastHLine(bar[num].x+1,downbar+9, 20, BLACK);
    tft.drawFastVLine(bar[num].x,downbar+10, 180, BLACK);
    tft.drawFastVLine(bar[num].x+17,downbar+10, 180, BLACK);
    }
  else{
    int newbar_len = bar[num].x + bar_len;
    tft.fillRect(0,0,newbar_len,bar[num].y,RED);//画上柱子
    tft.drawFastHLine(bar[num].x,bar[num].y-10, 20, BLACK);
    tft.drawFastHLine(bar[num].x+1,bar[num].y-9, 20, BLACK);
    tft.drawFastVLine(bar[num].x, 0, bar[num].y-10, BLACK);
    tft.drawFastVLine(bar[num].x+17, 0, bar[num].y-10, BLACK);
    
    tft.fillRect(0,downbar,newbar_len,downbar_h,RED);//画下柱子
    tft.drawFastHLine(bar[num].x,downbar+10, 20, BLACK);
    tft.drawFastHLine(bar[num].x+1,downbar+9, 20, BLACK);
    tft.drawFastVLine(bar[num].x,downbar+10, 180, BLACK);
    tft.drawFastVLine(bar[num].x+17,downbar+10, 180, BLACK);
    }
}
////////////////////////////////////////////////////////////////////////

void delete_bar(int num)
{
  if(bar[num].i == 0){
    }
  else if(bar[num].x > (-1*bar_len)){
    int delelte_x = bar[num].x + bar_len - movespeed;
    int downbar = bar[num].y + bar_blank;//下柱子的纵坐标
    int downbar_h = 320 - downbar;//下柱子的高度，都是由上柱子纵坐标计算得到
    
    tft.fillRect(delelte_x,0,movespeed,bar[num].y,BLACK);//画上柱子
    tft.fillRect(delelte_x,downbar,movespeed,downbar_h,BLACK);//画下柱子
    }

}
//////////////////////////////////////////////////////////////////////////

void move_bar(int num)
{
  if(bar[num].i == 0){
    }
  else if(bar[num].x > (-1*bar_len)){
    bar[num].x -= movespeed;
    }
  else{
    bar[num].i = 0;
    }
}
////////////////////////////////////////////////////////////////////

void move_bird()
{ 
  
  if(bird.i == 0 && bird.j == 0){
    bird.birdspeed = 1;
    bird.y += bird.birdspeed;
  }
  else if(bird.i == 1 && bird.j == 0){
    bird.j = 5;
    bird.birdspeed = -2;
    bird.y += bird.birdspeed;
  }
  else if(bird.i == 1 && bird.j != 0){
    bird.j = 5;
    bird.birdspeed = -2;
    bird.y += bird.birdspeed;
  }
  else if(bird.i == 0 && bird.j != 0){
    bird.j -= 1;
    bird.y += bird.birdspeed;
  }
}
//////////////////////////////////////////////////////////////////////

void scoreprint()
{
  tft.setCursor(0,0);//起始坐标
  tft.setTextColor(GREEN,BLACK); //颜色和背景
  tft.setTextSize(1); //文本大小
  tft.print("score");
  tft.print(score);
}
////////////////////////////////////////////////////////////////////

void collision_test()
{
  if(bird.y>310||bird.y<10){ //小球触碰上下壁
    mark=1;
    return;
  }
  for(int num=0;num<2;num++){
    if(bar[num].x==110){//如果下标为num的柱子的横坐标值为110，可以看成小鸟通过了柱子；
        score++;
     }
    if(bar[num].i==0){
      continue;
    }
    else if(bar[num].x-bird.x <10 && bird.x-bar[num].x <20){ //加一个else
      if(bar[num].y-bird.y> -10 ||bar[num].y+bar_blank-bird.y <10){
       mark=1;
       return;
      }
     } 
   }
}
/////////////////////////////////////////////////////////////////////

void game_fail()
{
  while(1){
       digitalWrite(13, HIGH);
       TSPoint g = ts.getPoint();
       digitalWrite(13, LOW);
       if(g.z> MINPRESSURE && g.z < MAXPRESSURE ){
       start_mark = 2;
       break;
       }
   }
}
///////////////////////////////////////////////////////////////////////////

void game_result()
{
      tft.fillScreen(BLACK);
      tft.setCursor(1, 40);
      tft.setTextColor(MAGENTA,BLACK); //颜色和背景
      tft.setTextSize(3); //文本大小
      tft.println("game over");
      tft.println("");
      tft.print("your score: ");
      tft.println(score);
      tft.setTextColor(WHITE,BLACK); //颜色和背景
      tft.setTextSize(2); //文本大小
      tft.println("");
      tft.println("Click on the screen");
      tft.println("");
      tft.println("begin game one more ");
      tft.println("");
      tft.print("time... ");
 }
/////////////////////////////////////////////////////////////////// 

 void initiateGame(){
  bird.x = 120;
  bird.y = 160;
  bird.i = 0;
  bird.j = 0;
  bar[0].i = 0;
  bar[1].i = 0;
  score = 0;
  count1 = 0;
  mark = 0;
  first_try = true;
  

}
////////////////////////////////////////////////////////////////////

void game_start()
{ 
 
  if(first_try){
    tft.fillScreen(BLACK);
    tft.setCursor(1, 200);
    tft.setTextColor(MAGENTA,BLACK); //颜色和背景
    tft.setTextSize(3); //文本大小
    tft.println("Click on the screen");
    tft.println("");
    tft.print("start game ");
    drawbird(120,80); 
  }
  if(first_try){
    delay(1000);
  }
  while(first_try){
    digitalWrite(13, HIGH);
    TSPoint st = ts.getPoint();
    digitalWrite(13, LOW);
    
//    if(st.z> MINPRESSURE && st.z < MAXPRESSURE ){
//      
//      first_try = false;
//      return;
//    }

      if(ReadOneByte()){
        first_try = false;
        return;
      }
        
  }
  
}
///////////////////////////////////////////////////////////////////////

void drawbird(int x,int y){
   tft.fillCircle(x,y,15,GREEN);//身体
   tft.fillCircle(x+8,y-8,8,WHITE);//眼睛
   tft.fillCircle(x+12,y-10,2,BLACK);//黑眼珠
   tft.fillCircle(x+12,y+5,5,RED);//嘴巴
   tft.fillCircle(x-10,y,6,WHITE);//翅膀
   //////////////////////翅膀上黑线////////////////////////
   tft.drawFastHLine(x-20, y-6, 15, BLACK);
   tft.drawFastHLine(x-20, y-5, 16, BLACK);
 /////////////////////////翅膀下黑线/////////////////////////
   tft.drawFastHLine(x-20, y+5, 16, BLACK);
   tft.drawFastHLine(x-20, y+6, 15, BLACK);
   //////////////////////嘴巴上黑线/////////////////////////////////
   tft.drawFastHLine(x+11, y+5, 7, BLACK);
   tft.drawFastHLine(x+11, y+6, 7, BLACK);
/////////////////////////身体上下黑线/////////////////////////////////////
   tft.drawFastHLine(x-15, y+15, 30, BLACK);
   tft.drawFastHLine(x-15, y-16, 30, BLACK);
  
}
////////////////////////////////////////////////////////////////////////

void deletebird(int x,int y){
   tft.fillCircle(x,y,15,BLACK);
   tft.fillCircle(x+8,y-8,8,BLACK);
   //tft.fillCircle(x+12,y-10,2,BLACK);
   tft.fillCircle(x+12,y+5,5,BLACK);
   tft.fillCircle(x-10,y,6,BLACK);
   
   /*tft.drawFastHLine(x-20, y-6, 15, BLACK);
   tft.drawFastHLine(x-20, y-5, 15, BLACK);
 
   tft.drawFastHLine(x-20, y+5, 15, BLACK);
   tft.drawFastHLine(x-20, y+6, 15, BLACK);
   
   tft.drawFastHLine(x+11, y+5, 7, BLACK);
   tft.drawFastHLine(x+11, y+6, 7, BLACK);

   tft.drawFastHLine(x-15, y+15, 30, BLACK);
   tft.drawFastHLine(x-15, y-16, 30, BLACK);
  */
}
///////////////////////////////////////////////////////////////////////

void restart_screen(){
  delay(1000);
  tft.reset();
  
  uint16_t identifier = tft.readID();
  if(identifier==0x0101)
      identifier=0x9341;

  tft.begin(identifier);

  //tft.fillScreen(BLACK);
  
}
//////////////////////////////////////////////////////////

byte ReadOneByte() {
  int ByteRead;
  while(!Serial.available());
  ByteRead = Serial.read();
  return ByteRead;
}
