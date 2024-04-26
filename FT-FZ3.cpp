#include <iostream>

int main() {
    int arr[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    int* ptr = &arr[0];

    std::cout << "The 4th element of the array: " << *(ptr + 3) << std::endl;

    return 0;
}
