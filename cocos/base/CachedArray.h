#ifndef CC_CORE_KERNEL_CACHED_ARRAY_H_
#define CC_CORE_KERNEL_CACHED_ARRAY_H_

namespace cc {

template <typename T>
class CachedArray : public Object {
public:
    CachedArray(uint size = 1u) {
        _size = 0;
        _capacity = std::max(size, 1u);
        _array = CC_NEW_ARRAY(T, _capacity);
    }

    // The rule of five applies here
    ~CachedArray() {
        CC_SAFE_DELETE_ARRAY(_array);
    }

    CachedArray(const CachedArray &other)
    : _size(other._size), _capacity(other._capacity), _array(CC_NEW_ARRAY(T, other._capacity)) {
        memcpy(_array, other._array, _size * sizeof(T));
    }

    CachedArray &operator=(const CachedArray &other) noexcept {
        if (this != &other) {
            CC_DELETE_ARRAY(_array);
            _size = other._size;
            _capacity = other._capacity;
            _array = CC_NEW_ARRAY(T, _capacity);
            memcpy(_array, other._array, _size * sizeof(T));
        }
        return *this;
    }

    CachedArray(CachedArray &&other)
    : _size(other._size), _capacity(other._capacity), _array(other._array) {
        other._size = 0;
        other._capacity = 0;
        other._array = nullptr;
    }

    CachedArray &operator=(CachedArray &&other) noexcept {
        if (this != &other) {
            CC_DELETE_ARRAY(_array);
            _size = other._size;
            _capacity = other._capacity;
            _array = other._array;
            other._size = 0;
            other._capacity = 0;
            other._array = nullptr;
        }
        return *this;
    }

    // Subscription operators
    T &operator[](int index) {
        return _array[index];
    }

    const T &operator[](int index) const {
        return _array[index];
    }

    CC_INLINE void clear() { _size = 0; }
    CC_INLINE uint size() const { return _size; }
    CC_INLINE T pop() { return _array[--_size]; }

    void reserve(uint size) {
        if (size > _capacity) {
            T *temp = _array;
            _array = CC_NEW_ARRAY(T, size);
            memcpy(_array, temp, _capacity * sizeof(T));
            _capacity = size;
            CC_DELETE_ARRAY(temp);
        }
    }

    void push(T item) {
        if (_size >= _capacity) {
            T *temp = _array;
            _array = CC_NEW_ARRAY(T, _capacity * 2);
            memcpy(_array, temp, _capacity * sizeof(T));
            _capacity *= 2;
            CC_DELETE_ARRAY(temp);
        }
        _array[_size++] = item;
    }

    void concat(const CachedArray<T> &array) {
        if (_size + array._size > _capacity) {
            T *temp = _array;
            uint size = std::max(_capacity * 2, _size + array._size);
            _array = CC_NEW_ARRAY(T, size);
            memcpy(_array, temp, _size * sizeof(T));
            _capacity = size;
            CC_DELETE_ARRAY(temp);
        }
        memcpy(_array + _size, array._array, array._size * sizeof(T));
        _size += array._size;
    }

    void concat(T *array, uint count) {
        if (_size + count > _capacity) {
            T *temp = _array;
            uint size = std::max(_capacity * 2, _size + count);
            _array = CC_NEW_ARRAY(T, size);
            memcpy(_array, temp, _size * sizeof(T));
            _capacity = size;
            CC_DELETE_ARRAY(temp);
        }
        memcpy(_array + _size, array, count * sizeof(T));
        _size += count;
    }

    void fastRemove(uint idx) {
        if (idx >= _size) {
            return;
        }
        _array[idx] = _array[--_size];
    }

    uint indexOf(T item) {
        for (uint i = 0; i < _size; ++i) {
            if (_array[i] == item) {
                return i;
            }
        }
        return UINT_MAX;
    }

private:
    uint _size = 0;
    uint _capacity = 0;
    T *_array = nullptr;
};

} // namespace cc

#endif // CC_CORE_KERNEL_CACHED_ARRAY_H_
