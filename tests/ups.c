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
	results[4] = XPR_UPS_SetString("/system/network/error/ipv4/address", "192.168.1.22.989", strlen("192.168.1.22.989"));

	//测试场景6：循环测试
	int count = 5000;
	while(--count) {
		size = sizeof(value);
    	results[5] = XPR_UPS_GetString("/system/network/eth0/ipv4/address", value, &size);
		if(results[5]!=0)
			break;
		results[5] = XPR_UPS_SetString("/system/network/eth0/ipv4/dns1", "192.168.0.4", strlen("192.168.0.4"));
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

void test_ups_set_sys_info()
{
    char* hardware = "v4.0.0";
    char* id = "sn-00000000";
    char* location = "USA";
    char* manufacturer = "EA";
    char* model = "00-000-000";
    char* name = "Best";
    char* onvif = "3.3";
    char* software = "v3.0.0";
    char* uri = "http://192.168.1.21/onvif/device_service";
    char* uuid = "BD4780B6-3136-4117-B365-000003BF3ACA";
    char* mac = "66:55:44:33:22:11";
    char* ip = "192.168.1.21";
    char* netmask = "255.255.0.0";
    char* dns1 = "192.168.0.3";
    char* dns2 = "192.168.0.4";
    char* ethname = "eth0";
    char* internal_model = "HD-25440-20D112000";
    char* serveradress = "0.japan.pool.ntp.org";

    char value[128] = {0}; 
	int size = sizeof(value);
    if(XPR_UPS_SetString("/system/information/uri", uri, strlen(uri)) != 0)
        printf("set info uri error!\n");
    else 
        printf("set info uri ok!\n");
    
    if(XPR_UPS_SetString("/system/information/hardware", hardware, strlen(hardware)) != 0)
        printf("set info hardware error!\n");
    else
        printf("set info hardware ok!\n");

    if(XPR_UPS_SetString("/system/information/id", id, strlen(id)) != 0)
        printf("set info id error!\n");
    else
        printf("set info hardware ok!\n");

    if(XPR_UPS_SetString("/system/information/location", location, strlen(location)) != 0)
        printf("set info location error!\n");
    else
        printf("set info location ok!\n");

    if(XPR_UPS_SetString("/system/information/manufacturer", manufacturer, strlen(manufacturer)) != 0)
        printf("set info manufacturer error!\n");
    else
        printf("set info manufacturer ok!\n");

    if(XPR_UPS_SetString("/system/information/model", model, strlen(model)) != 0)
        printf("set info model error!\n");
    else
        printf("set info model ok!\n");

    if(XPR_UPS_SetString("/system/information/name", name, strlen(name)) != 0)
        printf("set info name error!\n");
    else
        printf("set info name ok!\n");

    if(XPR_UPS_SetString("/system/information/onvif", onvif, strlen(onvif)) != 0)
        printf("set info onvif error!\n");
    else
        printf("set info onvif ok!\n");

    if(XPR_UPS_SetString("/system/information/software", software, strlen(software)) != 0)
        printf("set info software error!\n");
    else
        printf("set info software ok!\n");

    if(XPR_UPS_SetString("/system/information/uuid", uuid, strlen(uuid)) != 0)
        printf("set info uuid error!\n");
    else
        printf("set info uuid ok!\n");

    if(XPR_UPS_SetString("/system/network/eth0/ipv4/address", ip, strlen(ip)) != 0)
        printf("set net address error!\n");
    else
        printf("set net address ok!\n");

    if(XPR_UPS_SetString("/system/network/eth0/ipv4/netmask", netmask, strlen(netmask)) != 0)
        printf("set net netmask error!\n");
    else
        printf("set net netmask ok!\n");

    if(XPR_UPS_SetString("/system/network/eth0/name", ethname, strlen(ethname)) != 0)
        printf("set net eth0 name error!\n");
    else
        printf("set net eth0 name ok!\n");

    if(XPR_UPS_SetString("/system/information/internal-model", internal_model, strlen(internal_model)) != 0)
        printf("set info internal-model error!\n");
    else
        printf("set info internal-model ok!\n");

    if(XPR_UPS_SetString("/system/network/ntp/serveradress", serveradress, strlen(serveradress)) != 0)
        printf("set ntp serveradress error!\n");
    else
        printf("set ntp serveradress ok!\n");

   if(XPR_UPS_SetString("/system/network/eth0/ipv4/dns1", dns1, strlen(dns1)) != 0)
        printf("set net dns1 error!\n");
    else
        printf("set net dns1 ok!\n");

   if(XPR_UPS_SetString("/system/network/eth0/ipv4/dns2", dns2, strlen(dns2)) != 0)
        printf("set net dns2 error!\n");
    else
        printf("set net dns2 ok!\n");
}

void test_ups_get_sys_info()
{

    char value[128] = {0}; 
	int size = sizeof(value);

    if(XPR_UPS_GetString("/system/information/uri", value, &size) != 0)
        printf("get info uri error!\n");
    else 
        printf("uri = %s\n", value);
    
    size = sizeof(value);
    if(XPR_UPS_GetString("/system/information/hardware", value, &size) != 0)
        printf("get info hardware error!\n");
    else
        printf("hardware = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/information/id", value, &size) != 0)
        printf("get info id error!\n");
    else
        printf("info id = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/information/location", value, &size) != 0)
        printf("get info location error!\n");
    else
        printf("location = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/information/manufacturer", value, &size) != 0)
        printf("get info manufacturer error!\n");
    else
        printf("manufacturer = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/information/model", value, &size) != 0)
        printf("get info model error!\n");
    else
        printf("model = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/information/name", value, &size) != 0)
        printf("get info name error!\n");
    else
        printf("information name = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/information/onvif", value, &size) != 0)
        printf("get info onvif error!\n");
    else
        printf("onvif = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/information/software", value, &size) != 0)
        printf("get info software error!\n");
    else
        printf("software = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/information/uuid", value, &size) != 0)
        printf("get info uuid error!\n");
    else
        printf("uuid = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/network/eth0/ipv4/address", value, &size) != 0)
        printf("get net address error!\n");
    else
        printf("address = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/network/eth0/ipv4/netmask", value, &size) != 0)
        printf("get net netmask error!\n");
    else
        printf("netmask = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/network/eth0/name", value, &size) != 0)
        printf("get net eth0 name error!\n");
    else
        printf("ethernet name = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/information/internal-model", value, &size) != 0)
        printf("get info internal-model error!\n");
    else
        printf("internal-model = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/network/ntp/serveradress", value, &size) != 0)
        printf("get ntp serveradress error!\n");
    else
        printf("serveradress = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/network/eth0/ipv4/dns1", value, &size) != 0)
        printf("get net dns1 error!\n");
    else
        printf("dns1 = %s\n", value);

    size = sizeof(value);
    if(XPR_UPS_GetString("/system/network/eth0/ipv4/dns2", value, &size) != 0)
        printf("get net dns2 error!\n");
    else
        printf("dns2 = %s\n", value);

    int dhcp =0;
    if(XPR_UPS_GetBoolean("/system/network/eth0/ipv4/dhcp", &dhcp) != 0)
        printf("get net dhcp error!\n");
    else
        printf("dhcp = %d\n", dhcp);
}

int test_ups_get_set_string2()
{
	char value[128]={0};
	int size = sizeof(value);

	int err = 0;//XPR_UPS_SetStringVK(("192.168.1.18"), strlen("192.168.1.18"), "/system/network/eth0/ipv4/%s", "address");
	printf("err = %d\n",err);
   // err = XPR_UPS_GetString("/system/network/eth0/ipv4/address", value, &size);
	//printf("err = %d, value=%s\n",err, value);

}

int main(int argc, char* argv[])
{
	int result = 0;

	result = XPR_UPS_Init();
	if(result!=0) {
		printf("XPR_UPS_Init failed err = %d\n", result);
		return 0;
	}

	//test_ups_get_set_string();

   	test_ups_get_sys_info();

	XPR_UPS_Fini();
	return 0;
}
