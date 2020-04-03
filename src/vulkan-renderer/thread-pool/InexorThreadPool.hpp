#pragma once

#include <vector>
#include <thread>
#include <future>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <type_traits>

#include <spdlog/spdlog.h>

// We should at least create 6 worker threads.
// Most systems nowadays have at least 8 cores.
constexpr unsigned int INEXOR_THREADPOOL_MIN_THREAD_COUNT = 6;

// The maximum number of threads is not limitex here,
// but should not exceede std::thread::hardware_concurrency();

// std::thread::hardware_concurrency() might return 0 in some cases.
// The function should be interpreted as a hint only! In that case,
// let's use just 8 threads. In the worst case we generate more
// threads than cpu cores are available, generating overhead.
constexpr unsigned int INEXOR_THREADPOOL_BACKUP_CPU_CORE_COUNT = 8;


namespace inexor {

    
    // TODO: Minimum number of threads.
    // TODO: Maximum number of threads.
    // TODO: Method for changing the number of threads at runtime.


    /// @class InexorThreadPool
    /// @brief A C++17 threadpool implementation.
    class InexorThreadPool
    {
        public:

            /// @brief Standard constructor.
            /// @param thread_count [in] The number of threads to create for the threadpool.
            /// It is advisable to create as many threads as there are processor cores available,
            /// hence we are using std::thread::hardware_concurrency() as standard argument value.
            /// @warning You should not create too many threads because this increases overhead!
            InexorThreadPool(std::size_t thread_count = std::thread::hardware_concurrency());


            // @brief The default destructor destroys all threads.
            ~InexorThreadPool();

            
            /// @brief Because std::thread is not copiable, we need to delete the copy constructor!
            InexorThreadPool(const InexorThreadPool &) = delete;


            /// @brief Because std::thread is not copiable, we need to delete the assign operator as well!
            InexorThreadPool &operator=(const InexorThreadPool &) = delete;
            

            /// @brief Spawns a new worker thread.
            void start_thread();


            /// @brief Executes a task from the tasklist.
            /// @note We only accept invokable arguments in the template.
            template <typename F, typename... Args, std::enable_if_t<std::is_invocable_v<F&&, Args&&...>, int> = 0>
            auto execute(F, Args&&...);


        private:

            //_task_container_base and _task_container exist simply as a wrapper around a 
            //  MoveConstructible - but not CopyConstructible - Callable object. Since an
            //  std::function requires a given Callable to be CopyConstructible, we cannot
            //  construct one from a lambda function that captures a non-CopyConstructible
            //  object (such as the packaged_task declared in execute) - because a lambda
            //  capturing a non-CopyConstructible object is not CopyConstructible.

            /// @class InexorTaskContainerBase
            /// 
            class InexorTaskContainerBase
            {
                public:

                    virtual ~InexorTaskContainerBase()
                    {
                    };


                    virtual void operator()() = 0;

            };

            //_task_container takes a typename F, which must be Callable and MoveConstructible.
            //  Furthermore, F must be callable with no arguments; it can, for example, be a
            //  bind object with no placeholders.
            //  F may or may not be CopyConstructible.

            /// 
            /// 
            /// 
            template <typename F>
            class InexorTaskContainer : public InexorTaskContainerBase
            {
                public:

                    //here, std::forward is needed because we need the construction of _f *not* to
                    //  bind an lvalue reference - it is not a guarantee that an object of type F is
                    //  CopyConstructible, only that it is MoveConstructible.
                    
                    /// 
                    /// 
                    /// 
                    InexorTaskContainer(F &&func) : _f(std::forward<F>(func))
                    {
                    }


                    /// 
                    /// 
                    /// 
                    void operator()() override
                    {
                        _f();
                    }


                private:

                    F _f;

             };


            /// @brief Returns a unique pointer to a InexorWork container that wraps around a given function 
            template <typename InexorWork>
            static std::unique_ptr<InexorTaskContainerBase> allocate_task_container(InexorWork &&f)
            {
                spdlog::debug("Allocating task container.");

                //in the construction of the _task_container, f must be std::forward'ed because
                //  it may not be CopyConstructible - the only requirement for an instantiation
                //  of a _task_container is that the parameter is of a MoveConstructible type.
                return std::make_unique<InexorTaskContainer<InexorWork>>(std::forward<InexorWork>(f));
            }

            // The threads.
            std::vector<std::thread> threads;
            
            /// The tasklist contains the list of work that should be done.
            std::queue<std::unique_ptr<InexorTaskContainerBase>> tasklist;
            
            /// This mutex locks tasklist access.
            std::mutex tasklist_mutex;
            
            std::condition_variable tasklist_cv;

            bool stop_threads = false;


    };
    

    template <typename F, typename... Args, std::enable_if_t<std::is_invocable_v<F&&, Args&&...>, int>>
    auto InexorThreadPool::execute(F function, Args &&...args)
    {
        spdlog::warn("Executing task from task list.");
        
        // Lock the task list so we can add the new task.
        std::unique_lock<std::mutex> queue_lock(tasklist_mutex, std::defer_lock);
        
        // Bind the function pointer and the parameters to the task package.
        std::packaged_task<std::invoke_result_t<F, Args...>()> task_package(std::bind(function, args...));
    
        // 
        std::future<std::invoke_result_t<F, Args...>> future = task_package.get_future();

        spdlog::warn("Locking queue_lock.");

        // 
        queue_lock.lock();

        // This lambda move-captures the packaged_task declared above.
        // Since the packaged_task type is not CopyConstructible, the
        // function is not CopyConstructible either, hence the need
        // for a InexorTaskContainer to wrap around it.
        tasklist.emplace(allocate_task_container(std::move(task_package)));

        // 
        queue_lock.unlock();

        spdlog::warn("Unlocking queue_lock.");

        // 
        tasklist_cv.notify_one();

        // 
        return std::move(future);
    }


};
