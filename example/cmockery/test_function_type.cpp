#include <stdio.h>
#include <stdlib.h>
extern "C"{
    #include "cmockery.h"
}

// 1. Setup函数：测试前初始化（如分配资源）
void test_setup(void **state) {
    printf("执行Setup：初始化测试环境...\n");
    *state = malloc(1024);  // 示例：分配一块内存作为测试状态
    if (*state == NULL) {
        printf("Setup失败：内存分配失败\n");
    }
}

// 2. 实际测试函数：验证业务逻辑
void test_add_function(void **state) {
    printf("执行测试：验证加法逻辑...\n");
    int a = 2, b = 3;
    int result = a + b;
    if (result != 5) {
        printf("测试失败：%d + %d 应为 5，实际为 %d\n", a, b, result);
    } else {
        printf("测试成功：加法逻辑正确\n");
    }
    // 可以使用state中保存的资源（如从*state中读取初始化的数据）
}

// 3. Teardown函数：测试后清理（如释放资源）
void test_teardown(void **state) {
    printf("执行Teardown：清理测试环境...\n");
    if (*state != NULL) {
        free(*state);  // 释放setup中分配的内存
        *state = NULL;
    }
}

int main() 
{
    // 定义测试函数列表，指定每个函数的类型
    UnitTest tests[] = {
        {
            .name = "测试前初始化",
            .function = test_setup,
            .function_type = UNIT_TEST_FUNCTION_TYPE_SETUP
        },
        {
            .name = "加法逻辑测试",
            .function = test_add_function,
            .function_type = UNIT_TEST_FUNCTION_TYPE_TEST
        },
        {
            .name = "测试后清理",
            .function = test_teardown,
            .function_type = UNIT_TEST_FUNCTION_TYPE_TEARDOWN
        }
    };
    int test_count = sizeof(tests) / sizeof(UnitTest);

    // 模拟框架执行测试（按类型顺序执行）
    printf("开始执行测试套件...\n");

    // 1. 先执行所有setup函数
    for (int i = 0; i < test_count; i++) 
    {
        if (tests[i].function_type == UNIT_TEST_FUNCTION_TYPE_SETUP) 
        {
            printf("\n--- 执行 %s ---\n", tests[i].name);
            void *state = NULL;
            tests[i].function(&state);  // 调用setup函数
        }
    }

    // 2. 执行测试函数
    for (int i = 0; i < test_count; i++) 
    {
        if (tests[i].function_type == UNIT_TEST_FUNCTION_TYPE_TEST) 
        {
            printf("\n--- 执行 %s ---\n", tests[i].name);
            void *state = NULL;  // 实际框架中会传递setup初始化的state
            tests[i].function(&state);  // 调用测试函数
        }
    }

    // 3. 最后执行所有teardown函数
    for (int i = 0; i < test_count; i++) 
    {
        if (tests[i].function_type == UNIT_TEST_FUNCTION_TYPE_TEARDOWN) 
        {
            printf("\n--- 执行 %s ---\n", tests[i].name);
            void *state = NULL;
            tests[i].function(&state);  // 调用teardown函数
        }
    }

    printf("\n测试套件执行完毕\n");
    return 0;
}