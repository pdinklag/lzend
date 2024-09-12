/**
 * ordered/btree/internal/btree_impl.hpp
 * part of pdinklag/ordered
 * 
 * MIT License
 * 
 * Copyright (c) 2023 Patrick Dinklage
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _ORDERED_BTREE_INTERNAL_IMPL_HPP
#define _ORDERED_BTREE_INTERNAL_IMPL_HPP

#include <cassert>
#include <cstddef>
#include <iterator>
#include <type_traits>

#include "concepts.hpp"
#include "../../query_result.hpp"

namespace ordered::btree::internal {

/**
 * \brief A B-tree
 * 
 * \tparam NodeImpl the node implementation
 */
template<BTreeNode NodeImpl>
class BTree {
private:
    using Key = typename NodeImpl::Key;
    using Value = typename NodeImpl::Value;
    static constexpr size_t degree_ = NodeImpl::capacity() + 1;

    static_assert(degree_ > 1);
    static_assert(degree_ < 65536);
    static_assert((degree_ % 2) == 1, "only odd maximum degrees are allowed"); // we only allow odd maximum degrees for the sake of implementation simplicity

    using ChildCount = typename std::conditional_t<degree_ < 256, uint8_t, uint16_t>;
    
    static constexpr size_t node_capacity = NodeImpl::capacity();
    static constexpr size_t split_right_ = node_capacity / 2;
    static constexpr size_t split_mid_ = split_right_ - 1;
    static constexpr size_t deletion_threshold_ = degree_ / 2;

    class Node {
    private:
        friend class BTree;
    
        NodeImpl impl_;
        ChildCount num_children_;
        Node** children_;
    
    public:
        inline bool is_leaf() const { return children_ == nullptr; }
        inline size_t size() const { return impl_.size(); }
        ChildCount num_children() const { return num_children_; }
        Node const* child(size_t const i) { return children_[i]; }

    private:
        inline bool is_empty() const { return size() == 0; }
        inline bool is_full() const { return size() == node_capacity; }

        Node() : children_(nullptr), num_children_(0) {
        }
        
        ~Node() {
            for(size_t i = 0; i < num_children_; i++) {
                delete children_[i];
            }
            if(children_) delete[] children_;
        }

        Node(Node const&) = default;
        Node(Node&&) = default;
        Node& operator=(Node const&) = default;
        Node& operator=(Node&&) = default;

        inline void allocate_children() {
            if(children_ == nullptr) {
                children_ = new Node*[degree_];
            }
        }

        void insert_child(size_t const i, Node* node) {
            assert(i <= num_children_);
            assert(num_children_ < degree_);
            allocate_children();
            
            // insert
            for(size_t j = num_children_; j > i; j--) {
                children_[j] = children_[j-1];
            }
            children_[i] = node;
            ++num_children_;
        }
        
        void erase_child(size_t const i) {
            assert(num_children_ > 0);
            assert(i <= num_children_);

            for(size_t j = i; j < num_children_-1; j++) {
                children_[j] = children_[j+1];
            }
            children_[num_children_-1] = nullptr;
            --num_children_;
            
            if(num_children_ == 0) {
                delete[] children_;
                children_ = nullptr;
            }
        }

        void split_child(const size_t i) {
            assert(!is_full());
            
            Node* y = children_[i];
            assert(y->is_full());

            // allocate new node
            Node* z = new Node();

            // get the middle value
            Key const m = y->impl_[split_mid_];
            Value value_m;

            // move the keys larger than middle from y to z and remove the middle
            {
                Key keybuf[split_right_];
                for(size_t j = 0; j < split_right_; j++) {
                    keybuf[j] = y->impl_[j + split_right_];
                    z->impl_.insert(keybuf[j], y->impl_.value(j + split_right_));
                }
                for(size_t j = 0; j < split_right_; j++) {
                    y->impl_.erase(keybuf[j]);
                }
                y->impl_.erase(m, value_m);
            }

            // move the m_children right of middle from y to z
            if(!y->is_leaf()) {
                z->allocate_children();
                for(size_t j = split_right_; j <= node_capacity; j++) {
                    z->children_[z->num_children_++] = y->children_[j];
                }
                y->num_children_ -= (split_right_ + 1);
            }

            // insert middle into this and add z as child i+1
            impl_.insert(m, value_m);
            insert_child(i + 1, z);

            // some assertions
            assert(children_[i] == y);
            assert(children_[i+1] == z);
            assert(z->impl_.size() == split_right_);
            if(!y->is_leaf()) assert(z->num_children_ == split_right_ + 1);
            assert(y->impl_.size() == split_mid_);
            if(!y->is_leaf()) assert(y->num_children_ == split_mid_ + 1);
        }
        
        void insert(Key const key, Value const value) {
            assert(!is_full());
            
            if(is_leaf()) {
                // we're at a leaf, insert
                impl_.insert(key, value);
            } else {
                // find the child to descend into
                auto const r = impl_.predecessor(key);
                size_t i = r.exists ? r.pos + 1 : 0;
                
                if(children_[i]->is_full()) {
                    // it's full, split it up first
                    split_child(i);

                    // we may have to increase the index of the child to descend into
                    if(key > impl_[i]) ++i;
                }

                // descend into non-full child
                children_[i]->insert(key, value);
            }
        }

        bool erase(Key const key) {
            assert(!is_empty());

            if(is_leaf()) {
                // leaf - simply remove
                return impl_.erase(key);
            } else {
                // find the item, or the child to descend into
                auto const r = impl_.predecessor(key);
                size_t i = r.exists ? r.pos + 1 : 0;
                
                if(r.exists && impl_[r.pos] == key) {
                    // key is contained in this internal node
                    assert(i < degree_);

                    Node* y = children_[i-1];
                    const size_t ysize = y->size();
                    Node* z = children_[i];
                    const size_t zsize = z->size();
                    
                    if(ysize >= deletion_threshold_) {
                        // find predecessor of key in y's subtree - i.e., the maximum
                        Node* c = y;
                        while(!c->is_leaf()) {
                            c = c->children_[c->num_children_-1];
                        }
                        Key const key_pred = c->impl_[c->size()-1];
                        Value value_pred = c->impl_.value(c->size()-1);

                        // replace key by predecssor in this node
                        impl_.erase(key);
                        impl_.insert(key_pred, value_pred);

                        // recursively delete key_pred from y
                        y->erase(key_pred);
                    } else if(zsize >= deletion_threshold_) {
                        // find successor of key in z's subtree - i.e., its minimum
                        Node* c = z;
                        while(!c->is_leaf()) {
                            c = c->children_[0];
                        }
                        Key const key_succ = c->impl_[0];
                        Value const value_succ = c->impl_.value(0);

                        // replace key by successor in this node
                        impl_.erase(key);
                        impl_.insert(key_succ, value_succ);

                        // recursively delete key_succ from z
                        z->erase(key_succ);
                    } else {
                        // assert balance
                        assert(ysize == deletion_threshold_ - 1);
                        assert(zsize == deletion_threshold_ - 1);

                        // remove key from this node
                        Value value;
                        impl_.erase(key, value);

                        // merge key and all of z into y
                        {
                            // insert key and keys of z into y
                            y->impl_.insert(key, value);
                            for(size_t j = 0; j < zsize; j++) {
                                y->impl_.insert(z->impl_[j], z->impl_.value(j));
                            }

                            // move m_children from z to y
                            if(!z->is_leaf()) {
                                assert(!y->is_leaf()); // the sibling of an inner node cannot be a leaf
                                
                                size_t next_child = y->num_children_;
                                for(size_t j = 0; j < z->num_children_; j++) {
                                    y->children_[next_child++] = z->children_[j];
                                }
                                y->num_children_ = next_child;
                                z->num_children_ = 0;
                            }
                        }

                        // delete z
                        erase_child(i);
                        delete z;

                        // recursively delete key from y
                        y->erase(key);
                    }
                    return true;
                } else {
                    // get i-th child
                    Node* c = children_[i];

                    if(c->size() < deletion_threshold_) {
                        // preprocess child so we can safely remove from it
                        assert(c->size() == deletion_threshold_ - 1);
                        
                        // get siblings
                        Node* left  = i > 0 ? children_[i-1] : nullptr;
                        Node* right = i < num_children_-1 ? children_[i+1] : nullptr;
                        
                        if(left && left->size() >= deletion_threshold_) {
                            // there is a left child, so there must be a splitter
                            assert(i > 0);
                            
                            // retrieve splitter and move it into c
                            Key const splitter = impl_[i-1];
                            assert(key > splitter); // sanity
                            Value splitter_value;
                            auto const rem_splitter = impl_.erase(splitter, splitter_value);
                            assert(rem_splitter);
                            c->impl_.insert(splitter, splitter_value);
                            
                            // move largest key from left sibling to this node
                            Key const llargest = left->impl_[left->size()-1];
                            assert(splitter > llargest); // sanity
                            Value llargest_value;
                            auto const rem_llargest = left->impl_.erase(llargest, llargest_value);
                            assert(rem_llargest);
                            impl_.insert(llargest, llargest_value);
                            
                            // move rightmost child of left sibling to c
                            if(!left->is_leaf()) {
                                Node* lrightmost = left->children_[left->num_children_-1];
                                left->erase_child(left->num_children_-1);
                                c->insert_child(0, lrightmost);
                            }
                        } else if(right && right->size() >= deletion_threshold_) {
                            // there is a right child, so there must be a splitter
                            assert(i < impl_.size());
                            
                            // retrieve splitter and move it into c
                            Key const splitter = impl_[i];
                            assert(key < splitter); // sanity
                            Value splitter_value;
                            auto const rem_splitter = impl_.erase(splitter, splitter_value);
                            assert(rem_splitter);
                            c->impl_.insert(splitter, splitter_value);
                            
                            // move smallest key from right sibling to this node
                            Key const rsmallest = right->impl_[0];
                            assert(rsmallest > splitter); // sanity
                            Value rsmallest_value;
                            auto const rem_rsmallest = right->impl_.erase(rsmallest, rsmallest_value);
                            assert(rem_rsmallest);
                            impl_.insert(rsmallest, rsmallest_value);
                            
                            // move leftmost child of right sibling to c
                            if(!right->is_leaf()) {
                                Node* rleftmost = right->children_[0];
                                right->erase_child(0);
                                c->insert_child(c->num_children_, rleftmost);
                            }
                        } else {
                            // this node is not a leaf and is not empty, so there must be at least one sibling to the child
                            assert(left != nullptr || right != nullptr);
                            assert(left == nullptr || left->size() == deletion_threshold_ - 1);
                            assert(right == nullptr || right->size() == deletion_threshold_ - 1);
                            
                            // select the sibling and corresponding splitter to mergre with
                            if(right != nullptr) {
                                // merge child with right sibling
                                Key const splitter = impl_[i];
                                assert(key < splitter); // sanity
                                
                                // move splitter into child as new median
                                Value splitter_value;
                                auto const rem_splitter = impl_.erase(splitter, splitter_value);
                                assert(rem_splitter);
                                c->impl_.insert(splitter, splitter_value);
                                
                                // move keys right sibling to child
                                for(size_t j = 0; j < right->size(); j++) {
                                    c->impl_.insert(right->impl_[j], right->impl_.value(j));
                                }
                                
                                if(!right->is_leaf()) {
                                    assert(!c->is_leaf()); // the sibling of an inner node cannot be a leaf
                                    
                                    // append m_children of right sibling to child
                                    size_t next_child = c->num_children_;
                                    for(size_t j = 0; j < right->num_children_; j++) {
                                        c->children_[next_child++] = right->children_[j];
                                    }
                                    c->num_children_ = next_child;
                                    right->num_children_ = 0;
                                }
                                
                                // delete right sibling
                                erase_child(i+1);
                                delete right;
                            } else {
                                // merge child with left sibling
                                Key const splitter = impl_[i-1];
                                assert(key > splitter); // sanity
                                
                                // move splitter into child as new median
                                Value splitter_value;
                                auto const rem_splitter = impl_.erase(splitter, splitter_value);
                                assert(rem_splitter);
                                c->impl_.insert(splitter, splitter_value);
                                
                                // move keys left sibling to child
                                for(size_t j = 0; j < left->size(); j++) {
                                    c->impl_.insert(left->impl_[j], left->impl_.value(j));
                                }
                                
                                if(!left->is_leaf()) {
                                    assert(!c->is_leaf()); // the sibling of an inner node cannot be a leaf
                                    
                                    // move m_children of child to the back
                                    size_t next_child = left->num_children_;
                                    for(size_t j = 0; j < c->num_children_; j++) {
                                        c->children_[next_child++] = c->children_[j];
                                    }
                                    c->num_children_ = next_child;
                                    
                                    // prepend m_children of left sibling to child
                                    for(size_t j = 0; j < left->num_children_; j++) {
                                        c->children_[j] = left->children_[j];
                                    }
                                    left->num_children_ = 0;
                                }
                                
                                // delete left sibling
                                erase_child(i-1);
                                delete left;
                            }
                        }
                    }
                    
                    // remove from subtree
                    return c->erase(key);
                }
            }
        }
    } __attribute__((__packed__));

private:
    size_t size_;
    Node* root_;

    Node const& leftmost_leaf() const {
        Node* node = root_;
        while(!node->is_leaf()) {
            node = node->children_[0];
        }
        return *node;
    }

    Node const& rightmost_leaf() const {
        Node* node = root_;
        while(!node->is_leaf()) {
            node = node->children_[node->num_children_ - 1];
        }
        return *node;
    }

public:
    /**
     * \brief Constructs an empty container
     */
    inline BTree() : size_(0), root_(new Node()) {
    }

    inline BTree(BTree&& other) {
        *this = other;
    }

    inline BTree& operator=(BTree&& other) {
        size_ = other.size_;
        root_ = other.root_;
        other.root_ = nullptr;
        return *this;
    }

    BTree(BTree const&) = delete;
    BTree& operator=(BTree const&) = delete;

    inline ~BTree() {
        if(root_) {
            delete root_;
        }
    }

    /**
     * \brief Finds the predecessor of the given key, if any
     * 
     * If the key is contained, it will be returned as its own predecessor.
     * 
     * \param x the key in question
     * \return the query result
     */
    inline QueryResult<Key, Value> predecessor(Key const x) const {
        Node* node = root_;
        
        bool exists = false;
        Key key;
        Value value;
        
        auto r = node->impl_.predecessor(x);
        while(!node->is_leaf()) {
            exists = exists || r.exists;
            if(r.exists) {
                key = node->impl_[r.pos];
                value = node->impl_.value(r.pos);
                if(key == x) {
                    return { true, key, value };
                }
            }
               
            size_t const i = r.exists ? r.pos + 1 : 0;
            node = node->children_[i];
            r = node->impl_.predecessor(x);
        }
        
        exists = exists || r.exists;
        if(r.exists) {
            key = node->impl_[r.pos];
            value = node->impl_.value(r.pos);
        }
        
        return { exists, key, value };
    }

    /**
     * \brief Finds the successor of the given key, if any
     * 
     * If the key is contained, it will be returned as its own successor.
     * 
     * \param x the key in question
     * \return the query result
     */
    inline QueryResult<Key, Value> successor(Key const x) const {
        Node* node = root_;
        
        bool exists = false;
        Key key;
        Value value;
        
        auto r = node->impl_.successor(x);
        while(!node->is_leaf()) {
            exists = exists || r.exists;
            if(r.exists) {
                key = node->impl_[r.pos];
                value = node->impl_.value(r.pos);
                if(key == x) {
                    return { true, key, value };
                }
            }

            size_t const i = r.exists ? r.pos : node->num_children_ - 1;
            node = node->children_[i];
            r = node->impl_.successor(x);
        }
        
        exists = exists || r.exists;
        if(r.exists) {
            key = node->impl_[r.pos];
            value = node->impl_.value(r.pos);
        }
        
        return { exists, key, value };
    }

    /**
     * \brief Finds the given key
     * 
     * \param x the key in question
     * \return the query result
     */
    inline QueryResult<Key, Value> find(Key const x) const {
        if(size() == 0) [[unlikely]] return QueryResult<Key, Value>::none();
        auto r = predecessor(x);
        return (r.exists && r.key == x) ? r : QueryResult<Key, Value>::none();
    }

    /**
     * \brief Tests whether the given key is contained
     * 
     * \param x the key in question
     * \return true iff the key is contained
     * \return false otherwise
     */
    inline bool contains(Key const x) const {
        auto const r = find(x);
        return r.exists;
    }

    /**
     * \brief Reports the minimum key contained
     * 
     * Querying this on an empty container results in undefined behaviour
     * 
     * \return the minimum key contained
     */
    inline Key min_key() const {
        return leftmost_leaf().impl_[0];
    }

    /**
     * \brief Reports the maximum key contained
     * 
     * Querying this on an empty container results in undefined behaviour
     * 
     * \return the maximum key contained
     */
    inline Key max_key() const {
        auto const& rightmost = rightmost_leaf();
        return rightmost.impl_[rightmost.size() - 1];
    }

    /**
     * \brief Reports the minimum key contained and the associated value, if any
     * 
     * \return the minimum contained
     */
    inline QueryResult<Key, Value> min() const {
        if(size() == 0) [[unlikely]] return QueryResult<Key, Value>::none();

        auto const& leftmost = leftmost_leaf();
        return { true, leftmost.impl_[0], leftmost.impl_.value(0) };
    }

    /**
     * \brief Reports the minimum key contained and the associated value, if any
     *
     * \return the maximum contained
     */
    inline QueryResult<Key, Value> max() const {
        if(size() == 0) [[unlikely]] return QueryResult<Key, Value>::none();

        auto const& rightmost = rightmost_leaf();
        auto const i = rightmost.size() - 1;
        return { true, rightmost.impl_[i], rightmost.impl_.value(i) };
    }

    /**
     * \brief Inserts the given key and associated value
     * 
     * Inserting a key that is already contained results in undefined behaviour.
     * 
     * \param key the key to insert
     * \param value the value to associate with the key
     */
    inline void insert(Key const key, Value const value) {
        if(root_->is_full()) {
            // root is full, split it up
            Node* new_root = new Node();
            new_root->insert_child(0, root_);

            root_ = new_root;
            root_->split_child(0);
        }
        root_->insert(key, value);
        ++size_;
    }

    /**
     * \brief Inserts the given key
     * 
     * The associated value will is default-constructed.
     * Inserting a key that is already contained results in undefined behaviour.
     * 
     * \param key the key to insert
     */
    inline void insert(Key const key) {
        insert(key, Value{});
    }

    /**
     * \brief Removes the given key
     * 
     * \param key the key to remove
     * \return true if the key was contained and has been removed by the operation
     * \return false if the key was not contained and thus nothing was removed
     */
    inline bool erase(Key const key) {
        assert(size_ > 0);
        
        bool const result = root_->erase(key);
        
        if(result) {
            --size_;
        }
        
        if(root_->size() == 0 && root_->num_children_ > 0) {
            assert(root_->num_children_ == 1);
            
            // root is now empty but it still has a child, make that new root
            Node* new_root = root_->children_[0];
            root_->num_children_ = 0;
            delete root_;
            root_ = new_root;
        }
        return result;
    }

    /**
     * \brief Clears the container
     */
    inline void clear() {
        delete root_;
        root_ = new Node();
        size_ = 0;
    }

    /**
     * \brief Reports the number of keys contained
     * 
     * \return the number of keys contained 
     */
    inline size_t size() const { return size_; }

    /**
     * \brief Reports whether the container is empty
     * 
     * \return true iff the container's size is zero
     * \return false iff there are any keys contained
     */
    inline bool empty() const { return size_ == 0; }
};

}

#endif
