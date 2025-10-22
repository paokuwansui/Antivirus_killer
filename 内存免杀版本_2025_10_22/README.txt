1、将shellcode放入encode.cpp中的shellcode行
2、编译encode并执行
g++ encode.cpp -o encode.exe
.\encode.exe
3、利用vstudio编译loader.cpp为dll，名称为ProgramUpdate.dll
4、编译loader_main.cpp为exe
g++ -o ProgramUpdate.exe loader_main.cpp -fno-stack-protector -fvisibility=hidden -Wl,--dynamicbase -Wl,--nxcompat
5、把dll、exe和payload.txt放在同一文件夹下运行