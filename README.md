![build](https://github.com/frno7/gemini/actions/workflows/build.yml/badge.svg)

![Install desktop icon](https://raw.githubusercontent.com/frno7/gemini/main/doc/tos206uk-05.png)
![Set color and style](https://raw.githubusercontent.com/frno7/gemini/main/doc/tos206uk-13.png)

Gemini is the modern systems desktop twin of
[GEM](https://en.wikipedia.org/wiki/GEM_(desktop_environment))
for
[Atari TOS](https://en.wikipedia.org/wiki/Atari_TOS). The intention
is to adapt GEM to the
[X Window System](https://en.wikipedia.org/wiki/X_Window_System),
plain [framebuffers](https://en.wikipedia.org/wiki/Framebuffer),
and possibly other kinds of video display.

The Gemini library implements parts of the application environment
services (AES) and the virtual device interface (VDI) of GEM, to eventually
become compatible with a small subset of
[TOS/libc](https://github.com/frno7/toslibc). A carefully designed
GEM application would then be possible to have for both the Atari ST and
modern systems such as Linux.

The Gemini AES reference implementation draws objects by
[ray casting](https://en.wikipedia.org/wiki/Ray_casting) every
[pixel](https://en.wikipedia.org/wiki/Pixel). This method is somewhat slow
without optimisations, but there are several advantages:

- significantly simplified code, compared to
  [bit blit](https://en.wikipedia.org/wiki/Bit_blit) and similar
  traditional methods;
- small and constant
  [memory footprint](https://en.wikipedia.org/wiki/Memory_footprint)
  regardless of video resolution;
- [flicker-free](https://en.wikipedia.org/wiki/Flicker-free) video updates
  without [double buffering](https://en.wikipedia.org/wiki/Multiple_buffering)
  are possible, because no intermediate drawing is necessary;
- [write-only memory](https://en.wikipedia.org/wiki/Write-only_memory_(engineering))
  can be used for the video display;
- finally, drawing is
  [pleasingly parallel](https://en.wikipedia.org/wiki/Embarrassingly_parallel),
  because all pixels are independent of one another, so modern
  [multiprocessing](https://en.wikipedia.org/wiki/multiprocessing)
  hardware can be very effective.

The `--draw` option to the `rsc` tool demonstrates the ray casting method
by producing a multipart [TIFF](https://en.wikipedia.org/wiki/TIFF) image
file, having drawings of each object tree in the given RSC file.
The `make tiff` command generates `test/tos206se.tiff` and `test/tos206uk.tiff`.
The `make png` commands splits those TIFF images into separate
[PNG](https://en.wikipedia.org/wiki/Portable_Network_Graphics) image files
using [Image Magick](https://en.wikipedia.org/wiki/ImageMagick).

```
Usage: rsc [options]... <RSC-file>

Displays Atari TOS GEM resource (RSC) file header, strings, images, icons,
objects, and other details, as text on standard output.

Options:

    -h, --help            display this help and exit
    --version             display version and exit

    --identify            exit sucessfully if the file is a valid RSC
    --diagnostic          display diagnostic warnings and errors
    --encoding <original|utf-8>
                          display text with original or UTF-8 encoding;
                          default is UTF-8
    --map                 display RSC map

    --draw                draw RSC forms and dialogues as images
    -o, --output <path>   save images as a multipart TIFF file
```

```
Usage: fnt [options]... <FNT-file>

Displays Atari TOS GEM font (FNT) file header and character set, as text
on standard output.

Options:

    -h, --help            display this help and exit
    --version             display version and exit

    --identify            exit sucessfully if the file is a valid FNT
    --diagnostic          display diagnostic warnings and errors
```
