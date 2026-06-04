# WeComNotify — FeiQ to WeCom Message Forwarding Plugin

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Win32%20(x86)-lightgrey.svg)]()
[![Build](https://img.shields.io/badge/build-VS2022%20%7C%20VC6-green.svg)]()

**飞秋(FeiQ 2013)企业微信消息转发插件** — 将局域网飞秋收到的消息实时转发到企业微信群机器人，让你离开电脑时不错过任何消息。

> 📖 **纯中文文档请参阅 [README_CN.md](README_CN.md)** — 飞秋用户多为中文用户，建议优先阅读中文版。
> FeiQ (飞秋) is a popular LAN instant messaging software in China. This plugin forwards FeiQ messages to WeCom (企业微信) group chat bots in real time.

---

## ✨ Features

- **Message Forwarding** — Text messages (Chinese, English, numbers) forwarded to WeCom in real time
- **Screenshot Notification** — Detects incoming screenshots/pictures and sends a clean notification to WeCom (instead of garbled binary data)
- **Manual Toggle** — System tray menu switch to turn forwarding on/off anytime
- **Configuration Dialog** — In-app dialog to set Webhook URL and @mention user IDs
- **Persistent Config** — All settings saved automatically via FeiQ's UserCustomConfig
- **Non-blocking** — Messages still display normally in FeiQ after forwarding
- **Auto-truncation** — Long messages (>550 chars) automatically truncated to fit WeCom's 2048-byte limit

---

## 🚀 Quick Start

### Prerequisites

- [FeiQ 2013](http://www.feiq18.com/) installed (V2.5a or later)
- A WeCom (企业微信) group chat with a bot webhook
- Your computer can access `qyapi.weixin.qq.com` (internet required)

### Installation

1. **Download** the latest `WeComNotify.dll` from [Releases](../../releases)
2. Copy `WeComNotify.dll` to FeiQ's `Plugins\` directory (e.g., `C:\Program Files (x86)\飞秋2013\Plugins\`)
3. Restart FeiQ
4. Right-click the FeiQ tray icon → check "转发到企业微信 [已开启]"
5. Right-click again → "配置企业微信..." to set your Webhook URL
6. Test: send a message from another PC to yours, check if WeCom receives it

### Getting a WeCom Webhook URL

1. Open WeCom (企业微信)
2. Go to any group chat → `...` → 群机器人 → Add bot
3. Copy the Webhook URL (format: `https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key=xxxxxxxxxx`)
4. ⚠️ Keep this URL private — anyone with it can post to your group!

---

## 📖 Usage

### Normal Operation

1. Ensure FeiQ 2013 is running and the plugin is loaded (you'll see "转发到企业微信" in the tray menu)
2. Set your WeCom webhook URL via **右键飞秋托盘图标 → 配置企业微信...**
3. Enable forwarding: **右键飞秋托盘图标 → 转发到企业微信 [已开启]**
4. When another FeiQ user sends you a message:
   - **Text messages** → forwarded to WeCom as-is (Chinese/English/Numbers supported)
   - **Screenshots/Pictures** → WeCom receives a notification like: *"[Screenshot/Picture] Sent a screenshot or picture. Check FeiQ to view."*
5. To stop forwarding, uncheck the toggle in the tray menu

### @Mention Users

In the configuration dialog, enter comma-separated WeCom user IDs (e.g., `zhangsan,lisi`) under "@用户ID". The WeCom bot will @mention these users in each forwarded message.

### Switching On/Off

The tray menu toggle persists across FeiQ restarts. Your Webhook URL and mention list are saved in FeiQ's user config file.

---

## 🔧 Building from Source

### Option A: Visual Studio 2022 (Recommended)

1. Install [Visual Studio 2022 Community](https://visualstudio.microsoft.com/) with:
   - Workload: **Desktop development with C++**
   - Individual component: **C++ MFC for latest v143 build tools (x86 & x64)**
2. Place the FeiQ SDK (`include/` and `tlb/` directories) in the parent directory:
   ```
   your-workspace/
   ├── WeComNotify/      ← this repo
   ├── include/          ← FeiQ SDK headers (from feiq18.com)
   └── tlb/              ← FeiQ.tlb type library (from feiq18.com)
   ```
3. Open `src/WeComNotify.sln`
4. Select **Release** | **Win32** configuration
5. Build → Build Solution (Ctrl+Shift+B)
6. Output: `src/Release/WeComNotify.dll`

### Option B: Command Line (MSBuild)

```bat
cd src
msbuild WeComNotify.vcxproj /p:Configuration=Release /p:Platform=Win32
```

### Option C: Visual C++ 6.0 (Legacy)

Open `WeComNotify.dsp`, select Release configuration, then Build.

---

## 📁 Project Structure

```
WeComNotify/
├── src/
│   ├── WeComNotify.sln              # VS2022 solution
│   ├── WeComNotify.vcxproj          # VS2022 project file
│   ├── WeComNotify.dsp              # VC++6.0 project (legacy)
│   ├── WeComNotify.h / .cpp         # DLL entry point
│   ├── WeComNotify.def              # DLL exports
│   ├── WeComNotify.rc / resource.h  # Resources (dialog, menu)
│   ├── WeComNotifyModule.h / .cpp   # Core plugin logic
│   ├── StdAfx.h / .cpp              # Precompiled header
│   └── build-release.bat            # Quick build script
├── docs/
│   ├── BUILD.md                     # Detailed build guide (zh-CN)
│   └── ReadMe.txt                   # Original documentation (zh-CN)
├── LICENSE
└── README.md
```

---

## 🛠 Technical Architecture

| Layer | Technology |
|-------|-----------|
| Language | C++ (MFC, COM) |
| Character Set | MBCS (GBK internal, UTF-8 for WeCom API) |
| Network | WinHTTP (HTTPS POST to WeCom webhook) |
| Encoding Pipeline | GBK → UTF-16 → UTF-8 (triple conversion for full CJK compatibility) |
| Config Storage | FeiQ UserCustomConfig (IFQData interface) |
| Events | `BeforeRecvMsg` (message interception) |
| Binary Detection | Multi-tier: FeiQ protocol patterns → image headers → readability → non-printable ratio |

### Binary/Image Detection Pipeline

FeiQ encodes screenshots using its own ASCII protocol framing (not raw JPEG/PNG bytes), so the plugin uses a 5-tier detection strategy:

1. **P0** — FeiQ protocol frame delimiters (`~`, `/~`, `/-`, `/#`)
2. **P1** — Standard image headers (JPEG `FF D8 FF`, PNG `89 50 4E 47`, GIF, BMP, RIFF, ICO)
3. **P2** — Length > 10KB
4. **P3** — Text readability (no letters/CJK/spaces in first 64 bytes)
5. **P4** — Non-printable character ratio > 30% (GBK-aware)

---

## ⚠️ WeCom Webhook Limitations

| Limit | Value |
|-------|-------|
| Rate | 20 messages/minute per webhook |
| Text length | 2048 bytes (UTF-8), ~682 Chinese characters |
| Image (via API) | ≤ 2MB, JPG/PNG only |
| File (via API) | ≤ 20MB, enterprise-internal groups only |

> **Note**: This plugin does NOT forward actual image files to WeCom. When a screenshot is received, it sends a text notification like: *"[Screenshot/Picture] Sent a screenshot or picture. Check FeiQ to view."* The WeCom webhook text-message API does not support inline images.

---

## 🙏 Credits

This plugin is built upon the **FeiQ (飞秋) SDK** publicly provided by the original FeiQ author.

- FeiQ official website: [http://www.feiq18.com/](http://www.feiq18.com/)
- FeiQ SDK & development documentation: [http://www.feiq18.com/config_nav.php?id=36](http://www.feiq18.com/config_nav.php?id=36)

**Sincere thanks to the FeiQ original author** for making the SDK and development documentation available, enabling third-party plugin development like this project.

---

## 🤖 AI Generation Disclaimer

**This project's code was generated and iteratively refined with the assistance of [WorkBuddy](https://www.codebuddy.cn/) AI.** All code has been manually tested and verified to work correctly with FeiQ 2013. Key testing results:

| Test Item | Status |
|-----------|--------|
| Chinese text messages | ✅ Passed |
| English text messages | ✅ Passed |
| Mixed Chinese/English/Numbers | ✅ Passed |
| Screenshot notification | ✅ Passed |
| Tray menu toggle | ✅ Passed |
| Configuration dialog | ✅ Passed |
| Webhook message delivery | ✅ Passed |
| File transfer notification | ❌ Not supported (FeiQ uses separate event path) |

While this plugin has been tested and confirmed working, please be aware that:

- AI-generated code may contain unforeseen edge cases or bugs
- Test thoroughly in your own environment before relying on it for critical use
- The author(s) make no guarantees — see the MIT License for details
- FeiQ is a third-party software; compatibility may vary across FeiQ versions

---

## 📄 License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

The FeiQ SDK headers and type libraries (`include/`, `tlb/`) are the property of their respective copyright holders and are NOT included in this repository. Please download them from the official FeiQ website.

---

*Built with AI assistance via WorkBuddy, tested with ❤️ by [fengtai227-hash](https://github.com/fengtai227-hash) — 2026*
