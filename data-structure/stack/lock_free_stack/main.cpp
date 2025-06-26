// stack<T>
// members: 
//          head:atomic<node<T>*>
// methods:
//          push(node<T>*)
//          Node<T>* pop()

// node<T>
// members:
//          data:T
//          next:node<T>*

// node<T> *a = new node<T>(d);
// st.push(a);

// lock-free stack
// push(node<T> *n) (is called by multiple threads)
//          if (!n) return;
//          n->next = head; // head.load(acquire) ensure r/w operations after that can see the changes by store(release)
//          head = n; // race condition

//          while (head != n->next) |
//                n->next = head;   | Atomic operation
//          head = n;               |

// pop()
//          if (!head) return NULL;
//          n = head;
//          head = head->next; // race condition

//          while (n != head)       |
//              n = head;           | Atomic operation
//          head = head->next       |

//          return n;

// if node is subclass of stack, stack must define method to create node ? 
// it should be like that, the type of node must be similar to stack

// cpp doesn't guarantee that the method can't be called during executing destruction by another thread.
// we might need to use smart pointer to avoid this case ??

#include <atomic>
#include <memory>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

template<typename T>
class Node;

template<typename T>
class LkFreeStack 
{
public:
    LkFreeStack();
    ~LkFreeStack();

    bool push(Node<T>*);
    Node<T>* pop();

private:
    void free();

    std::atomic<Node<T>*> mHead;
};

template<typename T>
LkFreeStack<T>::LkFreeStack()
{
    mHead.store(nullptr);
}

template<typename T>
LkFreeStack<T>::~LkFreeStack()
{
    if (mHead.load() != nullptr)
        free();
}

template<typename T>
bool LkFreeStack<T>::push(Node<T> *node)
{
    if (node == nullptr)
        return false;
    
    node->mNext = mHead.load(std::memory_order_acquire);
    while (!mHead.compare_exchange_weak(node->mNext, node));

    return true;
}

template<typename T>
Node<T>* LkFreeStack<T>::pop()
{
    Node<T> *node = mHead.load(std::memory_order_acquire);
    while (node && !mHead.compare_exchange_weak(node, node->mNext));

    return node;
}

template<typename T>
void LkFreeStack<T>::free()
{
    while (mHead.load(std::memory_order_acquire) != nullptr)
    {
        Node<T> *node = pop();
        delete node;
    }
}

template<typename T>
class Node
{
public:
    Node();
    ~Node();
    T mData;
    Node<T> *mNext;
};

template<typename T>
Node<T>::Node()
    : mNext(nullptr)
{
}

template<typename T>
Node<T>::~Node()
{
}

class Stuff
{
public:
    Stuff() = default;
    ~Stuff() = default;

    int x;
    float y;
    std::string z;
};

std::shared_ptr<LkFreeStack<Stuff>> stuffStack;
std::atomic<int> count = 0;

void writer()
{
    for (int i = 0; i < 1000000; i++)
    {
        Node<Stuff>* node = new Node<Stuff>();
        node->mData.x = i;
        node->mData.z = "w1";
        stuffStack->push(node);
    }
}

void reader()
{
    Node<Stuff> *node = nullptr;

    while ((node = stuffStack->pop()))
    {
        count.fetch_add(1);
        delete node;
    }
}

int main(int argc, char *argv[])
{
    stuffStack = std::make_shared<LkFreeStack<Stuff>>();

    std::thread w1(writer), w2(writer);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread r1(reader), r2(reader), r3(reader), r4(reader);

    w1.join();
    w2.join();
    r1.join();
    r2.join();
    r3.join();
    r4.join();

    std::cout << count << std::endl;

    return 0;
}