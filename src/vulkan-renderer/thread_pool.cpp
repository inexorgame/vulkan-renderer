#include "inexor/vulkan-renderer/thread_pool.hpp"

namespace inexor {

ThreadPool::ThreadPool(std::size_t thread_count) {
    // Try to estimate the number of CPU cores available on the system.
    std::size_t number_of_cpu_cores = std::thread::hardware_concurrency();

    // Yes, this might be the case because std::thread::hardware_concurrency() is only a hint!
    if (0 == number_of_cpu_cores) {
        spdlog::warn("Number of CPU cores could not be determined!");

        // Let's just use the standard number of threads then.
        number_of_cpu_cores = INEXOR_THREADPOOL_BACKUP_CPU_CORE_COUNT;

        spdlog::warn("Using {} threads!", number_of_cpu_cores);
    } else {
        spdlog::debug("Number of CPU cores: {}", number_of_cpu_cores);
    }

    spdlog::debug("Constructing threads.");

    if (thread_count < INEXOR_THREADPOOL_MIN_THREAD_COUNT) {
        spdlog::warn("The desired number of threads to create is too small for the engine to run!");
        spdlog::warn("Creating {} threads ", INEXOR_THREADPOOL_MIN_THREAD_COUNT);

        // Let's just use the minimum number of threads then.
        thread_count = INEXOR_THREADPOOL_MIN_THREAD_COUNT;
    }

    // If the number of threads exceedes the number of cpu cores,
    // the rise of thread management overhead decreases performance!
    if (thread_count > number_of_cpu_cores) {
        spdlog::warn("Creating more threads than CPU cores are available!");
        spdlog::warn("This might decrease performance as thread management overhead increases!");
    }

    for (std::size_t i = 0; i < thread_count; ++i) {
        start_thread();
    }
}

ThreadPool::~ThreadPool() {
    // spdlog::debug("Shutting down worker threads.");

    stop_threads = true;

    // Notify all worker threads about program stop.
    tasklist_cv.notify_all();

    for (std::thread &thread : threads) {
        thread.join();
    }

    // spdlog::debug("All worker threads closed successfully.");
}

void ThreadPool::start_thread() {
    // TODO: Do we need additional locks here?
    // spdlog::debug("Starting new worker thread.");

    // Start waiting for threads.
    // Working threads listen for new tasks through ThreadPool's condition_variable.
    threads.emplace_back(

        std::thread([&]() {
            // Lock the queue so we can see which tasks are to ne done.
            std::unique_lock<std::mutex> queue_lock(tasklist_mutex, std::defer_lock);

            while (true) {
                // Lock the queue
                queue_lock.lock();

                // spdlog::debug("Waiting for work!.");

                // Use the conditional variable to wait for new tasks.
                tasklist_cv.wait(queue_lock, [&]() -> bool { return !tasklist.empty() || stop_threads; });

                // spdlog::debug("Starting a new task!.");

                // Check if we should finish the task.
                if (stop_threads && tasklist.empty())
                    return;

                // To initialise the task, we must move the unique pointer
                // from the queue to the loal stakc. Since a unique pointer
                // cannot be copie, it must be explicitly moved. This transfers
                // ownershp of the pointed-to object to *this.
                auto temp_task = std::move(tasklist.front());

                // Remove the task from the task list.
                tasklist.pop();

                queue_lock.unlock();

                // Run the task!
                (*temp_task)();

                // spdlog::debug("Task is done!");
            }
        }));
}

} // namespace inexor
