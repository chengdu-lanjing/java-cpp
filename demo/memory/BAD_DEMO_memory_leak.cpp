#include <iostream>
#include <Common.h>

using namespace std;
using namespace com_lanjing_cpp_common;

namespace demo_memory {
    class Person : extends Object {
    public:
        Ref<Person> lover;
    };
    class Man : extends Person{};
    class Woman : extends Person{};
}
using namespace demo_memory;

int main(int argc, char *argv[]) {

    Ref<Person> u = new_<Man>();
    Ref<Person> a = new_<Woman>();
    Ref<Person> b = new_<Man>();
    Ref<Person> x = new_<Woman>();
    Ref<Person> y = new_<Man>();
    Ref<Person> z = new_<Woman>();

    // Create reference cycle base on one object
    u->lover = u;

    // Create reference cycle base on two objects
    a->lover = b;
    b->lover =a;

    // Create reference cycle base on three objects
    x->lover = y;
    y->lover = z;
    z->lover = x;

    // If this source file is compiled with "-DDEBUG",
    // The log about memory leak will be printed after main function executed.
    // The class names of leaked objects will be print too
    return 0;
}
