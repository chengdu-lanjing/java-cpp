#include <iostream>
#include <string.h>
#include <Common.h>

using namespace std;
using namespace com_lanjing_cpp_common;

namespace demo_array {
    struct Value {
        Value() {
            cout << "Value::Value() is invoked" << endl;
        }
        ~Value() {
            cout << "Value::~Value() of '" << this->name << "'is invoked" << endl;
        }
        char name[32]; // This is char[], not std::string because it can work normal when the constructor is not invoked.
    };
    void testValueArray(bool considerAsCxxType) {
        Arr<Value> values = Array<Value>::newInstance(3, considerAsCxxType ? ArrayElementType::CPP : ArrayElementType::C);
        for (int i = 0; i < values.length(); i++) {
            ostringstream oss;
            oss << "Item-" << i;
            strcpy(values[i].name, oss.str().c_str());
        }
    }
    Arr<int> createFibonacciArray(int length) {
        if (length < 2) {
            throw_new(IllegalArgumentException, "length must >= 2");
        }
        Arr<int> arr = Array<int>::newInstance(length, false);
        arr[0] = arr[1] = 1;
        for (int i = 2; i < arr.length(); i++) {
            arr[i] = arr[i - 2] + arr[i - 1];
        }
        return arr;
    }
    void setElementsToBeNegatvie(Arr<int> arr) {
        for (int &i : arr) {
                    i = -i;
            }
    }
}

using namespace demo_array;

int main(int argc, char *argv[]) {

    cout << "Test value array, consider the element type as C++ type with constructor and destructor" << endl;
    testValueArray(true);

    cout << "Test value array, consider the element type as C type without constructor and destructor" << endl;
    testValueArray(false);

    Arr<int> arr = createFibonacciArray(10);
    cout << "Change the fibonacci array" << endl;
    setElementsToBeNegatvie(arr);
    cout << "Print the element of changed fibonacci array" << endl;
    for (int i : arr) {
        cout << i << endl;
    }

#ifdef DEBUG
    cout << "The macro 'DEBUG' is defined so that array checks the whether the index is out of range" << endl;
    try_ {
        arr[arr.length()];
    } catch_(Exception, ex) {
        ex->printStackTrace();
    } end_try
#endif //DEBUG

    return 0;
}
