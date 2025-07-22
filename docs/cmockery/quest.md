# mock

## test_mock

### 为啥需要两次will_return?


这个是：will_return 与函数行为的对应关系

在 cmockery 框架中，will_return 的作用是预先注册模拟函数（如 db_query）的返回值或输出参数，供模拟函数在被调用时 “返回” 给被测试代码（如 get_user_age）。

will_return 的调用次数，需要与模拟函数的实际行为严格对应：

    如果模拟函数需要返回一个值（如普通返回值），则需要 1 次 will_return；
    如果模拟函数不仅有返回值，还需要通过输出参数返回数据（如示例中的 result 指针），则需要额外的 will_return 来注册输出参数的值；
    简言之：模拟函数需要对外提供多少个 “结果”（返回值 + 输出参数），就需要多少次 will_return。

为什么这里需要两次 will_return？
在代码中，模拟函数 db_query 的行为是：

    通过输出参数 result 返回数据（指向 expected_age 的指针）；
    通过函数返回值返回状态码（0 表示成功）。


因此，需要两次 will_return 分别注册这两个 “结果”：
```c++
// 第一次：注册输出参数 result 的值（指向 expected_age 的指针）
will_return(db_query, &expected_age);
// 第二次：注册函数的返回值（状态码 0）
will_return(db_query, 0);
```


当 db_query 被 get_user_age 调用时：

    模拟函数会先通过 *result = mock_ptr() 获取第一个注册的值（&expected_age），赋值给输出参数；
    再通过 return mock_int() 获取第二个注册的值（0），作为函数返回值。

#### 不同will_return

**add**

```c++
// 模拟函数
int add(int a, int b) 
{
    return mock_int(); // 仅返回一个值
}

// 测试用例
void test_add(void **state) 
{
    will_return(add, 5); // 注册返回值 5
    assert_int_equal(add(2, 3), 5);
}
```

**多个输出参数 + 一个返回值的函数**

```c++
// 模拟函数
int divide(int a, int b, int* quotient, int* remainder) 
{
    *quotient = mock_int();   // 第一个输出参数
    *remainder = mock_int();  // 第二个输出参数
    return mock_int();        // 返回值（状态码）
}

// 测试用例
void test_divide(void** state) 
{
    will_return(divide, 3);   // 商
    will_return(divide, 1);   // 余数
    will_return(divide, 0);   // 状态码（成功）
    
    int q, r;
    assert_int_equal(divide(10, 3, &q, &r), 0);
    assert_int_equal(q, 3);
    assert_int_equal(r, 1);
}
```

## 如何实现：运行测试用例

```c++
/**
 * @brief 内部实现：运行单个测试函数（供 run_test() 宏调用）。
 */
int _run_test(
    const char * const function_name, const UnitTestFunction Function,
    void ** const state, const UnitTestFunctionType function_type,
    const void* const heap_check_point);

// _run_test
if (setjmp(global_run_test_env) == 0) 
{
    Function(state ? state : &current_state);
    // 检查是否有未被使用的预设值。若存在，打印错误并调用
    fail_if_leftover_values(function_name);

    /* If this is a setup function then ignore any allocated blocks
        * only ensure they're deallocated on tear down. */
    if (function_type != UNIT_TEST_FUNCTION_TYPE_SETUP) 
    {
        fail_if_blocks_allocated(check_point, function_name);
    }

    global_running_test = 0;

    if (function_type == UNIT_TEST_FUNCTION_TYPE_TEST) 
    {
        print_message("%s: Test completed successfully.\n", function_name);
    }
    rc = 0;
} 
else 
{
    global_running_test = 0;
    print_message("%s: Test failed.\n", function_name);
}
```

首次_run_test函数执行时候，会执行到Function(state ? state : &current_state); 从而执行用户的测试用例里

假设用户测试函数如下：
```c++
// example/cmockery/test_cmockery.cpp

// 测试函数：验证 add(-1, 1) 是否等于 0
void test_add_negative(void **state) 
{
    (void) state; // 未使用参数
    assert_int_equal(add(-1, 1), 0);
}
```

而assert_int_equal本身是一个宏函数定义：
```c++
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

void _assert_int_equal(
        const LargestIntegralType a, const LargestIntegralType b,
        const char * const file, const int line) 
{
    if (!values_equal_display_error(a, b)) 
    {
        _fail(file, line);
    }
}

    /* Returns 1 if the specified values are equal.  If the values are not equal
    * an error is displayed and 0 is returned. */
    static int values_equal_display_error(const LargestIntegralType left,
                                        const LargestIntegralType right) 
    {
        const int equal = left == right;
        if (!equal) 
        {
            print_error(LargestIntegralTypePrintfFormat " != "
                        LargestIntegralTypePrintfFormat "\n", left, right);
        }

        return equal;
    }

    void _fail(const char * const file, const int line) 
    {
        print_error("ERROR: " SOURCE_LOCATION_FORMAT " Failure!\n", file, line);
        exit_test(1);
    }

// Exit the currently executing test.
static void exit_test(const int quit_application) 
{
    if (global_running_test) 
    {
        longjmp(global_run_test_env, 1);
    } 
    else if (quit_application) 
    {
        exit(-1);
    }
}
```

调用链清晰：
- _assert_int_equal
- values_equal_display_error：实际的判等函数
- _fail： 不想等则进入此函数，实际是调用exit_test函数
- exit_test：实际是longjmp进行跳转