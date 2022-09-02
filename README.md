![build](https://github.com/frno7/gemini/actions/workflows/build.yml/badge.svg)

Gemini is the modern systems desktop twin of
[GEM](https://en.wikipedia.org/wiki/GEM_(desktop_environment))
for
[Atari TOS](https://en.wikipedia.org/wiki/Atari_TOS).

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
