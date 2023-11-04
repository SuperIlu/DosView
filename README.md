# DosView
This is a quick hack for an image viewer/converter for MS-DOS

People on [DOS ain't dead](https://www.bttr-software.de/forum/forum_entry.php?id=20792) asked if it would be possible to create such a program for MS-DOS supporting modern image formats. As I had quite a lot of the necessary code already available from [DOjS](https://github.com/SuperIlu/DOjS) I decided to give it a try.

`DosView` uses Allegro to display the images, it should support all VESA modes that are implemented by it.
The default screen format is 640x480x24, see below for other options.

**Binary downloads are available on the [Releases](https://github.com/SuperIlu/DosView/releases) page.**

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
- TIFF (using the `.TIF` file extension): only first image

### Writing
- BMP
- PCX
- TGA
- QOI
- JPG: quality can be controlled with `-q`.
- PNG
- WEBP (using the `.WEB` file extension): always lossy, quality can be controlled with `-q`.
- TIFF (using the `.TIF` file extension), LZW compression only

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
- `SHIFT`: move/scale 2x as fast
- `CTRL`: move/scale 4x as fast
- `ALT`: move/scale 8x as fast

`SHIFT`, `ALT` and `CTRL` can be used in any combination.

# Changelog
### 1.1 / November 4rd, 2023
* added TIFF
* fixed zoom and documentation
* added 24/32bpp autodetection

### 1.0 / November 3rd, 2023
* first public release
