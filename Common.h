//
// Created by Listening on 2022/11/17.
//

#ifndef _COMMON_H_
#define _COMMON_H_
namespace CM{
	/**
 	* @brief df 结构体
 	* 磁盘信息
 	*/
	struct DFStructInfo {
        using type = double;
		char fileSystem[80];
		type blocks;
		type used;
		type available;
		char useRate[10];
		char mounted[80];
	};
}

//去掉std::string 字符串前后的空格
template<typename String>
void string_trimmed(String &str)
{
    if (str.empty())
        return;
    //判断是否为空格
    auto isSpace = [](char ucs4) ->bool {
        /**
         * 0x20	space 空格
         * 0x09 HT - horizontal tab 水平制表符
         * 0x0A LF - line feed 换行键
         * 0x0B VT - vertical tab 垂直制表符
         * 0x0C FF - form feed 换页键
         * 0x0D CR - carriage return 回车键
         */
        return ucs4 == 0x20 || (ucs4 <= 0x0d && ucs4 >= 0x09);
    };

    auto begin = str.begin();
    auto end = str.end();
    while ((begin < end) && isSpace(end[-1]))
        --end;

    while ((begin < end) && isSpace(*begin))
        begin++;
    str = String(begin, end);
}

#endif //_COMMON_H_
