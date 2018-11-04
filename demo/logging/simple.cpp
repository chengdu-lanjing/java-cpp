#include <log/Logger.h>
#include <iostream>

using namespace std;
using namespace com_lanjing_cpp_common;
using namespace com_lanjing_cpp_common_log;

namespace demo_logging {

    class RawException : extends Exception {
    public:
        RawException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr)
            : Exception(exception_arg_prefix, message, cause) {}
    };
    class WrapperException : extends Exception {
    public:
        WrapperException(exception_param_prefix, const string &message, Ref<Exception> cause = nullptr)
            : Exception(exception_arg_prefix, message, cause) {}
    };

    static float sumSpeed(float v1, float v2) {
        static const float C = 299792458;
        return (v1 + v2) / (1 + v1 * v2 / (C * C));
    }
    static void generateRawException() {
        throw_new(RawException, "Holy crap, there is a raw exception");
    }
    static void generateWrapperException() {
        try_ {
            generateRawException();
        } catch_(Exception, ex) {
            throw_new(WrapperException, "This is a wrapper for raw exception", ex);
        } end_try
    }

    struct Business {
        declare_logger(Business)
    public:
        static void testArguments() {
            float v1 = 189765431, v2 = 159233007;
            logger().info("By relativity theory, {}m/s + {}/ms = {}m/s", v1, v2, sumSpeed(v1, v2));
        }
        static void testBigText() {
            logger().debug(
                "Peter Piper picked a peck of pickled peppers. \
                | A peck of pickled peppers Peter Piper picked. \
                | If Peter Piper picked a peck of pickled peppers, \
                | Where's the peck of pickled peppers Peter Piper picked?"
            );
        }
        static void testException() {
            try_ {
                generateWrapperException();
            } catch_(Exception, ex) {
                logger().error(ex, "OMG, there is an exception whose type is '{}'", Object::className(ex));
            } end_try
        }
    };
};

using namespace demo_logging;

int main(int argc, char *argv[]) {

    Ref<Appender> consoleAppender = new_<ConsoleAppender>(cerr);
    Ref<FileAppender> fileAppender = new_<FileAppender>("/tmp/logging.xml", false)
        ->setLayout(new_<XmlLayout>())
        ->setMinLevel(LogLevel::INFO);

    Configuration::root()->setLayout(new_<SimpleLayout>());
    Configuration::of("demo")
        ->setLevel(LogLevel::DBG)
        ->addAppender(consoleAppender)
        ->addAppender(fileAppender);

    Business::testArguments();
    Business::testBigText();
    Business::testException();

    fileAppender->flush();
    ifstream xmlFile("/tmp/logging.xml");
    cout << "----------Content of '/tmp/logging.xml'----------" << endl;
    string line;
    while (getline(xmlFile, line)) {
        cout << line << endl;
    }
    cout << "No end tag '</log-records>' here because it will be appended after the program exit, not now!";
    return 0;
}
