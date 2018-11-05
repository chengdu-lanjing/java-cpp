#!/bin/bash

function help {
    echo "-------------------------------------------------------------"
    echo "This is the menu"
    echo "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
    echo "h. Help, print all menu items"
    echo "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"
    echo "1. Memory management demos"
    echo "    1.1 Demo about Ref & WeakRef"
    echo "    1.2 Demo about object resurrection"
    echo "    1.3 Bad demo about memory leak"
    echo "    1.4 Integrative demo about complex object tree managment"
    echo "2. Exception demos"
    echo "    2.1 Demo about exception chain"
    echo "    2.2 Demo about how to catch and rethrow exception"
    echo "    2.3 Demo about how to use macro 'defer' to make up for the fact that C++ does not support 'finally'"
    echo "3. Array demo"
    echo "4. Functional interface demos"
    echo "    4.1 Demo about how to combine and split functional interfaces by '+', '-', '+=', '-='"
    echo "    4.2 Demo about how to use functional interface to support java bean event"
    echo "5. Multiple threads demos"
    echo "    5.1 Demo about blocking queue"
    echo "    5.2 Demo about simple thread pool"
    echo "    5.3 Demo about scheduled thread pool"
    echo "6. Logging demo"
    echo "7. HTTP demo (Please install curl first because it requires '*.h' and '*.so' of libcurl)"
    echo "8. Database demo (Please install sqlite3 first because it requires '*.h' and '*.so' of libsqlite3)"
    echo "-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -"
    echo "a. Run all demos"
    echo "x. Exit"
    echo "-------------------------------------------------------------"
}

function demo_header { # fun demo_header(title: String!): Unit
    echo "-------------------------------------------------------------"
    echo $1
    echo ">>>"
}

function memory {
    memory_simple
    memory_resurrection
    bad_demo_memory_leak
    memory_dom
}

function memory_simple {
    demo_header "1.1 Demo about Ref & WeakRef"
    ./memory_simple.sh
}

function memory_resurrection {
    demo_header "1.2 Demo about object resurrection"
    ./memory_resurrection.sh
}

function bad_demo_memory_leak {
    demo_header "1.3 Bad demo about memory leak"
    ./BAD_DEMO_memory_leak.sh
}

function memory_dom {
    demo_header "1.4 Integrative demo about complex object tree managment"
    ./memory_dom.sh
}

function exception {
    exception_chain
    exception_re_throw
    exception_finally
}

function exception_chain {
    demo_header "2.1 Demo about exception chain"
    ./exception_chain.sh
}

function exception_re_throw {
    demo_header "2.2 Demo about how to catch and rethrow exception"
    ./exception_rethrow.sh
}

function exception_finally {
    demo_header "2.3 Demo about how to use macro 'defer' to make up for the fact that C++ does not support 'finally'"
    ./exception_finally.sh
}

function array {
    demo_header "3. Array demo"
    ./array_simple.sh
}

function functional {
    functional_simple
    functional_event
}

function functional_simple {
    demo_header "4.1 Demo about how to combine and split functional interfaces by '+', '-', '+=', '-='"
    ./functional_simple.sh
}

function functional_event {
    demo_header "4.2 Demo about how to use functional interface to support java bean event"
    ./functional_event.sh
}

function threading {
    threading_queue
    threading_pool
    threading_scheduler
}

function threading_queue {
    demo_header "5.1 Demo about blocking queue"
    ./threading_queue.sh
}

function threading_pool {
    demo_header "5.2 Demo about simple thread pool"
    ./threading_pool.sh
}

function threading_scheduler {
    demo_header "5.3 Demo about scheduled thread pool"
    ./threading_scheduler.sh
}

function logging {
    demo_header "6. Logging"
    ./logging_simple.sh
}

function http {
    demo_header "7. HTTP demo"
    ./http_get.sh
}

function database {
    demo_header "8. Database demo"
    ./database_simple.sh
}


help

while true; do
    read -p "Please enter you choice: " choice
    case $choice in
    h)
        help
        ;;
    1)
        memory
        ;;
    1.1)
        memory_simple
        ;;
    1.2)
        memory_resurrection
        ;;
    1.3)
        bad_demo_memory_leak
        ;;
    1.4)
        memory_dom
        ;;
    2)
        exception
        ;;
    2.1)
        exception_chain
        ;;
    2.2)
        exception_rethrow
        ;;
    2.3)
        exception_finally
        ;;
    3)
        array
        ;;
    4)
        functional
        ;;
    4.1)
        functional_simple
        ;;
    4.2)
        functional_event
        ;;
    5)
        threading
        ;;
    5.1)
        threading_queue
        ;;
    5.2)
        threading_pool
        ;;
    5.3)
        threading_scheduler
        ;;
    6)
        logging
        ;;
    7)
        http
        ;;
    8)
        database
        ;;
    A|a)
        memory
        exception
        array
        functional
        threading
        logging
        http
        database
        ;;
    X|x)
        break
        ;;
    *)
        echo "Bad choice, please enter a valid choice"
    esac
    echo "-------------------------------------------------------------"
done
