/**
 * @file cmockery.h
 * @brief 轻量级单元测试框架 cmockery 的核心头文件，提供测试断言、Mock 函数、参数检查等功能。
 * 
 * cmockery 是一个用于 C 语言的单元测试框架，支持通过宏定义实现函数 Mock、参数预期检查、
 * 断言验证等功能，适用于嵌入式系统和跨平台环境。本头文件定义了框架的核心宏、数据结构和函数接口。
 * 
 * @see https://code.google.com/archive/p/cmockery/ （项目原地址）
 */


# pragma once

#include <stdint.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdarg.h>

/*
 * These headers or their equivalents should be included prior to including
 * this header file.
 *
 * #include <stdarg.h>
 * #include <stddef.h>
 * #include <setjmp.h>
 *
 * This allows test applications to use custom definitions of C standard
 * library functions and types.
 */

/**
 * @brief 对不支持 __func__ 宏的编译器提供兼容，等效于函数名字符串。
 * @note GCC 等编译器原生支持 __func__，此宏用于兼容其他编译器。
 */
#ifndef __func__
#define __func__ __FUNCTION__
#endif

/**
 * @def LargestIntegralType
 * @brief 最大整数类型，用于存储任何指针或整数（兼容不同编译器和平台）。
 * @note 需足够大以容纳当前编译器支持的所有指针和整数类型，默认使用 unsigned long long。
 */
#ifndef LargestIntegralType
#define LargestIntegralType unsigned long long
#endif // LargestIntegralType


/**
 * @def LargestIntegralTypePrintfFormat
 * @brief 打印 LargestIntegralType 类型的格式化字符串，适配不同平台。
 * @note Windows 平台使用 "%I64x"，其他平台使用 "%llx"。
 */
#ifndef LargestIntegralTypePrintfFormat
#ifdef _WIN32
#define LargestIntegralTypePrintfFormat "%I64x"
#else
#define LargestIntegralTypePrintfFormat "%llx"
#endif // _WIN32
#endif // LargestIntegralTypePrintfFormat

/**
 * @def cast_to_largest_integral_type(value)
 * @brief 将值强制转换为 LargestIntegralType 类型，用于统一存储指针或整数。
 * @param value 待转换的值（可为指针、整数等）。
 * @return 转换后的 LargestIntegralType 类型值。
 */
#define cast_to_largest_integral_type(value) \
    ((LargestIntegralType)(value))


////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------ Mock 函数与返回值控制 ------------------------------//
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @def mock()
 * @brief 获取当前函数通过 will_return() 预设置的返回值。
 * @details 需与 will_return() 配合使用，从 Mock 函数的返回值队列中取出下一个值。
 * @return 预设置的返回值（类型为 LargestIntegralType）。
 * @note 调用前需通过 will_return() 提前设置返回值，否则可能导致未定义行为。
 */
#define mock() _mock(__func__, __FILE__, __LINE__)

/**
 * @def will_return(function, value)
 * @brief 为指定函数预设置后续调用的返回值（默认返回 1 次）。
 * @param function 目标函数名（字符串形式，如 "add"）。
 * @param value 待返回的值（会被转换为 LargestIntegralType 存储）。
 * @see will_return_count() 可指定返回次数。
 */
#define will_return(function, value) \
    _will_return(#function, __FILE__, __LINE__, \
                 cast_to_largest_integral_type(value), 1)
#define will_return_count(function, value, count) \
    _will_return(#function, __FILE__, __LINE__, \
                 cast_to_largest_integral_type(value), count)


////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------- 参数检查与预期验证 -------------------------------///
////////////////////////////////////////////////////////////////////////////////////////


/**
 * @def expect_check(function, parameter, check_function, check_data)
 * @brief 为函数参数注册自定义检查函数，验证参数是否符合预期。
 * @param function 目标函数名（字符串形式）。
 * @param parameter 待检查的参数名（字符串形式）。
 * @param check_function 自定义检查函数（类型为 CheckParameterValue）。
 * @param check_data 传递给检查函数的自定义数据（会被转换为 LargestIntegralType）。
 * @details 当函数被调用时，check_function 会被触发，用于复杂参数验证（如范围、格式等）。
 */
#define expect_check(function, parameter, check_function, check_data) \
    _expect_check(#function, #parameter, __FILE__, __LINE__, check_function, \
                  cast_to_largest_integral_type(check_data), NULL, 0)

/**
 * @def expect_in_set(function, parameter, value_array)
 * @brief 验证函数参数是否在指定集合中（默认检查 1 次）。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名。
 * @param value_array 预期值数组（元素类型需与参数兼容）。
 * @see expect_in_set_count() 可指定检查次数。
 */
#define expect_in_set(function, parameter, value_array) \
    expect_in_set_count(function, parameter, value_array, 1)
#define expect_in_set_count(function, parameter, value_array, count) \
    _expect_in_set(#function, #parameter, __FILE__, __LINE__, value_array, \
                   sizeof(value_array) / sizeof((value_array)[0]), count)

/**
 * @def expect_not_in_set(function, parameter, value_array)
 * @brief 验证函数参数是否不在指定集合中（默认检查 1 次）。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名。
 * @param value_array 排除值数组。
 * @see expect_not_in_set_count() 可指定检查次数。
 */                
#define expect_not_in_set(function, parameter, value_array) \
    expect_not_in_set_count(function, parameter, value_array, 1)

    
/**
 * @def expect_not_in_set_count(function, parameter, value_array, count)
 * @brief 验证函数参数是否不在指定集合中，并指定检查次数。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名。
 * @param value_array 排除值数组。
 * @param count 检查次数：-1 表示永久检查，正数表示检查指定次数。
 */
#define expect_not_in_set_count(function, parameter, value_array, count) \
    _expect_not_in_set( \
        #function, #parameter, __FILE__, __LINE__, value_array, \
        sizeof(value_array) / sizeof((value_array)[0]), count)


/**
 * @def expect_in_range(function, parameter, minimum, maximum)
 * @brief 验证函数参数是否在 [minimum, maximum] 范围内（默认检查 1 次）。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名。
 * @param minimum 范围最小值。
 * @param maximum 范围最大值。
 * @see expect_in_range_count() 可指定检查次数。
 */
#define expect_in_range(function, parameter, minimum, maximum) \
    expect_in_range_count(function, parameter, minimum, maximum, 1)

/**
 * @def expect_in_range_count(function, parameter, minimum, maximum, count)
 * @brief 验证函数参数是否在 [minimum, maximum] 范围内，并指定检查次数。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名。
 * @param minimum 范围最小值。
 * @param maximum 范围最大值。
 * @param count 检查次数：-1 表示永久检查，正数表示检查指定次数。
 */   
#define expect_in_range_count(function, parameter, minimum, maximum, count) \
    _expect_in_range(#function, #parameter, __FILE__, __LINE__, minimum, \
                     maximum, count)

/**
 * @def expect_not_in_range(function, parameter, minimum, maximum)
 * @brief 验证函数参数是否在 (-∞, minimum) ∪ (maximum, +∞) 范围内（默认检查 1 次）。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名。
 * @param minimum 范围最小值。
 * @param maximum 范围最大值。
 * @see expect_not_in_range_count() 可指定检查次数。
 */
#define expect_not_in_range(function, parameter, minimum, maximum) \
    expect_not_in_range_count(function, parameter, minimum, maximum, 1)

/**
 * @def expect_not_in_range_count(function, parameter, minimum, maximum, count)
 * @brief 验证函数参数是否在范围外，并指定检查次数。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名。
 * @param minimum 范围最小值。
 * @param maximum 范围最大值。
 * @param count 检查次数：-1 表示永久检查，正数表示检查指定次数。
 */ 
#define expect_not_in_range_count(function, parameter, minimum, maximum, \
                                  count) \
    _expect_not_in_range(#function, #parameter, __FILE__, __LINE__, \
                         minimum, maximum, count)


/**
 * @def expect_value(function, parameter, value)
 * @brief 验证函数参数是否等于指定值（默认检查 1 次）。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名。
 * @param value 预期值。
 * @see expect_value_count() 可指定检查次数。
 */
#define expect_value(function, parameter, value) \
    expect_value_count(function, parameter, value, 1)

/**
 * @def expect_value_count(function, parameter, value, count)
 * @brief 验证函数参数是否等于指定值，并指定检查次数。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名。
 * @param value 预期值。
 * @param count 检查次数：-1 表示永久检查，正数表示检查指定次数。
 */ 
#define expect_value_count(function, parameter, value, count) \
    _expect_value(#function, #parameter, __FILE__, __LINE__, \
                  cast_to_largest_integral_type(value), count)

/**
 * @def expect_not_value(function, parameter, value)
 * @brief 验证函数参数是否不等于指定值（默认检查 1 次）。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名。
 * @param value 排除值。
 * @see expect_not_value_count() 可指定检查次数。
 */
#define expect_not_value(function, parameter, value) \
    expect_not_value_count(function, parameter, value, 1)

/**
 * @def expect_not_value_count(function, parameter, value, count)
 * @brief 验证函数参数是否不等于指定值，并指定检查次数。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名。
 * @param value 排除值。
 * @param count 检查次数：-1 表示永久检查，正数表示检查指定次数。
 */
#define expect_not_value_count(function, parameter, value, count) \
    _expect_not_value(#function, #parameter, __FILE__, __LINE__, \
                      cast_to_largest_integral_type(value), count)

/**
 * @def expect_string(function, parameter, string)
 * @brief 验证函数参数（字符串）是否与指定字符串相等（默认检查 1 次）。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名（需为字符串类型）。
 * @param string 预期字符串（以 '\0' 结尾）。
 * @see expect_string_count() 可指定检查次数。
 */
#define expect_string(function, parameter, string) \
    expect_string_count(function, parameter, string, 1)

/**
 * @def expect_string_count(function, parameter, string, count)
 * @brief 验证字符串参数是否相等，并指定检查次数。
 * @param function 目标函数名。
 * @param parameter 待检查的字符串参数名。
 * @param string 预期字符串。
 * @param count 检查次数：-1 表示永久检查，正数表示检查指定次数。
 */
#define expect_string_count(function, parameter, string, count) \
    _expect_string(#function, #parameter, __FILE__, __LINE__, \
                   (const char*)(string), count)

/**
 * @def expect_not_string(function, parameter, string)
 * @brief 验证函数参数（字符串）是否与指定字符串不相等（默认检查 1 次）。
 * @param function 目标函数名。
 * @param parameter 待检查的字符串参数名。
 * @param string 排除字符串。
 * @see expect_not_string_count() 可指定检查次数。
 */
#define expect_not_string(function, parameter, string) \
    expect_not_string_count(function, parameter, string, 1)

/**
 * @def expect_not_string_count(function, parameter, string, count)
 * @brief 验证字符串参数是否不相等，并指定检查次数。
 * @param function 目标函数名。
 * @param parameter 待检查的字符串参数名。
 * @param string 排除字符串。
 * @param count 检查次数：-1 表示永久检查，正数表示检查指定次数。
 */
#define expect_not_string_count(function, parameter, string, count) \
    _expect_not_string(#function, #parameter, __FILE__, __LINE__, \
                       (const char*)(string), count)

/**
 * @def expect_memory(function, parameter, memory, size)
 * @brief 验证函数参数（内存块）是否与指定内存块相等（默认检查 1 次）。
 * @param function 目标函数名。
 * @param parameter 待检查的参数名（需为指针类型）。
 * @param memory 预期内存块的起始地址。
 * @param size 内存块大小（字节数）。
 * @see expect_memory_count() 可指定检查次数。
 */
#define expect_memory(function, parameter, memory, size) \
    expect_memory_count(function, parameter, memory, size, 1)

/**
 * @def expect_memory_count(function, parameter, memory, size, count)
 * @brief 验证内存块参数是否相等，并指定检查次数。
 * @param function 目标函数名。
 * @param parameter 待检查的内存块参数名。
 * @param memory 预期内存块地址。
 * @param size 内存块大小（字节）。
 * @param count 检查次数：-1 表示永久检查，正数表示检查指定次数。
 */
#define expect_memory_count(function, parameter, memory, size, count) \
    _expect_memory(#function, #parameter, __FILE__, __LINE__, \
                   (const void*)(memory), size, count)

/**
 * @def expect_not_memory(function, parameter, memory, size)
 * @brief 验证函数参数（内存块）是否与指定内存块不相等（默认检查 1 次）。
 * @param function 目标函数名。
 * @param parameter 待检查的内存块参数名。
 * @param memory 排除内存块地址。
 * @param size 内存块大小（字节）。
 * @see expect_not_memory_count() 可指定检查次数。
 */
#define expect_not_memory(function, parameter, memory, size) \
    expect_not_memory_count(function, parameter, memory, size, 1)

/**
 * @def expect_not_memory_count(function, parameter, memory, size, count)
 * @brief 验证内存块参数是否不相等，并指定检查次数。
 * @param function 目标函数名。
 * @param parameter 待检查的内存块参数名。
 * @param memory 排除内存块地址。
 * @param size 内存块大小（字节）。
 * @param count 检查次数：-1 表示永久检查，正数表示检查指定次数。
 */
#define expect_not_memory_count(function, parameter, memory, size, count) \
    _expect_not_memory(#function, #parameter, __FILE__, __LINE__, \
                       (const void*)(memory), size, count)


/**
 * @def expect_any(function, parameter)
 * @brief 允许函数参数为任意值（默认允许 1 次）。
 * @param function 目标函数名。
 * @param parameter 待忽略检查的参数名。
 * @see expect_any_count() 可指定允许次数。
 */
#define expect_any(function, parameter) \
    expect_any_count(function, parameter, 1)

/**
 * @def expect_any_count(function, parameter, count)
 * @brief 允许函数参数为任意值，并指定允许次数。
 * @param function 目标函数名。
 * @param parameter 待忽略检查的参数名。
 * @param count 允许次数：-1 表示永久允许，正数表示允许指定次数。
 */
#define expect_any_count(function, parameter, count) \
    _expect_any(#function, #parameter, __FILE__, __LINE__, count)


    
/**
 * @def check_expected(parameter)
 * @brief 验证函数参数是否符合通过 expect_*() 宏设置的预期。
 * @param parameter 待验证的参数（需与 expect_*() 中指定的参数名一致）。
 * @details 需在被测试函数中调用，用于触发参数检查逻辑，确保参数符合预期。
 * @note 若未通过 expect_*() 设置预期，调用此宏会导致测试失败。
 */
#define check_expected(parameter) \
    _check_expected(__func__, #parameter, __FILE__, __LINE__, \
                    cast_to_largest_integral_type(parameter))

////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------- 断言宏（Assertion） -------------------------------//
////////////////////////////////////////////////////////////////////////////////////////


/**
 * @def assert_true(c)
 * @brief 断言表达式为真（非零），否则测试失败。
 * @param c 待验证的表达式。
 * @note 失败时会打印表达式、文件名和行号。
 */
#define assert_true(c) _assert_true(cast_to_largest_integral_type(c), #c, \
                                    __FILE__, __LINE__)

                                    
/**
 * @def assert_false(c)
 * @brief 断言表达式为假（零），否则测试失败。
 * @param c 待验证的表达式。
 * @note 失败时会打印表达式、文件名和行号。
 */
#define assert_false(c) _assert_false((cast_to_largest_integral_type(c)), #c, \
                                     __FILE__, __LINE__)


                                     
/**
 * @def assert_int_equal(a, b)
 * @brief 断言两个整数相等，否则测试失败。
 * @param a 第一个整数。
 * @param b 第二个整数。
 * @note 支持任意整数类型（会被转换为 LargestIntegralType 比较）。
 */
#define assert_int_equal(a, b) \
    _assert_int_equal(cast_to_largest_integral_type(a), \
                      cast_to_largest_integral_type(b), \
                      __FILE__, __LINE__)

                      
/**
 * @def assert_int_not_equal(a, b)
 * @brief 断言两个整数不相等，否则测试失败。
 * @param a 第一个整数。
 * @param b 第二个整数。
 */
#define assert_int_not_equal(a, b) \
    _assert_int_not_equal(cast_to_largest_integral_type(a), \
                          cast_to_largest_integral_type(b), \
                          __FILE__, __LINE__)


                          
/**
 * @def assert_string_equal(a, b)
 * @brief 断言两个字符串相等（strcmp 结果为 0），否则测试失败。
 * @param a 第一个字符串（以 '\0' 结尾）。
 * @param b 第二个字符串（以 '\0' 结尾）。
 */
#define assert_string_equal(a, b) \
    _assert_string_equal((const char*)(a), (const char*)(b), __FILE__, \
                         __LINE__)

                         
/**
 * @def assert_string_not_equal(a, b)
 * @brief 断言两个字符串不相等（strcmp 结果非 0），否则测试失败。
 * @param a 第一个字符串。
 * @param b 第二个字符串。
 */
#define assert_string_not_equal(a, b) \
    _assert_string_not_equal((const char*)(a), (const char*)(b), __FILE__, \
                             __LINE__)


/**
 * @def assert_memory_equal(a, b, size)
 * @brief 断言两块内存（指定大小）相等（memcmp 结果为 0），否则测试失败。
 * @param a 第一块内存的起始地址。
 * @param b 第二块内存的起始地址。
 * @param size 内存块大小（字节数）。
 */
#define assert_memory_equal(a, b, size) \
    _assert_memory_equal((const char*)(a), (const char*)(b), size, __FILE__, \
                         __LINE__)

                         

/**
 * @def assert_memory_not_equal(a, b, size)
 * @brief 断言两块内存（指定大小）不相等（memcmp 结果非 0），否则测试失败。
 * @param a 第一块内存的起始地址。
 * @param b 第二块内存的起始地址。
 * @param size 内存块大小（字节数）。
 */
#define assert_memory_not_equal(a, b, size) \
    _assert_memory_not_equal((const char*)(a), (const char*)(b), size, \
                             __FILE__, __LINE__)


                             
/**
 * @def assert_in_range(value, minimum, maximum)
 * @brief 断言值在 [minimum, maximum] 范围内，否则测试失败。
 * @param value 待验证的值。
 * @param minimum 范围最小值。
 * @param maximum 范围最大值。
 */
#define assert_in_range(value, minimum, maximum) \
    _assert_in_range( \
        cast_to_largest_integral_type(value), \
        cast_to_largest_integral_type(minimum), \
        cast_to_largest_integral_type(maximum), __FILE__, __LINE__)


        
/**
 * @def assert_not_in_range(value, minimum, maximum)
 * @brief 断言值在 (-∞, minimum) ∪ (maximum, +∞) 范围内，否则测试失败。
 * @param value 待验证的值。
 * @param minimum 范围最小值。
 * @param maximum 范围最大值。
 */
#define assert_not_in_range(value, minimum, maximum) \
    _assert_not_in_range( \
        cast_to_largest_integral_type(value), \
        cast_to_largest_integral_type(minimum), \
        cast_to_largest_integral_type(maximum), __FILE__, __LINE__)


/**
 * @def assert_in_set(value, values, number_of_values)
 * @brief 断言值在指定集合中，否则测试失败。
 * @param value 待验证的值。
 * @param values 集合数组（元素类型为 LargestIntegralType）。
 * @param number_of_values 集合中元素的数量。
 */
#define assert_in_set(value, values, number_of_values) \
    _assert_in_set(value, values, number_of_values, __FILE__, __LINE__)

    
/**
 * @def assert_not_in_set(value, values, number_of_values)
 * @brief 断言值不在指定集合中，否则测试失败。
 * @param value 待验证的值。
 * @param values 集合数组（元素类型为 LargestIntegralType）。
 * @param number_of_values 集合中元素的数量。
 */
#define assert_not_in_set(value, values, number_of_values) \
    _assert_not_in_set(value, values, number_of_values, __FILE__, __LINE__)


/**
 * @def fail()
 * @brief 强制测试失败并终止当前测试。
 * @note 失败时会打印文件名和行号。
 */
#define fail() _fail(__FILE__, __LINE__)


/**
 * @def fail_msg(msg, ...)
 * @brief 打印错误消息并强制测试失败。
 * @param msg 错误消息格式字符串（支持 printf 风格占位符）。
 * @param ... 格式字符串对应的参数。
 */
#define fail_msg(msg, ...) do { \
    print_error("ERROR: " msg "\n", ##__VA_ARGS__); \
    fail(); \
} while (0)

////////////////////////////////////////////////////////////////////////////////////////
// ---------------------------------- 测试用例管理 -----------------------------------//
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @def run_test(f)
 * @brief 运行单个测试函数。
 * @param f 测试函数（类型为 UnitTestFunction）。
 * @return 测试结果：0 表示成功，非 0 表示失败。
 */
#define run_test(f) _run_test(#f, f, NULL, UNIT_TEST_FUNCTION_TYPE_TEST, NULL)


/**
 * @def unit_test(f)
 * @brief 初始化一个 UnitTest 结构体（测试函数类型）。
 * @param f 测试函数。
 * @return 初始化后的 UnitTest 结构体。
 */
#define unit_test(f) { #f, f, UNIT_TEST_FUNCTION_TYPE_TEST }

/**
 * @def unit_test_with_prefix(prefix, f)
 * @brief 初始化带前缀的测试函数结构体。
 * @param prefix 前缀字符串（用于区分同名测试函数）。
 * @param f 测试函数。
 * @return 初始化后的 UnitTest 结构体。
 */
#define unit_test_with_prefix(prefix, f) { #prefix#f, f, UNIT_TEST_FUNCTION_TYPE_TEST }

/**
 * @def unit_test_setup(test, setup)
 * @brief 初始化测试函数的前置 setup 函数结构体。
 * @param test 测试函数名（用于生成 setup 函数名）。
 * @param setup 前置 setup 函数（测试前执行）。
 * @return 初始化后的 UnitTest 结构体。
 */
#define unit_test_setup(test, setup) \
    { #test "_" #setup, setup, UNIT_TEST_FUNCTION_TYPE_SETUP }

/**
 * @def unit_test_teardown(test, teardown)
 * @brief 初始化测试函数的后置 teardown 函数结构体。
 * @param test 测试函数名（用于生成 teardown 函数名）。
 * @param teardown 后置 teardown 函数（测试后执行）。
 * @return 初始化后的 UnitTest 结构体。
 */
#define unit_test_teardown(test, teardown) \
    { #test "_" #teardown, teardown, UNIT_TEST_FUNCTION_TYPE_TEARDOWN }


/**
 * @def unit_test_setup_teardown(test, setup, teardown)
 * @brief 初始化包含 setup、测试、teardown 的完整测试序列。
 * @param test 测试函数。
 * @param setup 前置 setup 函数（可为 NULL）。
 * @param teardown 后置 teardown 函数（可为 NULL）。
 * @return 包含三个阶段的 UnitTest 结构体数组。
 */
#define unit_test_setup_teardown(test, setup, teardown) \
    unit_test_setup(test, setup), \
    unit_test(test), \
    unit_test_teardown(test, teardown)

/**
 * @def run_tests(tests)
 * @brief 运行一组测试用例（UnitTest 数组）。
 * @param tests UnitTest 结构体数组。
 * @return 测试结果：0 表示全部成功，非 0 表示至少一个失败。
 * @example
 * const UnitTest tests[] = {
 *     unit_test(TestAdd),
 *     unit_test(TestSubtract)
 * };
 * return run_tests(tests);
 */
#define run_tests(tests) _run_tests(tests, sizeof(tests) / sizeof(tests)[0])



/**
 * @def test_malloc(size)
 * @brief 测试专用的 malloc 函数（记录内存分配信息，便于检测泄漏）。
 * @param size 分配大小（字节）。
 * @return 分配的内存块地址，失败返回 NULL。
 * @see test_free() 需配合使用以释放内存。
 */
#define test_malloc(size) _test_malloc(size, __FILE__, __LINE__)


/**
 * @def test_calloc(num, size)
 * @brief 测试专用的 calloc 函数（初始化内存为 0，记录分配信息）。
 * @param num 元素数量。
 * @param size 单个元素大小（字节）。
 * @return 分配的内存块地址，失败返回 NULL。
 */
#define test_calloc(num, size) _test_calloc(num, size, __FILE__, __LINE__)

/**
 * @def test_free(ptr)
 * @brief 测试专用的 free 函数（释放 test_malloc/test_calloc 分配的内存）。
 * @param ptr 待释放的内存块地址（可为 NULL）。
 */
#define test_free(ptr) _test_free(ptr, __FILE__, __LINE__)


/**
 * @brief 单元测试模式下，重定义标准内存分配函数为测试专用版本。
 * @note 需定义 UNIT_TESTING 宏才能启用此重定义。
 */
#if UNIT_TESTING
#define malloc test_malloc
#define calloc test_calloc
#define free test_free
#endif // UNIT_TESTING


////////////////////////////////////////////////////////////////////////////////////////
// ----------------------------- 断言预期（Assert 捕获） -----------------------------//
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @def expect_assert_failure(function_call)
 * @brief 验证函数调用是否触发断言失败（用于测试断言逻辑）。
 * @param function_call 待测试的函数调用（预期会触发 assert）。
 * @details 若函数调用触发断言，测试通过；否则测试失败。
 * @example
 * expect_assert_failure(show_message(NULL)); // 预期 show_message(NULL) 触发 assert
 */
#define expect_assert_failure(function_call) \
  { \
    const int expression = setjmp(global_expect_assert_env); \
    global_expecting_assert = 1; \
    if (expression) { \
      print_message("Expected assertion %s occurred\n", \
                    *((const char**)&expression)); \
      global_expecting_assert = 0; \
    } else { \
      function_call ; \
      global_expecting_assert = 0; \
      print_error("Expected assert in %s\n", #function_call); \
      _fail(__FILE__, __LINE__); \
    } \
  }

////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------------ 类型定义 -------------------------------------//
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @typedef UnitTestFunction
 * @brief 测试函数、setup 函数、teardown 函数的通用类型。
 * @param state 测试状态指针（可用于传递测试上下文，可为 NULL）。
 */
typedef void (*UnitTestFunction)(void **state);


/**
 * @typedef CheckParameterValue
 * @brief 参数检查函数的类型，用于自定义参数验证逻辑。
 * @param value 实际传递的参数值（转换为 LargestIntegralType）。
 * @param check_value_data 自定义检查数据（通过 expect_check() 传递）。
 * @return 检查结果：1 表示通过，0 表示失败。
 */
typedef int (*CheckParameterValue)(const LargestIntegralType value,
                                   const LargestIntegralType check_value_data);


/**
 * @enum UnitTestFunctionType
 * @brief 测试函数的类型枚举（测试函数、setup 函数、teardown 函数）。
 */
typedef enum UnitTestFunctionType 
{
    UNIT_TEST_FUNCTION_TYPE_TEST = 0,
    UNIT_TEST_FUNCTION_TYPE_SETUP,
    UNIT_TEST_FUNCTION_TYPE_TEARDOWN,
} UnitTestFunctionType;


/**
 * @struct UnitTest
 * @brief 存储测试函数信息的结构体。
 * @note setup 函数必须与 teardown 函数配对使用，函数指针可为 NULL。
 */
typedef struct UnitTest 
{
    const char* name;
    UnitTestFunction function;
    UnitTestFunctionType function_type;
} UnitTest;


/**
 * @struct SourceLocation
 * @brief 源代码位置信息（文件名和行号）。
 */
typedef struct SourceLocation 
{
    const char* file;
    int line;
} SourceLocation;

/**
 * @struct CheckParameterEvent
 * @brief 参数检查事件结构体，用于存储参数检查的相关信息。
 */
typedef struct CheckParameterEvent 
{
    SourceLocation location;
    const char *parameter_name;
    CheckParameterValue check_value;
    LargestIntegralType check_value_data;
} CheckParameterEvent;

////////////////////////////////////////////////////////////////////////////////////////
// ---------------------------------- 全局变量声明 -----------------------------------//
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @var global_expecting_assert
 * @brief 标记是否正在等待断言失败（用于 expect_assert_failure()）。
 * @note 1 表示等待，0 表示不等待。
 */
extern int global_expecting_assert;

/**
 * @var global_expect_assert_env
 * @brief 用于捕获断言失败的跳转环境（配合 setjmp/longjmp 使用）。
 */
extern jmp_buf global_expect_assert_env;

////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------- 内部函数声明（供宏调用） ------------------------------//
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief 内部实现：获取 Mock 函数的返回值（供 mock() 宏调用）。
 * @param function 函数名。
 * @param file 文件名。
 * @param line 行号。
 * @return 预设置的返回值。
 */
LargestIntegralType _mock(const char * const function, const char* const file,
                          const int line);

/**
 * @brief 内部实现：注册参数检查事件（供 expect_check() 宏调用）。
 */
void _expect_check(
    const char* const function, const char* const parameter,
    const char* const file, const int line,
    const CheckParameterValue check_function,
    const LargestIntegralType check_data, CheckParameterEvent * const event,
    const int count);

/**
 * @brief 内部实现：验证参数是否在集合中（供 expect_in_set() 宏调用）。
 */
void _expect_in_set(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const LargestIntegralType values[],
    const size_t number_of_values, const int count);

/**
 * @brief 内部实现：验证参数是否不在集合中（供 expect_not_in_set() 宏调用）。
 */
void _expect_not_in_set(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const LargestIntegralType values[],
    const size_t number_of_values, const int count);

/**
 * @brief 内部实现：验证参数是否在范围内（供 expect_in_range() 宏调用）。
 */
void _expect_in_range(
    const char* const function, const char* const parameter,
    const char* const file, const int line,
    const LargestIntegralType minimum,
    const LargestIntegralType maximum, const int count);

/**
 * @brief 内部实现：验证参数是否在范围外（供 expect_not_in_range() 宏调用）。
 */
void _expect_not_in_range(
    const char* const function, const char* const parameter,
    const char* const file, const int line,
    const LargestIntegralType minimum,
    const LargestIntegralType maximum, const int count);


/**
 * @brief 内部实现：验证参数是否等于指定值（供 expect_value() 宏调用）。
 */
void _expect_value(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const LargestIntegralType value,
    const int count);

/**
 * @brief 内部实现：验证参数是否不等于指定值（供 expect_not_value() 宏调用）。
 */
void _expect_not_value(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const LargestIntegralType value,
    const int count);

/**
 * @brief 内部实现：验证字符串参数是否相等（供 expect_string() 宏调用）。
 */
void _expect_string(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const char* string,
    const int count);

/**
 * @brief 内部实现：验证字符串参数是否不相等（供 expect_not_string() 宏调用）。
 */
void _expect_not_string(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const char* string,
    const int count);

/**
 * @brief 内部实现：验证内存块参数是否相等（供 expect_memory() 宏调用）。
 */
void _expect_memory(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const void* const memory,
    const size_t size, const int count);

    
/**
 * @brief 内部实现：验证内存块参数是否不相等（供 expect_not_memory() 宏调用）。
 */
void _expect_not_memory(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const void* const memory,
    const size_t size, const int count);

/**
 * @brief 内部实现：允许参数为任意值（供 expect_any() 宏调用）。
 */
void _expect_any(
    const char* const function, const char* const parameter,
    const char* const file, const int line, const int count);

/**
 * @brief 内部实现：检查参数是否符合预期（供 check_expected() 宏调用）。
 */
void _check_expected(
    const char * const function_name, const char * const parameter_name,
    const char* file, const int line, const LargestIntegralType value);


/**
 * @brief 替换标准 assert 宏的实现，用于捕获断言失败而不终止测试。
 * @param result 断言表达式的结果（非零为真）。
 * @param expression 断言表达式的字符串形式。
 * @param file 文件名。
 * @param line 行号。
 * @note 配合 expect_assert_failure() 可验证断言是否触发。
 */
void mock_assert(const int result, const char* const expression,
                 const char * const file, const int line);

/**
 * @brief 内部实现：为函数设置返回值（供 will_return() 宏调用）。
 */
void _will_return(const char * const function_name, const char * const file,
                  const int line, const LargestIntegralType value,
                  const int count);

                  
/**
 * @brief 内部实现：断言表达式为真（供 assert_true() 宏调用）。
 */
void _assert_true(const LargestIntegralType result,
                  const char* const expression,
                  const char * const file, const int line);

/**
 * @brief 内部实现：断言表达式为假（供 assert_false() 宏调用）。
 */
void _assert_false(const LargestIntegralType result,
                  const char* const expression,
                  const char * const file, const int line);


/**
 * @brief 内部实现：断言两个整数相等（供 assert_int_equal() 宏调用）。
 */
void _assert_int_equal(
    const LargestIntegralType a, const LargestIntegralType b,
    const char * const file, const int line);

/**
 * @brief 内部实现：断言两个整数不相等（供 assert_int_not_equal() 宏调用）。
 */
void _assert_int_not_equal(
    const LargestIntegralType a, const LargestIntegralType b,
    const char * const file, const int line);

/**
 * @brief 内部实现：断言字符串相等（供 assert_string_equal() 宏调用）。
 */
void _assert_string_equal(const char * const a, const char * const b,
                          const char * const file, const int line);

/**
 * @brief 内部实现：断言字符串不相等（供 assert_string_not_equal() 宏调用）。
 */
void _assert_string_not_equal(const char * const a, const char * const b,
                              const char *file, const int line);

/**
 * @brief 内部实现：断言内存块相等（供 assert_memory_equal() 宏调用）。
 */
void _assert_memory_equal(const void * const a, const void * const b,
                          const size_t size, const char* const file,
                          const int line);

/**
 * @brief 内部实现：断言内存块不相等（供 assert_memory_not_equal() 宏调用）。
 */
void _assert_memory_not_equal(const void * const a, const void * const b,
                              const size_t size, const char* const file,
                              const int line);

/**
 * @brief 内部实现：断言值在范围内（供 assert_in_range() 宏调用）。
 */
void _assert_in_range(
    const LargestIntegralType value, const LargestIntegralType minimum,
    const LargestIntegralType maximum, const char* const file, const int line);


/**
 * @brief 内部实现：断言值在范围外（供 assert_not_in_range() 宏调用）。
 */
void _assert_not_in_range(
    const LargestIntegralType value, const LargestIntegralType minimum,
    const LargestIntegralType maximum, const char* const file, const int line);

/**
 * @brief 内部实现：断言值在集合中（供 assert_in_set() 宏调用）。
 */
void _assert_in_set(
    const LargestIntegralType value, const LargestIntegralType values[],
    const size_t number_of_values, const char* const file, const int line);

/**
 * @brief 内部实现：断言值不在集合中（供 assert_not_in_set() 宏调用）。
 */
void _assert_not_in_set(
    const LargestIntegralType value, const LargestIntegralType values[],
    const size_t number_of_values, const char* const file, const int line);


/**
 * @brief 内部实现：测试专用 malloc（供 test_malloc() 宏调用）。
 */
void* _test_malloc(const size_t size, const char* file, const int line);

/**
 * @brief 内部实现：测试专用 calloc（供 test_calloc() 宏调用）。
 */
void* _test_calloc(const size_t number_of_elements, const size_t size,
                   const char* file, const int line);

/**
 * @brief 内部实现：测试专用 free（供 test_free() 宏调用）。
 */
void _test_free(void* const ptr, const char* file, const int line);

/**
 * @brief 内部实现：强制测试失败（供 fail() 宏调用）。
 */
void _fail(const char * const file, const int line);

/**
 * @brief 内部实现：运行单个测试函数（供 run_test() 宏调用）。
 */
int _run_test(
    const char * const function_name, const UnitTestFunction Function,
    void ** const state, const UnitTestFunctionType function_type,
    const void* const heap_check_point);

/**
 * @brief 内部实现：运行一组测试（供 run_tests() 宏调用）。
 */
int _run_tests(const UnitTest * const tests, const size_t number_of_tests);

/**
 * @brief 打印普通消息（格式化输出）。
 * @param format 消息格式字符串。
 * @param ... 格式字符串对应的参数。
 */
void print_message(const char* const format, ...);

/**
 * @brief 打印错误消息（格式化输出到 stderr）。
 * @param format 错误消息格式字符串。
 * @param ... 格式字符串对应的参数。
 */
void print_error(const char* const format, ...);

/**
 * @brief 打印普通消息（va_list 版本）。
 * @param format 消息格式字符串。
 * @param args va_list 类型的参数列表。
 */
void vprint_message(const char* const format, va_list args);

/**
 * @brief 打印错误消息（va_list 版本）。
 * @param format 错误消息格式字符串。
 * @param args va_list 类型的参数列表。
 */
void vprint_error(const char* const format, va_list args);


////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------- cmockeryEx（enhance） ------------------------------//
////////////////////////////////////////////////////////////////////////////////////////

/**
 * @def mock_ptr()
 * @brief 显式获取指针类型的 Mock 返回值（解决 C++ 类型转换问题）。
 * @details 对 mock() 的返回值进行显式指针转换，适用于获取指针类型的返回值。
 * @return 转换后的 void* 类型指针。
 * @note 仅在 C++ 环境下需要显式转换，C 环境中可直接使用 mock()。
 */
#define mock_ptr() ((void*)mock())

/**
 * @def mock_int()
 * @brief 显式获取 int 类型的 Mock 返回值（解决 C++ 类型转换问题）。
 * @details 对 mock() 的返回值进行显式 int 转换，适用于获取整数类型的返回值。
 * @return 转换后的 int 类型值。
 */
#define mock_int() ((int)mock())

/**
 * @def mock_long()
 * @brief 显式获取 long 类型的 Mock 返回值。
 * @return 转换后的 long 类型值。
 */
#define mock_long() ((long)mock())

/**
 * @def mock_ptr_as(type)
 * @brief 显式将 Mock 返回值转换为指定指针类型（如 char*、int* 等）。
 * @param type 目标指针类型（如 char*）。
 * @return 转换后的指定指针类型。
 * @example mock_ptr_as(char*)  // 转换为字符指针
 */
#define mock_ptr_as(type) ((type)mock())
