#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <type_traits>

extern "C" {
    #include <cmockery.h>
}

#include "cmockery.h"

// 被测试函数依赖的外部接口
int db_query(const char* sql, void** result);

// 被测试函数
int get_user_age(const char* username) {
    char sql[100];
    void* result;
    sprintf(sql, "SELECT age FROM users WHERE name='%s'", username);
    if (db_query(sql, &result) != 0) return -1;
    return *(int*)result;
}

// 模拟 db_query 函数（C/C++ 兼容）
int db_query(const char* sql, void** result) {
    check_expected(sql);               // 验证输入参数
    *result = mock_ptr();              // 显式获取指针类型返回值
    return mock_int();                 // 显式获取 int 类型返回值
}

// 测试用例
void test_get_user_age_success(void** state) {
    int expected_age = 30;
    const char* expected_sql = "SELECT age FROM users WHERE name='Alice'";
    
    expect_string(db_query, sql, expected_sql);  // 设置参数预期
    will_return(db_query, &expected_age);        // 注册指针返回值
    will_return(db_query, 0);                    // 注册整数返回值
    
    assert_int_equal(get_user_age("Alice"), expected_age);
}

// 测试套件
const UnitTest tests[] = {
    unit_test(test_get_user_age_success)
};

int main(int argc, char* argv[]) {
    return run_tests(tests);
}
