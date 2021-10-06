#include <deque>
#include <optional>
#include <mutex>
#include <condition_variable>

template<class T>
class BlockingQueue {
public:
    explicit BlockingQueue(size_t max_size) : max_size_(max_size) {}

    BlockingQueue(const BlockingQueue&) = delete;

    BlockingQueue(BlockingQueue&&) = delete;

    BlockingQueue& operator=(const BlockingQueue&) = delete;

    BlockingQueue& operator=(BlockingQueue&&) = delete;

    bool put(T element) {
        // your code here
    }

    [[nodiscard]] bool offer(T element) {
        // your code here
    }

    std::optional<T> take() {
        // your code here
    }

    void close() {
        // your code here
    }

    [[nodiscard]] bool is_closed() const { // const may be removed if hard to support
        // your code here
    }

private:
    std::deque<T> container_{};
    size_t max_size_;
    // your code here
};
