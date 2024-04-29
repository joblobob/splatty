/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;


#include <coroutine>
#include <string>

export module splat.coroutine;


//coroutine!
export struct CountLogger {
	struct promise_type {
		std::string logMessage {};

		CountLogger get_return_object() noexcept { return CountLogger { this }; } // #C Coroutine creation
		std::suspend_never initial_suspend() noexcept { return {}; }              // #D Startup
		std::suspend_always final_suspend() noexcept { return {}; }               // #E Ending
		std::suspend_always yield_value(std::string msg) noexcept                 // #F Value from co_yield
		{
			logMessage = std::move(msg);
			return {};
		}
		void return_value(std::string msg) noexcept { logMessage = std::move(msg); } // #I Value from co_return
		void unhandled_exception() noexcept {}
	};

	std::coroutine_handle<promise_type> co_handle {};

	explicit CountLogger(promise_type* p) : co_handle { std::coroutine_handle<promise_type>::from_promise(*p) } {} // #C Get the handle form the promise
	CountLogger(CountLogger&& rhs) noexcept : co_handle { std::exchange(rhs.co_handle, nullptr) } {}               // #D Move only!

	~CountLogger() noexcept // #E Care taking, destroying the handle if needed
	{
		if (co_handle) {
			co_handle.destroy();
		}
	}

	void count() // activate it.
	{
		if (not co_handle.done()) {
			co_handle.resume();
		}
	}

	std::string message() // #F Activate the coroutine and return the data
	{
		if (not co_handle.done()) {
			co_handle.resume();
		}
		return std::move(co_handle.promise().logMessage);
	}
};
