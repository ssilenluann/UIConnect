#include <boost/coroutine/all.hpp>
#include <iostream>

void fibonacci(boost::coroutines::coroutine<int>::push_type& coro, int n) {
    int a = 0, b = 1;
    for (int i = 0; i < n; ++i) {
        coro(a);
        int tmp = b;
        b += a;
        a = tmp;
    }
}

int main() {
    boost::coroutines::coroutine<int>::push_type coro{fibonacci};
    while (coro) {
        int value = coro.get<int>();
        std::cout << value << ' ';
        coro(value);
    }
    std::cout << std::endl;
    return 0;
}