#include <coroutine>
#include <iostream>

#ifndef CO_TASKS_HPP
#define CO_TASKS_HPP


namespace ps_framework {
// DONE: implement done method
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

            T value_;
        };

        co_task(std::coroutine_handle<promise_type> handle)
                : handle_(std::move(handle)) {}

        ~co_task() {}

        void destroyMe() {
            if (handle_)
                handle_.destroy();
        }

        void resume() {
            handle_.resume();
        }

        int result() const {
            return handle_.promise().value_;
        }

        bool done() const noexcept {
            return handle_.done();
        }

        std::coroutine_handle<promise_type> handle_{nullptr};
    };


    struct co_task_void {
        struct promise_type {
            promise_type() {}

            co_task_void get_return_object() {
                return std::coroutine_handle<promise_type>::from_promise(*this);
            }

            std::suspend_always initial_suspend() {
                return {};
            }

            std::suspend_always final_suspend() noexcept {
                return {};
            }

            void unhandled_exception() noexcept {
                std::cout<<"Something went wrong!!!"<<std::endl;
            };

            void return_void() {}
        };

        co_task_void (std::coroutine_handle<promise_type> handle)
                : handle_(std::move(handle)) {}

        ~co_task_void() {}

        void destroyMe() {
//            std::cout<<"Coroutine destroyed"<<std::endl;
            if (handle_){
                handle_.destroy();
            }
        }

        void resume() {
            handle_.resume();
        }


        bool done() const noexcept {
            return handle_.done();
        }

        std::coroutine_handle<promise_type> handle_{nullptr};
    };

}


#endif