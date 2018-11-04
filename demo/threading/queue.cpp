#include <iostream>
#include <string>
#include <thread>
#include <BlockingQueue.h>

using namespace std;
using namespace com_lanjing_cpp_common;

namespace demo_threading {

    abstract class Message : extends Object {};

    class TextMessage : extends Message {
    public:
        TextMessage(const string &text) : text(text) {}
        const string &getText() const {
            return this->text;
        }
        virtual string toString() const override {
            ostringstream oss;
            oss << "Message \"" << this->text << '"';
            return oss.str();
        }
    private:
        string text;
    };

    class QuitMessage : extends Message {};

    void producer(Ref<BlockingQueue<Message>> messageQueue) {
        for (int i = 0; i < 5; i++) {
            ostringstream oss;
            oss << "This is message-" << (i + 1);
            messageQueue->put(new_<TextMessage>(oss.str()));
            this_thread::sleep_for(chrono::seconds(1));
        }
        messageQueue->put(new_<QuitMessage>());
    }
    void consumer(Ref<BlockingQueue<Message>> messageQueue) {
        while (true) {
            Ref<Message> message = messageQueue->take();
            if (message.dynamicCast<QuitMessage>() != nullptr) {
                break;
            }
            cout << "Received: " << message << endl;
        }
    }
};

using namespace demo_threading;

int main(int argc, char *argv[]) {
    Ref<BlockingQueue<Message>> messageQueue = new_<LinkedBlockingQueue<Message>>();
    thread producerThread(producer, messageQueue);
    thread consumerThread(consumer, messageQueue);
    producerThread.join();
    consumerThread.join();
    return 0;
}
