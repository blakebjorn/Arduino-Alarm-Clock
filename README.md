# Arduino Alarm Clock

Forces you to leave bed to retrieve an RFID tag in order to disable the alarm.

Requires:
- 1) DS1302 RTC Module (battery optional) + library (http://www.rinkydinkelectronics.com/library.php?id=5)
- 2) RFID-RC522 Module + library (https://www.addicore.com/RFID-AddiKit-p/126.htm)
- 3) LCD Display, 16x2 characters and 4 pin output + Liquid Crystal L2C library (https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads)
- 4) Piezo Buzzer
- 5) 3x Push Buttons

Usage:
- If RTC time needs to be set, uncomment/edit relevant lines () in setup(185-192) and 59-66 in displayDateTimeCallback() if Serial output is desired. Some modules require the write protection to be enabled and disabled first before setting the time. Some modules will not function without being wired to the digital ground instead. 
- Upload main alarm sketch
- Button 1 enters the set alarm page. Button 2 increases hour, button 3 increases minute, pressing button 1 again saves changes and arms  the alarm
- Button 2 turns on the backlight (hold to keep on)
- Button 3 enables/disables the alarm
- When the alarm triggers it will emit 3 beeps at 10 second intervals, afterwards it will beep continually until slept for 60 seconds (button 1) or an RFID emitting device is brought within readable distance.
