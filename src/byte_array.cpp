#include "webflex/byte_array.hpp"
#include <QByteArray>

namespace webflex::impl {
    struct ByteArrayImpl
    {
        QByteArray byte_array_;
    };
}

namespace webflex {
    ByteArray::ByteArray() : impl_(std::make_unique<impl::ByteArrayImpl>()) {}

    ByteArray::ByteArray(const std::string &data) : ByteArray() 
    {
        impl_->byte_array_ = QByteArray::fromStdString(data);
    }

    ByteArray::ByteArray(const std::vector<uint8_t> &data) : ByteArray() 
    {
        impl_->byte_array_ = QByteArray(reinterpret_cast<const char *>(data.data()), static_cast<int>(data.size()));
    }

    ByteArray::~ByteArray() = default;

    ByteArray::ByteArray(const ByteArray &other)
        : impl_(std::make_unique<impl::ByteArrayImpl>(*other.impl_)) {}

    ByteArray::ByteArray(ByteArray &&other) noexcept = default;

    ByteArray &ByteArray::operator=(const ByteArray &other) {
        if (this != &other) {
            impl_ = std::make_unique<impl::ByteArrayImpl>(*other.impl_);
        }
        return *this;
    }

    ByteArray &ByteArray::operator=(ByteArray &&other) noexcept = default;

    std::string ByteArray::toString() const 
    {
        return impl_->byte_array_.toStdString();
    }

    std::vector<uint8_t> ByteArray::data() const {
        const auto *data = reinterpret_cast<const uint8_t*>(impl_->byte_array_.constData());
        return {data, data + size()};
    }

    std::size_t ByteArray::size() const {
        return static_cast<std::size_t>(impl_->byte_array_.size());
    }

    void ByteArray::append(const std::string &data) {
        impl_->byte_array_.append(QByteArray::fromStdString(data));
    }

    void ByteArray::append(uint64_t n, char ch) 
    {
        impl_->byte_array_.append(static_cast<qsizetype>(n), ch);
    }

    void ByteArray::append(const char *s, uint64_t len) 
    {
        impl_->byte_array_.append(s, static_cast<qsizetype>(len));
    }

    void ByteArray::append(const char* s) 
    {
        impl_->byte_array_.append(s);
    }
    
    void ByteArray::append(char ch) 
    {
        impl_->byte_array_.append(ch);
    }

    void ByteArray::clear() 
    {
        impl_->byte_array_.clear();
    }   

    char& ByteArray::back() const 
    {
        return impl_->byte_array_.back();
    }

    char& ByteArray::front() const 
    {
        return impl_->byte_array_.front();
    }
}