# Video Recorder System
视频记录仪系统是基于FPGA+Hi3531架构的视频存储设备。<br/>

## 说明
VRS是基于Qt + Plugins的方式开发的音视频记录系统，其底层库采用C89实现，上层应用使用Qt库来实现。


## 使用
使用之前请先确保你的交叉工具链环境已经导出<br/>

<strong>步骤 1:</strong> 获取 VRS
<pre>
git clone https://10.0.2.2/git/rdst/vrs.git
cd vrs
make
make install
</pre>

<strong>步骤 2: </strong> 运行测试程序<br/>
<pre>
 cp apps/plugins/plugin_*.p /plugins/
 cp lib/libxpr.so  /usr/lib/
 cp tests/test /
 ./test
</pre>
