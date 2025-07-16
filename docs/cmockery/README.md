[toc]

# cmockery

## 编译警告处理

```shell
error: src/cmockery/src/cmockery.c:467:35: error: cast from pointer to integer of different size [-Werror=pointer-to-int-cast]
  467 |     const unsigned int children = (unsigned int)cleanup_value_data;
      |                                   ^
src/cmockery/src/cmockery.c:471:15: error: cast to pointer from integer of different size [-Werror=int-to-pointer-cast]
  471 |               (void*)(children - 1));
      |               ^
src/cmockery/src/cmockery.c: In function ‘mock_assert’:
src/cmockery/src/cmockery.c:1214:47: error: cast from pointer to integer of different size [-Werror=pointer-to-int-cast]
 1214 |             longjmp(global_expect_assert_env, (int)expression);
      |                                               ^
src/cmockery/src/cmockery.c: In function ‘_test_free’:
src/cmockery/src/cmockery.c:1397:18: error: cast from pointer to integer of different size [-Werror=pointer-to-int-cast]
 1397 |     _assert_true((int)ptr, "ptr", file, line);
      |                  ^
cc1: all warnings being treated as errors
```

```c++

// a/src/cmockery/src/cmockery.c
// b/src/cmockery/src/cmockery.c

// @@ -464,7 +465,7 @@ free_symbol_map_value(const void *value, void *cleanup_value_data) 
 
     /* 减少子测试的数量 */
// -    const unsigned int children = (unsigned int)cleanup_value_data;
// +    const uintptr_t children = (uintptr_t)cleanup_value_data;
// -              (void*)(children - 1));
// +              (void*)(uintptr_t)(children - 1));
 
// @@ -1211,7 +1212,7 @@ void mock_assert(bool expression, const char *expression_string, const char 

// -          longjmp(global_expect_assert_env, (int)expression);
// +          longjmp(global_expect_assert_env, (intptr_t)expression);

 
// @@ -1394,7 +1395,7 @@ void _test_free(void *ptr, const char *file, int line)
// -    _assert_true((int)ptr, "ptr", file, line);
// +    _assert_true((intptr_t)ptr, "ptr", file, line);


```