#include <iostream>

int main() {
    // ����һ�� int ���͵ı���
    int value = 42;

    // ����һ��ָ�룬��ָ�� value �ĵ�ַ
    int* ptr = &value;

    // ͨ��ָ����ʺʹ�ӡ value ��ֵ
    std::cout << "value = " << *ptr << std::endl;

    // ������ͨ��ָ���޸�ֵ
    *ptr = 100;
    std::cout << "value (after change) = " << value << std::endl;

    return 0;
}
