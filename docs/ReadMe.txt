================================================================================
  飞秋企业微信消息转发插件 (WeComNotify)
  FeiQ Enterprise WeChat Notification Plugin
================================================================================

版本: 1.0.0
编译环境: Visual C++ 6.0 + FeiQ SDK
适用飞秋: V2.5a 及以上


===== 功能概述 =====

本插件为飞秋局域网聊天软件的第三方插件，实现以下功能:

1. 截获所有收到的飞秋消息 (通过BeforeRecvMsg事件)
2. 将消息转发到企业微信群机器人 (Webhook)
3. 托盘菜单手动开关 (带√标记)
4. 消息转发后飞秋正常显示 (不拦截原消息)
5. 开关状态和Webhook URL持久化保存


===== 工作流程 =====

  局域网飞秋消息
      │
      ▼
  BeforeRecvMsg 事件 ──→ 检查转发开关
      │                    │
      │              ┌─────┴──────┐
      │              │ ON          │ OFF → 不做任何事
      │              ▼             │
      │         检查Webhook URL    │
      │              │             │
      │         ┌────┴────┐       │
      │         │ 已设置   │ 未设置│
      │         ▼         ▼       │
      │    组装消息JSON  跳过     │
      │         │                 │
      │         ▼                 │
      │    WinHTTP POST          │
      │    (UTF-8 JSON)          │
      │         │                 │
      │    ┌────┴────┐           │
      │    │成功│失败│           │
      │    └────┴────┘           │
      │                          │
      ▼                          │
  *pResult = NORMAL              │
  (飞秋正常显示消息) ◄───────────┘


===== 文件清单 =====

  WeComNotify/
  ├── StdAfx.h               预编译头
  ├── StdAfx.cpp
  ├── WeComNotify.h          DLL入口头文件
  ├── WeComNotify.cpp        DLL入口实现 + 导出函数
  ├── WeComNotifyModule.h    插件模块头文件
  ├── WeComNotifyModule.cpp  插件核心实现
  ├── WeComNotify.def        DLL导出定义
  ├── WeComNotify.dsp        VC++6.0项目文件
  ├── WeComNotify.odl        COM类型库定义
  ├── WeComNotify.rc         资源文件
  ├── resource.h             资源ID定义
  └── ReadMe.txt             本说明文件


===== 编译步骤 =====

前提:
  - Visual C++ 6.0 已安装
  - 飞秋SDK (include目录 + tlb目录) 在同级目录
  - FeiQPluginWizard.awx 已安装到VC模板目录

步骤:
  1. 用VC++6.0打开 WeComNotify.dsp
  2. 选择 Release 配置
  3. 菜单: Build → Build WeComNotify.dll
  4. 编译成功后, WeComNotify.dll 在 Release/ 目录下
  5. 将 WeComNotify.dll 复制到飞秋安装目录的 Plugins/ 文件夹
  6. 重启飞秋


===== 使用说明 =====

第一步: 获取企业微信Webhook URL
  1. 在企业微信中创建一个群聊
  2. 群设置 → 群机器人 → 添加机器人 → 新创建一个
  3. 复制Webhook地址, 格式:
     https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key=xxx-xxx-xxx
  4. ⚠ 注意: 不要让此URL泄露!

第二步: 设置Webhook URL (方法一: 修改源码)
  1. 打开 WeComNotifyModule.cpp
  2. 找到 OnLoad() 函数中 m_strWebhookUrl = _T("") 这一行
  3. 将空字符串替换为你的Webhook URL
  4. 重新编译

第二步: 设置Webhook URL (方法二: 通过插件配置)
  1. 插件加载后打开飞秋
  2. 双击任意好友打开发送窗口
  3. 在窗口中底部的插件按钮区域
  4. 找到"企业微信消息转发"插件
  5. 点击设置按钮
  6. 在弹出的对话框中查看当前的Webhook URL
  (注: 本版本通过对话框告知当前配置, 修改URL需通过源码)

第三步: 开启转发
  1. 右键飞秋系统托盘图标
  2. 在菜单中勾选 "转发到企业微信 [已开启]" 
     (显示√表示开启, 无√表示关闭)
  3. 点击后弹出确认提示

第四步: 验证
  1. 用另一台电脑向本机发送一条飞秋消息
  2. 检查企业微信群中是否收到转发消息
  3. 消息格式: 【飞秋】发送者(IP): 消息内容


===== 配置存储 =====

插件配置通过飞秋的 UserCustomConfig 机制持久化:

  键名                         类型    含义
  ─────────────────────────────────────────
  WECOM_FORWARD_ENABLED       LONG    转发开关: 1=开启, 0=关闭
  WECOM_WEBHOOK_URL           STRING  企业微信Webhook URL

配置在飞秋退出时自动保存, 在飞秋启动时自动读取。


===== 企业微信限制说明 =====

  限制项          数值                说明
  ──────────────────────────────────────────
  发送频率        20条/分钟           单个Webhook全局限制
  文本长度        2048字节(UTF-8)    约682个汉字
  图片大小        ≤2MB(base64前)    仅JPG/PNG
  文件大小        ≤20MB             仅企业内群

  本插件已实现:
  - 自动截断过长消息(约550字符)
  - 消息在飞秋正常显示(pResult=NORMAL)
  - 手动开关避免不必要的转发

  建议:
  - 局域网消息频繁时手动关闭转发
  - 离开电脑时再开启转发


===== 注意事项 =====

  1. 必须保持飞秋运行, 插件随飞秋启动加载
  2. 电脑需要能访问外网 (qyapi.weixin.qq.com)
  3. Webhook URL 不要分享到公开平台
  4. 不要在企业微信聊天记录中暴露Webhook URL
  5. VC++6.0编译的DLL需要MFC运行时库 (msvcrt.dll, mfc42.dll)
     这些文件在Windows系统中通常已包含


===== 故障排查 =====

  问题: 插件无法加载, 飞秋Plugins目录下没有反应
  → 检查WeComNotify.dll是否放在正确的Plugins目录
  → 检查DLL依赖: 用depends.exe查看缺少哪些DLL
  → 检查编译配置: 必须是Release模式

  问题: 消息没有转发到企业微信
  → 确认Webhook URL正确
  → 确认转发开关已开启(托盘菜单有√)
  → 检查本机能否访问 qyapi.weixin.qq.com
  → 检查是否超过20条/分钟限制

  问题: 企业微信收到乱码
  → 确认JSON使用UTF-8编码
  → 确认飞秋系统区域设置为中文


===== 技术架构 =====

  语言:     C++ (VC++6.0, MFC)
  接口:     COM (IFQModule, IFQUICommand)
  事件:     BeforeRecvMsg (消息截获)
  网络:     WinHTTP (HTTPS POST)
  编码:     GBK → UTF-16 → UTF-8 (三重转换确保中文兼容)
  配置:     UserCustomConfig (IFQData接口)


===== 许可 =====

本插件基于飞秋公开SDK开发, 仅供学习和内部使用。
请遵守企业微信使用条款。


================================================================================
                          Architecture by WorkBuddy AI
================================================================================
