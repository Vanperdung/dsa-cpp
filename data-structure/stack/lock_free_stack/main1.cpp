#include <atomic>
#include <memory>
#include <string>
#include <iostream>
#include <thread>
#include <chrono>

template<typename T>
class LkFreeStack 
{
public:
    class Node
    {
    public:
        Node();
        ~Node();
        friend class LkFreeStack;
        
    private:
        T data;
        Node *mNext; 
    };

    LkFreeStack();
    ~LkFreeStack();

    bool push(Node*);
    Node* pop();

private:
    void free();

    std::atomic<Node*> mHead;
};

template<typename T>
LkFreeStack<T>::LkFreeStack()
{
    mHead.store(nullptr, std::memory_order_release);
}

template<typename T>
LkFreeStack<T>::~LkFreeStack()
{
    if (mHead.load(std::memory_order_acquire) != nullptr)
        free();
}

template<typename T>
bool LkFreeStack<T>::push(Node *node)
{
    if (node == nullptr)
        return false;

    node->mNext = mHead.load(std::memory_order_acquire);
    while (!mHead.compare_exchange_weak(node->mNext, node));

    return true;
}

template<typename T>
typename LkFreeStack<T>::Node* LkFreeStack<T>::pop()
{
    Node *node = mHead.load(std::memory_order_acquire);
    while (node && !mHead.compare_exchange_str(node, node->mNext));

    return node;
}

template<typename T>
void LkFreeStack<T>::free()
{
    while (mHead.load(std::memory_order_acquire) != nullptr)
    {
        Node *node = pop();
        delete node;
    }
}

template<typename T>
LkFreeStack<T>::Node::Node()
    : mNext(nullptr)
{
}

template<typename T>
LkFreeStack<T>::Node::~Node()
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
    using StackNode = LkFreeStack<Stuff>::Node;
    for (int i = 0; i < 1000000; i++)
    {
        StackNode* node = new StackNode();
        stuffStack->push(node);
    }
}

void reader()
{
    using StackNode = LkFreeStack<Stuff>::Node;
    StackNode *node = nullptr;

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