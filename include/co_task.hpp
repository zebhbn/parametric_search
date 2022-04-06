#include <coroutine>
#include <iostream>

#ifndef CO_TASKS_HPP
#define CO_TASKS_HPP


namespace ps_framework {
// TODO: implement done method
    template<typename T>
    struct co_task {
        struct promise_type {
            promise_type() {}

            co_task<T> get_return_object() {
                return std::coroutine_handle<promise_type>::from_promise(*this);
            }

            std::suspend_always initial_suspend() {
                return {};
            }

            std::suspend_always final_suspend() noexcept {
                return {};
            }

            void unhandled_exception() noexcept {};

            void return_value(T value) {
                value_ = value;
            }

            T value_ = NULL;
        };

        co_task(std::coroutine_handle<promise_type> handle)
                : handle_(std::move(handle)) {}

        ~co_task() {
            if (handle_)
                handle_.destroy();
        }

        void resume() {
            handle_.resume();
        }

        int result() const {
            return handle_.promise().value_;
        }

        std::coroutine_handle<promise_type> handle_{nullptr};
    };

}


#endif