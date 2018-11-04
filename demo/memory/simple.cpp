#include <iostream>
#include <Common.h>
using namespace std;
using namespace com_lanjing_cpp_common;

namespace demo_memory {
    class TestObject : extends Object {
    protected:
        virtual void initialize() override {
            Object::initialize();
            cout << this << " has been created" << endl;
        }
        virtual void finalize() override {
            cout << this << " will be destroyed" << endl;
            Object::finalize();
        }
    };
}

int main(int argc, char *argv[]) {

    Ref<Object> ref1 = new_<demo_memory::TestObject>();
    Ref<Object> ref2 = ref1;
    WeakRef<Object> weakRef1 = ref1;
    WeakRef<Object> weakRef2 = weakRef1;

    cout << "demo> Initialize variables" << endl;
    cout << "ref1 = " << ref1 << ", ref2 = " << ref2 << ", weakRef1 = " << weakRef1 << ", weakRef2 = " << weakRef2 << endl;

    cout << "demo> Let ref1 be nullptr" << endl;
    ref1 = nullptr;
    cout << "ref1 = " << ref1 << ", ref2 = " << ref2 << ", weakRef1 = " << weakRef1 << ", weakRef2 = " << weakRef2 << endl;

    cout << "demo> Let ref2 be nullptr(The object will be destroyed and all the weak references will be cleared automatically)" << endl;
    ref2 = nullptr;
    cout << "ref1 = " << ref1 << ", ref2 = " << ref2 << ", weakRef1 = " << weakRef1 << ", weakRef2 = " << weakRef2 << endl;
    return 0;
}
