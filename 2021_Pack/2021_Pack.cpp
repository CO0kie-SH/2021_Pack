// 2021_Pack.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "CPE.h"
#define FPath "D:\\cacheD\\202101\\demo.exe"
#define FPath2 "D:\\cacheD\\202101\\demo.new.exe"

int main()
{
    std::cout << "Hello World!\n";
    CPE pe;
    if (pe.ReadFile(FPath))
    {
        pe.AddSection(0, FPath2);
    }
}
