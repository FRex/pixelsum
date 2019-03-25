#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#define STB_IMAGE_STATIC
#define STBI_WINDOWS_UTF8
#include "stb_image.h"

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

static int my_utf8_main(int argc, char ** argv)
{
    int i, x, y, c, ret;
    unsigned char * pixels;

    if(argc < 2)
        return print_usage(argv[0]);

    ret = 0;
    for(i = 1; i < argc; ++i)
    {
        pixels = stbi_load(argv[i], &x, &y, &c, 4);
        if(!pixels)
        {
            print_file_error("stbi_load returned NULL", argv[i]);
            ret = 1;
        }
        else
        {
            printf("%s: %016llX\n", argv[i], hash_fnv1_64(pixels, 4 * x * y));
            stbi_image_free(pixels);
        }
    }
    return ret;
}

#ifndef _MSC_VER

int main(int argc, char ** argv)
{
    return my_utf8_main(argc, argv);
}

#else

/* for wcslen */
#include <wchar.h>

int wmain(int argc, wchar_t ** argv)
{
    int i, retcode;
    char ** utf8argv = (char **)calloc(argc + 1, sizeof(char*));
    if(!utf8argv)
    {
        fputs("calloc error in wmain\n", stderr);
        return 1;
    }

    retcode = 0;
    for(i = 0; i < argc; ++i)
    {
        const size_t utf8len = wcslen(argv[i]) * 3 + 10;
        utf8argv[i] = (char*)malloc(utf8len);
        if(!utf8argv[i])
        {
            retcode = 1;
            fputs("malloc error in wmain\n", stderr);
            break;
        }
        stbi_convert_wchar_to_utf8(utf8argv[i], utf8len, argv[i]);
    }

    if(retcode == 0)
        retcode = my_utf8_main(argc, utf8argv);

    for(i = 0; i < argc; ++i)
        free(utf8argv[i]);

    free(utf8argv);
    return retcode;
}

#endif /* _MSC_VER */
