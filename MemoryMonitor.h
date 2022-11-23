//
// Created by Listening on 2022/11/14.
//

#ifndef _MEMORYMONITOR_H_
#define _MEMORYMONITOR_H_

#include <memory>

namespace CM
{
	/**
	 * @brief Memory class
	 */
	class MemoryPrivate;
	class MemoryMonitor {
	 public:
		MemoryMonitor() noexcept;
		~MemoryMonitor();
		/**
		 * 获取总内存
		 * @return 单位为GB
		 */
		unsigned long GetMemoryTotal();
		/**
		 * 获取未使用的内存
		 * @return 单位为GB
		 */
		unsigned long GetMemoryUnused();
		/**
		 * 获取内存使用率
		 * @return 小数 0.55
		 */
		std::string GetMemoryUsage();

	 private:
		std::unique_ptr<MemoryPrivate> d_ptr;
		const int GB{1024*1024};
	};

} // CM

#endif //_MEMORYMONITOR_H_
