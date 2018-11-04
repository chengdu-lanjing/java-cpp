#include <iostream>
#include <Functional.h>

using namespace std;
using namespace com_lanjing_cpp_common;

int main(int argc, char *argv[]) {
    Ref<Function<double(double, double)>> plus = Function<double(double, double)>::of([=](double a, double b) {
        cout << "Function '+' is invoked" << endl;
        return a + b;
    });
    Ref<Function<double(double, double)>> substract = Function<double(double, double)>::of([=](double a, double b) {
        cout << "Function '-' is invoked" << endl;
        return a - b;
    });
    Ref<Function<double(double, double)>> multiply = Function<double(double, double)>::of([=](double a, double b) {
        cout << "Function '*' is invoked" << endl;
        return a * b;
    });
    Ref<Function<double(double, double)>> divide = Function<double(double, double)>::of([=](double a, double b) {
        cout << "Function '/' is invoked" << endl;
        return a / b;
    });

    Ref<Function<double(double, double)>> all = plus + substract + multiply + divide;
    cout << all(45, 7) << endl;

    all -= substract + divide;
    cout << all(45, 7) << endl;

    all -= plus + multiply;
    cout << "all is " << (all == nullptr ? "nullptr" : "a valid function") << endl;

    return 0;
};
