# Ohos Native Renderer

---

Hippy 渲染器对接了 Ohos 的 ArkUI。ArkUI 提供了声明式 [TS API](https://developer.huawei.com/consumer/cn/doc/harmonyos-references-V13/arkui-api-V13) 和过程式 [C API](https://gitee.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-arkui/_ark_u_i___native_module.md)，TS API 开发方便，但性能远不及 C API。
Hippy 内部使用 C API 实现 UI 组件的绘制，同时支持业务通过 TS API 来实现自定义组件，也支持通过 C API 来实现自定义组件。

Hippy 渲染流程：Dom层、渲染器层、View、ArkUINode

图：Dom RenderView ArkUINode

双语言组件带来的不同
表：
- TS 自定义组件
- TS 获取 C 组件信息
- TS 操作 C 组件
- TS 扩展 C 组件
- TS 组件关联 C 组件



## 自定义组件

TS 语言实现自定义组件
跨语言组件嵌套机制

图：嵌套 属性 方法 事件

C 语言实现自定义组件

图：直接调用

## TS 语言和 C 组件

业务获取 Hippy 内部组件关系：方法
业务操作 Hippy 内部组件：方法 事件
业务扩展 Hippy 内部组件：方法
业务关联 Hippy 内部组件：方法

图

## 组件懒加载


## 组件复用


## Text 组件

文本测量

TODO(hot): doc
