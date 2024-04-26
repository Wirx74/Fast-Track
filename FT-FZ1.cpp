#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

struct Student {
    int id;
    std::string name;
    int age;
};

class StudentDatabase {
public:
    // ���������� ������ ��������
    void addStudent(int id, const std::string& name, int age) {
        std::lock_guard<std::mutex> lock(mutex_);
        students_[id] = std::make_shared<Student>(Student{ id, name, age });
    }

    // �������� �������� �� ��������������
    void removeStudent(int id) {
        std::lock_guard<std::mutex> lock(mutex_);
        students_.erase(id);
    }

    // ��������� ���������� � �������� �� ��������������
    std::shared_ptr<Student> getStudent(int id) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = students_.find(id);
        if (it != students_.end()) {
            return it->second;
        }
        else {
            return nullptr;
        }
    }

private:
    std::map<int, std::shared_ptr<Student>> students_;
    std::mutex mutex_;
};

// �������, ����������� � ������ ������ (����� ������ � ����������)
void writerThread(StudentDatabase& db) {
    for (int i = 0; i < 5; ++i) {
        db.addStudent(i, "Student" + std::to_string(i), 20 + i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // ��� �������� ������
    }
}

// �������, ����������� �� ������ ������ (������ ������ �� ����������)
void readerThread(StudentDatabase& db) {
    for (int i = 0; i < 5; ++i) {
        auto student = db.getStudent(i);
        if (student) {
            std::cout << "ID: " << student->id << ", Name: " << student->name << ", Age: " << student->age << std::endl;
        }
        else {
            std::cout << "Student with ID " << i << " not found." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // ��� �������� ������
    }
}

int main() {
    StudentDatabase db;

    // �������� �������
    std::thread writer(writerThread, std::ref(db));
    std::thread reader(readerThread, std::ref(db));

    // �������� ���������� ������ �������
    writer.join();
    reader.join();

    return 0;
}