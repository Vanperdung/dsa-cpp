## Lock-Free Data Structures

For a data structure to qualify as **lock-free**, more than one thread must be able to access the (shared) data structure concurrently. Multiple threads can access this (shared) data structure concurrently, and some threads always make progress, even if others are paused or retrying.

A lock-free queue might allow one thread to push and one to pop at the same time as long as they don’t interfere in a way that causes one to block the other.

> **If two threads try to push new items at the same time?**  
> The undefined behavior might not appear, but only one thread wins to push its item to the queue — the other must retry later. It’s still lock-free because:  **At least one thread makes progress, even if the others are retrying.**

> 💡 **Tip**  
> If you're wondering why one wins and one retries, read here: [Atomic loads and stores](https://www.notion.so/Atomic-loads-and-stores-217b2c2bbea080cd8edae7f8e1867aaa?pvs=21)

---

### ✅ The Pros and Cons

#### **Pros:**

- The primary reason for using a lock-free data structure is to **enable maximum concurrency**.  
  Lock-based containers can cause one thread to wait for another ⇒ reducing concurrency efficiency.
- **No deadlocks:** If a thread dies while holding a lock, the data structure may be broken forever.  
  But with lock-free structures, if a thread dies mid-operation, only its data is lost — others continue.

#### **Cons:**

- Writing thread-safe code **without locks is harder**.  
  You must:
  - Avoid data races using **atomic operations**
  - Ensure changes are **visible in the correct order** (memory visibility)
  - Handle **memory reordering**
- **No deadlocks** — but **live locks** can happen:  
  Threads keep retrying and none succeed.
- **Lock-free ≠ always faster**:  
  Atomic operations are often **slower** than non-atomic ones, and cause **cache contention** (ping-ponging).

> 📢 **Always measure performance before choosing lock-free over lock-based designs.**
