//
// Created by Listening on 2022/11/14.
//

#ifndef _CPUMONITOR_H_
#define _CPUMONITOR_H_

#include <memory>
#include <string>

namespace CM
{
	/**
	 * @brief CPU class
	 */
	class CPUPrivate;
	class CPUMonitor{
	 public:
		CPUMonitor() noexcept;
		~CPUMonitor();

		/**
		 * @brief 获取CPU使用率
		 * @return string
		 */
		std::string GetCpuUsage();
		/**
		 * @brief 获取处理器型号
		 * @return
		 */
		std::string GetCpuProcessor();
		/**
		 * @brief 获取CPU主频
		 * @return
		 */
		std::string GetCpuBaseClock();
	 private:
		std::unique_ptr<CPUPrivate> d_ptr;
	};


} // CM

#endif //_CPUMONITOR_H_
