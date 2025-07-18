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