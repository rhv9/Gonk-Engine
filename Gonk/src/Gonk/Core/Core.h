#pragma once

#include <memory>

// Platform detection using predefined macros
#ifdef _WIN32
	/* Windows x64/x86 */
	#ifdef _WIN64
	/* Windows x64  */
		// This is removed because GK_PLATFORM_WINDOWS should be defined at compiler args level to prevent issues that may arise depending on order and if Core.h is included
		//#define GK_PLATFORM_WINDOWS 
#else
	/* Windows x86 */
	#error "x86 Builds are not supported!"
#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	/* TARGET_OS_MAC exists on all the platforms
	 * so we must check all of them (in this order)
	 * to ensure that we're running on MAC
	 * and not some other Apple platform */
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not supported!"
	#elif TARGET_OS_IPHONE == 1
		#define GK_PLATFORM_IOS
	#error "IOS is not supported!"
		#elif TARGET_OS_MAC == 1
	#define GK_PLATFORM_MACOS
		#error "MacOS is not supported!"
#else
	#error "Unknown Apple platform!"
#endif
 /* We also have to check __ANDROID__ before __linux__
  * since android is based on the linux kernel
  * it has __linux__ defined */
#elif defined(__ANDROID__)
	#define GK_PLATFORM_ANDROID
	#error "Android is not supported!"
#elif defined(__linux__)
	#define GK_PLATFORM_LINUX
	#error "Linux is not supported!"
#else
	/* Unknown compiler/platform */
	#error "Unknown platform!"
#endif // End of platform detection




#ifdef GK_PLATFORM_WINDOWS
	#ifdef GK_DYANMIC_LINK
		#ifdef GK_BUILD_DLL
			#define GONK_API __declspec(dllexport)
		#else
			#define GONK_API __declspec(dllimport)
		#endif
	#else
		#define GONK_API
	#endif
#else
	#error Gonk only supports Windows!
#endif

#ifdef GK_DEBUG
	#define GK_ENABLE_ASSERTS
#endif


// TODO: make this macro able to take 0 arguments except condition
#ifdef GK_ENABLE_ASSERTS
	#define GK_ASSERT(x,...) { if (!(x)) { GK_ERROR("Assertion Failed: {0}",__VA_ARGS__); __debugbreak();} } 
	#define GK_CORE_ASSERT(x,...) { if (!(x)) { GK_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();} } 
#else
	#define GK_ASSERT(x, ...) 
	#define GK_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)
//#define GK_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1) TODO: Why is this worse?
#define GK_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Gonk {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}


}