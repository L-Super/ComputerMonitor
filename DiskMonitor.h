//
// Created by Listening on 2022/11/14.
//

#ifndef _DISKMONITOR_H_
#define _DISKMONITOR_H_

#include <memory>
#include <vector>
#include "Common.h"

namespace CM
{
	/**
	 * @brief Disk class
	 */
	class DiskPrivate;
	class DiskMonitor{
	 public:
		DiskMonitor() noexcept;
		~DiskMonitor();
		void GetDiskInfo();
		ulong GetDiskTotal();
		ulong GetDiskUsed();
		ulong GetDiskFree();

		std::vector<DFStructInfo> GetDisksByDf();
	 private:
		std::unique_ptr<DiskPrivate> d_ptr;
	};

} // CM

#endif //_DISKMONITOR_H_
