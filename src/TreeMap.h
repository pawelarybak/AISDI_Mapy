#ifndef AISDI_MAPS_TREEMAP_H
#define AISDI_MAPS_TREEMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>

namespace aisdi {

    template <typename KeyType, typename ValueType>
    class TreeMap {
    public:
        using key_type = KeyType;
        using mapped_type = ValueType;
        using value_type = std::pair<const key_type, mapped_type>;
        using size_type = std::size_t;
        using reference = value_type&;
        using const_reference = const value_type&;

        class ConstIterator;
        class Iterator;
        using iterator = Iterator;
        using const_iterator = ConstIterator;

        using node = struct TreeNode {
            value_type val;
            TreeNode* parent;
            TreeNode* leftChild;
            TreeNode* rightChild;
            int height;

            TreeNode() : val(std::make_pair(key_type(), mapped_type())), parent(nullptr), leftChild(nullptr),
                         rightChild(nullptr), height(0) {}

            TreeNode(value_type value, TreeNode* parent=nullptr) : val(value), parent(parent), leftChild(nullptr),
                                                                   rightChild(nullptr), height(0) {}

            key_type key() {
                return val.first;
            }

            mapped_type& value() {
                return val.second;
            }
        };
        using node_pointer = node*;

        TreeMap() : root(nullptr), size(0) {}

        TreeMap(std::initializer_list<value_type> list) : TreeMap() {
            for (auto& val : list) {
                this->operator[](val.first) = val.second;
            }
        }

        TreeMap(const TreeMap& other) : TreeMap() {
            for (auto& val : other) {
                this->operator[](val.first) = val.second;
            }
        }

        TreeMap(TreeMap&& other) {
            this->root = other.root;
            this->size = other.size;
            other.root = nullptr;
        }

        ~TreeMap() {
            clearTree();
        }

        TreeMap& operator=(const TreeMap& other) {
            if (this == &other) {
                return *this;
            }
            clearTree();
            for (auto& val : other) {
                this->operator[](val.first) = val.second;
            }
            return *this;
        }

        TreeMap& operator=(TreeMap&& other) {
            if (this == &other) {
                return *this;
            }
            clearTree();
            this->root = other.root;
            this->size = other.size;
            other.root = nullptr;
            return *this;
        }

        bool isEmpty() const {
            return getSize() == 0;
        }

        mapped_type& operator[](const key_type& key) {
            node_pointer* node_placeholder = &root;
            node_pointer parent = nullptr;

            if (root == nullptr) {
                // Tree empty -> creating new node
                root = new TreeNode(std::make_pair(key, mapped_type()));
                ++size;
                return root->value();
            }

            while (*node_placeholder != nullptr && (*node_placeholder)->key() != key) {
                parent = *node_placeholder;
                if ((*node_placeholder)->key() > key) {
                    node_placeholder = &(*node_placeholder)->leftChild;
                }
                else {
                    node_placeholder = &(*node_placeholder)->rightChild;
                }
            }

            if (*node_placeholder != nullptr) {
                // Key found
                return (*node_placeholder)->value();
            }
            // Key not found -> creating new node
            *node_placeholder = new TreeNode(std::make_pair(key, mapped_type()), parent);
            // After rebalance node_placeholder might reference something else
            node_pointer retVal = *node_placeholder;
            ++size;
            rebalance(parent);

            return retVal->value();
        }

        const mapped_type& valueOf(const key_type& key) const {
            return (*find(key)).second;
        }

        mapped_type& valueOf(const key_type& key) {
            return (*find(key)).second;
        }

        const_iterator find(const key_type& key) const {
            node_pointer currentNode = root;
            while (currentNode != nullptr && currentNode->key() != key) {
                if (currentNode->key() > key) {
                    currentNode = currentNode->leftChild;
                }
                else {
                    currentNode = currentNode->rightChild;
                }
            }

            return const_iterator(*this, currentNode);
        }

        iterator find(const key_type& key) {
            node_pointer currentNode = root;
            while (currentNode != nullptr && currentNode->key() != key) {
                if (currentNode->key() > key) {
                    currentNode = currentNode->leftChild;
                }
                else {
                    currentNode = currentNode->rightChild;
                }
            }

            return iterator(*this, currentNode);
        }

        void remove(const key_type& key) {
            remove(find(key));
        }

        void remove(const const_iterator& it) {
            if (it == cend()) {
                throw std::out_of_range("Removing end iterator");
            }

            auto deletedNode = it.currentNode;
            if (deletedNode->leftChild != nullptr && deletedNode->rightChild != nullptr) {
                // Node inside of the tree - swap with next value
                iterator swappedNode = it;
                ++swappedNode;
//                it.currentNode->val = swappedNode.currentNode->val;
                override(it.currentNode, swappedNode.currentNode);
                deletedNode = swappedNode.currentNode;
            }

            auto parentOfDeleted = deletedNode->parent;
            node_pointer* deletedNodePointer;
            if (deletedNode->parent == nullptr) {
                deletedNodePointer = &root;
            }
            else if (deletedNode->parent->leftChild == deletedNode) {
                deletedNodePointer = &deletedNode->parent->leftChild;
            }
            else {
                deletedNodePointer = &deletedNode->parent->rightChild;
            }

            if (deletedNode->leftChild == nullptr && deletedNode->rightChild == nullptr) {
                // Deleting leaf
                *deletedNodePointer = nullptr;
            }
            else {
                // Node has only one branch
                auto branch = deletedNode->rightChild == nullptr ? deletedNode->leftChild : deletedNode->rightChild;
                branch->parent = deletedNode->parent;
                *deletedNodePointer = branch;
            }
            delete deletedNode;
            --size;
            rebalance(parentOfDeleted);
        }

        size_type getSize() const {
            return size;
        }

        bool operator==(const TreeMap& other) const {
            if (size != other.size) {
                return false;
            }

            for (auto& val : other) {
                if (this->valueOf(val.first) != val.second) {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(const TreeMap& other) const {
            return !(*this == other);
        }

        iterator begin() {
            return iterator(*this, minElement());
        }

        iterator end() {
            return iterator(*this, nullptr);
        }

        const_iterator cbegin() const {
            return const_iterator(*this, minElement());
        }

        const_iterator cend() const {
            return const_iterator(*this, nullptr);
        }

        const_iterator begin() const {
            return cbegin();
        }

        const_iterator end() const {
            return cend();
        }

    private:
        node_pointer root;
        size_type size;

        node_pointer minElement() const {
            node_pointer element = root;
            while (element != nullptr && element->leftChild != nullptr) {
                element = element->leftChild;
            }
            return element;
        }

        node_pointer maxElement() const {
            node_pointer element = root;
            while (element != nullptr && element->rightChild != nullptr) {
                element = element->rightChild;
            }
            return element;
        }

        node_pointer override(node_pointer a, node_pointer b) {
            auto newNode = new TreeNode(*b);
            if (a->parent != nullptr) {
                if (a->parent->leftChild == a) {
                    a->parent->leftChild = newNode;
                }
                else {
                    a->parent->rightChild = newNode;
                }
            }
            newNode->parent = a->parent;
            if (a->leftChild != nullptr) {
                a->leftChild->parent = newNode;
            }
            newNode->leftChild = a->leftChild;

            if (a->rightChild != nullptr) {
                a->rightChild->parent = newNode;
            }
            newNode->rightChild = a->rightChild;
            delete(a);
            return newNode;
        }

        void rebalance(node_pointer balanceRoot) {
            if (balanceRoot == nullptr) {
                return;
            }

            balanceRoot->height = 1 + std::max(getHeight(balanceRoot->leftChild), getHeight(balanceRoot->rightChild));
            auto balance = getBalance(balanceRoot);

            if (balance == -2) {
                if (getBalance(balanceRoot->leftChild) > 0) {
                    balanceRoot->leftChild = rotateLeft(balanceRoot->leftChild);
                }
                rotateRight(balanceRoot);
            }
            else if (balance == 2) {
                if (getBalance(balanceRoot->rightChild) < 0) {
                    balanceRoot->rightChild = rotateRight(balanceRoot->rightChild);
                }
                rotateLeft(balanceRoot);
            }

            if (balanceRoot->parent != nullptr) {
                rebalance(balanceRoot->parent);
            }
            else {
                root = balanceRoot;
            }
        }

        node_pointer rotateLeft(node_pointer rotationRoot) {
            node_pointer newRoot = rotationRoot->rightChild;
            newRoot->parent = rotationRoot->parent;
            rotationRoot->rightChild = newRoot->leftChild;

            if (rotationRoot->rightChild != nullptr) {
                rotationRoot->rightChild->parent = rotationRoot;
            }

            newRoot->leftChild = rotationRoot;
            rotationRoot->parent = newRoot;

            if (newRoot->parent != nullptr) {
                if (newRoot->parent->leftChild == rotationRoot) {
                    newRoot->parent->leftChild = newRoot;
                }
                else {
                    newRoot->parent->rightChild = newRoot;
                }
            }

            rotationRoot->height = std::max(getHeight(rotationRoot->leftChild), getHeight(rotationRoot->rightChild)) + 1;
            newRoot->height = std::max(getHeight(newRoot->leftChild), getHeight(newRoot->rightChild)) + 1;

            return newRoot;
        }

        node_pointer rotateRight(node_pointer rotationRoot) {
            node_pointer newRoot = rotationRoot->leftChild;
            newRoot->parent = rotationRoot->parent;
            rotationRoot->leftChild = newRoot->rightChild;

            if (rotationRoot->leftChild != nullptr) {
                rotationRoot->leftChild->parent = rotationRoot;
            }

            newRoot->rightChild = rotationRoot;
            rotationRoot->parent = newRoot;

            if (newRoot->parent != nullptr) {
                if (newRoot->parent->rightChild == rotationRoot) {
                    newRoot->parent->rightChild = newRoot;
                }
                else {
                    newRoot->parent->leftChild = newRoot;
                }
            }

            rotationRoot->height = std::max(getHeight(rotationRoot->leftChild), getHeight(rotationRoot->rightChild)) + 1;
            newRoot->height = std::max(getHeight(newRoot->leftChild), getHeight(newRoot->rightChild)) + 1;

            return newRoot;
        }

        void clearTree() {
            auto it = begin();
            while (it != end()) {
                auto node = it.currentNode;
                ++it;
                delete(node);
            }
            root = nullptr;
            size = 0;
        }

        inline int getHeight(node_pointer n) const {
            return n == nullptr ? -1 : n->height;
        }

        inline int getBalance(node_pointer n) const {
            return getHeight(n->rightChild) - getHeight(n->leftChild);
        }
    };

    template <typename KeyType, typename ValueType>
    class TreeMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename TreeMap::const_reference;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename TreeMap::value_type;
        using pointer = const typename TreeMap::value_type*;

        friend class TreeMap;

        explicit ConstIterator(const TreeMap& parent, node_pointer currentNode) : parent(parent),
                                                                                  currentNode(currentNode) {}

        ConstIterator(const ConstIterator& other) : parent(other.parent), currentNode(other.currentNode) {}

        ConstIterator& operator++() {
            if (currentNode == nullptr) {
                throw std::out_of_range("Incrementing end iterator");
            }

            if (currentNode->rightChild != nullptr) {
                // One child right and then left till the end
                currentNode = currentNode->rightChild;
                while (currentNode->leftChild != nullptr) {
                    currentNode = currentNode->leftChild;
                }
            }
            else {
                while (currentNode->parent != nullptr && currentNode->parent->rightChild == currentNode) {
                    currentNode = currentNode->parent;
                }
                currentNode = currentNode->parent;
            }
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator ret = *this;
            ++*this;
            return ret;
        }

        ConstIterator& operator--() {
            // Empty map -> begin == end -> no decrementing allowed
            if (parent.isEmpty()) {
                throw std::out_of_range("Decrementing begin iterator");
            }

            // Decrementing end iterator
            if (currentNode == nullptr) {
                currentNode = parent.maxElement();
                return *this;
            }

            node_pointer initialValue = currentNode;
            if (currentNode->leftChild != nullptr) {
                currentNode = currentNode->leftChild;
                while (currentNode->rightChild != nullptr) {
                    currentNode = currentNode->rightChild;
                }
            }
            else {
                while (currentNode->parent != nullptr && currentNode->parent->leftChild == currentNode) {
                    currentNode = currentNode->parent;
                }
                if (currentNode->parent == nullptr) {
                    // Decrementing begin operator
                    currentNode = initialValue;
                    throw std::out_of_range("Decrementing begin iterator");
                }
                currentNode = currentNode->parent;
            }
            return *this;
        }

        ConstIterator operator--(int) {
            ConstIterator ret = *this;
            --*this;
            return ret;
        }

        reference operator*() const {
            if (currentNode == nullptr) {
                throw std::out_of_range("Dereferencing end iterator");
            }
            return currentNode->val;
        }

        pointer operator->() const {
            return &this->operator*();
        }

        bool operator==(const ConstIterator& other) const {
            return this->currentNode == other.currentNode;
        }

        bool operator!=(const ConstIterator& other) const {
            return !(*this == other);
        }

    private:
        const TreeMap& parent;
        node_pointer currentNode;
    };

    template <typename KeyType, typename ValueType>
    class TreeMap<KeyType, ValueType>::Iterator : public TreeMap<KeyType, ValueType>::ConstIterator {
    public:
        using reference = typename TreeMap::reference;
        using pointer = typename TreeMap::value_type*;

        explicit Iterator(const TreeMap& parent, node_pointer currentNode) : ConstIterator(parent, currentNode) {}

        Iterator(const ConstIterator& other)
                : ConstIterator(other) {}

        Iterator& operator++() {
            ConstIterator::operator++();
            return *this;
        }

        Iterator operator++(int) {
            auto result = *this;
            ConstIterator::operator++();
            return result;
        }

        Iterator& operator--() {
            ConstIterator::operator--();
            return *this;
        }

        Iterator operator--(int) {
            auto result = *this;
            ConstIterator::operator--();
            return result;
        }

        pointer operator->() const {
            return &this->operator*();
        }

        reference operator*() const {
            // ugly cast, yet reduces code duplication.
            return const_cast<reference>(ConstIterator::operator*());
        }
    };

}

#endif /* AISDI_MAPS_MAP_H */
