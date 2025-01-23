# Hippy Ohos SDK集成指引

这篇教程，讲述了如何将 Hippy SDK 集成到一个现有的 Ohos 工程。

> 注：以下文档都是假设您已经具备一定的 Ohos 开发经验。

---

## 一、环境准备

- 安装 


## 二、集成 Ohos SDK


## 三、编写SDK接入代码，加载本地或远程的 Hippy 资源包

TODO(hot): doc

源码集成文档

debug inspector true

自定义组件call的返回值，callback()补个例子

Hippy har包产物构建方法：
DevEco Studio里Build Mode选择release或debug
DevEco Studio里选择Hippy模块下文件，比如选择/framework/ohos/src/main/cpp/CMakeLists.txt
DevEco Studio菜单里Build - Make Module 'hippy'
目录/framework/ohos/build/default/outputs/default/里生成hippy.har

Release har包大小说明：
为了方便定位crash，配置了debugSymbol strip为false，构建的har包里so带详细符号，所以size较大，App集成后会自动strip掉符号变小。
比如：har包有3.8M，其中解压后libhippy.so大小为13.9M，strip符号变小后har包大小为1.9M，解压后so大小为6.2M。

