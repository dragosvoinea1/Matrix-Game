# Matrix Game
This is a representation of a simple game using a 8x8 Matrix, an LCD and a joystick.

<details>
<summary> <h2>GAME INFO</h2> </summary>
<br>

## Short description

This point of the game is to turn off the 10 random LEDs that appear on the matrix as fast as possible.<br> 
You can jump on diagonals, hold the joystick to jump over more leds at once, or even go from one wall to another. Find the best way !

<br>
Here is everything you need to know:  

[Project Requirements](https://github.com/dragosvoinea1/Matrix-Game/files/13707587/Introduction.to.Robotics.Matrix.Project.5p.pdf)

<br>

## Features that do not work

The Highscore and Difficulty tabs are not working.

</details>

<details>

<summary><h2>COMPONENTS USED</h2></summary>

<ul>
  <li> Arduino UNO </li>
  <li> 16x2 LCD </li>
  <li> 8x8 led matrix </li>
  <li> joystick </li>
  <li> 10UF electrolytic capacitor </li>
  <li> 104pF ceramic capacitor </li>
  <li> Wires and resistors as needed </li>
</ul>
  
</details>

<details>

<summary><h2> CODE </h2></summary>

### Libraries used:

<ul>
  <li> LiquidCrystal.h (for LCD control) </li>
  <li> LedControl.h (for matrix control) </li>
  <li> EEPROM.h (for memory) </li>
</ul>


### You can find the code that I used 

### [Here](https://github.com/dragosvoinea1/Matrix-Game/blob/main/proiect_matrice.ino)
  
</details>

<details>

  <summary><h2>MENU</h2></summary>
  
### Main Menu:

<ul>
  <li> Start </li>
  <li> Rules </li>
  <li> Highscore </li>
  <li> Settings </li>
  <li> About </li>
</ul>

### Settings submenu: 

<ul>
  <li> Difficulty </li>
  <li> Brightness (saves to EEPROM) </li>
  <li> Reset Highscore </li>
  <li> Exit </li>
</ul>

<br>
The menu can be controlled with the joystick, ONLY left and right.
</details>

<br>

### How it works: [click here](https://youtu.be/DgvAx-0V3is)

<br>

## Image of the project:
![WhatsApp Image 2023-12-18 at 21 48 54](https://github.com/dragosvoinea1/Matrix-Game/assets/115077134/12f77b10-c157-45b8-a2b9-7e7ddd704e6a)
