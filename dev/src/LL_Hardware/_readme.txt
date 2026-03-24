LL_Hardware_cfg(template).h
	模板文件，仅供参考，不可直接使用。
	使用 LL_Hardware 模块的用户，需要参考此模板内容，实现一个 LL_Hardware_cfg.h。
	LL_Hardware_cfg.h 中具体的配置值，取决于用户自己的硬件设计。

LL_Xxxx.h
	硬件模块“Xxxx”的对外统一接口文件，如：Power、Clock。
LL_Xxxx.c
	硬件模块“Xxxx”的实现文件。
	与厂商/硬件平台/型号等无关的一些纯逻辑实现。
LL_Xxxx_Yyyy.c
	硬件模块“Xxxx”的实现文件。
	“Yyyy”表示某厂商/硬件平台/型号等，如：LIS3DH、nRF51。