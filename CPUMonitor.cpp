//
// Created by Listening on 2022/11/14.
//
#include <future>
#include <thread>
#include "CPUMonitor.h"
#include "Common.h"
#include "spdlog/spdlog.h"

namespace CM {
    /**
     * cpu info结构体存放cpu相关信息
     */
    struct CPUInfo {
        char name[20];
        unsigned int user;
        unsigned int nice;
        unsigned int system;
        unsigned int idle;
        unsigned int iowait;
        unsigned int irq;
        unsigned int softirq;
    };

    class CPUPrivate {
    public:
        CPUPrivate();

        ~CPUPrivate();

        auto GetCPUInfo();

        auto CalcCPUUsage(const CPUInfo &oldInfo, const CPUInfo &newInfo);

        void RunCPUThread();

        double GetCPUUSage();

        auto GetCPUModelNameAndBaseClock();

    private:
        std::atomic<double> cpuUsage;
        std::atomic_bool running;
    };

    CPUPrivate::CPUPrivate()
            : cpuUsage(-1.0),
              running(true) {
//		std::async(/*std::launch::async, */&CPUPrivate::RunCPUThread,this);
        std::thread t(&CPUPrivate::RunCPUThread, this);
        t.detach();

    }

    CPUPrivate::~CPUPrivate() {
        //detach线程停止运行
        running.store(false);
    }

    auto CPUPrivate::GetCPUInfo() {
        //从/proc/stat文件中获取cpu的相关信息
        char buf[256];
        CPUInfo info{};
//		memset(&info, 0, sizeof(CPUInfo));

        FILE *fp = fopen("/proc/stat", "r");
        if (!fp) {
            spdlog::error("/proc/stat open failed");
//			return CPUInfo{};
        }

        fgets(buf, sizeof(buf), fp);

        sscanf(buf, "%s %u %u %u %u %u %u %u",
               info.name,
               &info.user,
               &info.nice,
               &info.system,
               &info.idle,
               &info.iowait,
               &info.irq,
               &info.softirq);

        fclose(fp);
        return info;
    }

    auto CPUPrivate::CalcCPUUsage(const CPUInfo &oldInfo, const CPUInfo &newInfo) {
        double od = (double) (oldInfo.user +
                              oldInfo.nice +
                              oldInfo.system +
                              oldInfo.idle +
                              oldInfo.iowait +
                              oldInfo.irq +
                              oldInfo.softirq);
        double nd = (double) (newInfo.user +
                              newInfo.nice +
                              newInfo.system +
                              newInfo.idle +
                              newInfo.iowait +
                              newInfo.irq +
                              newInfo.softirq);

        double sumTime = nd - od;
        double freeTime = newInfo.idle - oldInfo.idle;

        double cpuUsageRate = 1 - freeTime / sumTime;

        //TODO:两者暂未确定谁更准确
//		freeTime = newInfo.user + newInfo.system + newInfo.nice - oldInfo.user - oldInfo.system - oldInfo.nice;
//		auto cpu_use2 = freeTime / sumTime;
//		auto freeTimeStr = spdlog::fmt_lib::format(" {:.2f}",cpu_use2);

//		spdlog::info("cpu:{} freetime2:{}", cpuUsageRateStr, freeTimeStr);
//		return cpuUsageRateStr;
        if (cpuUsageRate > 0) {
            return cpuUsageRate;
        }

        return (double) (0.0);
    }

    double CPUPrivate::GetCPUUSage() {
#if 0
        using namespace std::chrono_literals;
        auto oldCPU = GetCPUInfo();
        std::this_thread::sleep_for(0.5s);
        auto newCPU = GetCPUInfo();
//		cpuUsage.store(CalcCPUUsage(oldCPU, newCPU));
#endif
        if (cpuUsage < 0)
            return 0.0;
        return cpuUsage;
    }

    void CPUPrivate::RunCPUThread() {
        using namespace std::chrono_literals;
        while (running) {
            auto oldCPU = GetCPUInfo();
            std::this_thread::sleep_for(0.5s);
            auto newCPU = GetCPUInfo();
            cpuUsage.store(CalcCPUUsage(oldCPU, newCPU));
//			std::this_thread::yield();
        }
        return;
    }

    auto CPUPrivate::GetCPUModelNameAndBaseClock() {
        FILE *fp = fopen("/proc/cpuinfo", "r");
        if (NULL == fp)
            spdlog::error("open /proc/cpuinfo failed");

        char buff[1024];
        std::string modelName;
        std::string cpuMhz;
        //多核会冗余遍历多次，故findNum计数到两次时，表示都找到了，提前退出循环
        std::atomic_int findNum{0};
        //read file line by line
        while (!feof(fp)) {
            memset(buff, 0, sizeof(buff));
            fgets(buff, sizeof(buff) - 1, fp);

            if (strncmp("model name", buff, 9) == 0) {
                std::string tmp(buff);
                auto pos = tmp.find(":");
                modelName = tmp.substr(pos + 1);
                string_trimmed(modelName);
                findNum++;
//                spdlog::warn("find {}", modelName);
            }
            if (strncmp("cpu MHZ", buff, 6) == 0) {
                std::string tmp(buff);
                cpuMhz = tmp.substr(tmp.find(":") + 1);
                string_trimmed(cpuMhz);
                findNum++;
            }

            if(findNum == 2)
                break;
        }
        fclose(fp);
        return std::tuple<std::string, std::string>(modelName, cpuMhz);
    }

    CPUMonitor::CPUMonitor() noexcept
            : d_ptr(new CPUPrivate()) {
//		auto future = std::async(std::launch::async, &CPUPrivate::RunCPUThread,d_ptr.get());

    }

    //https://blog.csdn.net/no_say_you_know/article/details/127855984
    CPUMonitor::~CPUMonitor() = default;

    std::string CPUMonitor::GetCpuUsage() {
        //保留两位小数
        auto cpuUsageRateStr = spdlog::fmt_lib::format(" {:.2f}", d_ptr->GetCPUUSage());
//		spdlog::info("CPU str {} stof {}", cpuUsageRateStr, std::stof(cpuUsageRateStr));
        return cpuUsageRateStr;
    }

    std::string CPUMonitor::GetCPUModelName() {
        if (modelName.empty())
            modelName = std::get<0>(d_ptr->GetCPUModelNameAndBaseClock());
        return modelName;
    }

    std::string CPUMonitor::GetCpuBaseClock() {
        if (baseClock.empty())
            baseClock = std::get<1>(d_ptr->GetCPUModelNameAndBaseClock());
        return baseClock;
    }

} // CM