# Miniproject 0: Coin Accepter

## Setup

This particular repository makes use of the PIC24FJ128GB206 microcontroller and [Microchip’s xc16 compiler](http://www.microchip.com/compilers). In addition, we used SCons as a Python-based build tool in order to convert the .c files for use on the microcontroller and in the compiler. 

To set up the hardware, the coin acceptor should be plugged into a 12-volt power supply via its power and ground lines. The signal line will be plugged into a digital input pin, and supplied with a pull-up resistor - for our purposes, we used a 20k resistor - to ensure that the reading is accurate. Once this hardware is in place, you can then run the project. The particular pin used in this code is D0.

Once the repository is cloned onto the local machine, run the “scons” command in your terminal to create the hex file necessary. Then, navigate to /bootloader/software, and run bootloadergui.py. Connect to the board and import the hex file that was created by SCons, and then write it to the microcontroller. After pressing Disconnect/Run, the red LED should blink when a coin is inserted into the accepter. The steps necessary are listed below as well:

1. Navigate to project folder
2. Run the command **scons** or **scons3** to create the hex file
3. Navigate to **/bootloader/software** and run **bootloadergui.py**
4. Import the hex file created by Scons and connect the board
5. Press the **Write** button to write to the board
6. Select **Disconnect/Run** to run the program
