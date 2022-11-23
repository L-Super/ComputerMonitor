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
		 * @return string
		 */
		std::string GetCPUModelName();
		/**
		 * @brief 获取CPU主频
		 * 单位 MHZ
		 * @return
		 */
		std::string GetCpuBaseClock();
	 private:
		std::unique_ptr<CPUPrivate> d_ptr;
        std::string modelName;
        std::string baseClock;
	};


} // CM

#endif //_CPUMONITOR_H_
