//
// Created by Listening on 2022/11/14.
//

#include "MemoryMonitor.h"
#include "spdlog/spdlog.h"
#include <cstring>

namespace CM {
    using type = unsigned long;

    /**
     * @brief 内存结构体
     * proc/meminfo文件结构
     * MemTotal:       32362796 kB
     * MemFree:        28058464 kB
     * MemAvailable:   29889448 kB
     * Buffers:            5780 kB
     * Cached:          2269436 kB
     * SwapCached:            0 kB
     * Active:          1553468 kB
     * Inactive:        1777756 kB
     */
    struct MemoryPacket {
        char MemTotalStr[20];//所有可用的内存大小，物理内存减去预留位和内核使用。
        unsigned long MemTotal;
        char MemFreeStr[20];//表示系统尚未使用的内存。
        unsigned long MemFree;
        char MemAvailableStr[20];//真正的系统可用内存，可回收的内存加上MemFree才是系统可用的内存
        unsigned long MemAvailable;
        char BuffersStr[20];//用来给块设备做缓存的内存
        unsigned long Buffers;
        char CachedStr[20];//分配给文件缓冲区的内存
        unsigned long Cached;
        unsigned long Shmem;
        unsigned long SReclaimable;
    };

    class MemoryPrivate {
    public:
        MemoryPrivate() = default;

        ~MemoryPrivate() = default;

        auto GetMemoryInfo();

        type GetMemoryTotal();

        type GetMemoryUnused();

        auto GetMemoryUsage();

    private:
        type totalMemory;
        type usedMemory;
        type freeMemory;
    };

    auto MemoryPrivate::GetMemoryInfo() {
        char buff[256];
        MemoryPacket m{};
        memset(buff, 0, sizeof(buff));
        memset(&m, 0, sizeof(m));

        FILE *fp = fopen("/proc/meminfo", "r");
        if (!fp) {
            spdlog::error("/proc/meminfo open failed");
        }

        //按序读取
        //从fd文件中读取长度为buff的字符串再存到起始地址为buff这个空间里
//		fgets(buff, sizeof(buff), fp);
//		sscanf(buff, "%s %lu ", m.MemTotalStr, &m.MemTotal);
//		fgets(buff, sizeof(buff), fp);
//		sscanf(buff, "%s %lu ", m.MemFreeStr, &m.MemFree);
//		fgets(buff, sizeof(buff), fp);
//		sscanf(buff, "%s %lu ", m.MemAvailableStr, &m.MemAvailable);
//		fgets(buff, sizeof(buff), fp);
//		sscanf(buff, "%s %lu ", m.BuffersStr, &m.Buffers);
//		fgets(buff, sizeof(buff), fp);
//		sscanf(buff, "%s %lu", m.CachedStr, &m.Cached);

        while (!feof(fp)) {
            if (fgets(buff, 255, fp) == NULL) {
                break;
            }
            if (strncmp(buff, "MemTotal:", 9) == 0) {
                sscanf(buff, "%*s %lu", &m.MemTotal);
            } else if (strncmp(buff, "MemFree:", 8) == 0) {
                sscanf(buff, "%*s %lu", &m.MemFree);
            } else if (strncmp(buff, "Buffers:", 8) == 0) {
                sscanf(buff, "%*s %lu", &m.Buffers);
            } else if (strncmp(buff, "Cached:", 7) == 0) {
                sscanf(buff, "%*s %lu", &m.Cached);
            } else if (strncmp(buff, "Shmem:", 6) == 0) {
                sscanf(buff, "%*s %lu", &m.Shmem);
            } else if (strncmp(buff, "SReclaimable:", 13) == 0) {
                sscanf(buff, "%*s %lu", &m.SReclaimable);
            }
        }

        fclose(fp);//关闭文件fd

        return m;
    }

    type MemoryPrivate::GetMemoryTotal() {
        return GetMemoryInfo().MemTotal;
    }

    type MemoryPrivate::GetMemoryUnused() {
        return GetMemoryInfo().MemAvailable;
    }

    auto MemoryPrivate::GetMemoryUsage() {
        auto memInfo = GetMemoryInfo();

//        auto total = memInfo.MemTotal;
//        auto used = memInfo.MemTotal - GetMemoryInfo().MemAvailable;

        //http://calimeroteknik.free.fr/blag/?article20/really-used-memory-on-gnu-linux
        //https://github.com/netdata/netdata/blob/2d5f3acf71f0c759056a3269987fee484566bc4c/collectors/proc.plugin/proc_meminfo.c#L157-L163
        //Used RAM = MemTotal - (MemFree + Buffers + SReclaimable + Cached - Shmem)
        auto total = memInfo.MemTotal;
        auto used = total - (memInfo.MemFree + memInfo.Buffers + memInfo.Cached + memInfo.SReclaimable - memInfo.Shmem);
        //保留两位小数
        auto re = spdlog::fmt_lib::format(" {:.2f}", static_cast<double>(used) / total);

//		return std::stof(re);
        return re;
    }

    MemoryMonitor::MemoryMonitor() noexcept
            : d_ptr(new MemoryPrivate) {
    }

    //https://blog.csdn.net/no_say_you_know/article/details/127855984
    MemoryMonitor::~MemoryMonitor() = default;

    unsigned long MemoryMonitor::GetMemoryTotal() {
        return d_ptr->GetMemoryTotal() / GB;
    }

    unsigned long MemoryMonitor::GetMemoryUnused() {
        return d_ptr->GetMemoryUnused() / GB;
    }

    std::string MemoryMonitor::GetMemoryUsage() {
        return d_ptr->GetMemoryUsage();
    }
} // CM