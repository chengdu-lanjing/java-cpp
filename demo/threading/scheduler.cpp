#include <iostream>
#include <thread>
#include <ExecutorService.h>
#include <Auxiliary.h>

using namespace std;
using namespace com_lanjing_cpp_common;

void threadSafeOutputWithTime(const string &text) {
    static Mutex mutex;
    Mutex::Scope scope(mutex);
    cout << "[millis: " << System::currentTimeMillis() << "] "  << text << endl;
}

int main(int argc, char *argv[]) {

    Ref<ScheduledExecutorService> scheduledExecutorService = new_<ScheduledExecutorService>(1);

    cout << "--------------------schedule--------------------" << endl;
    scheduledExecutorService->schedule(
        Runnable::of([=] { threadSafeOutputWithTime("Scheduled task-A"); }),
        1000
    );
    threadSafeOutputWithTime("Scheduled-A task will be executed 1 second later");
    scheduledExecutorService->schedule(
        Runnable::of([=] { threadSafeOutputWithTime("Scheduled task-B"); }),
        500
    );
    threadSafeOutputWithTime("Scheduled-B task will be executed 0.5 second later");
    this_thread::sleep_for(chrono::milliseconds(1000 + 100));

    cout << "--------------------sheduleAtFixedRate--------------------" << endl;
    Ref<ScheduledFuture> scheduledFuture = scheduledExecutorService->scheduleAtFixedRate(
        Runnable::of([=] {
            threadSafeOutputWithTime("Scheduled task at fixed rate: 500ms");
            this_thread::sleep_for(chrono::milliseconds(500));
        }),
        1000,
        1000
    );
    this_thread::sleep_for(chrono::milliseconds(5 * 1000 + 100));
    scheduledFuture->cancel(); //Cancel "fixedRateAtFixedRate" to prepare "scheduledWithFixedDelay"

    cout << "--------------------sheduleWithFixedDelay--------------------" << endl;
    scheduledExecutorService->scheduleWithFixedDelay(
        Runnable::of([=] {
            threadSafeOutputWithTime("Scheduled task with fixed delay: 500ms");
            this_thread::sleep_for(chrono::milliseconds(500));
        }),
        1000,
        1000
    );
    this_thread::sleep_for(chrono::milliseconds(5 * 1000 + 100));

    return 0;
}
