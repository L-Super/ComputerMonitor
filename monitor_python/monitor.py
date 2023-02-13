"""
监控系统CPU、内存、硬盘等信息
https://www.liaoxuefeng.com/wiki/1016959663602400/1183565811281984
https://www.cnblogs.com/billie52707/p/12468740.html
https://blog.csdn.net/weixin_44621343/article/details/112726406?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522166995737716782429712960%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=166995737716782429712960&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~top_positive~default-1-112726406-null-null.142^v67^control,201^v3^control,213^v2^t3_control2&utm_term=psutil&spm=1018.2226.3001.4187
"""
import json
import re
import os
import subprocess
import time
import psutil

# 1B为单位
GB = 1024.0 * 1024.0 * 1024.0
TB = 1024.0 * 1024.0 * 1024.0 * 1024.0


def cpu_percent():
    """CPU使用率.

    :return: 返回 10.5%形式
    """
    # cpu = str(psutil.cpu_percent(interval=0.1)) + '%'
    # print(f"cup使用率: {cpu}")
    return psutil.cpu_percent(interval=0.1) / 100


def cpu_current_frequency():
    """获取CPU当前频率, GHz单位.

    在Linux上，当前频率报告实时值，在所有其他平台上，这通常代表名义上的“固定”值（永不改变）。
    :return:
    """
    return psutil.cpu_freq().current / 1000


def memory_total():
    """总内存.

    psutil.virtual_memory()返回的是字节为单位的整数（1KB = 1024B（字节 Byte））
    1.total表示内存总的大小
    2.percent表示实际已经使用的内存占比。
    3.available表示还可以使用的内存。
    4.used表示已经使用的内存
    :return:
    """
    total = psutil.virtual_memory().total / GB
    # round()四舍五入保留两位小数
    return round(total, 2)


def memory_used():
    """已经使用的内存.

    :return:
    """
    return round(psutil.virtual_memory().used / GB, 2)


def memory_available():
    """未使用的内存.

    :return:
    """
    return round(psutil.virtual_memory().free / GB, 2)


def memory_percent():
    """内存使用率.

    :return:
    """
    return psutil.virtual_memory().percent / 100


def disk_info():
    """磁盘容量等信息.

    返回 位置 总容量 空余容量 已用容量
    :return: 磁盘列表 [["位置", total TB, free TB, used TB]]
    """
    disklist = []
    for part in psutil.disk_partitions(all=False):
        if os.name == 'nt':
            if 'cdrom' in part.opts or part.fstype == '':
                # skip cd-rom drives with no disk in it; they may raise
                # ENOENT, pop-up a Windows GUI error for a non-ready
                # partition or just hang.
                continue
        if part.mountpoint[0:5] == "/ftp/":
            # print("/ftp/ {}".format(part.mountpoint[0:5]))
            continue
        if part.mountpoint[0:6] == "/data/":
            continue

        usage = psutil.disk_usage(part.mountpoint)

        disk = [part.mountpoint, usage.total / TB, usage.free / TB, usage.used / TB]
        disklist.append(disk)

    return disklist


def disk_all_total():
    """所有磁盘总共大小.

   :return 返回元组（总容量，使用量）
    """
    total = 0.0
    used = 0.0
    for part in disk_info():
        total += part[1]
        used += part[3]

    # 返回元组形式
    return total, used


def all_disks_io():
    """磁盘io统计信息(汇总的)，包括读、写的速率.

    :return: kb/s
    """
    # TODO:解决磁盘IO精确计算问题
    old = psutil.disk_io_counters()
    t = 0.2
    time.sleep(t)
    new = psutil.disk_io_counters()

    read_io = (new.read_bytes - old.read_bytes) / 1024.0 / t
    write_io = (new.write_bytes - old.write_bytes) / 1024.0 / t
    # 返回元组形式(rio,wio)
    return read_io, write_io


def raid_disk_all_info():
    """通过MegaCli工具获取RAID磁盘信息
    """
    # python2
    # disks_info = subprocess.check_output(['/opt/MegaRAID/MegaCli/MegaCli64','-PDList','-aALL'])
    # python3.5
    disks_info = subprocess.run(['/opt/MegaRAID/MegaCli/MegaCli64', '-PDList', '-aALL'], check=True,
                                stdout=subprocess.PIPE).stdout
    # print(disks_info)

    raid_info_list = []
    for line in disks_info.decode().split('\n'):
        raid_info_list.append(line)
    return raid_info_list


def match_data_from_raid(raid_info_list):
    """从MegaCli工具返回的磁盘信息中过滤需要的数据
    note:严格按序排列进行匹配
    """
    enclosure_device_id = re.compile("Enclosure Device ID")
    slot_number = re.compile("Slot Number")
    media_error_count = re.compile('Media Error Count')
    other_error_count = re.compile('Other Error Count')
    predictive_failure_count = re.compile("Predictive Failure Count")
    coerced_size = re.compile("Coerced Size")
    state = re.compile("Firmware state")

    all_result = []
    match_result = {}
    # 严格按序匹配
    for info in raid_info_list:
        if enclosure_device_id.match(info):
            match_result["Enclosure Device ID"] = info.split(':')[1].strip()
        if slot_number.match(info):
            match_result["Slot Number"] = info.split(':')[1].strip()
        if media_error_count.match(info):
            match_result["Media Error Count"] = info.split(':')[1].strip()
        if other_error_count.match(info):
            match_result["Other Error Count"] = info.split(':')[1].strip()
        if predictive_failure_count.match(info):
            match_result["Predictive Failure Count"] = info.split(':')[1].strip()
        if coerced_size.match(info):
            # tmp = 3.637 TB [0x1d1b00000 Sectors]
            tmp = info.split(':')[1].strip()
            match_result["Coerced Size"] = tmp.split("TB")[0].strip()
        if state.match(info):
            match_result["Firmware state"] = info.split(':')[1].strip()

            # append dict to list
            all_result.append(match_result.copy())
            match_result.clear()
    return all_result


def get_raid_disk_info():
    """获取json规定形式的raid列表
    """
    pd_list = match_data_from_raid(raid_disk_all_info())
    re_list = []
    for d in pd_list:
        position = d.get("Enclosure Device ID") + "-" + d.get("Slot Number")
        capacity = d.get("Coerced Size")
        status = 0 if d.get("Firmware state") == "Online, Spun Up" else 1
        progress = 1
        result = 0
        aver_used_disk = disk_all_total()[1] / 4
        aver_free_disk = round(float(capacity) - aver_used_disk, 3)
        re_list.append({"diskPosition": position,
                        "diskCapacity": capacity,
                        "diskSpace": str(aver_free_disk),
                        "diskState": status,
                        "testProgress": progress,
                        "testResult": result,
                        "enable": True})
    return re_list


class Monitor:
    def __int__(self):
        pass

    @staticmethod
    def json_cpu_percent() -> json:
        cpu = cpu_percent()

        # data = json.dumps({'cpu': {'usage': cpu}}, sort_keys=True, indent=4, separators=(',', ': '))
        data = json.dumps({'cpu': {'usage': cpu}})

        return data

    @staticmethod
    def json_disk_overview():
        disk = disk_all_total()
        # lastCount 总容量 useCount 已用容量
        data = json.dumps({
            "data": {
                "lastCount": round(disk[0], 2),
                "useCount": round(disk[1], 2)
            }})

        return data

    @staticmethod
    def json_disks_info():
        d = get_raid_disk_info()

        data = json.dumps({
            "data": {
                "list": d,
                "total": round(disk_all_total()[0], 2)
            }
        })
        return data

    @staticmethod
    def json_all_disks_io() -> json:
        disk_io = all_disks_io()
        data = json.dumps({
            "data": {
                "writeIo": disk_io[1],
                "readIo": disk_io[0]
            }
        })
        return data


if __name__ == '__main__':
    pass

    # m = Monitor()
    # print(m.json_disk_overview())
