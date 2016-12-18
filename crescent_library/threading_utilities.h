#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#include <mutex>
#include <condition_variable>

namespace crsc {
    /**
     * \class semaphore
     *
     * \brief A data type used for controlling access to a common resource in a concurrent system.
     */
    class semaphore {
    public:
        /**
         * \brief Constructs the semaphore with `_count` units of resource.
         *
         * \param _count Units of resource (default set to `0`).
         */
        explicit semaphore(std::size_t _count = 0)
            : count(_count) {}
        /**
         * \brief Increments the value of the semaphore count by 1 unit and transfers
         *        blocked process from semaphore's waiting queue to the ready queue.
         */
        void notify() {
            std::unique_lock<std::mutex> lock(mut);
            ++count;
            cv.notify_one();
        }
        /**
         * \brief Process executing wait is blocked until semaphore count value is 
         *        greater than 0 and count is decremented.
         */
        void wait() {
            std::unique_lock<std::mutex> lock(mut);
            cv.wait(lock, [this]() {return count > 0; });
            --count;
        }
    private:
        std::mutex mut;
        std::condition_variable cv;
        std::size_t count;
    };
}

#endif // !SEMAPHORE_H