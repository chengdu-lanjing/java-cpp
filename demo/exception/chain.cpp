#include <iostream>
#include <Common.h>

using namespace std;
using namespace com_lanjing_cpp_common;

namespace demo_exception {
    class AException : extends Exception {
    public:
        AException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr)
            : Exception(exception_arg_prefix, message, cause) {}
    };
    class BException : extends Exception {
    public:
        BException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr)
            : Exception(exception_arg_prefix, message, cause) {}
    };
    class CException : extends Exception {
    public:
        CException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr)
            : Exception(exception_arg_prefix, message, cause) {}
    };

    void businessProcessorA() {
        throw_new(AException, "Business Processor 'A' met some problems");
    }
    void businessProcessorB() {
        try_ {
            businessProcessorA();
        } catch_(Exception, ex) {
            throw_new(BException, "Business processor 'B' met some problems", ex);
        } end_try
    }
    void businessProcessorC() {
        try_ {
            businessProcessorB();
        } catch_(Exception, ex) {
            throw_new(CException, "Business processor 'C' met some prolbems", ex);
        } end_try
    }
}

int main(int argc, char *argv[]) {
    try_ {
        demo_exception::businessProcessorC();
    } catch_(Exception, ex) {
        ex->printStackTrace(cerr);
    } end_try
    return 0;
};
