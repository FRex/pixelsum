#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#define STB_IMAGE_STATIC
#define STBI_WINDOWS_UTF8
#include "stb_image.h"

#define BLAWORK_IMPLEMENTATION
#define BLAWORK_IMPL_WINAPI
#include "blawork.h"

static int my_utf8_main(int argc, char ** argv);
#define BLA_WMAIN_FUNC my_utf8_main
#include "blawmain.h"

#define BLASHA1_IMPLEMENTATION
#include "blasha1.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char * filepath_to_filename(const char * path)
{
    size_t i, len, lastslash;

    len = strlen(path);
    lastslash = 0;
    for(i = 0; i < len; ++i)
        if(path[i] == '/' || path[i] == '\\')
            lastslash = i + 1;

    return path + lastslash;
}

static int print_usage(const char * argv0)
{
    argv0 = filepath_to_filename(argv0);
    fprintf(stderr, "%s - load image pixels as 4 8-bit channels and calculate their sha1 and 64-bit fnv1\n", argv0);
    fprintf(stderr, "    Info : blawork_implementation_name() = '%s'\n", blawork_implementation_name());
    fprintf(stderr, "    Info : BLA_WMAIN_USING_WMAIN_BOOLEAN = %d\n", BLA_WMAIN_USING_WMAIN_BOOLEAN);
    fprintf(stderr, "    Info : %d-bit executable\n", 8 * (int)sizeof(void*));
    fprintf(stderr, "    Usage: %s [options..] input.png...\n", argv0);
    fprintf(stderr, "       --space - separate fnv1 and sha1 in output by space (default = one hex string)\n");
    fprintf(stderr, "       --alpha - consider pixels with alpha = 0 to be equal no matter their RGB\n");
    fprintf(stderr, "       --count - count distinct RGB values\n");
    return 1;
}

static int print_file_error(const char * msg, const char * filename)
{
    fprintf(stderr, "%s for '%s'\n", msg, filename);
    return 1;
}

typedef unsigned long long u64;

static u64 hash_fnv1_64(unsigned char * data, int datalen)
{
    u64 ret;
    int i;

    ret = 14695981039346656037ull;
    for(i = 0; i < datalen; ++i)
    {
        ret = ret * 1099511628211ull;
        ret = ret ^ data[i];
    }

    return ret;
}

struct mywork {
    blawork_t bwork;
    const char * fname;
    int alpha0same;
    int stbiretnull;
    int countrgb;
    u64 hash;
    char sha1hash[41];
    int rgbcount;

    /* bit is set if that RGB is present */
#define RGBMAP_SIZE ((1 << 24) / 32)
    unsigned rgbmap[RGBMAP_SIZE];
};

static int populationCount(unsigned x)
{
    /* this (hopefully) generates a pop count instruction */
    x = (x - ((x >> 1) & 0x55555555));
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0f0f0f0f;
    return (x * 0x01010101) >> 24;
}

static void domywork(void * ptr)
{
    struct mywork * work = (struct mywork*)ptr;
    unsigned char * pixels;
    int x, y, c, i;

    pixels = stbi_load(work->fname, &x, &y, &c, 4);
    if(!pixels)
    {
        work->stbiretnull = 1;
        return;
    }

    if(work->alpha0same)
    {
        for(i = 3; i < 4 * x * y; i += 4)
        {
            if(pixels[i] == 0)
            {
                pixels[i - 3] = 0;
                pixels[i - 2] = 0;
                pixels[i - 1] = 0;
            } /* if pixel.a is 0 */
        } /* for */
    } /* if work alpha rgb zero */

    if(work->countrgb)
    {
        int rgb;
        for(i = 0; i < 4 * x * y; i += 4)
        {
            rgb = (pixels[i + 0] << 16) + (pixels[i + 1] << 8) + pixels[i + 2];
            work->rgbmap[rgb / 32] |= 1u << (rgb % 32);
        } /* for i */

        work->rgbcount = 0;
        for(i = 0; i < RGBMAP_SIZE; ++i)
            work->rgbcount += populationCount(work->rgbmap[i]);
    }

    work->hash = hash_fnv1_64(pixels, 4 * x * y);
    blasha1_text(pixels, 4 * x * y, work->sha1hash);
    stbi_image_free(pixels);
}

static int samestring(const char * a, const char * b)
{
    return 0 == strcmp(a, b);
}

static int isoption(const char * a)
{
    return samestring(a, "--space") || samestring(a, "--alpha") || samestring(a, "--count");
}

static int my_utf8_main(int argc, char ** argv)
{
    int i, ret, insertspace, alpha0same, countrgb;

    insertspace = 0;
    alpha0same = 0;
    countrgb = 0;

    for(i = 1; i < argc; ++i)
    {
        if(samestring(argv[i], "--space"))
            insertspace = 1;

        if(samestring(argv[i], "--alpha"))
            alpha0same = 1;

        if(samestring(argv[i], "--count"))
            countrgb = 1;
    } /* for i */

    struct mywork * works = (struct mywork*)calloc(argc, sizeof(struct mywork));
    if(!works)
    {
        fputs("calloc error for works array\n", stderr);
        return 1;
    }

    ret = 1;
    for(i = 1; i < argc; ++i)
    {
        if(isoption(argv[i]))
            continue;

        ret = 0;
        works[i].fname = argv[i];
        works[i].alpha0same = alpha0same;
        works[i].countrgb = countrgb;
        blawork_begin(&works[i].bwork, domywork, &works[i]);
    }

    if(ret != 0)
    {
        free(works);
        return print_usage(argv[0]);
    }

    for(i = 1; i < argc; ++i)
    {
        if(isoption(argv[i]))
            continue;

        blawork_end(&works[i].bwork);
        if(works[i].stbiretnull)
        {
            print_file_error("stbi_load returned NULL", works[i].fname);
            ret = 1;
        }
        else
        {
            printf("%016llx%s%s %s",
                works[i].hash,
                insertspace ? " " : "",
                works[i].sha1hash,
                works[i].fname
            );

            if(works[i].countrgb)
                printf(" %d colors", works[i].rgbcount);

            puts("");
        }
    }

    free(works);
    return ret;
}
