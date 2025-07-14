/**
 * @file inih.c
 * @brief 简易INI文件解析器的核心头文件
 * 
 * @copyright Copyright (C) 2009-2025, Ben Hoyt
 * @license SPDX-License-Identifier: BSD-3-Clause
 * 
 * inih（INI file parser）是一个轻量级的INI文件解析库，支持标准INI格式的解析，
 * 并提供灵活的配置选项（如多行值、注释处理、自定义内存分配等）。
 * 
 * 项目主页：https://github.com/benhoyt/inih
 * 详细许可信息参见LICENSE.txt文件。
 */

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "ini.h"

#if !INI_USE_STACK
#if INI_CUSTOM_ALLOCATOR
#include <stddef.h>
void* ini_malloc(size_t size);
void ini_free(void* ptr);
void* ini_realloc(void* ptr, size_t size);
#else
#include <stdlib.h>
#define ini_malloc malloc
#define ini_free free
#define ini_realloc realloc
#endif
#endif

#define MAX_SECTION 50
#define MAX_NAME 50

// 用于跟踪字符串解析状态
typedef struct 
{
    const char* ptr;
    size_t num_left;
} ini_parse_string_ctx;

// 将空白字符从给定字符串的末尾删除，then Return s. 
static char* ini_rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

// 返回指向给定字符串中第一个非空格字符的指针
static char* ini_lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s ++ ;
    return (char*)s;
}

// 返回指向给定字符串中第一个字符（或多个字符）或内联注释的指针，
// 或者，如果两者都找不到，则指向字符串末尾的NUL。内联注释必须
// 前缀为空白字符以注册为注释
static char* ini_find_chars_or_comment(const char* s, const char* chars)
{
#if INI_ALLOW_INLINE_COMMENTS
    int was_space = 0;
    while (*s && (!chars || !strchr(chars, *s)) && !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) 
    {
        was_space = isspace((unsigned char)(*s));
        s++;
    }
#else
    while (*s && (!chars || !strchr(chars, *s))) 
    {
        s ++ ;
    }
#endif
    return (char*)s;
}

// 类似于strncpy，但确保dest（大小字节）为NUL终止，并且不使用NUL填充
static char* ini_strncpy0(char* dest, const char* src, size_t size)
{
    /* Could use strncpy internally, but it causes gcc warnings (see issue #91) */
    size_t i;
    for (i = 0; i < size - 1 && src[i]; i++)
        dest[i] = src[i];
    dest[i] = '\0';
    return dest;
}


int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
                     void* user)
{
    // 是否使用栈
#if INI_USE_STACK
    char line[INI_MAX_LINE];
    size_t max_line = INI_MAX_LINE;
#else
    char* line;
    size_t max_line = INI_INITIAL_ALLOC;
#endif

    // 是否允许动态扩展行缓冲区
#if INI_ALLOW_REALLOC && !INI_USE_STACK
    char* new_line;
#endif

    char section[MAX_SECTION] = "";

    // 是否允许多行值解析
#if INI_ALLOW_MULTILINE
    char prev_name[MAX_NAME] = "";
#endif

    size_t offset;
    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;
    char abyss[16];  /* Used to consume input when a line is too long. */

#if !INI_USE_STACK
    line = (char*)ini_malloc(INI_INITIAL_ALLOC);
    if (!line) 
    {
        return -2;
    }
#endif

    // 回调函数原型
#if INI_HANDLER_LINENO
#define HANDLER(u, s, n, v) handler(u, s, n, v, lineno)
#else
#define HANDLER(u, s, n, v) handler(u, s, n, v)
#endif


    while (reader(line, (int)max_line, stream) != NULL) 
    {
        offset = strlen(line);

#if INI_ALLOW_REALLOC && !INI_USE_STACK
        while (offset == max_line - 1 && line[offset - 1] != '\n') 
        {
            max_line *= 2;
            if (max_line > INI_MAX_LINE)
                max_line = INI_MAX_LINE;

            new_line = ini_realloc(line, max_line);
            if (!new_line) 
            {
                ini_free(line);
                return -2;
            }
            line = new_line;
            if (reader(line + offset, (int)(max_line - offset), stream) == NULL)
                break;
            offset += strlen(line + offset);
            if (max_line >= INI_MAX_LINE)
                break;
        }
#endif

        lineno ++ ;

        // 如果行超过INI_MAX_line字节，则丢弃，直到行结束
        if (offset == max_line - 1 && line[offset - 1] != '\n') 
        {
            while (reader(abyss, sizeof(abyss), stream) != NULL) 
            {
                if (!error)
                    error = lineno;
                if (abyss[strlen(abyss) - 1] == '\n')
                    break;
            }
        }

        start = line;
#if INI_ALLOW_BOM
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF)
        {
            start += 3;
        }
#endif
        start = ini_rstrip(ini_lskip(start));

        if (strchr(INI_START_COMMENT_PREFIXES, *start)) 
        {
            // 行首注释
        }

#if INI_ALLOW_MULTILINE
        else if (*prev_name && *start && start > line) 
        {
#if INI_ALLOW_INLINE_COMMENTS
            end = ini_find_chars_or_comment(start, NULL);
            if (*end)
                *end = '\0';
            ini_rstrip(start);
#endif
            // 带有前导空格的非空行，视为续行
            if (!HANDLER(user, section, prev_name, start) && !error)
                error = lineno;
        }
#endif
        else if (*start == '[') 
        {
            // 找到新的一section
            end = ini_find_chars_or_comment(start + 1, "]");
            if (*end == ']') 
            {
                *end = '\0';
                ini_strncpy0(section, start + 1, sizeof(section));
#if INI_ALLOW_MULTILINE
                *prev_name = '\0';
#endif
#if INI_CALL_HANDLER_ON_NEW_SECTION
                if (!HANDLER(user, section, NULL, NULL) && !error)
                    error = lineno;
#endif
            }
            else if (!error) 
            {
                // 这个新的section没有以]结尾
                error = lineno;
            }
        }
        else if (*start) 
        {
            // 不是注释的话，必须是name=value或者name:value
            end = ini_find_chars_or_comment(start, "=:");
            if (*end == '=' || *end == ':') 
            {
                *end = '\0';
                name = ini_rstrip(start);
                value = end + 1;
#if INI_ALLOW_INLINE_COMMENTS
                end = ini_find_chars_or_comment(value, NULL);
                if (*end)
                    *end = '\0';
#endif
                value = ini_lskip(value);
                ini_rstrip(value);

#if INI_ALLOW_MULTILINE
                ini_strncpy0(prev_name, name, sizeof(prev_name));
#endif
                // 调用回调
                if (!HANDLER(user, section, name, value) && !error)
                    error = lineno;
            }
            else 
            {
                /// 没有找到=或者:
                // 是否允许无值键，不允许则视为错误
#if INI_ALLOW_NO_VALUE
                *end = '\0';
                name = ini_rstrip(start);
                if (!HANDLER(user, section, name, NULL) && !error)
                    error = lineno;
#else
                if (!error)
                    error = lineno;
#endif
            }
        }

        // 是否在遇到首个错误时停止解析
#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

#if !INI_USE_STACK
    ini_free(line);
#endif

    return error;
}


int ini_parse_file(FILE* file, ini_handler handler, void* user)
{
    return ini_parse_stream((ini_reader)fgets, file, handler, user);
}


int ini_parse(const char* filename, ini_handler handler, void* user)
{
    FILE* file;
    int error;

    file = fopen(filename, "r");
    if (!file)
    {
        printf("open failed\n");
        perror(filename);
        return -1;
    }
    error = ini_parse_file(file, handler, user);
    fclose(file);
    return error;
}

// 用于从字符串缓冲区读取下一行。这是ini_parse_string（）使用的fgets（）等效函数
static char* ini_reader_string(char* str, int num, void* stream) 
{
    ini_parse_string_ctx* ctx = (ini_parse_string_ctx*)stream;
    const char* ctx_ptr = ctx->ptr;
    size_t ctx_num_left = ctx->num_left;
    char* strp = str;
    char c;

    if (ctx_num_left == 0 || num < 2)
        return NULL;

    while (num > 1 && ctx_num_left != 0) 
    {
        c = *ctx_ptr ++ ;
        ctx_num_left -- ;
        *strp ++ = c;
        if (c == '\n')
            break;
        num -- ;
    }

    *strp = '\0';
    ctx->ptr = ctx_ptr;
    ctx->num_left = ctx_num_left;
    return str;
}

int ini_parse_string(const char* string, ini_handler handler, void* user) 
{
    return ini_parse_string_length(string, strlen(string), handler, user);
}

int ini_parse_string_length(const char* string, size_t length, ini_handler handler, void* user) 
{
    ini_parse_string_ctx ctx;

    ctx.ptr = string;
    ctx.num_left = length;
    return ini_parse_stream((ini_reader)ini_reader_string, &ctx, handler, user);
}
