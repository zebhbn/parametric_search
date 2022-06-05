#include <coroutine>
#include <iostream>

#ifndef CO_TASKS_HPP
#define CO_TASKS_HPP


namespace ps_framework {
    struct promise_type_void;

    struct coroTaskVoid : std::coroutine_handle<promise_type_void> {
        using promise_type = struct promise_type_void;

        coroTaskVoid (std::coroutine_handle<promise_type_void> handle)
                : handle_(std::move(handle)) {}

        ~coroTaskVoid() {
//            if (handle_){
//                handle_.destroy();
//            }
        }

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

        std::coroutine_handle<promise_type_void> handle_{nullptr};
    };

    struct promise_type_void {
        promise_type_void() {}

        coroTaskVoid get_return_object() {
            return {coroTaskVoid::from_promise(*this)};
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

        int id = -1;
        int parentId = -1;
        bool transferred = false;
    };


}


#endif