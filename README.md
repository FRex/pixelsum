# pixelsum

Small C program to load image using [stb](https://github.com/nothings/stb) and
then hash its pixels with 64 bit [fnv1](http://www.isthe.com/chongo/tech/comp/fnv/)
and sha1 using [my sha1 lib](https://github.com/FRex/blasha1).

Optionally uses 1 thread per file via [blawork](https://github.com/FRex/blawork).

See releases on this repo to get a 32-bit unoptimized Windows exe made with
Pelles C or a 64-bit optimized Windows exe made with GCC.

If using a thread per file, it might run out of memory in 32-bit version or
use a lot of memory (few gigs) in 64-bit version when handling many big files.

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
$ pixelsum
pixelsum.exe - load image pixels as 4 8-bit channels and calculate their sha1 and 64-bit fnv1
    Info : blawork_implementation_name() = 'WINAPI'
    Info : BLA_WMAIN_USING_WMAIN_BOOLEAN = 1
    Info : 64-bit executable
    Usage: pixelsum.exe [options..] input.png...
       --space - separate fnv1 and sha1 in output by space (default = one hex string)
       --alpha - consider pixels with alpha = 0 to be equal no matter their RGB
       --count - count distinct RGB values for shifts from 0 to 3 (inclusive)
```

```
$ pixelsum.exe img.png
9560b80d56acae1c8691c71c2024eef29d512c960af241c57ad295a0 img.png
```

Option `--count` counts distinct RGB colors present in the image plus shifts (to show if colors differ only in low bits).
```
$ pixelsum --count img.png
9560b80d56acae1c8691c71c2024eef29d512c960af241c57ad295a0 img.png colors: 65172 45645 13408 3388
```

`test.jpg` is the original, `test.png` was converted using `topng`, `optd.png`
was optimized using `optipng -o7`, `test.bmp` was resaved using Paint.

```
$ pixelsum test.bmp test.png test.jpg optd.png
ded4bada98b7346c7dcd6957d00d30fca8b6491032aaba0429776224 test.bmp
ded4bada98b7346c7dcd6957d00d30fca8b6491032aaba0429776224 test.png
ded4bada98b7346c7dcd6957d00d30fca8b6491032aaba0429776224 test.jpg
ded4bada98b7346c7dcd6957d00d30fca8b6491032aaba0429776224 optd.png

$ sha1sum test.bmp test.png test.jpg optd.png
c125c1b89b5fe88eea200afc86cd3a1043096223 *test.bmp
330def315e450a1f728119e0c643cfea1d93a7de *test.png
369088150cfe4e86667233d4046dafdd941618ca *test.jpg
4bdfe5fe8acc19c8d9cb5a8c80572c4146d0394c *optd.png
```

Option `--space` can help separet visually the 64-bit fnv-1 and sha1 parts.
E.g. with pngs derived from PDFs from [shattered.io](https://shattered.io/)
which have different pixels with same sha1 but different fnv1.
```
$ pixelsum shattered-png-?.png | colors
cad823d59e013cdff92d74e3874587aaf443d1db961d4e26dde13e9c shattered-png-1.png
00607af98be22fe7f92d74e3874587aaf443d1db961d4e26dde13e9c shattered-png-2.png

$ pixelsum --space shattered-png-?.png | colors
cad823d59e013cdf f92d74e3874587aaf443d1db961d4e26dde13e9c shattered-png-1.png
00607af98be22fe7 f92d74e3874587aaf443d1db961d4e26dde13e9c shattered-png-2.png
```

Option `--alpha` makes pixels with alpha equal to 0 compare as equal, even if
their RGB values are different (the RGB is set to all 0s). Running with this
option after first running without it allows detecting files that have
transparent pixels that have non-0 R or G or B values (their checksum will be
different with this option), and to verify that images are the same except for
the different colors of transparent pixels.
```
$ pixelsum alpha?.png
6cd8e1a0da55c3253fb5222fe18468d1886aeba65c77b02ea6082b5f alpha1.png
2c235a22d301a3255c21549c74311495af972997e19b0dcb2be5e80e alpha2.png

$ pixelsum --alpha alpha?.png
6cd8e1a0da55c3253fb5222fe18468d1886aeba65c77b02ea6082b5f alpha1.png
6cd8e1a0da55c3253fb5222fe18468d1886aeba65c77b02ea6082b5f alpha2.png
```
