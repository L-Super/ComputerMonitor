//
// Created by Listening on 2022/11/14.
//

#include "DiskMonitor.h"
#include "spdlog/spdlog.h"
#include <sys/statfs.h>
#include <atomic>
#include <thread>

namespace CM {
    class DiskPrivate {
    public:
        DiskPrivate();

        ~DiskPrivate();

        auto DiskInfoByStatfs();

        auto DiskInfoByDf();

        type GetDiskTotal();

        type GetDiskUsed();

        type GetDiskFree();

        /**
         * @brief 查询磁盘IO
         * 使用iostat.系统可能需要安装sysstat
         */
        void DiskIO();

        void DiskIOThread();

    public:
        std::atomic<double> diskReadIO;//磁盘读取IO速度
        std::atomic<double> diskWriteIO;//磁盘写入IO速度
    private:
        type totalDisk;
        type usedDisk;
        type freeDisk;
        std::atomic_bool ioThreadRunning{true};
        std::thread ioThread;
    };

    DiskPrivate::DiskPrivate() {
        ioThread = std::thread(&DiskPrivate::DiskIOThread, this);
    }

    DiskPrivate::~DiskPrivate() {
        ioThreadRunning.store(false);
        ioThread.join();
    }

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
            if (strncmp("/dev/", fileSystem, 5) == 0) {
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

    void DiskPrivate::DiskIO() {
        //查看系统中硬盘的使用情况,每隔一秒查询两次
        //第一次输出的数据是系统从启动以来直到统计时的所有传输信息，第二次输出的数据，才代表在指定检测时间段内系统的传输值。
        char cmd[] = "iostat -d 1 2";
        char buffer[1024];
        char devName[20];
        double data[8];
        bzero(data, sizeof(data));

        FILE *pipe = popen(cmd, "r");

        if (!pipe) {
            spdlog::error("run iostat failed");
        }

        //Linux 5.10.102.1-microsoft-standard-WSL2 (DESKTOP-LMR)  12/02/22        _x86_64_        (12 CPU)
        fgets(buffer, sizeof(buffer) - 1, pipe);
        //
        fgets(buffer, sizeof(buffer) - 1, pipe);
        //Device             tps    kB_read/s    kB_wrtn/s    kB_dscd/s    kB_read    kB_wrtn    kB_dscd
        fgets(buffer, sizeof(buffer) - 1, pipe);

        std::atomic_bool secondTime{false};//第二次遇到Device
        std::atomic<double> readIO{0};//磁盘读取IO速度
        std::atomic<double> writeIO{0};//磁盘写入IO速度

        //过滤第一次
        while (fgets(buffer, sizeof(buffer) - 1, pipe) != NULL) {
            //Device tps kB_read/s kB_wrtn/s kB_dscd/s kB_read kB_wrtn kB_dscd
            sscanf(buffer, "%s %lf %lf %lf %lf %lf %lf %lf", devName, &data[0], &data[1], &data[2], &data[3], &data[4],
                   &data[5], &data[6]);

            spdlog::warn("{} {} {} {} {} {} {}", devName, data[0], data[1], data[2], data[3], data[4], data[5],
                         data[6]);
            if (secondTime.load() == false && strncmp("Device", devName, 6) == 0) {
//                spdlog::warn("second title {}", buffer);
                secondTime.store(true);
                continue;
            }
            if (secondTime.load() == true && strncmp("Device", devName, 6) != 0) {
                //std::atomic<double>不支持fetch_add()
                std::atomic<double> tmp{readIO.load() + data[1]};
                readIO.exchange(tmp);
                tmp.store(writeIO.load() + data[2]);
                writeIO.exchange(tmp);
//                spdlog::error("IO {} {}", readIO, writeIO);
            }
        }
        this->diskReadIO.store(readIO);
        this->diskWriteIO.store(writeIO);

        pclose(pipe);
    }

    void DiskPrivate::DiskIOThread() {
        while (ioThreadRunning) {
            DiskIO();
        }
        spdlog::warn("IO thread quit");
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

    std::tuple<double, double> DiskMonitor::GetDiskIO() {
        return std::make_tuple(d_ptr->diskReadIO.load(), d_ptr->diskWriteIO.load());
    }
} // CM