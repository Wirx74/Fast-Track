#include <iostream>
#include <stdexcept>

template<typename T>
class SingleList {
public:
    SingleList();
    ~SingleList();

    void pop_front();
    void push_back(T data);
    void clear();
    int GetSize() { return Size; }
    void push_front(T data);
    void insert(T value, int index);
    void removeAt(int index);
    T& operator[](const int index);

private:
    struct Node {
        T data;
        Node* pNext;

        Node(T data = T(), Node* pNext = nullptr) : data(data), pNext(pNext) {}
    };

    int Size;
    Node* head;
};

template<typename T>
SingleList<T>::SingleList() : Size(0), head(nullptr) {}

template<typename T>
SingleList<T>::~SingleList() {
    clear();
}

template<typename T>
void SingleList<T>::pop_front() {
    if (head == nullptr) return;
    Node* temp = head;
    head = head->pNext;
    delete temp;
    Size--;
}

template<typename T>
void SingleList<T>::push_back(T data) {
    if (head == nullptr) {
        head = new Node(data);
    }
    else {
        Node* current = head;
        while (current->pNext != nullptr) {
            current = current->pNext;
        }
        current->pNext = new Node(data);
    }
    Size++;
}

template<typename T>
void SingleList<T>::clear() {
    while (head != nullptr) {
        Node* temp = head;
        head = head->pNext;
        delete temp;
    }
    Size = 0;
}

template<typename T>
void SingleList<T>::push_front(T data)
{
    head = new Node(data, head);
    Size++;
}

template<typename T>
void SingleList<T>::insert(T value, int index)
{
    if (index == 0) {
        push_front(value);
    }
    else {
        Node* previous = head;
        for (int i = 0; i < index - 1; ++i) {
            if (previous == nullptr) {
                throw std::out_of_range("Index out of range");
            }
            previous = previous->pNext;
        }

        if (previous == nullptr) {
            throw std::out_of_range("Index out of range");
        }

        previous->pNext = new Node(value, previous->pNext);
        Size++;
    }
}

template<typename T>
void SingleList<T>::removeAt(int index)
{
    if (index < 0 || index >= Size) {
        throw std::out_of_range("Index out of range");
    }
    if (index == 0) {
        pop_front();
    }
    else {
        Node* previous = head;
        for (int i = 0; i < index - 1; ++i) {
            if (previous == nullptr) {
                throw std::out_of_range("Index out of range");
            }
            previous = previous->pNext;
        }

        if (previous == nullptr || previous->pNext == nullptr) {
            throw std::out_of_range("Index out of range");
        }

        Node* toDelete = previous->pNext;
        previous->pNext = toDelete->pNext;
        delete toDelete;
        Size--;
    }
}

template<typename T>
T& SingleList<T>::operator[](const int index) {
    if (index < 0 || index >= Size) {
        throw std::out_of_range("Index out of range");
    }
    int counter = 0;
    Node* current = head;
    while (current != nullptr) {
        if (counter == index) {
            return current->data;
        }
        current = current->pNext;
        counter++;
    }
    throw std::out_of_range("Index out of range");
}


int main() {
    SingleList<int> slst;
    slst.push_back(5);
    slst.push_back(10);
    slst.push_back(15);

    std::cout << slst.GetSize() << std::endl;
    std::cout << slst[2] << std::endl;
    slst.clear();
    std::cout << slst.GetSize() << std::endl;
    return 0;
}