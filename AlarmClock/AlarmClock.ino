#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <stdio.h>
#include <DS1302.h>
#include <TaskScheduler.h>
#include <AddicoreRFID.h>
#include <SPI.h>
#include <TimeLib.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Number of lines and i2c address of the display
DS1302 rtc(5, 6, 7);

const int button0Pin = 2;   
const int button1Pin = 4;   
const int button2Pin = 8;   
const int buzzerPin = 9;

int button0State = 0;
int button1State = 0;
int button2State = 0;

const int chipSelectPin = 10;
const int NRSTPD = 3;  
unsigned char serNumA[5];
unsigned char fifobytes;
unsigned char fifoValue;
AddicoreRFID myRFID; 

bool alarmEnabled=false;
int alarmHour = 0;
int alarmMinute = 0;

String alarmHourString = "";
String alarmMinuteString = "";
String alarmAMPM = "";
String alarmString = "Not Set";

String alarmStringShortHour = "";
String alarmStringShortMinute = "";
String alarmStringShort = "";

int currentMenuDialog = 0;

long alarmDelta = 0;
long currentDelta = 0;
long delayDeltaMS = 0;

bool backlightEnabled=false;

String currentTimeString = "";

Time alarmTime;
Time currentTime;

void beep(unsigned char delayms){
  analogWrite(buzzerPin, 20);      // Tone frequency. Integer between, but not equal to 0 or 255.
  delay(delayms);          // wait for a delayms ms
  analogWrite(buzzerPin, 0);       // 0 turns it off
  delay(delayms);          // wait for a delayms ms   
}  


void displayDateTimeCallback(){
//   // Send Day-of-Week
//  Serial.print(rtc.getDOWStr());
//  Serial.print(" ");
//  // Send date
//  Serial.print(rtc.getDateStr());
//  Serial.print(" -- ");
//  // Send time
//  Serial.println(rtc.getTimeStr());
  
  lcd.setCursor(0,0);
  currentTimeString = rtc.getTimeStr(FORMAT_LONG);
  if (currentTimeString.substring(0,2).toInt()>12){
    currentTimeString.substring(0,2) = String(currentTimeString.substring(0,2).toInt()-12);
  }
  
  lcd.print(currentTimeString);
  String d_o_w = rtc.getDOWStr();
  String month = rtc.getMonthStr();
  String dateRead = rtc.getDateStr(FORMAT_LITTLEENDIAN);
  String day = dateRead.substring(0,2);
  String dateDraw = d_o_w.substring(0,3) + " " + month.substring(0,3) + " " + day;
  lcd.setCursor(0,1);
  lcd.print(dateDraw);  
  lcd.print(" "+alarmStringShort);
}

void updateAlarmString(){
    if (alarmEnabled){
    alarmStringShortHour = String(alarmHour);
    alarmStringShortMinute = String(alarmMinute);
    if (alarmHour<10){alarmStringShortHour="0"+alarmStringShortHour;}
    if (alarmMinute<10){alarmStringShortMinute="0"+alarmStringShortMinute;}
    alarmStringShort = "A"+alarmStringShortHour+alarmStringShortMinute;
    }
    
    alarmMinuteString = String(alarmMinute);
    if (alarmMinuteString.length()==1){alarmMinuteString = "0"+alarmMinuteString;}
    
    if (alarmHour>12){
      alarmHourString = String(alarmHour-12);
      alarmAMPM = "PM";
    } else {
      alarmHourString = String(alarmHour);
      alarmAMPM = "AM";
    }
    if (alarmHourString.length()==1){alarmHourString = "0"+alarmHourString;}
    alarmString = alarmHourString + ":" + alarmMinuteString + " " + alarmAMPM;
}

Task displayDateTime(1000, TASK_FOREVER, &displayDateTimeCallback);
Task displayMenu(1000, TASK_FOREVER, &openMenu);
Task startAlarm(1000, TASK_FOREVER, &buzzer);

void openMenu(){
  currentMenuDialog = 0;
  lcd.backlight();
  lcd.clear();
  delay(300);
  while(currentMenuDialog<1){
    button0State = digitalRead(button0Pin);
    button1State = digitalRead(button1Pin);
    button2State = digitalRead(button2Pin);
    if (button0State==LOW){currentMenuDialog++; lcd.clear(); beep(50); delay(300);}   
    lcd.setCursor(0,0);
    if (currentMenuDialog==0){
      lcd.print("Set Alarm Time");
      if (button1State==LOW){alarmHour++; updateAlarmString(); delay(75); if (alarmHour>24){alarmHour=0;}}
      if (button2State==LOW){alarmMinute++; updateAlarmString(); delay(50); if (alarmMinute>59){alarmMinute=0;}}
      lcd.setCursor(0,1);
      lcd.print(alarmString);
    }
  }
startAlarm.disable();
armAlarmClock();
}

bool makeNoise=true;
void buzzer(){
  if (alarmEnabled){
    beep(75);
    beep(75);
    delay(10000);
    beep(75);
    beep(75);
    delay(10000);
    beep(75);
    beep(75);
    delay(10000);
    while(true){
    button0State = digitalRead(button0Pin);
    //if (button0State==LOW){makeNoise=false;}else{makeNoise=true;}
    if (button0State==LOW){delay(60000);}
    if (makeNoise){beep(75);}
    
    //  Listen for RFID
    unsigned char status;
    unsigned char str[MAX_LEN];
    status = myRFID.AddicoreRFID_Request(PICC_REQIDL, str); 
    if (status == MI_OK)
    {
      startAlarm.disable();
      armAlarmClock();
      break;
    }  
  }
  }
}

void armAlarmClock(){
    alarmEnabled = true;
    currentTime = rtc.getTime();
    alarmTime.hour = alarmHour;
    alarmTime.min = alarmMinute;    
    alarmDelta = alarmTime.hour*60 + alarmTime.min;
    currentDelta = currentTime.hour*60 + currentTime.min;
    if (alarmDelta>currentDelta){delayDeltaMS = alarmDelta-currentDelta;}
    else {delayDeltaMS = 1440 - (currentDelta-alarmDelta);}
    updateAlarmString();
    lcd.clear();
    lcd.print(String(delayDeltaMS/60)+"H "+String(delayDeltaMS%60)+"M");
    lcd.setCursor(0,1);
    lcd.print("Remain");
    lcd.setCursor(0,0);
    delayDeltaMS = (delayDeltaMS*60-currentTime.sec)*1000;
    startAlarm.enableDelayed(delayDeltaMS);
    delay(1500); //Time until next alarm stays on screen for 1.5 seconds after being set
}

Scheduler mainLoop;

void setup()   {
//// INITIALIZE RTC - RUN ONCE ///
//  Serial.begin(9600);
//  rtc.halt(false);
//  rtc.setTCR(TCR_OFF); //D1R2K);

//  rtc.writeProtect(false);
//  rtc.setDOW(SATURDAY);        // Set Day-of-Week to FRIDAY
//  rtc.setDate(11, 1, 2016);   // Set the date to August 6th, 2010
//  rtc.setTime(23, 21, 00);     // Set the time to 12:00:00 (24hr format)


//  rtc.writeProtect(true);
//  rtc.setTCR(TCR_OFF); Optionally disable battery trickle charge
  
  pinMode(9, OUTPUT);
  pinMode(button0Pin, INPUT_PULLUP);
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  
  beep(50);
  beep(50);
  delay(250);
  lcd.begin(16,2);   // initialize the lcd for 16 chars 2 lines, turn on backlight
//  lcd.noBacklight();

  // start the SPI library:
  SPI.begin();  
  pinMode(chipSelectPin,OUTPUT);            // Set digital pin 10 as OUTPUT to connect it to the RFID /ENABLE pin 
  digitalWrite(chipSelectPin, LOW);         // Activate the RFID reader
  pinMode(NRSTPD,OUTPUT);                   // Set digital pin 10 , Not Reset and Power-down
  digitalWrite(NRSTPD, HIGH);
  myRFID.AddicoreRFID_Init();   

  mainLoop.init();
  mainLoop.addTask(displayDateTime);
  mainLoop.addTask(displayMenu);
  mainLoop.addTask(startAlarm);
  displayDateTime.enable();
}


void loop() {
  mainLoop.execute();
  displayMenu.disable();  
  
  button0State = digitalRead(button0Pin);
  button1State = digitalRead(button1Pin);
  button2State = digitalRead(button2Pin);
  
  if (button0State == LOW) {
    beep(50);
    displayMenu.enable();  }
  
  if (button1State == LOW) {
    if (backlightEnabled == true){
      lcd.noBacklight();
      backlightEnabled = false;
      delay(500);
    } else {
      lcd.backlight();
      backlightEnabled = true;
      delay(500);
    }
  }

  if (button2State == LOW) {
    beep(150);
    if (alarmEnabled){
      alarmEnabled=false;
      startAlarm.disable();
      alarmStringShort = "     ";
    } else {
      alarmEnabled=true;
      armAlarmClock();  }  } 
}
