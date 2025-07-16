#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include "cmockery.h"
}

// 被测试函数：计算字符串长度
size_t my_strlen(const char *str) 
{
    const char *s = str;
    while (*s) ++s;
    return s - str;
}

// 测试用例：空字符串
void test_empty_string(void **state) 
{
    assert_int_equal(my_strlen(""), 0);
}

// 测试用例：单字符字符串
void test_single_char(void **state) 
{
    assert_int_equal(my_strlen("a"), 1);
}

// 测试用例：多字符字符串
void test_multi_char(void **state) 
{
    assert_int_equal(my_strlen("hello"), 5);
}

// 测试用例：包含空格的字符串
void test_with_space(void **state) 
{
    assert_int_equal(my_strlen("hello world"), 11);
}

int main(int argc, char **argv) 
{
    const UnitTest tests[] = 
    {
        unit_test(test_empty_string),
        unit_test(test_single_char),
        unit_test(test_multi_char),
        unit_test(test_with_space),
    };
    return run_tests(tests);
}