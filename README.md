# DosView
This is a quick hack for an image viewer/converter for MS-DOS

People on [DOS ain't dead](https://www.bttr-software.de/forum/forum_entry.php?id=20792) asked if it would be possible to create such a program for MS-DOS supporting modern image formats. As I had quite a lot of the necessary code already available from [DOjS](https://github.com/SuperIlu/DOjS) I decided to give it a try.

`DosView` uses Allegro to display the images, it should support all VESA modes that are implamanted by it.
The default screen format is 640x480x24, see below for other options.

# Usage

## Supported formats
### Reading
- BMP
- PCX
- TGA
- LBM
- QOI
- JPG
- PNG
- WEBP (using the `.WEB` file extension)

### Writing
- BMP
- PCX
- TGA
- QOI
- JPG
- PNG
- WEBP (using the `.WEB` file extension)

## Command line arguments
```
Usage:
  DOSVIEW.EXE [-hbl] [-q <quality>] [-w <width>] [-s <outfile>] <infile>
  -h           : show this screen
  -l           : list know screen modes
  -b           : try 32bpp screen mode instead of 24bpp
  -w <width>   : screen width to use.
  -s <outfile> : do not show the image, save it to outfile instead.
  -q <quality> : Quality for writing JPEG and WEPB image (1..100).
```

## Keys
- `ESC`/`Q`: quit
- `F`: show actual size
- `Z`: fit to screen
- `PAGE UP`: increase zoom
- `PAGE DOWN`: decrease zoom
- `UP`/`DOWN`/`LEFT`/`RIGHT`: move image

# Changelog
### 1.0 / November 3rd, 2023
* first public release
