# sw
Make any script an executable

Script Wrap can take any script file and make it an .exe

Note: the script you convert MUST be in the same directory as sw.exe. If you use the front end, frontend.exe MUST be in the same
directory as sw.exe

Script Wrap has the following modules:

script.c - contains the source for an executable which can read data appended to it's end, extract it, and then perform a command on
it. script_image_csv.h contains a binary dump of the compiled script.c, which sw.exe uses as a template.

sw.exe - this module takes as an input a script file, the run line for the script, and optionally an icon. It dumps a compiled
script.c and appends the script data to it's end. Also, if an icon input was specified it calls ic.exe to change the image icon.

ic.exe - takes as an input an icon file and an image file and changes it's icon

frontend.exe - GUI frontend made with AutoIt 3

When you "convert" your script to an .exe and run it, the .exe reads the data off of itself, extracts it in the Temp folder, and
runs the script with the run line/interpeter command you have entered. If you run the .exe from cmd, all entered arguments get
transfered to the script. Running <your_wrapped_script>.exe -x will make the .exe extract it's script data in a
<your_wrapped_script>.txt file in the current directory. 

Note: your actual script is appended to the .exe in binary form, so you can use bytecode as well as text scripts. Also simple xor
encryption is used, so if you try to read the hex don't be surprised by the gibberish.

sw.exe and ic.exe can be used independently. 

If you want your script to run in a cmd window, start the run line with "cmd /k" like in the example.
