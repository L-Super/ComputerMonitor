/**
 * \brief  计算机性能监视类
 * 监测CPU、内存、磁阵数据
 * \author Listening
 * \date September 2022
 */

#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <memory>
#include <string>
#include "nlohmann/json.hpp"

#include "CPUMonitor.h"
#include "DiskMonitor.h"
#include "MemoryMonitor.h"


namespace CM
{
	using json = nlohmann::json;

	struct Factory
	{
		template<typename OS>
		static std::unique_ptr<OS> Create()
		{
			auto os = std::make_unique<OS>();
			return os;
		}
	};

	class Monitor {
	 public:
		Monitor();
		~Monitor() = default;
		json GetCPU();
		json GetMemory();
		json GetDisk();
        /**
         * @brief 获取cpu memory disk信息
         * @return json
         */
		json GetInfoByJson();

        /**
         * @brief 返回总容量和使用量
         * 服务器规定json形式
         * @return TB
         */
         json DiskInfoOverview();
         /**
          * @brief 返回各磁盘信息
          * 服务器规定json形式
          * 磁盘位置、磁盘容量TB、磁盘剩余空间TB、磁盘状态
          * @return
          */
         json DisksInfo();
	 private:
		template<typename T>
		using smart_ptr = std::unique_ptr<T>;

		smart_ptr<CPUMonitor> cpuMonitor;
		smart_ptr<MemoryMonitor> memMonitor;
		smart_ptr<DiskMonitor> diskMonitor;
	};

	/**
 	* \brief monitor factory create.
    *
 	* \return std::shared_ptr<Monitor>
 	*/
	template<typename Factory = CM::Factory>
	inline std::unique_ptr<Monitor> CreateMonitor()
	{
		return Factory::template Create<Monitor>();
	}
} // CM

#endif //_MONITOR_H_
