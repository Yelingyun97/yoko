#include <iostream>
#include <vector>
#include "shared_ptr.h"

using namespace std;
using namespace yoko;

class Animal {
public:
    virtual  ~Animal() {}
    virtual void info() const = 0;
};

class Cat : public Animal {
public:
    Cat() {
        cout << "create a cat" << endl;
    }

    ~Cat() override {
        cout << "destroy a cat" << endl;
    }

    void info() const override {
        cout << "I am a cat" << endl;
    }
};

class Dog : public Animal {
public:
    Dog() {
        cout << "create a dog" << endl;
    }

    ~Dog() override {
        cout << "destroy a dog" << endl;
    }

    void info() const override {
        cout << "I am a dog" << endl;
    }
};


int main() {
    Cat *cat = new Cat();
    Dog *dog = new Dog();
    SharedPtr<Animal> cat_sp_;
    cat_sp_.reset(cat);
    SharedPtr<Animal> dog_sp_;
    dog_sp_.reset(dog);
    SharedPtr<Animal> sp_(cat_sp_);
    std::cout << "cat: " << cat_sp_.use_count() << std::endl;
    std::cout << "dog: " << dog_sp_.use_count() << std::endl;
    sp_ = dog_sp_;
    // sp_ = std::move(dog_sp_);
    std::cout << "cat: " << cat_sp_.use_count() << std::endl;
    std::cout << "dog: " << sp_.use_count() << std::endl;

    SharedPtr<Cat> sp2_cat = MakeShared<Cat>();
    return 0;
}