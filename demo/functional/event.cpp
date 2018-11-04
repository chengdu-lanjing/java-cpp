#include <iostream>
#include <Functional.h>

using namespace std;
using namespace com_lanjing_cpp_common;

namespace demo_functional {

    abstract class EventArgs : extends Object {
    public:
        EventArgs(Ref<Object> source) : source(source) {}
        Ref<Object> getSource() const {
            return this->source;
        }
    private:
        Ref<Object> source;
    };

    class NameChangedEventArgs : extends EventArgs {
    public:
        NameChangedEventArgs(Ref<Object> source, const string &oldName, const string &newName)
            : EventArgs(source), oldName(oldName), newName(newName) {}
        const string &getOldValue() const {
            return this->oldName;
        }
        const string &getNewValue() const {
            return this->newName;
        }
        virtual string toString() const override {
            ostringstream oss;
            oss << "The name of '"
                << this->getSource()
                << "' has been changed, the old name is '"
                << this->oldName
                << "' and the new name is '"
                << this->newName
                << '\'';
            return oss.str();
        }
    private:
        string oldName;
        string newName;
    };

    class Person : extends Object {
    public:
        const string &getName() const {
            return this->name;
        }
        void setName(const string &name) {
            if (this->name != name) {
                Ref<NameChangedEventArgs> e = new_<NameChangedEventArgs>(this, this->name, name);
                this->name = name;
                if (this->nameChangedListener != nullptr) {
                    this->nameChangedListener(e);
                }
            }
        }
        void addNameChangedListener(Ref<RefConsumer<NameChangedEventArgs>> listener) {
            this->nameChangedListener += listener;
        }
        void removeNameChangedListener(Ref<RefConsumer<NameChangedEventArgs>> listener) {
            this->nameChangedListener -= listener;
        }
    private:
        string name;
        Ref<RefConsumer<NameChangedEventArgs>> nameChangedListener;
    };

    class NameChangedEventHandler : extends Object {
    public:
        NameChangedEventHandler(const string identifier, int indent = 0) : identifier(identifier), indent(indent) {}
        void handle(Ref<NameChangedEventArgs> e) {
            this->coutIndent();
            cout << "Use the handler object '"
                << this->identifier
                << "' to handle the event:"
                << endl;
            this->coutIndent();
            cout << '\t'
                << e
                << endl;
        }
    private:
        void coutIndent() {
            for (int i = this->indent; i > 0; --i) {
                cout << '\t';
            }
        }
        string identifier;
        int indent;
    };
};

using namespace demo_functional;

int main(int argc, char *argv[]) {

    Ref<Person> person = new_<Person>();
    Ref<RefConsumer<NameChangedEventArgs>> listener1 = RefConsumer<NameChangedEventArgs>::of(
        new_<NameChangedEventHandler>("event-handler-1", 1),
        &NameChangedEventHandler::handle
    );
    Ref<RefConsumer<NameChangedEventArgs>> listener2 = RefConsumer<NameChangedEventArgs>::of(
        new_<NameChangedEventHandler>("event-handler-2", 1),
        &NameChangedEventHandler::handle
    );
    Ref<RefConsumer<NameChangedEventArgs>> listener3 = RefConsumer<NameChangedEventArgs>::of(
        new_<NameChangedEventHandler>("event-handler-3", 1),
        &NameChangedEventHandler::handle
    );

    person->addNameChangedListener(listener1);
    cout << "Change name to 'Kate'" << endl;
    person->setName("Kate");

    person->addNameChangedListener(listener2);
    cout << "Change name to 'Tom'" << endl;
    person->setName("Tom");

    person->addNameChangedListener(listener3);
    cout << "Change name to 'Mary'" << endl;
    person->setName("Mary");

    person->removeNameChangedListener(listener1);
    cout << "Change name to 'Linda'" << endl;
    person->setName("Linda");

    person->removeNameChangedListener(listener2);
    cout << "Change name to 'Smith'" << endl;
    person->setName("Smith");

    person->removeNameChangedListener(listener3);
    cout << "Change name to 'Lucy'" << endl;
    person->setName("Lucy");

    return 0;
}
