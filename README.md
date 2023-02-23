# AI application in game programming

Refer to the book "AI for game developers" - David M. Bourg.

The solution is under construction.

Each project is my implementation of a chapter in this book.

# How to build

Download the repository and [SFML packages](https://www.sfml-dev.org/files/SFML-2.5.1-windows-vc15-32-bit.zip) for Visual Studio.
Extract the package and link it into the properties sheet inside Intercept folder. Details can be found [here](https://www.sfml-dev.org/tutorials/2.5/start-vc.php) 
and [here](https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&ved=2ahUKEwi_4LuZ76v9AhVPrVYBHcrCDxEQFnoECAwQAQ&url=https%3A%2F%2Flearn.microsoft.com%2Fen-us%2Fcpp%2Fbuild%2Fcreate-reusable-property-configurations&usg=AOvVaw2BZzw3KD0h13AnThktIF61).
After that, build the project in debug version.

## 0.0 Intercept

Test project.

## 0.1. Shared library

A header with common functions.

## 1. Bresenham's line drawing

Utilise bresenham's line drawing algorithm to make NPC chase player by sight, i.e go straight up to player, in grid space.

## 2. Steering

Control NPC to follow player by sight. Press Enter to switch to intercept mode, at which the NPC try to guess the next collision point and block the way. 
Control: Arrow keys.

## 3. Pattern & Pattern Physics

Implement a way to pre-define patterns of NPC through a PatternManager class. Can be extended to scripting the patterns beforehand. There are two versions: grid and continous.
