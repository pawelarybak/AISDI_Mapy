#ifndef AISDI_MAPS_HASHMAP_H
#define AISDI_MAPS_HASHMAP_H

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <list>
#include <array>
#include <algorithm>
#include <stdexcept>

namespace aisdi
{

    template <typename KeyType, typename ValueType>
    class HashMap
    {
        static const int MAP_SIZE = 11;

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



        HashMap() : size(0) {}

        HashMap(std::initializer_list<value_type> list) : HashMap() {
            for (const auto& val : list) {
                this->operator[](val.first) = val.second;
            }
        }

        HashMap(const HashMap& other) : HashMap() {
            for (auto& element : other) {
                this->operator[](element.first) = element.second;
            }
        }

        HashMap(HashMap&& other) {
            this->buckets = std::move(other.buckets);
            this->size = other.size;
        }

        HashMap& operator=(const HashMap& other) {
            if (this == &other) {
                return *this;
            }
            this->size = 0;
            for (auto& element : other) {
                this->operator[](element.first) = element.second;
            }
            return *this;
        }

        HashMap& operator=(HashMap&& other) {
            if (this == &other) {
                return *this;
            }
            this->buckets = std::move(other.buckets);
            this->size = other.size;
            return *this;
        }

        bool isEmpty() const {
            return this->size == 0;
        }

        mapped_type& operator[](const key_type& key) {
            // Find appropriate bucket
            auto& bucket = buckets[std::hash<key_type>{}(key) % MAP_SIZE];
            // Find given key in it
            auto bucket_it = std::find_if(
                    bucket.begin(),
                    bucket.end(),
                    [&key](const value_type& v) { return v.first == key; }
            );
            if (bucket_it == bucket.end()) {
                // Bucket doesn't have such key - create new entry
                bucket.emplace_back(std::make_pair(key, mapped_type{}));
                this->size++;
                return bucket.back().second;
            }
            // Key found - return value
            return (*bucket_it).second;
        }

        const mapped_type& valueOf(const key_type& key) const {
            // Find appropriate bucket
            auto& bucket = buckets[std::hash<key_type>{}(key) % MAP_SIZE];
            // Find given key in it
            auto bucket_it = std::find_if(
                    bucket.begin(),
                    bucket.end(),
                    [&key](const value_type& v) { return v.first == key; }
            );
            if (bucket_it == bucket.end()) {
                throw std::out_of_range("Key not in map");
            }
            return (*bucket_it).second;
        }

        mapped_type& valueOf(const key_type& key) {
            // Find appropriate bucket
            auto& bucket = buckets[std::hash<key_type>{}(key) % MAP_SIZE];
            // Find given key in it
            auto bucket_it = std::find_if(
                    bucket.begin(),
                    bucket.end(),
                    [&key](const value_type& v) { return v.first == key; }
            );
            if (bucket_it == bucket.end()) {
                throw std::out_of_range("Key not in map");
            }
            return (*bucket_it).second;
        }

        const_iterator find(const key_type& key) const {
            // Find appropriate bucket
            const auto& bucket = buckets.begin() + (std::hash<key_type>{}(key) % MAP_SIZE);
            // Find given key in it
            auto bucket_it = std::find_if(
                    bucket->begin(),
                    bucket->end(),
                    [&key](const value_type& v) { return v.first == key; }
            );
            if (bucket_it == bucket->end()) {
                return end();
            }
            return const_iterator(buckets, bucket, bucket_it);
        }

        iterator find(const key_type& key) {
            // Find appropriate bucket
            const auto& bucket = buckets.begin() + (std::hash<key_type>{}(key) % MAP_SIZE);
            // Find given key in it
            auto bucket_it = std::find_if(
                    bucket->begin(),
                    bucket->end(),
                    [&key](const value_type& v) { return v.first == key; }
            );
            if (bucket_it == bucket->end()) {
                return end();
            }
            return iterator(buckets, bucket, bucket_it);
        }

        void remove(const key_type& key) {
            // Find appropriate bucket
            const auto& bucket = buckets.begin() + (std::hash<key_type>{}(key) % MAP_SIZE);
            // Find given key in it
            auto bucket_it = std::find_if(
                    bucket->begin(),
                    bucket->end(),
                    [&key](const value_type& v) { return v.first == key; }
            );
            if (bucket_it == bucket->end()) {
                throw std::out_of_range("No such key");
            }
            bucket->erase(bucket_it);
            --size;
        }

        void remove(const const_iterator& it) {
            if (it == end()) {
                throw std::out_of_range("Deleting end iterator");
            }

            it.currentBucket->erase(it.iter);
            --size;
        }

        size_type getSize() const {
            return this->size;
        }

        bool operator==(const HashMap& other) const {
            if (size != other.size) {
                return false;
            }

            for (auto& element : other) {
                if (valueOf(element.first) != element.second) {
                    return false;
                }
            }

            return true;
        }

        bool operator!=(const HashMap& other) const {
            return !(*this == other);
        }

        iterator begin() {
            return iterator(buckets, buckets.begin(), buckets.begin()->begin());
        }

        iterator end() {
            return iterator(buckets, buckets.end() - 1, buckets.rbegin()->end());
        }

        const_iterator cbegin() const {
            return const_iterator(buckets, buckets.begin(), buckets.begin()->begin());
        }

        const_iterator cend() const {
            return const_iterator(buckets, buckets.end() - 1, buckets.rbegin()->end());
        }

        const_iterator begin() const {
            return const_iterator(buckets, buckets.begin(), buckets.begin()->begin());
        }

        const_iterator end() const {
            return const_iterator(buckets, buckets.end() - 1, buckets.rbegin()->end());
        }

    private:
        mutable std::array<std::list<value_type>, MAP_SIZE> buckets;
        size_type size;
    };

    template <typename KeyType, typename ValueType>
    class HashMap<KeyType, ValueType>::ConstIterator
    {
    public:
        using reference = typename HashMap::const_reference;
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = typename HashMap::value_type;
        using pointer = const typename HashMap::value_type*;

        friend class HashMap;

        explicit ConstIterator(
                std::array<std::list<value_type>, MAP_SIZE>& buckets,
                typename std::array<std::list<value_type>, MAP_SIZE>::iterator currentBucket,
                typename std::list<value_type>::iterator iter
        ) : buckets(buckets), currentBucket(currentBucket), iter(iter) {
            // If given bucket is empty or iter is end of list, we need to find next non-empty bucket
            nextNonEmpty();
        }

        ConstIterator(const ConstIterator& other) : buckets(other.buckets), currentBucket(other.currentBucket)
                , iter(other.iter) {}

        ConstIterator& operator++() {
            if (isEnd()) {
                throw std::out_of_range("Index out of range");
            }
            ++iter;
            nextNonEmpty();
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator ret = *this;
            ++*this;
            return ret;
        }

        ConstIterator& operator--() {
            if (iter == currentBucket->begin()) {
                --currentBucket;
                iter = currentBucket->end();
                prevNonEmpty();
            }
            else {
                --iter;
            }
            return *this;
        }

        ConstIterator operator--(int) {
            ConstIterator ret = *this;
            --*this;
            return ret;
        }

        reference operator*() const {
            if (isEnd()) {
                throw std::out_of_range("Index out of range");
            }
            return *iter;
        }

        pointer operator->() const {
            if (isEnd()) {
                throw std::out_of_range("Index out of range");
            }
            return &this->operator*();
        }

        bool operator==(const ConstIterator& other) const {
            return currentBucket == other.currentBucket && iter == other.iter;
        }

        bool operator!=(const ConstIterator& other) const {
            return !(*this == other);
        }

    private:
        void nextNonEmpty() {
            while (iter == currentBucket->end() && currentBucket != buckets.end() - 1) {
                ++currentBucket;
                iter = currentBucket->begin();
            }
        }

        void prevNonEmpty() {
            while (currentBucket->empty() && currentBucket != buckets.begin()) {
                --currentBucket;
            }
            if (currentBucket->empty()) {
                throw std::out_of_range("Index out of range");
            }
            iter = --(currentBucket->end());
        }

        inline bool isEnd() const {
            return iter == buckets.rbegin()->end();
        }

        std::array<std::list<value_type>, MAP_SIZE>& buckets;
        typename std::array<std::list<value_type>, MAP_SIZE>::iterator currentBucket;
        typename std::list<value_type>::iterator iter;
    };

    template <typename KeyType, typename ValueType>
    class HashMap<KeyType, ValueType>::Iterator : public HashMap<KeyType, ValueType>::ConstIterator
    {
    public:
        using reference = typename HashMap::reference;
        using pointer = typename HashMap::value_type*;

        explicit Iterator(
                std::array<std::list<value_type>, MAP_SIZE>& buckets,
                typename std::array<std::list<value_type>, MAP_SIZE>::iterator currentBucket,
                typename std::list<value_type>::iterator iter
        ) : ConstIterator(buckets, currentBucket, iter) {}

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

#endif /* AISDI_MAPS_HASHMAP_H */
