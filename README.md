# DosView
This is a quick hack for an image viewer/converter for MS-DOS

People on [DOS ain't dead](https://www.bttr-software.de/forum/forum_entry.php?id=20792) asked if it would be possible to create such a program for MS-DOS supporting modern image formats. As I had quite a lot of the necessary code already available from [DOjS](https://github.com/SuperIlu/DOjS) I decided to give it a try.

`DosView` uses Allegro to display the images, it should support all VESA modes that are implemented by it.
It tries 32bpp first and 24bpp is 32bpp is not available. The default screen format is 640x480, see below for other options.

Please note: Although `DosView` should work from a i386 upwards, this programm can eat huge amounts of RAM (>64MiB) if you feed it large images and/or screen sizes.

**Binary downloads are available on the [Releases](https://github.com/SuperIlu/DosView/releases) page.**

The source is on [GitHub](https://github.com/SuperIlu/DosView/)

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
- JPEG 2000 (using the `.JP2` file extension)

### Writing
- BMP
- PCX
- TGA
- QOI
- JPG: quality can be controlled with `-q`.
- PNG
- WEBP (using the `.WEB` file extension): always lossy, quality can be controlled with `-q`.
- TIFF (using the `.TIF` file extension), LZW compression only
- JPEG 2000 (using the `.JP2` file extension)

## Command line arguments
```
Usage:
  DOSVIEW.EXE [-hbl] [-q <quality>] [-w <width>] [-s <outfile>] <infile>
  -h           : show this screen
  -l           : list know screen modes
  -w <width>   : screen width to use.
  -s <outfile> : do not show the image, save it to outfile instead.
  -q <quality> : Quality for writing JPEG and WEPB image (1..100). Default: 95
```

## Keys
- `ESC`/`Q`: quit
- `F`: show actual size
- `Z`: fit to screen
- `I`: toggle image info
- `PAGE UP`/`9`: increase zoom
- `PAGE DOWN`/`3`: decrease zoom
- `UP`/`8`: move image up
- `DOWN`/`2`: move image down
- `LEFT`/`4`: move image left
- `RIGHT`/`6`: move image right
- `SHIFT`: move/scale 2x as fast
- `CTRL`: move/scale 4x as fast
- `ALT`: move/scale 8x as fast

`SHIFT`, `ALT` and `CTRL` can be used in any combination.

# LICENSE
Please see the attached [LICENSE](LICENSE) file for the license of all involved libraries/files.

# Known problems
* error handling is a mess
* code is badly documentes
* can be very slow on old machines
* eats HUGE amounts of memory
* if loading/saving fails you get no info why (if you are not running a debug build that is)
* image conversion always needs a working display mode (Allegros fault)

# Changelog
### 1.1 / November 4rd, 2023
* added TIFF
* added JPEG 2000
* fixed zoom and documentation
* added 24/32bpp autodetection
* added image info
* added num pad keys
* added check to list modes

### 1.0 / November 3rd, 2023
* first public release
