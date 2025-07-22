#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include "cmockery.h"
}

// 自定义断言失败处理函数
void custom_assert_failed(const char *file, int line, const char *func, const char *format, ...) {
    fprintf(stderr, "Assertion failed: %s:%d: %s: ", file, line, func);
    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

// 手动实现 assert_null
#define assert_null(pointer) \
    do { \
        if ((pointer) != NULL) { \
            custom_assert_failed(__FILE__, __LINE__, __func__, \
                "expected NULL pointer, but got %p", (void *)(pointer)); \
        } \
    } while (0)

// 手动实现 assert_non_null
#define assert_non_null(pointer) \
    do { \
        if ((pointer) == NULL) { \
            custom_assert_failed(__FILE__, __LINE__, __func__, \
                "expected non-NULL pointer, but got NULL"); \
        } \
    } while (0)

// 链表节点结构
typedef struct Node 
{
    int data;
    struct Node* next;
} Node;

// 链表结构
typedef struct 
{
    Node* head;
    int size;
} LinkedList;

// 创建新节点
Node* create_node(int data) 
{
    Node* node = (Node*)malloc(sizeof(Node));
    node->data = data;
    node->next = NULL;
    return node;
}

// 创建链表
LinkedList* create_list() 
{
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    list->head = NULL;
    list->size = 0;
    return list;
}

// 销毁链表
void destroy_list(LinkedList* list) 
{
    Node* current = list->head;
    Node* next;
    while (current != NULL) 
    {
        next = current->next;
        free(current);
        current = next;
    }
    free(list);
}

// 在链表尾部添加元素
void add_to_list(LinkedList* list, int data) 
{
    Node* new_node = create_node(data);
    if (list->head == NULL) 
    {
        list->head = new_node;
    } 
    else 
    {
        Node* current = list->head;
        while (current->next != NULL) 
        {
            current = current->next;
        }
        current->next = new_node;
    }
    list->size++;
}

// 从链表中查找元素
Node* find_in_list(LinkedList* list, int data) 
{
    Node* current = list->head;
    while (current != NULL) 
    {
        if (current->data == data) 
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// 从链表中删除元素
int remove_from_list(LinkedList* list, int data) 
{
    if (list == NULL) return 0; 
    
    Node* current = list->head;
    Node* previous = NULL;

    while (current != NULL && current->data != data) 
    {
        previous = current;
        current = current->next;
    }

    if (current == NULL) 
    {
        printf("DEBUG: Element %d not found, returning 0\n", data);
        return 0;  // 未找到元素，返回 0（false）
    }

    // 删除节点
    if (previous == NULL) 
    {
        list->head = current->next;
    } 
    else 
    {
        previous->next = current->next;
    }

    free(current);
    list->size--;
    printf("DEBUG: Removed element %d, returning 1\n", data);
    return 1;  // 成功删除，返回 1（true）
}

// 测试链表初始化
void test_list_initialization(void **state) 
{
    LinkedList* list = create_list();
    assert_null(list->head);
    assert_int_equal(list->size, 0);
    destroy_list(list);
}

// 测试添加元素
void test_add_element(void **state) 
{
    LinkedList* list = create_list();
    add_to_list(list, 10);
    assert_non_null(list->head);
    assert_int_equal(list->head->data, 10);
    assert_int_equal(list->size, 1);
    destroy_list(list);
}

// 测试添加多个元素
void test_add_multiple_elements(void **state) 
{
    LinkedList* list = create_list();
    add_to_list(list, 10);
    add_to_list(list, 20);
    add_to_list(list, 30);

    Node* current = list->head;
    assert_int_equal(current->data, 10);
    assert_int_equal(current->next->data, 20);
    assert_int_equal(current->next->next->data, 30);
    assert_int_equal(list->size, 3);
    destroy_list(list);
}

// 测试查找元素
void test_find_element(void **state) 
{
    LinkedList* list = create_list();
    add_to_list(list, 10);
    add_to_list(list, 20);

    Node* node = find_in_list(list, 20);
    assert_non_null(node);
    assert_int_equal(node->data, 20);

    node = find_in_list(list, 30);
    assert_null(node);
    destroy_list(list);
}

// 测试删除元素
void test_remove_element(void **state) 
{
    LinkedList* list = create_list();
    add_to_list(list, 10);
    add_to_list(list, 20);
    add_to_list(list, 30);

    // 测试删除存在的元素
    int result = remove_from_list(list, 20);
    printf("DEBUG: result after removing 20 = %d\n", result);
    if (result != 1) 
    {
        printf("ERROR: Expected result to be 1, but got %d\n", result);
        exit(EXIT_FAILURE);
    }
    assert_int_equal(list->size, 2);

    Node* node = find_in_list(list, 20);
    assert_null(node);

    // 测试删除不存在的元素
    result = remove_from_list(list, 40);
    printf("DEBUG: result after removing 40 = %d\n", result);
    if (result != 0) 
    {
        printf("ERROR: Expected result to be 0, but got %d\n", result);
        exit(EXIT_FAILURE);  // 直接退出，避免依赖 assert_false
    }
    assert_int_equal(list->size, 2);

    destroy_list(list);
}

// 测试删除头节点
void test_remove_head(void **state) 
{
    LinkedList* list = create_list();
    add_to_list(list, 10);
    add_to_list(list, 20);

    int result = remove_from_list(list, 10);
    assert_true(result);
    assert_int_equal(list->head->data, 20);
    assert_int_equal(list->size, 1);
    destroy_list(list);
}

int main(int argc, char **argv) 
{
    const UnitTest tests[] = 
    {
        unit_test(test_list_initialization),
        unit_test(test_add_element),
        unit_test(test_add_multiple_elements),
        unit_test(test_find_element),
        unit_test(test_remove_element),
        unit_test(test_remove_head)
    };
    return run_tests(tests);
}