#include <math.h>
extern "C"{
#include "cmockery.h"
}

// 模拟被测试的数学库函数
double calculate_sqrt(double value) 
{
    if (value < 0) return -1.0; // 错误处理
    return sqrt(value);
}

int* allocate_array(size_t size) 
{
    if (size == 0) return NULL;
    return (int*)malloc(size * sizeof(int));
}

void free_array(int* arr) 
{
    free(arr);
}

// 测试浮点比较
void test_float_comparison(void** state) 
{
    double input = 2.0;
    double expected = 1.4142; // 近似值
    double epsilon = 0.0001;
    
    double result = calculate_sqrt(input);
    assert_floats_equal(expected, result, epsilon);
    
    // 测试失败案例（用于演示，应注释掉）
    assert_floats_equal(expected, result, 0.00001); // 误差太小会失败
}

// 测试 NULL 指针检查
void test_null_pointer(void** state) 
{
    int* ptr = allocate_array(0);
    assert_null(ptr);
    
    ptr = allocate_array(10);
    assert_non_null(ptr);
    
    free_array(ptr);
}

// 测试非 NULL 指针检查
void test_non_null_pointer(void** state) 
{
    int* ptr = allocate_array(5);
    assert_non_null(ptr);
    
    // 测试失败案例（用于演示，应注释掉）
    free_array(ptr);
    assert_non_null(ptr); // 释放后指针为 NULL，会触发断言
}

// 测试套件
const UnitTest tests[] = 
{
    unit_test(test_float_comparison),
    unit_test(test_null_pointer),
    unit_test(test_non_null_pointer)
};

int main(int argc, char* argv[]) 
{
    return run_tests(tests);
}