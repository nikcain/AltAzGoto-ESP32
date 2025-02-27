# AltAzGoto-ESP32

This is a project to control my 10 inch Dobsonian telescope with stepper motors. An initial version using an Arduino Uno (at https://github.com/nikcain/AltAzGoto), worked fairly well, but was a bit cumbersome with the controls (the IR detector was a bit iffy, and you had to set the date in every time you used it). So this version will use an ESP32 dev kit (specifically an ESP-WROOM-32 board), which has the advantage of wi-fi. This means I can push a lot of the work to an external source (which will be an android app), and this app will only do the following:
* Receive RA and Dec goto locations
* Slew to target, and then track
* Allow control to move, and also apply movement as a correction to where the scope thinks it's pointing

So, a lot simpler! Although that doesn't mean the complexity has disappeared, it's just gone to the android app (which will appear as a repository soon...)
