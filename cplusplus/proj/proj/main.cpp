#include <iostream>

int main() {
    // 定义一个 int 类型的变量
    int value = 42;

    // 定义一个指针，并指向 value 的地址
    int* ptr = &value;

    // 通过指针访问和打印 value 的值
    std::cout << "value = " << *ptr << std::endl;

    // 还可以通过指针修改值
    *ptr = 100;
    std::cout << "value (after change) = " << value << std::endl;

    return 0;
}
