# pixelsum

Small C program to load image using [stb](https://github.com/nothings/stb) and
then hash its pixels with 64 bit [fnv1](http://www.isthe.com/chongo/tech/comp/fnv/).

See releases on this repo to get a Windows exe made with Pelles C with no `-O2`.

**Warning**: when using Pelles C with `-O2` a certain operation in `stb_image`
(average of two bytes in code handling average filter in png decoding) will
produce incorrect result which will cause some png images to be badly corrupted:
[Pelles C forum bug report](https://forum.pellesc.de/index.php?topic=7837.0).

It'll force `stb_image` to load 4 8-bit channels to ensure that two images that
have same pixels but encoded differently (e.g. one has no alpha channel and
other has one but it's always 255) hash to same value.

It might be useful to verify that some transformation of an image file has not
lost any pixel data. E.g. running [FRex/topng](https://github.com/FRex/topng) on
a bmp or jpeg, running [optipng](http://optipng.sourceforge.net/) on a png file, etc.

```
$ pixelsum.exe
pixelsum.exe - load image pixels as 4 8-bit channels and 64-bit fnv1 hash them
Usage: pixelsum.exe input.png...
```

```
$ pixelsum.exe img.png
img.png: 9560B80D56ACAE1C
```

`test.jpg` is the original, `test.png` was converted using `topng`, `optd.png`
was optimized using `optipng -o7`, `test.bmp` was resaved using Paint.

```
$ pixelsum test.bmp test.png test.jpg optd.png
test.bmp: DED4BADA98B7346C
test.png: DED4BADA98B7346C
test.jpg: DED4BADA98B7346C
optd.png: DED4BADA98B7346C
```
