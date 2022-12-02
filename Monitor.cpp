//
// Created by Listening on 2022/11/9.
//

#include "Monitor.h"
#include "spdlog/spdlog.h"
#include "Common.h"

#define USEDF_METHOD 1

namespace CM {
    Monitor::Monitor()
            :
            cpuMonitor(Factory::Create<CPUMonitor>()),
            memMonitor(Factory::Create<MemoryMonitor>()),
            diskMonitor(Factory::Create<DiskMonitor>()) {
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
        const double GB = 1024 * 1024;
        auto disks = diskMonitor->GetDisksByDf();
        size_t num = 0;
        for (const auto &it: disks) {
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

    json Monitor::DiskInfoOverview() {
        json diskJson;
        diskJson["data"] = {
                {"lastCount", diskMonitor->DiskTotalByDf()},
                {"useCount",  diskMonitor->DiskUsedByDf()}
        };
        return diskJson;
    }

    json Monitor::DisksInfo() {
        json disksJson, diskListJson;
        auto disks = diskMonitor->GetDisksByDf();
        const double TB{1024 * 1024 * 1024};
        for (const auto &it: disks) {
            json data;
            data["diskPosition"] = it.mounted;
            data["diskCapacity"] = std::to_string(it.blocks / TB);
            data["diskSpace"] = std::to_string(it.available / TB);
            data["diskState"] = 0;
            data["testProgress"] = 1;
            data["testResult"] = 1;
            data["enable"] = true;
            diskListJson["list"].push_back(data);
        }

        disksJson["data"] = {diskListJson,
                             {"total", diskMonitor->DiskTotalByDf()}};

        return disksJson;
    }

    json Monitor::DisksIO() {
        json ioJson;
        auto [read, write] = diskMonitor->GetDiskIO();
        ioJson["data"] = {
                {"writeIo", write},
                {"readIo", read}
        };
        return ioJson;
    }

} // CM