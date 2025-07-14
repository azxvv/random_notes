/**
 * @file inih.h
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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/**
 * @def INI_HANDLER_LINENO
 * @brief 控制回调函数是否接收行号参数
 * 
 * 若为非零值，ini_handler回调函数原型将包含`lineno`参数（表示当前解析行的行号）；
 * 若为0，回调函数不包含行号参数。
 * 
 * @note 该宏会影响ini_handler的函数指针类型定义，使用时需保持一致。
 * @default 0（不传递行号）
 */
#ifndef INI_HANDLER_LINENO
#define INI_HANDLER_LINENO 0
#endif

/**
 * @def INI_API
 * @brief 控制函数/符号的可见性（主要用于跨平台动态库编译）
 * 
 * 根据编译环境自动定义符号可见性属性：
 * - 在Windows或Cygwin环境下，若编译共享库（INI_SHARED_LIB），则：
 *   - 编译库时定义为`__declspec(dllexport)`（导出符号）
 *   - 使用库时定义为`__declspec(dllimport)`（导入符号）
 * - 在GCC 4及以上版本中，定义为`__attribute__ ((visibility ("default")))`
 * - 其他情况默认不添加可见性修饰符
 * 
 * @note 主要用于生成或使用动态链接库（DLL）时确保符号正确导出/导入。
 * 
明确指定一个符号具有 "默认" 可见性，即该符号会被导出到共享库的符号表中，可供外部程序使用。这个属性通常与编译选项 -fvisibility=hidden 配合使用：
-fvisibility=hidden 选项会将所有符号的默认可见性设为隐藏（即不导出到符号表）。
然后，通过 __attribute__ ((visibility ("default"))) 显式地将需要公开的符号标记为可见。
 */
#ifndef INI_API
#if defined _WIN32 || defined __CYGWIN__
#	ifdef INI_SHARED_LIB
#		ifdef INI_SHARED_LIB_BUILDING
#			define INI_API __declspec(dllexport)
#		else
#			define INI_API __declspec(dllimport)
#		endif
#	else
#		define INI_API
#	endif
#else
#	if defined(__GNUC__) && __GNUC__ >= 4
#		define INI_API __attribute__ ((visibility ("default")))
#	else
#		define INI_API
#	endif
#endif
#endif

/**
 * @typedef ini_handler
 * @brief 回调函数原型，用于处理从INI文件解析出的键值对
 * 
 * 每次解析到有效的name=value对时，解析器会调用此回调函数。
 * 用户可通过`user`参数传递上下文数据，并根据需要处理键值对。
 * 
 * @param user 由用户在调用ini_parse/ini_parse_stream时传入的自定义数据指针
 * @param section 键值对所属的节名称（若未指定节，默认为空字符串）
 * @param name 解析出的键名（指向内部缓冲区，不可修改）
 * @param value 解析出的键值（指向内部缓冲区，调用结束后可能被覆盖）
 * @param lineno 解析该行的行号（仅当定义了INI_HANDLER_LINENO时有效）
 * 
 * @return 
 *   - 非零值：表示继续解析后续内容
 *   - 零值：表示停止解析，函数将返回当前行号作为错误位置
 * 
 * @remarks 
 *   - section和name参数指向内部缓冲区，内容不可修改
 *   - value参数虽为const char*，但可安全转换为char*进行临时修改
 *   - 回调函数返回后，value指向的内存可能被重复使用或释放
 */
#if INI_HANDLER_LINENO
typedef int (*ini_handler)(void* user, const char* section,
                           const char* name, const char* value,
                           int lineno);
#else
typedef int (*ini_handler)(void* user, const char* section,
                           const char* name, const char* value);
#endif

/* Typedef for fgets样式读取器的函数. */
typedef char* (*ini_reader)(char* str, int num, void* stream);



/* @brief: 解析给定的INI样式文件
 * @return
 *    - 0：success
 *    - -1：open filename failed
 *    - -2：memory allocation error
 *    - >0：parse error line number
 * @param filename: 文件名
 * @param handler: 处理函数
 * @param user: 传递用户自定义数据指针, 用户可以在回调函数handler内访问自己的数据，从而实现数据的传递。
 * @details: 
 *    - 值会删除空格
 *    - 会忽略ini文件的';'，注意：当 INI_USE_STACK 定义为 0 时才会发生内存分配错误 (-2)
 *    - 支持 [section] 格式，支持 name=value 和 name:value 两种赋值语法
 *    - 空行和以 ';' 开头的行将被忽略，section 名称默认为空字符串。
 *    - 回调函数返回 0 时会记录错误行号，但解析会继续直到文件结束（除非 INI_STOP_ON_FIRST_ERROR 被定义）
 */
INI_API int ini_parse(const char* filename, ini_handler handler, void* user);



/**
 * @brief 从已打开的文件流解析INI配置
 * 
 * 功能与ini_parse()相同，但直接操作FILE*流而非文件名。
 * 解析完成后不会关闭文件，需调用者自行处理。
 * 
 * @param file 已打开的INI文件流
 * @param handler 回调处理函数
 * @param user 传递给回调函数的用户数据
 * @return 
 *   - 0：解析成功
 *   - -1：读取文件失败
 *   - -2：内存分配错误（仅当INI_USE_STACK为0时）
 *   - >0：首条错误所在行号
 */
INI_API int ini_parse_file(FILE* file, ini_handler handler, void* user);




/**
 * @brief 使用自定义读取器解析INI数据
 * 
 * 功能与ini_parse()相同，但通过回调函数读取数据，
 * 适用于实现自定义或基于字符串的I/O场景（如网络数据流）。
 * 
 * @param reader 数据读取回调函数，原型为：
 *               char* reader(char* buffer, int size, void* stream);
 * @param stream 传递给reader的流指针
 * @param handler 键值对处理回调
 * @param user 传递给handler的用户数据
 * @return 
 *   - 0：解析成功
 *   - -2：内存分配错误（仅当INI_USE_STACK为0时）
 *   - >0：首条错误所在行号
 */
INI_API int ini_parse_stream(ini_reader reader, void* stream, ini_handler handler,
                     void* user);


                     

/**
 * @brief 从字符串解析INI配置
 * 
 * 功能与ini_parse()相同，但直接解析以NULL结尾的INI字符串数据，
 * 适用于解析已在内存中的INI数据（如网络接收的配置）。
 * 
 * @param string 包含INI配置的零终止字符串
 * @param handler 回调处理函数
 * @param user 传递给回调函数的用户数据
 * @return 
 *   - 0：解析成功
 *   - -2：内存分配错误（仅当INI_USE_STACK为0时）
 *   - >0：首条错误所在行号
 */
INI_API int ini_parse_string(const char* string, ini_handler handler, void* user);








/**
 * @brief 从指定长度的字符串解析INI配置
 * 
 * 功能与ini_parse_string()相同，但直接指定字符串长度而不依赖NULL终止符，
 * 避免调用strlen()，适用于处理非NULL终止的内存块（如网络数据）。
 * 
 * @param string 包含INI配置的字符串
 * @param length 字符串长度（字节数）
 * @param handler 回调处理函数
 * @param user 传递给回调函数的用户数据
 * @return 
 *   - 0：解析成功
 *   - -2：内存分配错误（仅当INI_USE_STACK为0时）
 *   - >0：首条错误所在行号
 */
INI_API int ini_parse_string_length(const char* string, size_t length, ini_handler handler, void* user);



/**
 * @def INI_ALLOW_MULTILINE
 * @brief 是否允许多行值解析（模仿Python configparser）
 * 
 * 若启用，后续带有前导空格的行会被视为前一个值的延续。
 * 解析器会对每一行调用一次handler，name参数保持不变。
 * 
 * 默认值：1（允许）
 */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif



/**
 * @def INI_ALLOW_BOM
 * @brief 是否允许UTF-8 BOM头（0xEF 0xBB 0xBF）
 * 
 * 若启用，解析器会自动跳过INI文件开头的BOM标记。
 * 
 * 默认值：1（允许）
 * @see https://github.com/benhoyt/inih/issues/21
 */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif


/**
 * @def INI_START_COMMENT_PREFIXES
 * @brief 行首注释起始字符集合
 * 
 * 指定哪些字符开头的行会被视为注释。
 * 默认为";#"，即支持;和#开头的注释（模仿Python configparser）。
 */
#ifndef INI_START_COMMENT_PREFIXES
#define INI_START_COMMENT_PREFIXES ";#"
#endif



/**
 * @def INI_ALLOW_INLINE_COMMENTS
 * @brief 是否允许行内注释
 * 
 * 若启用，行内出现指定字符后的内容会被视为注释。
 * 行内注释字符由INI_INLINE_COMMENT_PREFIXES定义。
 * 设置为0可匹配Python 3.2+ configparser行为。
 * 
 * 默认值：1（允许）
 */
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif


/**
 * @def INI_INLINE_COMMENT_PREFIXES
 * @brief 行内注释起始字符集合
 * 
 * 仅当INI_ALLOW_INLINE_COMMENTS启用时有效。
 * 
 * 默认值：";"（仅允许;作为行内注释起始符）
 */
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";"
#endif



/**
 * @def INI_USE_STACK
 * @brief 是否使用栈内存存储行缓冲区
 * 
 * 若为1，使用栈分配行缓冲区（固定大小）；
 * 若为0，使用堆分配（需通过malloc/free管理）。
 * 
 * 默认值：1（使用栈）
 */
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif




/**
 * @def INI_MAX_LINE
 * @brief 允许的最大行长度（字节）
 * 
 * 定义解析器能处理的最大行长度。
 * 实际缓冲区需额外3字节用于存储'\r'、'\n'和'\0'。
 * 
 * 默认值：200
 */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif



/**
 * @def INI_ALLOW_REALLOC
 * @brief 是否允许动态扩展行缓冲区
 * 
 * 仅当INI_USE_STACK为0时有效。
 * 若启用，行缓冲区可通过realloc动态增长；
 * 否则使用固定大小为INI_MAX_LINE的缓冲区。
 * 
 * 默认值：0（不允许）
 */
#ifndef INI_ALLOW_REALLOC
#define INI_ALLOW_REALLOC 0
#endif



/**
 * @def INI_INITIAL_ALLOC
 * @brief 堆分配行缓冲区的初始大小（字节）
 * 
 * 仅当INI_USE_STACK为0时有效。
 * 指定使用malloc分配的初始缓冲区大小。
 * 
 * 默认值：200
 */
#ifndef INI_INITIAL_ALLOC
#define INI_INITIAL_ALLOC 200
#endif




/**
 * @def INI_STOP_ON_FIRST_ERROR
 * @brief 是否在遇到首个错误时停止解析
 * 
 * 若为1，解析器在遇到首个错误时立即返回；
 * 若为0，继续解析以收集更多错误信息（默认行为）。
 * 
 * 默认值：0（继续解析）
 */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif



/**
 * @def INI_CALL_HANDLER_ON_NEW_SECTION
 * @brief 是否在每个新节开始时调用处理函数
 * 
 * 若为1，解析器在遇到新的[section]时会调用handler，
 * 此时name和value参数为NULL；
 * 若为0，仅在遇到键值对时调用handler。
 * 
 * 默认值：0（仅处理键值对）
 */
#ifndef INI_CALL_HANDLER_ON_NEW_SECTION
#define INI_CALL_HANDLER_ON_NEW_SECTION 0
#endif



/**
 * @def INI_ALLOW_NO_VALUE
 * @brief 是否允许无值键（name without value）
 * 
 * 若为1，允许出现不含=或:的行，解析器会将其视为
 * 无值键，并以value=NULL调用handler；
 * 若为0，此类行将被视为错误。
 * 
 * 默认值：0（不允许）
 */
#ifndef INI_ALLOW_NO_VALUE
#define INI_ALLOW_NO_VALUE 0
#endif



/**
 * @def INI_CUSTOM_ALLOCATOR
 * @brief 是否使用自定义内存分配函数
 * 
 * 若为1，使用自定义的ini_malloc、ini_free和ini_realloc函数，
 * 需自行实现这些函数（签名需与标准malloc/free/realloc一致）。
 * 注意：必须同时将INI_USE_STACK设为0。
 * 
 * 默认值：0（使用标准库函数）
 */

#ifndef INI_CUSTOM_ALLOCATOR
#define INI_CUSTOM_ALLOCATOR 0
#endif


#ifdef __cplusplus
}
#endif
