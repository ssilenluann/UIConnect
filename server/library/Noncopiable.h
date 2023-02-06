#ifndef NONCOPIABLE_H
#define NONCOPIABLE_H

class Noncopyable {
public:
    Noncopyable() = default;

    ~Noncopyable() = default;

    Noncopyable(const Noncopyable&) = delete;

    Noncopyable& operator=(const Noncopyable&) = delete;
};

#endif