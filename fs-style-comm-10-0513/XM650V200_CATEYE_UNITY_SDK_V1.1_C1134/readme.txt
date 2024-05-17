cat_eye_unity(rt version)

/******************目录*******************/
pack:		升级包打包目录
rt-thread-4.1.0:	rtos源码
serial_protol_app:	串口通讯协议相关代码（可以根据通讯协议需求修改这里的代码）
toolchain:		编译工具链
pc_tool:		pc串口通讯工具
platform_libs：	不同芯片平台库文件

/****************Makefile*****************/
pack: 	make all

clean: 	make clean

/*****************开发说明****************/
1.开发者首先解压编译工具链到对应目录

2.linux上需要支持scons相关命令，可以网上自行下载安装

3. 合一小升级包(SDK打包后生成的pack/rt_thread/rtthread.bin)，以及配置文件（pack/ConFigFile/ProductDefinition），
    都可以通过pc串口通讯工具单独进行升级

4. 开发者只需根据需求修改serial_protol_app目录下内容

5. 修改完成后进行打包，在目录pack/rt_thread/目录下会生成rtthread.bin文件，可使用pc串口通讯工具进行升级这一部分
   或者升级完整的升级程序（此目录下生成的target目录下的upall_xxx.bin，可用pc串口工具升级或者正常烧片）

6. pc串口通讯工具的详细使用有单独的文档进行说明（请严格按照说明文档使用）

