//
// Created by 罗旭维 on 2021/11/18.
//

#ifndef MYLEVELDB_SKIPLIST_H
#define MYLEVELDB_SKIPLIST_H
#include "arena.h"
namespace myleveldb {
    template <typename Key, class Comparator>
    class SkipList {
    private:
        struct Node;
    public:
    // Create a new SkipList object that will use "cmp" for comparing keys,
    // and will allocate memory using "*arena".  Objects allocated in the arena
    // must remain allocated for the lifetime of the skiplist object.
    explicit SkipList(Comparator cmp, Arena* arena);

    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;

    // Insert key into the list.
    // REQUIRES: nothing that compares equal to key is currently in the list.
    void Insert(const Key& key);

    // Returns true iff an entry that compares equal to key is in the list.
    bool Contains(const Key& key) const;

    // Iteration over the contents of a skip list
    class Iterator {
    public:
        // Initialize an iterator over the specified list.
        // The returned iterator is not valid.
        explicit Iterator(const SkipList* list);

        // Returns true iff the iterator is positioned at a valid node.
        bool Valid() const;

        // Returns the key at the current position.
        // REQUIRES: Valid()
        const Key& key() const;

        // Advances to the next position.
        // REQUIRES: Valid()
        //注意next操作时沿节点最低层的指针前进的，实际上prev也是，这样保证可以遍历skiplist每个节点。
        void Next();

        // Advances to the previous position.
        // REQUIRES: Valid()
        void Prev();

        // Advance to the first entry with a key >= target
        void Seek(const Key& target);

        // Position at the first entry in list.
        // Final state of iterator is Valid() iff list is not empty.
        void SeekToFirst();

        // Position at the last entry in list.
        // Final state of iterator is Valid() iff list is not empty.
        void SeekToLast();

    private:
        const SkipList* list_;
        Node* node_;
        // Intentionally copyable
    };

    private:
    enum { kMaxHeight = 12 };

    inline int GetMaxHeight() const {
        return max_height_.load(std::memory_order_relaxed);
    }

    Node* NewNode(const Key& key, int height);
    int RandomHeight();
    bool Equal(const Key& a, const Key& b) const { return (compare_(a, b) == 0); }

    // Return true if key is greater than the data stored in "n"
    bool KeyIsAfterNode(const Key& key, Node* n) const;

    // Return the earliest node that comes at or after key.
    // Return nullptr if there is no such node.
    //
    // If prev is non-null, fills prev[level] with pointer to previous
    // node at "level" for every level in [0..max_height_-1].
    Node* FindGreaterOrEqual(const Key& key, Node** prev) const;

    // Return the latest node with a key < key.
    // Return head_ if there is no such node.
    Node* FindLessThan(const Key& key) const;

    // Return the last node in the list.
    // Return head_ if list is empty.
    Node* FindLast() const;

    //key的比较器
    Comparator const compare_;
    //内存池
    Arena* const arena_;  // Arena used for allocations of nodes
    //跳表的头结点
    Node* const head_;

    // Modified only by Insert().  Read racily by readers, but stale
    // values are ok.
    // 跳表的最大层数，不包括head节点，head节点的key为0，小于任何key，层数为kmaxheight=12
    std::atomic<int> max_height_;  // Height of the entire list

    // Read/written only by Insert().
    Random rnd_;

    };

    template <typename Key, class Comparator>
    struct SkipList<Key, Comparator>::Node {
        explicit Node(const Key& k) : key(k) {}

        //所携带的数据，memtable中为我们指明了实例化版本是char* key
        Key const key;

        // Accessors/mutators for links.  Wrapped in methods so we can
        // add the appropriate barriers as necessary.
        //返回本节点第n层的下一个节点指针
        Node* Next(int n) {
            assert(n >= 0);
            // Use an 'acquire load' so that we observe a fully initialized
            // version of the returned Node.
            return next_[n].load(std::memory_order_acquire);
        }
        //重设n层的next指针
        void SetNext(int n, Node* x) {
            assert(n >= 0);
            // Use a 'release store' so that anybody who reads through this
            // pointer observes a fully initialized version of the inserted node.
            next_[n].store(x, std::memory_order_release);
        }

        // No-barrier variants that can be safely used in a few locations.
        Node* NoBarrier_Next(int n) {
            assert(n >= 0);
            return next_[n].load(std::memory_order_relaxed);
        }
        void NoBarrier_SetNext(int n, Node* x) {
            assert(n >= 0);
            next_[n].store(x, std::memory_order_relaxed);
        }
    private:
        // 本节点的n层后向的指针
        //只有一个成员？因为节点的高度需要由一个随机算子产生，也就是说height对于每个节点是无法提前预知的，自然也就不能在node定义中确定next数组的大小，那么如何保证next数组足够用呢？
        // newnode为我们展现了这样的一种奇技淫巧，实际上新节点在确定height后，向内存申请空间时，申请了一块sizeof(Node) + sizeof(port::AtomicPointer) * (height - 1)大小的空间，确保next指针的空间足够
        std::atomic<Node*> next_[1];
    };

    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::NewNode(
                    const Key& key, int height) {
        char* const node_memory = arena_->AllocateAligned(
                sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
        return new (node_memory) Node(key);
    }

    template <typename Key, class Comparator>
    inline SkipList<Key, Comparator>::Iterator::Iterator(const SkipList* list) {
        list_ = list;
        node_ = nullptr;
    }

    template <typename Key, class Comparator>
    inline bool SkipList<Key, Comparator>::Iterator::Valid() const {
        return node_ != nullptr;
    }

    template <typename Key, class Comparator>
    inline const Key& SkipList<Key, Comparator>::Iterator::key() const {
        assert(Valid());
        return node_->key;
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::Next() {
        assert(Valid());
        node_ = node_->Next(0);
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::Prev() {
        // Instead of using explicit "prev" links, we just search for the
        // last node that falls before key.
        assert(Valid());
        node_ = list_->FindLessThan(node_->key);
        if (node_ == list_->head_) {
            node_ = nullptr;
        }
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::Seek(const Key& target) {
        node_ = list_->FindGreaterOrEqual(target, nullptr);
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::SeekToFirst() {
        node_ = list_->head_->Next(0);
    }

    template <typename Key, class Comparator>
    inline void SkipList<Key, Comparator>::Iterator::SeekToLast() {
        node_ = list_->FindLast();
        if (node_ == list_->head_) {
            node_ = nullptr;
        }
    }

    template <typename Key, class Comparator>
    int SkipList<Key, Comparator>::RandomHeight() {
        // Increase height with probability 1 in kBranching
        static const unsigned int kBranching = 4;
        int height = 1;
        while (height < kMaxHeight && ((rnd_.Next() % kBranching) == 0)) {
            height++;
        }
        assert(height > 0);
        assert(height <= kMaxHeight);
        return height;
    }

    template <typename Key, class Comparator>
    bool SkipList<Key, Comparator>::KeyIsAfterNode(const Key& key, Node* n) const {
        // null n is considered infinite
        return (n != nullptr) && (compare_(n->key, key) < 0);
    }

    //寻找关键字大于等于key值的最近节点，指针数组prev保存此节点每一层上访问的前一个节点。
    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node*
    SkipList<Key, Comparator>::FindGreaterOrEqual(const Key& key,
                                                  Node** prev) const {
        Node* x = head_;
        //首先获得跳表的最高层数，减一是数组next最大下标
        int level = GetMaxHeight() - 1;
        //查找操作开始
        while (true) {
            //跳表可以看成多层的链表，层数越高，链表的节点数越少，查找也就从高层数的链表开始
            //如果key在本节点node之后，继续前进
            //若果小于本节点node，把本节点的level层上的前节点指针记录进数组prev中，并跳向第一层的链表
            //重复上述过程，直至来到最底层
            Node* next = x->Next(level);
            if (KeyIsAfterNode(key, next)) {
                x = next;
            } else {
                if (prev != nullptr) prev[level] = x;
                if (level == 0) {
                    return next;
                } else {
                    level--;
                }
            }
        }
    }

    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node*
    SkipList<Key, Comparator>::FindLessThan(const Key& key) const {
        Node* x = head_;
        int level = GetMaxHeight() - 1;
        while (true) {
            assert(x == head_ || compare_(x->key, key) < 0);
            Node* next = x->Next(level);
            if (next == nullptr || compare_(next->key, key) >= 0) {//与FindGreaterOrEqual类似最终都走到 x < key <= next, greater or equal 就是 next， less than 就是x
                if (level == 0) {
                    return x;
                } else {
                    level--;
                }
            } else {
                x = next;
            }
        }
    }

    template <typename Key, class Comparator>
    typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::FindLast()
        const {
        Node* x = head_;
        int level = GetMaxHeight() - 1;
        while (true) {
            Node* next = x->Next(level);
            if (nullptr == next) {
                if (level == 0) {
                    return x;
                } else {
                    level--;
                }
            } else {
                x = next;
            }
        }
    }

    template <typename Key, class Comparator>
    SkipList<Key, Comparator>::SkipList(Comparator cmp, Arena* arena)
    : compare_(cmp),
    arena_(arena),
    head_(NewNode(0 /* any key will do */, kMaxHeight)),
    max_height_(1),
    rnd_(0xdeadbeef) {
        for (int i = 0; i < kMaxHeight; i++) {
            head_->SetNext(i, nullptr);
        }
    }

    template <typename Key, class Comparator>
    void SkipList<Key, Comparator>::Insert(const Key& key) {
        Node* prev[kMaxHeight];
        Node* x = FindGreaterOrEqual(key, prev);

        // Our data structure does not allow duplicate insertion
        assert(x == nullptr || !Equal(key, x->key));
        //随机生成结点层数，
        int height = RandomHeight();
        if (height > GetMaxHeight()) {//高出当前最大高度的层的prev都设置层head_
            for (int i = GetMaxHeight(); i < height; i++) {
                prev[i] = head_;
            }
            // It is ok to mutate max_height_ without any synchronization
            // with concurrent readers.  A concurrent reader that observes
            // the new value of max_height_ will see either the old value of
            // new level pointers from head_ (nullptr), or a new value set in
            // the loop below.  In the former case the reader will
            // immediately drop to the next level since nullptr sorts after all
            // keys.  In the latter case the reader will use the new node.
            max_height_.store(height, std::memory_order_relaxed);
        }

        x = NewNode(key, height);
        for (int i = 0; i < height; i++) {
            // NoBarrier_SetNext() suffices since we will add a barrier when
            // we publish a pointer to "x" in prev[i].
            x->NoBarrier_SetNext(i, prev[i]->NoBarrier_Next(i));
            //这里使用了内存屏障，保证下面这条SetNext一定在上面setNext后发生，
            prev[i]->SetNext(i, x);
        }
    }

    template <typename Key, class Comparator>
    bool SkipList<Key, Comparator>::Contains(const Key& key) const {
        Node* x = FindGreaterOrEqual(key, nullptr);
        if (x != nullptr && Equal(key, x->key)) {
            return true;
        } else {
            return false;
        }
    }
}



#endif //MYLEVELDB_SKIPLIST_H
