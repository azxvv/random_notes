#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include "cmockery.h"
}

// 待测试的函数：计算两个整数的和
int add(int a, int b) 
{
    return a + b;
}

// 测试函数：验证 add(2, 3) 是否等于 5
void test_add(void **state) 
{
    (void) state; // 未使用参数
    assert_int_equal(add(2, 3), 5);
}

// 测试函数：验证 add(-1, 1) 是否等于 0
void test_add_negative(void **state) 
{
    (void) state; // 未使用参数
    assert_int_equal(add(-1, 1), 0);
}

int main(int argc, char *argv[]) 
{
    const UnitTest tests[] = 
    {
        unit_test(test_add),
        unit_test(test_add_negative),
    };
    return run_tests(tests);
}