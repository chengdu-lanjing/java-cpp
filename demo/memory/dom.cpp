#include <iostream>
#include <vector>
#include <map>
#include <Common.h>

using namespace std;
using namespace com_lanjing_cpp_common;

namespace demo_memory {

    class Node;
    class Element;
    class Text;
    class Comment;

    interface Visitor : extends Interface { //Interface type must inherits 'com_lanjing_cpp_common::Interface'
        virtual void visitBeginTag(Ref<Element> element) = 0;
        virtual void visitText(Ref<Text> text) = 0;
        virtual void visitComment(Ref<Comment> comment) = 0;
        virtual void visitEndTag(Ref<Element> element) = 0;
    };

    class Node : extends Object {
    public:
        Ref<Element> getParent() const {
            return this->parentElementWeakRef.get();
        }
        virtual void accept(Ref<Visitor> visitor) = 0;
    private:
        WeakRef<Element> parentElementWeakRef;
        friend class Element;
    };

    class Element : extends Node {
    public:
        Element(const string &name) : name(name) {}
        const string &getName() const {
            return this->name;
        }
        int getChildCount() const {
            return this->childNodes.size();
        }
        Ref<Node> getChild(int index) const {
            return this->childNodes[index];
        }
        Ref<Element> addChild(Ref<Node> childNode) {
            Ref<Element> oldParent = childNode->getParent();
            if (oldParent != nullptr) {
                oldParent->removeChild(childNode);
            }
            this->childNodes.push_back(childNode);
            childNode->parentElementWeakRef = this;
            return this;
        }
        Ref<Element> addChild(const string &text) {
            return this->addChild(new_<Text>(text));
        }
        Ref<Element> removeChild(Ref<Node> childNode) {
            Ref<Element> oldParent = childNode->getParent();
            if (oldParent == this) {
                auto itr = this->childNodes.begin();
                auto endItr = this->childNodes.end();
                                while (itr != endItr) {
                    if (*itr == childNode) {
                        (*itr)->parentElementWeakRef = nullptr;
                        this->childNodes.erase(itr);
                        break;
                    }
                    itr++;
                }
            }
            return this;
        }
        map<string, string> &getAttributeMap() { // The returned map can be modified, but use chain style method 'setAttribute' is a better choice
            return this->attributeMap;
        }
        Ref<Element> setAttribute(const string &name, const string &value) {
            this->attributeMap[name] = value;
            return this;
        }
        virtual void accept(Ref<Visitor> visitor) override {
            visitor->visitBeginTag(this);
            for (auto childNode : this->childNodes) {
                childNode->accept(visitor);
            }
            visitor->visitEndTag(this);
        }
    private:
        string name;
        vector<Ref<Node>> childNodes;
        map<string, string> attributeMap;
    };

    class Text : extends Node {
    public:
        Text(const string &value) : value(value) {}
        const string &getValue() const {
            return this->value;
        }
        virtual void accept(Ref<Visitor> visitor) override {
            visitor->visitText(this);
        }
    private:
        string value;
    };

    class Comment : extends Node {
    public:
        Comment(const string &value) : value(value) {}
        const string &getValue() const {
            return this->value;
        }
        virtual void accept(Ref<Visitor> visitor) override {
            visitor->visitComment(this);
        }
    private:
        string value;
    };

    class OutputVisitor : extends Object, implements Visitor { // extends 'Object' is required
    public:
        OutputVisitor(basic_ostream<char> &stream) : stream(stream) {}
        virtual void visitBeginTag(Ref<Element> element) override {
            this->appendIndent();
            stream << '<' << element->getName();
            for (auto &pair : element->getAttributeMap()) {
                stream << ' ' << pair.first << '=' << '"' << pair.second << '"';
            }
            if (element->getChildCount() == 0) {
                if (element->getName() == "script") {
                    stream << "></script>";
                } else {
                    stream << "/>";
                }
            } else {
                stream << '>';
            }
            stream << endl;
            ++this->tabCount;
        }
        virtual void visitText(Ref<Text> text) override {
            this->appendIndent();
            stream << text->getValue() << endl;
        }
        virtual void visitComment(Ref<Comment> comment) override {
            this->appendIndent();
            stream << "<!--" << comment->getValue() << "-->" << endl;
        }
        virtual void visitEndTag(Ref<Element> element) override {
            --this->tabCount;
            if (element->getChildCount() != 0) {
                this->appendIndent();
                stream << "</" << element->getName() << '>' << endl;
            }
        }
    private:
        void appendIndent() {
            for (int i = this->tabCount; i > 0; --i) {
                stream << '\t';
            }
        }
        basic_ostream<char> &stream;
        int tabCount = 0;
        interface_refcount() //Necessary for classes that implements some interfaces
    };
}

using namespace demo_memory;

int main(int argc, char *argv[]) {
    Ref<Element> html = new_<Element>("html")
        ->addChild(
            new_<Element>("head")
            ->addChild(
                new_<Element>("title")
                ->addChild("Helloworld for angular")
            )
            ->addChild(
                new_<Element>("link")
                ->setAttribute("rel", "stylesheet")
                ->setAttribute("type", "text/css")
                ->setAttribute("href", "material.css")
            )
            ->addChild(
                new_<Element>("script")
                ->setAttribute("type", "text/javascript")
                ->setAttribute("src", "angular.js")
            )
        )
        ->addChild(
            new_<Element>("body")
            ->setAttribute("ng-app", "helloworld")
            ->setAttribute("ng-init", "backgroundColor='white'")
            ->setAttribute("ng-style", "background-color: backgroundColor")
            ->addChild("Please change the color of this page")
            ->addChild(
                new_<Comment>("The background color of this page will be changed automatically when this input element is changed")
            )
            ->addChild(
                new_<Element>("input")
                ->setAttribute("type", "text")
                ->setAttribute("ng-model", "backgroundColor")
            )
        );
    html->accept(new_<OutputVisitor>(cout));
}
