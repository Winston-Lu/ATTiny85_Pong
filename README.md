# ATTiny85_Pong

A modification of the classic game "Pong" with some additional features to run on the ATTiny85 using the SSD1306 Library. Running on an ATTiny85-20xx. OLED runs down to 2.2v without much error, and microcontroller can go lower than the rated 2.7v to 2.2v before running into OLED brightness issues.

Wiring of the ATTiny is as follows:
```
               _____
 NC (RST)[1] -|o    |- [5]3v-5v DC
SDA(OLED)[2] -|     |- [6]Button Voltage Divider (A1)
SCK(OLED)[3] -|     |- [7]LED1 (Shoot)
      GND[4] -|_____|- [8]LED0 (Spike)

```
And the voltage divider I used is as follows:
```
VCC __10k_______ 10k_________10k_________10k_____
            |           |           |           |
          Button      Button      Button      Button
A1 _________|___________|___________|___________|
 |
10k
 |
GND
```

In order to save on IO pins, an analog voltage divider has been used. The values set in the code were tested at 5v USB, thus values many change if a different voltage source is used. Uncomment `#define getAnalogStrength` in order to recalibrate the analogRead values; see under Customization for further details.

# Features

## Buttons - Movement and Abilities
The buttons above are for Up, Right, Down, Left respectively; where up and down moves the paddle, left shoots a projectile that can cause the ball to reflect (3s default cooldown), and right causes the ball to "spike" towards the opponent, with a chance of the opponent not being able to block the spike (15s default cooldown). Cooldowns are reset when a side has scored a point. The opponent does not have any of these special abilities.

## Ball Speed
The ball starts at an inital speed at 1px/frame, or about 1px/10ms. Each frame, the ball picks up speed to no limit until a side loses, in which the ball's speed resets to 1. When spiking the ball, the ball's speed increases by 6x, and if reflected, reverts back to the speed the ball should be if the spike were to never happen.

## LED's
The LED's correspond with what abilities are able to be executed. 
- Every 3s since the last use of the ability or the start of the round, the shoot LED should light up, indiciating that you are able to shoot a projectile by pressing the right button. 
- Every 15s since the last use of the ability or the start of the round, the spike LED should light up. Pressing the left button will activate the ability, causing it to slowly flash until the ability has been executed.

## Customization
On the top of the file, serveral defines are set to change how the game is played, and can even customize the sprites that are in the game.
### FrameTime
The minimum amount of milliseconds it takes for the next frame to come up. Changing this value will affect the speed of the game, as it is designed with framerate determining the movement speeds. The actual translation to real-time will be slower, as the calculations and drawings will take up time that is not considered by this constant. However, this can be changed if we used a `millis()` timer instead, but this current version is more than sufficient. In the future, I may convert over as there is enough space to add this feature.
### Height and width
I designed the game with a 128x32 OLED display. If you do decide to change this, you should also consider changing the init sequence for the display in the `setup()` function that correctly fits the display.
### BallDiameter
How big the ball is in px. By default the ball sprite is 8x8, hence the ball is 8px in diameter.
### PaddleLength and PaddleWidth
How long/wide the paddle is. If you change the paddle sprite, you might want to change these values as well.
### ScoreToWin
How many points each side needs in order to win. Current max is 15 because the score is stored in a byte, with 4 bits for the cpu score and 4 bits for the player score.
### AIDifficulty
From a scale of 0% - 100% on how well the cpu is wiling to move towards the y-position of the ball. The AI follows a simple "follow the y-position of the ball" type of movement. Default at 65% is enough for the opponent to make the occasional mistake, but still challenging enough for the player with a beginner level of skill.
### SpikeSpeed
Multiplier for how fast the ball moves when the ball is spiked. Values greater than 2 may cause occasional phasing through the cpu's paddle due to how the hit-detection is implemented, but it can still also bounce off the opponent. Having a bit of RNG from this feature makes it a bit more interesting, like a spike in volleyball having the chance to either end the game or the opponent can recover from the spike.
### SpikeCooldown
How long the spike takes to recharge in frames. This can be disabled by making it a very high value, or be constantly on when set to 0.
### Bullet Speed
Speed in which the bullet travels. Not recommended for the bullet speed to be greater than the ball diameter.
### Bullet Cooldown
How long the spike takes to recharge in frames. This can be changed or disabled by making it a very high value, or constantly avaliable when set to 0. Note: a new bullet can not be created unless the existing one has been reset.

# Additional Notes
The current usage of this sketch is 8076/8192 bytes (98%), leaving only 116 bytes left for additional features. 
The current usage of SRAM is 168/512 bytes (32%), so more variables can be used and expanded on such as score.
The commented out `#define getAnalogStrength` runs after analogRead reads a value on A1 > 10, and instead of running the game, shows the value of `analogRead(A1);`. Comment this line to return to regular game functionality.


# Uploading
I used an Arduino Uno to upload the sketch with this config as well as a 10uF capaciator between reset and ground:
```
            _____
      D10 -|o    |- 5v DC
          -|     |- D13
          -|     |- D12 
      GND -|_____|- D11

```
The two pins (pin 2&3 where the OLED i2c com pins are) can be left connected.
Uploaded using ATTiny85 8MHz internal with Arduino as ISP.
Additional board manager used: 
https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json

Library used can be searched for in the library browser under "ssd1306" by Alexey Dynda, which can also be downloaded here:
https://github.com/lexus2k/ssd1306


# Modification and Usage
Feel free to modify or use any of this code for your own purposes. See LICENSE for details
