#include <iostream>
#include <thread>
#include <ExecutorService.h>

using namespace std;
using namespace com_lanjing_cpp_common;

void threadSafeOutputWithTime(const string &text) {
    static Mutex mutex;
    Mutex::Scope scope(mutex);
    cout << "[millis: " << System::currentTimeMillis() << "] "  << text << endl;
}

int main(int argc, char *argv[]) {
    Ref<ExecutorService> executorService = new_<ExecutorService>(4);
    for (int i = 0; i < 12; i++) {
        executorService->execute(
            Runnable::of([=] {
                ostringstream oss;
                oss << "Task-" << (i + 1);
                threadSafeOutputWithTime(oss.str());
                this_thread::sleep_for(chrono::seconds(1));
            })
        );
    }

    this_thread::sleep_for(chrono::milliseconds(100));
    for (int i = 0; i < 3; i++) {
        cout << "--------------------------------------" << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
}
