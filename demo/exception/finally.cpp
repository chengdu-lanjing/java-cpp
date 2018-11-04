#include <iostream>
#include <functional>
#include <Common.h>

using namespace std;
using namespace com_lanjing_cpp_common;

namespace demo_exception {
    void testWrapper(function<void()> testCore) {

        defer([=]{ cout << "Teardown-1" << endl; });
        defer([=]{ cout << "Teardown-2" << endl; });
        defer([=]{ cout << "Teardonw-3" << endl; });

        cout << "Setup-1" << endl;
        cout << "Setup-2" << endl;
        cout << "Setup-3" << endl;

        testCore();
    }
    void goodTest() {
        cout << "Good test without exception" << endl;
    }
    void badTest() {
        throw_new(Exception, "Bad test with exception");
    }
}

using namespace demo_exception;

int main(int argc, char *argv[]) {

    testWrapper(goodTest);

    try_ {
        testWrapper(badTest);
    } catch_(Exception, ex) {
        ex->printStackTrace(cerr);
    } end_try

    return 0;
}
