#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#define STB_IMAGE_STATIC
#define STBI_WINDOWS_UTF8
#include "stb_image.h"

#define BLAWORK_IMPLEMENTATION
#define BLAWORK_IMPL_C11
#include "blawork.h"

static int my_utf8_main(int argc, char ** argv);
#define BLA_WMAIN_FUNC my_utf8_main
#include "blawmain.h"

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
    fprintf(stderr, "%s - load image pixels as 4 8-bit channels and 64-bit fnv1 hash them\n", argv0);
    fprintf(stderr, "Info: blawork_implementation_name() = '%s'\n", blawork_implementation_name());
    fprintf(stderr, "Info: BLA_WMAIN_USING_WMAIN_BOOLEAN = %d\n", BLA_WMAIN_USING_WMAIN_BOOLEAN);
    fprintf(stderr, "Usage: %s input.png...\n", argv0);
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
    int stbiretnull;
    u64 hash;
};

static void domywork(void * ptr)
{
    struct mywork * work = (struct mywork*)ptr;
    unsigned char * pixels;
    int x, y, c;

    pixels = stbi_load(work->fname, &x, &y, &c, 4);
    if(!pixels)
    {
        work->stbiretnull = 1;
        return;
    }

    work->hash = hash_fnv1_64(pixels, 4 * x * y);
    stbi_image_free(pixels);
}

static int my_utf8_main(int argc, char ** argv)
{
    int i, ret;

    if(argc < 2)
        return print_usage(argv[0]);

    struct mywork * works = (struct mywork*)calloc(argc, sizeof(struct mywork));
    if(!works)
    {
        fputs("calloc error for works array\n", stderr);
        return 1;
    }

    for(i = 1; i < argc; ++i)
    {
        works[i].fname = argv[i];
        blawork_begin(&works[i].bwork, domywork, &works[i]);
    }

    ret = 0;
    for(i = 1; i < argc; ++i)
    {
        blawork_end(&works[i].bwork);
        if(works[i].stbiretnull)
        {
            print_file_error("stbi_load returned NULL", works[i].fname);
            ret = 1;
        }
        else
        {
            printf("%016llx %s\n", works[i].hash, works[i].fname);
        }
    }

    free(works);
    return ret;
}
