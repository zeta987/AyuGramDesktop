# AyuGram

![AyuGram Logo](.github/AyuGram.png) ![AyuChan](.github/AyuChan.png)

[ [English](README.md)  |  繁體中文  |  [Русский](README-RU.md) ]

## 功能特色

- 完整幽靈模式（可彈性調整）
- 訊息歷史紀錄
- 防撤回（Anti-recall）
- 字型自訂
- 直播主模式
- 本機 Telegram Premium
- 翻譯器
- 強制點擊媒體預覽與快速反應（macOS）
- 強化外觀

還有更多功能，請參考我們的[官方文件](https://docs.ayugram.one/desktop/)。

<h3>
  <details>
    <summary>預覽</summary>
    <table>
      <tr>
        <td><img src='.github/demos/demo1.png' width='268' alt='偏好設定'></td>
        <td><img src='.github/demos/demo2.png' width='268' alt='AyuGram 選項'></td>
        <td><img src='.github/demos/demo3.png' width='268' alt='訊息過濾器'></td>
      </tr>
      <tr>
        <td><img src='.github/demos/demo4.png' width='268' alt='外觀'></td>
        <td><img src='.github/demos/demo5.png' width='268' alt='聊天'></td>
      </tr>
    </table>
  </details>
</h3>

## 本 fork 的發布版本（v6.9.4 beta 系列）

本 fork 在 [Releases 頁面](https://github.com/zeta987/AyuGramDesktop/releases)
發布自建的 Windows x64 版本。以下是各 beta 的摘要；詳細內容請見
[CHANGELOG.md](CHANGELOG.md)。

- **beta.8** — 訊息截圖彈窗自適應視窗寬度（預覽等比縮小、儲存／複製按鈕始終可見）；App 內版本文字顯示 beta 序號；埃及聖書體字型 fallback。
- **beta.7** — 本機 Windows Release 封裝流程（含簽署 tag 驗證）；GitHub Actions 降為手動備援；AyuGram 設定中文顯示跟隨 App 語言。
- **beta.6** — 修正 CI 的 NASM 執行檔路徑（絕對路徑加引號）。
- **beta.5** — 固定 libvpx 使用 NASM 3.01。
- **beta.4** — libvpx 改用 NASM 組譯，修正 Windows Release 建置。
- **beta.3** — CI 改用 VS 2022 runner 取得 v143 工具組。
- **beta.2** — 將 6.9.4 整合分支合併進 dev；強化發布 CI 驗證。
- **beta.1** — 整合上游 Telegram 6.9.4 Rich Messages（防撤回保存、訊息截圖預載、轉傳與編輯修正）；AyuGram 設定正體中文化；submodule 與版本同步。

## 下載

### Windows

#### 官方版本

可以從 [Releases 頁面](https://github.com/AyuGram/AyuGramDesktop/releases)或
[Telegram 頻道](https://t.me/AyuGramReleases)下載預先建置的 Windows 執行檔。

#### Winget

```bash
winget install RadolynLabs.AyuGramDesktop
```

#### Scoop

```bash
scoop bucket add extras
scoop install ayugram
```

#### 自行建置

依照本 fork 的[可攜式 Windows 建置指南](docs/building-win-x64.md)，
在任何磁碟建立建置環境。請選擇不含空白的短絕對父路徑，
並讓 repository 保持為其直接子目錄。

### macOS

#### 官方版本

可以從 [Releases 頁面](https://github.com/AyuGram/AyuGramDesktop/releases)下載預先建置的 macOS 套件。

#### Homebrew

```bash
brew install --cask ayugram
```

### Arch Linux

#### 從原始碼安裝（建議）

從 [AUR](https://aur.archlinux.org/packages/ayugram-desktop) 安裝 `ayugram-desktop`。

#### 預先建置的執行檔

從 [AUR](https://aur.archlinux.org/packages/ayugram-desktop-bin) 安裝 `ayugram-desktop-bin`。

注意：這些執行檔並非由我們官方維護。

### NixOS

#### Flake（建議）

從 [ndfined-crp/ayugram-desktop](https://github.com/ndfined-crp/ayugram-desktop) 安裝 `ayugram-desktop`

#### Nixpkgs

從 [nixpkgs](https://search.nixos.org/packages?channel=unstable&show=ayugram-desktop) 安裝 `ayugram-desktop`

### ALT Linux

[Sisyphus](https://packages.altlinux.org/en/sisyphus/srpms/ayugram-desktop/)

### Gentoo Linux

安裝方式請見[這個 repository](https://codeberg.org/OverLessArtem/ayugram-ebuild-gentoo)。

### Void Linux

安裝方式請見[這個 repository](https://codeberg.org/OverLessArtem/ayugram-template-void)。

### EPM

`epm play ayugram`

### Fedora

來自 [RPM Fusion](https://admin.rpmfusion.org/pkgdb/package/free/ayugram-desktop/) repository。

```bash
dnf install ayugram-desktop
```

### 其他 Linux 發行版

Flatpak：https://github.com/0FL01/AyuGramDesktop-flatpak

或依照[官方指南](https://github.com/AyuGram/AyuGramDesktop/blob/dev/docs/building-linux.md)建置。

### Windows 注意事項

請確認 VS Build Tools 已安裝以下元件：

- C++ MFC latest（x86 與 x64）
- C++ ATL latest（x86 與 x64）
- 最新版 Windows 11 SDK

## 贊助

喜歡 **AyuGram** 嗎？歡迎請我們喝杯咖啡！

[這裡是可用的贊助方式。](https://docs.ayugram.one/donate/)

## 致謝

### Telegram 客戶端

- [Telegram Desktop](https://github.com/telegramdesktop/tdesktop)
- [Kotatogram](https://github.com/kotatogram/kotatogram-desktop)
- [64Gram](https://github.com/TDesktop-x64/tdesktop)
- [Forkgram](https://github.com/forkgram/tdesktop)

### 使用的函式庫

- [JSON for Modern C++](https://github.com/nlohmann/json)
- [SQLite](https://github.com/sqlite/sqlite)
- [sqlite_orm](https://github.com/fnc12/sqlite_orm)
- [androidx sources](https://github.com/androidx/androidx)

### 圖示

- [Solar Icon Set](https://www.figma.com/community/file/1166831539721848736)

### 機器人

- [TelegramDB](https://t.me/tgdatabase)：以 ID 查詢使用者名稱（至 2026 年 4 月 2 日免費 inline 模式關閉為止）
