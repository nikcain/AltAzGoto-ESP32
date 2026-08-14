# AltAzGoto-ESP32

This is a project to control my 10 inch Dobsonian telescope with stepper motors. An initial version using an Arduino Uno (at https://github.com/nikcain/AltAzGoto), worked fairly well, but was a bit cumbersome with the controls (the IR detector was a bit iffy, and you had to set the date in every time you used it). So this version will use an ESP32 dev kit (specifically an ESP-WROOM-32 board), which has the advantage of wi-fi - the biggest of which is it creates a soft access point. The board will create a wifi access point called Astro. Connect the phone to Astro with the password in AppConnect.cpp, and it's a direct connection between the phone and ESP32.

This means I can push a lot of the work to an external source (an android app), and this app will only do the following:
* Receive RA and Dec goto locations
* Slew to target, and then track
* Allow control to move, and also apply movement as a correction to where the scope thinks it's pointing

So, a lot simpler! Although that doesn't mean the complexity has disappeared, it's just gone to the android app, which is here https://github.com/nikcain/AltAzESP32-AndroidController
