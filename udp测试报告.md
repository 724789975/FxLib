# 服务器UDP性能测试
------------------------

# 1.引言
## 1.1 编写目的
本文档是对`服务器UDP性能`测试所做的说明，为充分利用已有的软硬件资源，配合对各系统应用模块的运行测试方案,查缺补漏完善系统的各项具体功能,保证项目的顺利进行，本测试报告有助于实现以下目标：

* 明确本次性能测试的测试资源;
* 明确本次性能测试的测试内容;
* 明确本次性能测试的测试方法;
* 确本次性能测试的系统性能.

## 1.2 系统概述
### 1.2.1 项目名称
`项目名称`: UDP测试服务器
### 1.2.2 总体目标
暂时未定
### 1.2.3 技术目标
暂时未定  

# 2.测试环境
## 2.1 软硬件环境
### 2.1.1硬件环境
* cpu:Intel(R) Xeon(R) CPU E5-2690 v3 @ 2.60GHz * 8
* menory:16G

### 2.1.2 软件环境
* CentOS Linux release 7.1.1503 (Core)

## 2.2 测试环境约束
* 本次测试结果依据目前被测系统的软/硬件环境
* 本次测试结果依据目前被测系统的程序版本
* 本次测试结果依据目前被测系统的网络环境
* 本次测试结果依据目前被测系统的测试数据量

# 3.测试范围及测试要求
## 3.1 测试
### 3.1.1 测试内容
模拟数据收发过程 测试服务器在500个连接下的状态.(单条消息长度 768~1024 字节)
### 3.1.2测试用例
开启一个`TestServer`进程 并在另一台机器开启500个`TestClient`进程 每个TestClient进程1个连接 不断施压30分钟(主要测试收发包稳定性以及`IO`的吞吐效率)

### 3.1.3 测试通过标准
* 系统在链接500时，系统运行稳定，消息能正常收发，系统CPU仍然有空闲

# 4.测试结果
## 4.1 网络流量记录

* localhost.example:dnp&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;=> 210.73.214.213&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;220Mb   221Mb   223Mb
* &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<=&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;211Mb   212Mb   212Mb

## 4.2 CPU状态记录
* VIRT&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;RES&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;SHR&nbsp;&nbsp;&nbsp;&nbsp;%CPU&nbsp;&nbsp;&nbsp;&nbsp;%MEM
* 3387800&nbsp;&nbsp;&nbsp;472908&nbsp;&nbsp;1660&nbsp;&nbsp;&nbsp;&nbsp;92.0&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;2.9

## 4.3 消息采样
* 500个连接时收到54600条数据(单条消息约896字节 每秒约47M有效信息)
* 发出消息基本与收到消息持平

## 4.4 测试结果分析
* 正常情况下流量不会有这么大 满负荷运行`TestServer` cpu 92% 与预想相差不远 真正生产环境 流量在应10M/s左右

# 5.结论
* 在连接数接近上限时 单个cpu满负荷 但生产环境的流量应是测试场景的1/4(可通过减少客户端给服务器发送的单条消息长度来测试) 

# 6.测试人员
* 时间 2018/1/6
* 人员 dengxiaobo
* 邮箱 [724789975@qq.com](724789975@qq.com)

