isp_rt_sdk

/******************目录*******************/
include:		编译所需头文件
libautoscene:	用于修改部分画面参数和添加额外的画面逻辑
libsns:		用于修改sensor相关与画面参数初始化


/****************Makefile*****************/
make: 	make

clean: 	make clean

/*****************开发说明****************/
1.进入libsns，运行make clean;make;编译Makefile，编译后在_libs内生成libsns_XAx.a，并替换打包环境内的libsns_XAx.a

2.libsns画面相关的参数在/libsns/snsX20/xxx__cmos.c内	(xxx为sensor名称)

3.进入libautoscene，运行make clean;make;编译Makefile，编译后在_libs内生成libautoscene_fv.a，并替换打包环境内的libautoscene_fv.a

4.libautoscene画面相关的参数在/libautoscene/src/sensor/xxx.c内	(xxx为sensor名称)