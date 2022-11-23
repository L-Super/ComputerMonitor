//
// Created by Listening on 2022/11/9.
//

#include "Monitor.h"
#include "spdlog/spdlog.h"

#define USEDF_METHOD 1

namespace CM {
    Monitor::Monitor()
            :
            cpuMonitor(Factory::Create<CPUMonitor>()),
            memMonitor(new MemoryMonitor()),
            diskMonitor(new DiskMonitor()) {
    }

    json Monitor::GetCPU() {
        json cpuJson;
        //乱序输出是因为进行了自动排序
        cpuJson["cpu"] = {
                {"modelName", cpuMonitor->GetCPUModelName()},
                {"baseClock", cpuMonitor->GetCpuBaseClock()},
                {"usage",     cpuMonitor->GetCpuUsage()}
        };
        return cpuJson;
    }

    json Monitor::GetMemory() {
        json memJson;
        memJson["memory"] = {
                {"total",  memMonitor->GetMemoryTotal()},
                {"unused", memMonitor->GetMemoryUnused()},
                {"usage",  memMonitor->GetMemoryUsage()}
        };

        return memJson;
    }

    json Monitor::GetDisk() {
#if !USEDF_METHOD
        json diskJson;
        diskJson["disk0"] = {
            { "total", diskMonitor->GetDiskTotal() },
            { "unused", diskMonitor->GetDiskFree() },
            { "used", diskMonitor->GetDiskUsed() }
        };
#else
        json diskJson;
        const int GB = 1024 * 1024;
        auto disks = diskMonitor->GetDisksByDf();
        size_t num = 0;
        for (auto &it: disks) {
//			spdlog::info("disk{} mounted on:{}", it.fileSystem, it.mounted);
            std::string name{"disk" + std::to_string(num)};
            diskJson[name] = {
                    {"name",   it.mounted},
                    {"total",  it.blocks / GB},
                    {"unused", it.available / GB},
                    {"used",   it.used / GB}
            };
            num++;
        }
#endif
        return diskJson;
    }

    nlohmann::json Monitor::GetInfoByJson() {
        json result;
        //merge json
        result.merge_patch(GetCPU());
        result.merge_patch(GetMemory());
        result.merge_patch(GetDisk());

        if (result.empty())
            return {};

        return result;
    }

} // CM