```
> ./main
double free or corruption (fasttop)
double free or corruption (fasttop)
[1]    308892 IOT instruction (core dumped)  ./main
```

This issue comes from the `while (node && !mHead.compare_exchange_weak(node, node->mNext));` inside the `pop()` method. 

Below is the timeline that causes double use-heap-after-free exception.

1. Thread A is running:
```
Node<T>* node = mHead.load();  // node = 0x1000`
```
2. Thread A is preempted.
3. Thread B is scheduled.
4. Thread B is running:
```
Node<T>* node = mHead.load();  // node = 0x1000
while (node && !mHead.compare_exchange_weak(node, node->mNext));
return node;
```
5. Thread B deletes `node`, `0x1000` now is invalid address:
```
count.fetch_add(1);
delete node; // node = 0x1000
```
6. Thread A resumes and will run:
```
while (node && !mHead.compare_exchange_weak(node, node->mNext)); // node = 0x1000
```

=> Step 6 causes the exception. `node = 0x1000` != `nullptr`, but `0x1000` now is an invalid address because it was freed before. Therefore `node->mNext` causes the use-heap-after-free exception.

