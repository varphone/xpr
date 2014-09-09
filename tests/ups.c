/* Note: 1.为保证库的绝对健壮性和稳定性，对库的实现必须添加单元测试,测试的格式和步骤及示例请参考test_ups_get_set_string
 *       2.对与最后一步为测试内存泄漏，我们使用工具来测试，代码里面用来测试一个正确的值被执行多次的示例. ***这一步必须要测试，防止以后混入大的项目无法测试内存问题***
 *       3.对于验收时，没有单元测试统一验收不合格，代码不允许合并到代码仓库.
 *		
*/


#include <stdio.h>
#include <string.h>
#include <xpr/xpr_ups.h>

/*
  @brief 单元测试字符串函数
  @interface: XPR_UPS_GetString, XPR_UPS_SetString
  @author: 李武祥
  @date:  2014/09/05
  @purpose： 测试目标函数在接受任何复杂参数的时候能够正常运行，会不会崩溃
  @scene: 1.空字符串指针，或者接受缓存指针为空
		  2.不存在的key
		  3.超长的key
		  4.接受的缓冲区偏小
		  5.参数有效性检查，如IP地址是否是正确的
		  6.循环测试5000次，查看是否有内存泄漏
  @valgrind:
			valgrind  ./ups
			==8413== Memcheck, a memory error detector
			==8413== Copyright (C) 2002-2013, and GNU GPL'd, by Julian Seward et al.
			==8413== Using Valgrind-3.9.0 and LibVEX; rerun with -h for copyright info
			==8413== Command: ./ups
			==8413== 
			场景[1]测试成功---
			场景[2]测试成功---
			场景[3]测试成功---
			场景[4]测试成功---
			场景[5]测试成功---
			场景[6]测试成功---
			==8413== 
			==8413== HEAP SUMMARY:
			==8413==     in use at exit: 0 bytes in 0 blocks
			==8413==   total heap usage: 15,164 allocs, 15,164 frees, 409,492 bytes allocated
			==8413== 
			==8413== All heap blocks were freed -- no leaks are possible
			==8413== 
			==8413== For counts of detected and suppressed errors, rerun with: -v
			==8413== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)

  @result: passed
  @note: 额外的一些说明....
*/

int test_ups_get_set_string()
{

	char value[128] = {0},value1[8]={0};
	int results[6]={0}, i=0;
	int size = sizeof(value);
	//测试场景1：空指针
	results[0] = XPR_UPS_GetString(NULL, NULL, NULL);

	//测试场景2：key名字输入错误
	results[1] = XPR_UPS_GetString("/system/network/error/ipv4/address", value, &size);

	//测试场景3：超级长的key.
	results[2] = XPR_UPS_SetString("/system/sdsds/fdfdf/ggdgdg/gfgfg/ererer/asdasd/asdw/qweq/s/wq/as/wewq/qwe/qwe/network/eth0/ipv4/addresssdasd/asdasd", "192.168.1.22", strlen("192.168.1.22"));

	//测试场景4：接受的缓冲区偏小
	size = sizeof(value1);
	results[3] = XPR_UPS_GetString("/system/network/error/ipv4/address", value1, &size);

	//测试场景5：参数有效性检查，检查是否是有效的IP地址
	results[4] = XPR_UPS_SetString("/system/network/error/ipv4/address", "192.168.1.22.99", strlen("192.168.1.22.99"));

	//测试场景6：循环测试
	int count = 5000;
	while(--count) {
		size = sizeof(value);
    	results[5] = XPR_UPS_GetString("/system/network/eth0/ipv4/address", value, &size);
		if(results[5]!=0)
			break;
		results[5] = XPR_UPS_SetString("/system/network/eth0/ipv4/address", "192.168.1.22", strlen("192.168.1.22"));
		if(results[5]!=0)
			break;
	}
	for(i=0; i<6; i++) {
		if(results[i]==0 && i!=5)
			printf("场景[%d]测试失败!!!\n",(i+1));
		else
			printf("场景[%d]测试成功---\n",(i+1));
	}
	return 0;
}

int main(int argc, char* argv[])
{
	int result = 0;

	result = XPR_UPS_Init();
	if(result!=0) {
		printf("XPR_UPS_Init failed\n");
		return 0;
	}

	test_ups_get_set_string();

	XPR_UPS_Fini();
	return 0;
}
