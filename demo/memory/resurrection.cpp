#include <iostream>
#include <Common.h>
using namespace std;
using namespace com_lanjing_cpp_common;

namespace demo_memory {

    Ref<Object> globalRef;

    class ResurrectedObject : extends Object {
    public:
        ResurrectedObject(int resurrectionCount) : resurrectionCount(resurrectionCount) {}
        ~ResurrectedObject() {
            cout << "Finally, the C++ desctructor is invoked" << endl;
        }
    protected:
        virtual void resurrect() override {
            cout << "Important message : Object " << className(this) << " is resurrected!" << endl;
        }
        virtual void finalize() override {
            if (this->resurrectionCount-- > 0) {
                globalRef = this;
                cout << "Object " << className(this) << " is finalized, it  will be resurrected soon" << endl;
            } else {
                cout << "Object " << className(this) << " is finalized, but it won't be resurrected" << endl;
            }
        }
    private:
        int resurrectionCount;
    };
}

using namespace demo_memory;

int main(int argc, char *argv[]) {
    globalRef = new_<ResurrectedObject>(3);
    do {
        globalRef = nullptr;
    } while (globalRef != nullptr);
    return 0;
}
