#include <stdio.h>
#include <stdlib.h>
#include <math.h>
extern "C" {
#include "cmockery.h"
}

// 自定义浮点比较（替代 assert_double_equal）
void assert_floats_equal(double expected, double actual, double epsilon) 
{
    assert_true(fabs(expected - actual) < epsilon);
}

// 计算器错误码
typedef enum {
    CALC_SUCCESS = 0,
    CALC_DIVIDE_BY_ZERO = -1
} CalcError;

// 计算器结构体
typedef struct {
    double result;
    CalcError error;
} Calculator;

// 初始化计算器
Calculator* calc_init() 
{
    Calculator* calc = (Calculator*)malloc(sizeof(Calculator));
    calc->result = 0;
    calc->error = CALC_SUCCESS;
    return calc;
}

// 释放计算器
void calc_free(Calculator* calc) 
{
    free(calc);
}

// 加法
void calc_add(Calculator* calc, double value) 
{
    calc->result += value;
}

// 减法
void calc_subtract(Calculator* calc, double value) {
    calc->result -= value;
}

// 乘法
void calc_multiply(Calculator* calc, double value) {
    calc->result *= value;
}

// 除法
CalcError calc_divide(Calculator* calc, double value) {
    if (fabs(value) < 1e-9) {
        calc->error = CALC_DIVIDE_BY_ZERO;
        return CALC_DIVIDE_BY_ZERO;
    }
    calc->result /= value;
    return CALC_SUCCESS;
}



// 测试初始化
void test_initial_value(void **state) 
{
    Calculator* calc = calc_init();
    assert_floats_equal(calc->result, 0.0, 1e-9);
    assert_int_equal(calc->error, CALC_SUCCESS);
    calc_free(calc);
}

// 测试加法
void test_add(void **state) 
{
    Calculator* calc = calc_init();
    calc_add(calc, 5.0);
    assert_floats_equal(calc->result, 5.0, 1e-9);
    calc_free(calc);
}

// 测试减法
void test_subtract(void **state) {
    Calculator* calc = calc_init();
    calc_subtract(calc, 3.0);
    assert_floats_equal(calc->result, -3.0, 1e-9);
    calc_free(calc);
}

// 测试乘法
void test_multiply(void **state) {
    Calculator* calc = calc_init();
    calc_add(calc, 2.0);
    calc_multiply(calc, 4.0);
    assert_floats_equal(calc->result, 8.0, 1e-9);
    calc_free(calc);
}

// 测试除法
void test_divide(void **state) {
    Calculator* calc = calc_init();
    calc_add(calc, 10.0);
    CalcError err = calc_divide(calc, 2.0);
    assert_int_equal(err, CALC_SUCCESS);
    assert_floats_equal(calc->result, 5.0, 1e-9);
    calc_free(calc);
}

// 测试除以零
void test_divide_by_zero(void **state) {
    Calculator* calc = calc_init();
    calc_add(calc, 5.0);
    CalcError err = calc_divide(calc, 0.0);
    assert_int_equal(err, CALC_DIVIDE_BY_ZERO);
    calc_free(calc);
}

int main(int argc, char **argv) {
    const UnitTest tests[] = {
        unit_test(test_initial_value),
        unit_test(test_add),
        unit_test(test_subtract),
        unit_test(test_multiply),
        unit_test(test_divide),
        unit_test(test_divide_by_zero)
    };
    return run_tests(tests);
}