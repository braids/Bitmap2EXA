# Bitmap2EXA
Generate EXA sprites from a monochrome BMP. 

This is for use with the Redshift game console in EXAPUNKS.

## Setup
Open the solution up in Visual C++ and compile.

To change the default image used when running Bitmap2EXA in Visual Studio, open the Bitmap2EXA project properties and change the first argument in "Configuration Properties > Debugging > Command Arguments" to the file path of your choice.

### BMP Files
Draw your desired sprite image in black and white, then save the image as a Monochrome BMP. Pixels drawn as white will be converted into DATA lines and GP draw instructions.

## Usage
Runs via command line.

Bitmap2EXA.exe [Absolute path to file] [X position] [Y position]

### Arguments
- Absolute path to file, ex: C:\Bitmap2EXA\image\exatest.bmp
- (Optional) Initial X coordinate of sprite in Redshift. Defaults to 0 if not specified. Values from 0 - 120 recommended.
- (Optional) Initial Y coordinate of sprite in Redshift. Defaults to 0 if not specified. Values from 0 - 100 recommended.

Example:
Bitmap2EXA.exe C:\Bitmap2EXA\image\exatest.bmp 50 50
