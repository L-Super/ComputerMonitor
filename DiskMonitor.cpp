//
// Created by Listening on 2022/11/14.
//

#include "DiskMonitor.h"
#include "spdlog/spdlog.h"
#include <sys/statfs.h>

namespace CM
{
//	/**
//	 * @brief df 结构体
//	 * 磁盘信息
//	 */
//	struct DFStructInfo {
//		char fileSystem[80];
//		ulong blocks;
//		ulong used;
//		ulong available;
//		char useRate[10];
//		char mounted[80];
//	};

	class DiskPrivate {
	 public:
		DiskPrivate() = default;
		~DiskPrivate() = default;
		auto DiskInfoByStatfs();
		auto DiskInfoByDf();
		ulong GetDiskTotal();
		ulong GetDiskUsed();
		ulong GetDiskFree();

	 private:
		ulong totalDisk;
		ulong usedDisk;
		ulong freeDisk;
	};

	auto DiskPrivate::DiskInfoByStatfs()
	{
		struct statfs diskInfo;

		//TODO:位置
		statfs("/", &diskInfo);
		auto blockSize = diskInfo.f_bsize;    //每个block里包含的字节数
		ulong totalSize = blockSize * diskInfo.f_blocks;   //总的字节数，f_blocks为block的数目
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
	auto DiskPrivate::DiskInfoByDf()
	{
		char buf[256], fileSystem[80], useRate[10], mountedOn[80];
		memset(buf, 0, sizeof(buf));

		ulong blocks, used, available;
		ulong total = 0, used2 = 0;

		FILE* fp = popen("df", "r");
		//先get标题栏，过滤
		//Filesystem 1K-blocks Used Available Use% Mounted on
		fgets(buf, 256, fp);

		DFStructInfo dfDisk;
		std::vector<DFStructInfo> vDisks;
		while (6 == fscanf(fp, "%s %lu %lu %lu %s %s", fileSystem, &blocks, &used, &available, useRate, mountedOn))
		{
//			spdlog::info("Filesystem:{} 1K-blocks:{} Used:{} Available:{} Use%:{} Mounted on:{}",
//				fileSystem, blocks, used, available, useRate, mountedOn);
			if (strncmp("/dev/", fileSystem, 4) == 0)
			{
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
	ulong DiskPrivate::GetDiskTotal()
	{
		DiskInfoByStatfs();
		return totalDisk;
	}
	ulong DiskPrivate::GetDiskUsed()
	{
		DiskInfoByStatfs();
		return usedDisk;
	}
	ulong DiskPrivate::GetDiskFree()
	{
		DiskInfoByStatfs();
		return freeDisk;
	}

	DiskMonitor::DiskMonitor() noexcept
		: d_ptr(new DiskPrivate())
	{

	}
	//https://blog.csdn.net/no_say_you_know/article/details/127855984
	DiskMonitor::~DiskMonitor() = default;

	void DiskMonitor::GetDiskInfo()
	{
		d_ptr->DiskInfoByStatfs();
	}
	ulong DiskMonitor::GetDiskTotal()
	{
		return d_ptr->GetDiskTotal() >> 30;
	}
	ulong DiskMonitor::GetDiskUsed()
	{
		return d_ptr->GetDiskUsed() >> 30;
	}
	ulong DiskMonitor::GetDiskFree()
	{
		return d_ptr->GetDiskFree() >> 30;
	}
	std::vector<DFStructInfo> DiskMonitor::GetDisksByDf()
	{
		return d_ptr->DiskInfoByDf();
	}
} // CM