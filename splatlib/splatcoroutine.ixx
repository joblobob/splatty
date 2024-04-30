/*
* Yes this is a C++ module! I love it!
* 
* It's intended to be a single file for the whole implementation for the moment, step by step!
*/

module;


#include <coroutine>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <vector>

export module splat.coroutine;

import splat.data;
import splat.opengl;


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

//coroutine 2!
struct SplatData;
export struct TextureGenerator {
	glsplat* m_gl; //ptr to connect to setTextureData

	std::thread myThread;

	struct promise_type {
		std::vector<unsigned int> textureData {}; //what the coroutine produces
		std::unique_ptr<SplatData> m_splatData;   //what it needs to produce


		TextureGenerator get_return_object() noexcept { return TextureGenerator { this }; } // #C Coroutine creation
		std::suspend_always initial_suspend() noexcept { return {}; }                       // #D Startup
		std::suspend_always final_suspend() noexcept { return {}; }                         // #E Ending
		std::suspend_always yield_value(std::vector<unsigned int> data) noexcept            // #F Value from co_yield
		{
			textureData = std::move(data);
			return {};
		}
		void unhandled_exception() noexcept {}
		void return_void() noexcept {}

		auto await_transform(std::unique_ptr<SplatData>) noexcept //G Value from co_await 14
		{
			struct awaiter {
				// Customized version instead of using suspend_always or suspend_never
				promise_type& pt;
				constexpr bool await_ready() const noexcept { return true; }
				std::unique_ptr<SplatData> await_resume() const noexcept { return std::move(pt.m_splatData); }
				void await_suspend(std::coroutine_handle<>) const noexcept {}
			};
			return awaiter { *this };
		}
	};

	std::coroutine_handle<promise_type> co_handle {};

	explicit TextureGenerator(promise_type* p) :
	    myThread(), co_handle { std::coroutine_handle<promise_type>::from_promise(*p) } // #C Get the handle form the promise
	{}
	TextureGenerator(TextureGenerator&& rhs) noexcept : co_handle { std::exchange(rhs.co_handle, nullptr) } {} // #D Move only!

	~TextureGenerator() noexcept // #E Care taking, destroying the handle if needed
	{
		myThread.join();
		if (co_handle) {
			co_handle.destroy();
		}
	}

	void setData(std::unique_ptr<SplatData> data) { co_handle.promise().m_splatData = std::move(data); }
	void setOpenGL(glsplat* gl) { m_gl = gl; }

	void texture(int texwidth, int texheight) // #F Activate the coroutine and return the data
	{
		if (!myThread.joinable()) {
			myThread = std::move(std::thread(([this] {
				if (not co_handle.done()) {
					co_handle.resume();
				}
			})));
		}

		m_gl->setTextureData(co_handle.promise().textureData, texwidth, texheight);
	}
};
