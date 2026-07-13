# AyuGram

![AyuGram Logo](.github/AyuGram.png) ![AyuChan](.github/AyuChan.png)

[ English  |  [正體中文](README.zh-TW.md)  |  [Русский](README-RU.md) ]

## Features

- Full ghost mode (flexible)
- Messages history
- Anti-recall
- Font customization
- Streamer mode
- Local Telegram Premium
- Translator
- Media preview & quick reaction on force click (macOS)
- Enhanced appearance

And many more. Check out our [Documentation](https://docs.ayugram.one/desktop/).

<h3>
  <details>
    <summary>Preview</summary>
    <table>
      <tr>
        <td><img src='.github/demos/demo1.png' width='268' alt='Preferences'></td>
        <td><img src='.github/demos/demo2.png' width='268' alt='AyuGram Options'></td>
        <td><img src='.github/demos/demo3.png' width='268' alt='Message Filters'></td>
      </tr>
      <tr>
        <td><img src='.github/demos/demo4.png' width='268' alt='Appearance'></td>
        <td><img src='.github/demos/demo5.png' width='268' alt='Chats'></td>
      </tr>
    </table>
  </details>
</h3>

## Fork releases (v6.9.4 beta series)

This fork publishes its own Windows x64 releases on the
[Releases page](https://github.com/zeta987/AyuGramDesktop/releases).
The v6.9.4 beta series is based on upstream commit
[telegramdesktop/tdesktop@ed73b49d0110](https://github.com/telegramdesktop/tdesktop/commit/ed73b49d0110a8e949e7fe06a8a90bd7e7b421a8).
Summary of each beta; see [CHANGELOG.md](CHANGELOG.md) for details.

- **beta.8** — Message Shot box adapts to the window width (proportional preview downscaling, Save/Copy always visible); beta serial shown in the in-app version text; Egyptian Hieroglyphs font fallback.
- **beta.7** — Local Windows release packaging flow with signed-tag validation; GitHub Actions demoted to manual fallback; Chinese Ayu settings follow the active app language.
- **beta.6** — Fixed the NASM executable path in CI (absolute, quoted).
- **beta.5** — Pinned NASM 3.01 for libvpx.
- **beta.4** — Built libvpx with NASM to fix the Windows release build.
- **beta.3** — Switched CI to the VS 2022 runner for the v143 toolset.
- **beta.2** — Merged 6.9.4 integrations into dev; hardened release CI validation.
- **beta.1** — Merged upstream Telegram 6.9.4 Rich Messages into AyuGram (anti-recall persistence, Message Shot preloading, forward/edit fixes); Traditional Chinese localization for Ayu settings; fork submodules and version sync.

## Downloads

### Windows

#### Official

You can download prebuilt Windows binary from [Releases tab](https://github.com/AyuGram/AyuGramDesktop/releases) or from
the [Telegram channel](https://t.me/AyuGramReleases).

#### Winget

```bash
winget install RadolynLabs.AyuGramDesktop
```

#### Scoop

```bash
scoop bucket add extras
scoop install ayugram
```

#### Self-built

Follow this fork's [portable Windows build guide](docs/building-win-x64.md) to
create a build environment on any drive. Select a short absolute parent path
without spaces, and keep the repository as its direct child.

### macOS

#### Official

You can download prebuilt macOS package from [Releases tab](https://github.com/AyuGram/AyuGramDesktop/releases).

#### Homebrew

```bash
brew install --cask ayugram
```

### Arch Linux

#### From source (recommended)

Install `ayugram-desktop` from [AUR](https://aur.archlinux.org/packages/ayugram-desktop).

#### Prebuilt binaries

Install `ayugram-desktop-bin` from [AUR](https://aur.archlinux.org/packages/ayugram-desktop-bin).

Note: these binaries aren't officially maintained by us.

### NixOS

#### Flake (recommended)

Install `ayugram-desktop` from [ndfined-crp/ayugram-desktop](https://github.com/ndfined-crp/ayugram-desktop)

#### Nixpkgs

Install `ayugram-desktop` from [nixpkgs](https://search.nixos.org/packages?channel=unstable&show=ayugram-desktop)

### ALT Linux

[Sisyphus](https://packages.altlinux.org/en/sisyphus/srpms/ayugram-desktop/)

### Gentoo Linux

See [this repository](https://codeberg.org/OverLessArtem/ayugram-ebuild-gentoo) for installation manual.

### Void Linux
See [this repository](https://codeberg.org/OverLessArtem/ayugram-template-void) for installation manual.

### EPM

`epm play ayugram`

### Fedora

From [RPM Fusion](https://admin.rpmfusion.org/pkgdb/package/free/ayugram-desktop/) repository.

```bash
dnf install ayugram-desktop
```

### Any other Linux distro

Flatpak: https://github.com/0FL01/AyuGramDesktop-flatpak

Or follow the [official guide](https://github.com/AyuGram/AyuGramDesktop/blob/dev/docs/building-linux.md).

### Remarks for Windows

Make sure you have these components installed with VS Build Tools:

- C++ MFC latest (x86 & x64)
- C++ ATL latest (x86 & x64)
- latest Windows 11 SDK

## Donation

Enjoy using **AyuGram**? Consider sending us a tip!

[Here's available methods.](https://docs.ayugram.one/donate/)

## Credits

### Telegram clients

- [Telegram Desktop](https://github.com/telegramdesktop/tdesktop)
- [Kotatogram](https://github.com/kotatogram/kotatogram-desktop)
- [64Gram](https://github.com/TDesktop-x64/tdesktop)
- [Forkgram](https://github.com/forkgram/tdesktop)

### Libraries used

- [JSON for Modern C++](https://github.com/nlohmann/json)
- [SQLite](https://github.com/sqlite/sqlite)
- [sqlite_orm](https://github.com/fnc12/sqlite_orm)
- [androidx sources](https://github.com/androidx/androidx)

### Icons

- [Solar Icon Set](https://www.figma.com/community/file/1166831539721848736)

### Bots

- [TelegramDB](https://t.me/tgdatabase) for username lookup by ID (until closing free inline mode at 2 April 2026)
