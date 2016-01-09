#include <DS1302.h>
#include <LiquidCrystal_I2C.h>
DS1302 rtc(5, 6, 7);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);   

void beep(unsigned char delayms){
  analogWrite(9, 20);      // Tone, cannot == (0 || 255)
  delay(delayms);          // wait for a delayms ms
  analogWrite(9, 0);       // 0 turns it off
  delay(delayms);          // wait for a delayms ms   
}  

void displayDateTimeCallback(){
  lcd.setCursor(0,0);
  lcd.print(rtc.getTimeStr(FORMAT_LONG));
  String d_o_w = rtc.getDOWStr();
  String month = rtc.getMonthStr();
  String dateRead = rtc.getDateStr(FORMAT_LITTLEENDIAN);
  String day = dateRead.substring(0,2);
  String dateDraw = d_o_w + " " + month + " " + day;
  lcd.setCursor(0,1);
  lcd.print(dateDraw);  
//  lcd.print(" "+alarmStringShort);
}

void setup(){
  //// INITIALIZE UP RTC - RUN ONCE ///
  rtc.setTCR(TCR_OFF);
  rtc.halt(false);

  rtc.writeProtect(false);
//  rtc.writeProtect(true); // Some modules default to no defined value, 
                            // may need to enable then disable write protect 
                            // before setting clock

  rtc.setTime(12, 22, 00);     // Set the time to 12:00:00 (24hr format).
  rtc.setDate(9, 01, 2015);    // Set the date (DD/MM/YYYY). DD is not zero padded 
  rtc.setDOW(SATURDAY);        // Set Day-of-Week

  pinMode(9, OUTPUT);
  beep(50);
  lcd.begin(16,2);
        
}

void loop(){
  lcd.setCursor(0,0);
  lcd.print(rtc.getTimeStr(FORMAT_LONG));
  delay(1000);
}


