/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;


#include <coroutine>
#include <cstdio>
#include <exception> // std::terminate
#include <iostream>
#include <list>
#include <new>
#include <string>
#include <utility>
export module splat.coroutine;


//coroutine!
export struct Chat {
	struct promise_type {
		std::string _msgOut {}, _msgIn {}; // #A Storing a value from or for the coroutine

		void unhandled_exception() noexcept {}                        // #B What to do in case of an exception
		Chat get_return_object() { return Chat { this }; }            // #C Coroutine creation
		std::suspend_always initial_suspend() noexcept { return {}; } // #D Startup
		std::suspend_always yield_value(std::string msg) noexcept     // #F Value from co_yield
		{
			_msgOut = std::move(msg);
			return {};
		}

		auto await_transform(std::string) noexcept // #G Value from co_await
		{
			struct awaiter { // #H Customized version instead of using suspend_always or suspend_never
				promise_type& pt;
				constexpr bool await_ready() const noexcept { return true; }
				std::string await_resume() const noexcept { return std::move(pt._msgIn); }
				void await_suspend(std::coroutine_handle<>) const noexcept {}
			};

			return awaiter { *this };
		}

		void return_value(std::string msg) noexcept { _msgOut = std::move(msg); } // #I Value from co_return
		std::suspend_always final_suspend() noexcept { return {}; }               // #E Ending
	};

	using Handle = std::coroutine_handle<promise_type>; // #A Shortcut for the handle type
	Handle mCoroHdl {};                                 // #B

	explicit Chat(promise_type* p) : mCoroHdl { Handle::from_promise(*p) } {}        // #C Get the handle form the promise
	Chat(Chat&& rhs) noexcept : mCoroHdl { std::exchange(rhs.mCoroHdl, nullptr) } {} // #D Move only!

	~Chat() noexcept // #E Care taking, destroying the handle if needed
	{
		if (mCoroHdl) {
			mCoroHdl.destroy();
		}
	}

	std::string listen() // #F Activate the coroutine and wait for data.
	{
		if (not mCoroHdl.done()) {
			mCoroHdl.resume();
		}
		return std::move(mCoroHdl.promise()._msgOut);
	}

	void answer(std::string msg) // #G Send data to the coroutine and activate it.
	{
		mCoroHdl.promise()._msgIn = std::move(msg);
		if (not mCoroHdl.done()) {
			mCoroHdl.resume();
		}
	}
};

export Chat Fun() // #A Wrapper type Chat containing the promise type
{
	co_yield "Hello!\n"; // #B Calls promise_type.yield_value

	int i = 0;

	while (i < 50) {
		std::cout << i++ << "\n" << co_await std::string {}; // #C Calls promise_type.await_transform

		co_yield "Here! %1\n " + std::to_string(i); // #D Calls promise_type.return_value
	}
}
