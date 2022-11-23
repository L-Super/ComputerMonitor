//
// Created by Listening on 2022/11/17.
//

#ifndef _COMMON_H_
#define _COMMON_H_
namespace CM{
	using ulong = unsigned long;
	/**
 	* @brief df 结构体
 	* 磁盘信息
 	*/
	struct DFStructInfo {
		char fileSystem[80];
		ulong blocks;
		ulong used;
		ulong available;
		char useRate[10];
		char mounted[80];
	};

}
#endif //_COMMON_H_
