//
// Created by Listening on 2022/11/14.
//

#include "DiskMonitor.h"
#include "spdlog/spdlog.h"
#include <sys/statfs.h>

namespace CM {
    class DiskPrivate {
    public:
        DiskPrivate() = default;

        ~DiskPrivate() = default;

        auto DiskInfoByStatfs();

        auto DiskInfoByDf();

        type GetDiskTotal();

        type GetDiskUsed();

        type GetDiskFree();

    private:
        type totalDisk;
        type usedDisk;
        type freeDisk;
    };

    auto DiskPrivate::DiskInfoByStatfs() {
        struct statfs diskInfo;

        //TODO:位置
        statfs("/", &diskInfo);
        auto blockSize = diskInfo.f_bsize;    //每个block里包含的字节数
        type totalSize = blockSize * diskInfo.f_blocks;   //总的字节数，f_blocks为block的数目
//		unsigned long long freeDisk = diskInfo.f_bfree * blockSize; //剩余空间的大小
        unsigned long availableDisk = diskInfo.f_bavail * blockSize;   //可用空间大小

//		printf("Total_size = %llu B = %llu KB = %llu MB = %llu GB\n",
//			totalSize, totalSize >> 10, totalSize >> 20, totalSize >> 30);

//		printf("Total disk = %lu MB = %lu GB\n", totalSize >> 20, totalSize >> 30);
//		printf("Disk available = %lu MB = %lu GB\n", availableDisk >> 20, availableDisk >> 30);

        this->totalDisk = totalSize;
        this->freeDisk = availableDisk;
        this->usedDisk = totalSize - availableDisk;
    }

    auto DiskPrivate::DiskInfoByDf() {
        char buf[256], fileSystem[80], useRate[10], mountedOn[80];
        memset(buf, 0, sizeof(buf));

        type blocks, used, available;
        //type total = 0, used2 = 0;

        FILE *fp = popen("df", "r");
        //先get标题栏，过滤
        //Filesystem 1K-blocks Used Available Use% Mounted on
        fgets(buf, 256, fp);

        DFStructInfo dfDisk;
        std::vector<DFStructInfo> vDisks;
        while (6 == fscanf(fp, "%s %lf %lf %lf %s %s", fileSystem, &blocks, &used, &available, useRate, mountedOn)) {
//			spdlog::info("Filesystem:{} 1K-blocks:{} Used:{} Available:{} Use%:{} Mounted on:{}",
//				fileSystem, blocks, used, available, useRate, mountedOn);
            if (strncmp("/dev/", fileSystem, 4) == 0) {
//				spdlog::info("find {} disk", fileSystem);
                strcpy(dfDisk.fileSystem, fileSystem);
                dfDisk.blocks = blocks;
                dfDisk.used = used;
                dfDisk.available = available;
                strcpy(dfDisk.useRate, useRate);
                strcpy(dfDisk.mounted, mountedOn);

                vDisks.push_back(dfDisk);
            }
        }
        pclose(fp);
        return vDisks;
    }

    type DiskPrivate::GetDiskTotal() {
        DiskInfoByStatfs();
        return totalDisk;
    }

    type DiskPrivate::GetDiskUsed() {
        DiskInfoByStatfs();
        return usedDisk;
    }

    type DiskPrivate::GetDiskFree() {
        DiskInfoByStatfs();
        return freeDisk;
    }

    DiskMonitor::DiskMonitor() noexcept
            : d_ptr(new DiskPrivate()) {

    }

    //https://blog.csdn.net/no_say_you_know/article/details/127855984
    DiskMonitor::~DiskMonitor() = default;

    type DiskMonitor::GetDiskTotal() {
        return d_ptr->GetDiskTotal() / TB;
    }

    type DiskMonitor::GetDiskUsed() {
        return d_ptr->GetDiskUsed() / TB;
    }

    type DiskMonitor::GetDiskFree() {
        return d_ptr->GetDiskFree() / TB;
    }

    std::vector<DFStructInfo> DiskMonitor::GetDisksByDf() {
        return d_ptr->DiskInfoByDf();
    }

    double DiskMonitor::DiskTotalByDf() {
        auto disks = d_ptr->DiskInfoByDf();
        double total{0};
        for (auto &it: disks) {
            total += it.blocks / TB;
        }
        return total;
    }

    double DiskMonitor::DiskUsedByDf() {
        auto disks = d_ptr->DiskInfoByDf();
        double used{0};
        for (auto &it: disks) {
            used += it.used / TB;
        }
        return used;
    }
} // CM