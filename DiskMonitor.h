//
// Created by Listening on 2022/11/14.
//

#ifndef _DISKMONITOR_H_
#define _DISKMONITOR_H_

#include <memory>
#include <vector>
#include "Common.h"

namespace CM {
    /**
     * @brief Disk class
     */
    class DiskPrivate;

    class DiskMonitor {
    public:
        DiskMonitor() noexcept;

        ~DiskMonitor();

        /**
         * @brief 获取当前磁盘总容量
         * 通过 statfs("/", &diskInfo)方式
         * @return
         */
        type GetDiskTotal();
        /**
         * @brief 获取当前磁盘使用量
         * 通过 statfs("/", &diskInfo)方式
         * @return
         */
        type GetDiskUsed();
        /**
         * @brief 获取当前磁盘剩余容量
         * 通过 statfs("/", &diskInfo)方式
         * @return
         */
        type GetDiskFree();

        /**
         * @brief 返回磁盘总容量
         * 通过df命令获取 /sdX的总容量
         * @return 单位TB
         */
        double DiskTotalByDf();

        /**
         * @brief 返回磁盘使用量
         * 通过df命令获取 /sdx的总使用量
         * @return 单位TB
         */
        double DiskUsedByDf();

        /**
         * @brief 通过df命令获取磁盘信息
         * @return std::vector<DFStructInfo> 磁盘结构体的vector
         */
        std::vector<DFStructInfo> GetDisksByDf();

    private:
        std::unique_ptr<DiskPrivate> d_ptr;
        const double TB{1024 * 1024 * 1024};
    };

} // CM

#endif //_DISKMONITOR_H_
