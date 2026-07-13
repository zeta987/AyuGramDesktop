# Agent Guide for Telegram Desktop

This guide defines repository-wide instructions for coding agents working with
the Telegram Desktop codebase and the zeta987 AyuGram fork.

## AyuGram fork priorities

The active checkout is a native Windows repository, but its drive and parent
directory are not fixed. Resolve the current repository root before using any
path, and call it `<RepoRoot>`. Its direct parent is `<BuildRoot>`. Read
`docs/building-win-x64.md` before preparing a new machine and read
`docs/ayugram-local-development.md` before changing branches, submodules,
build configuration, tags, or release assets.

- `dev` is the default integration and release-source branch.
- Work on `feat/*` or `fix/*`, then produce a Debug EXE for developer
  confirmation. Build Release only after that confirmation and after the
  signed change is integrated into `dev`.
- Do not start a build unless the user asks for one or the work has reached
  the documented validation stage.
- `origin` (`zeta987/AyuGramDesktop`) is the only writable superproject remote.
  Treat `upstream`, `telegram`, and every other upstream as read-only.
- Never push or create a Pull Request in an upstream repository. To modify an
  external repo or submodule, create or reuse a `zeta987` fork first.
- Create a Pull Request in a `zeta987` repository only when the user explicitly
  asks. State in its description that it was AI generated.
- Issues and PRDs belong only to `zeta987/AyuGramDesktop`; see
  `docs/agents/issue-tracker.md`.
- Engineering skills read their tracker, triage, and domain configuration from
  `docs/agents/`.

## Alternate WSL checkouts

The following rules apply only when a different checkout is opened through the
Windows UNC path
`\\wsl.localhost\<distro>\home\<linux-user>\Telegram\tdesktop`, whose real
path is `/home/<linux-user>/Telegram/tdesktop`. Treat that checkout as
WSL/Linux. They do not override the native Windows procedure for the active
checkout.

- Prefer running repository-aware commands through WSL:

```powershell
wsl.exe -d <distro> --cd /home/<linux-user>/Telegram/tdesktop -- <command>
```

- PowerShell can read and write files through the UNC path, but native Windows tools may see different ownership, path, executable, or line-ending behavior than Linux tools.
- Git from PowerShell over `\\wsl.localhost\...` can fail with `detected dubious ownership`. Use WSL Git instead. Do not change global Git `safe.directory` settings unless the user explicitly asks for that.
- Keep path styles matched to the shell. Use
  `/home/<linux-user>/Telegram/tdesktop/...` with WSL commands, and quoted
  `\\wsl.localhost\<distro>\home\<linux-user>\Telegram\tdesktop\...` paths
  with native Windows commands. Avoid passing UNC paths to Linux tools or
  Linux paths to native Windows tools unless the tool explicitly supports
  them.
- If a command behaves strangely from the PowerShell UNC working directory,
  retry the same command through
  `wsl.exe -d <distro> --cd /home/<linux-user>/Telegram/tdesktop -- ...`
  before concluding the repository or command is broken.
- Recursive searches and repo inspection are usually faster and more faithful
  through WSL, for example
  `wsl.exe -d <distro> --cd /home/<linux-user>/Telegram/tdesktop -- rg ...`.
- Do not assume the WSL host has the build toolchain installed directly. In this setup, WSL may not have `cmake`, while Windows may have `cmake`, and the configured `out/` tree may still target the Linux Docker toolchain. Do not run native Windows `cmake --build out` against a Linux/Docker build tree.
- For WSL/Linux builds, use the Docker build entry point from the repository root: `Telegram/build/docker/centos_env/build_debug.sh`. The Docker daemon must be reachable from WSL; checking `docker info` is fine, but do not start a build unless the user asked for one.
- Existing build outputs may be Linux binaries, for example `out/Debug/Telegram` as an ELF executable, not `Telegram.exe`. Verify the build tree before assuming which platform produced it.
- Be careful with text file line endings. In a WSL/Linux checkout, files should remain LF-only unless the file already uses another convention. CRLF finishing applies only to native, non-WSL Windows runs/checkouts. Do not let PowerShell or Windows tools silently rewrite WSL files to CRLF. If a file becomes mixed, normalize it back to the convention appropriate for the current checkout, without adding a UTF-8 BOM.
- When using the local `task-think` skill from this WSL checkout, keep `.ai/...` artifacts and edited project text files LF-only. Treat the skill's Windows text-normalization phase as not applicable to WSL, except to record that line endings were checked and kept LF/no-BOM. Run CRLF normalization for `task-think` only in a native, non-WSL Windows checkout.

## Build System Structure

The native Windows preparation scripts derive the build root from the parent
of the repository. Use this portable layout on any drive:

```text
<BuildRoot>\                      # Parent directory, not the superproject
<BuildRoot>\<RepoFolder>\         # <RepoRoot>, the superproject repository
<BuildRoot>\Libraries\win64\      # Windows x64 dependencies
<BuildRoot>\ThirdParty\           # NuGet, Python, MSYS2, and build tools
<BuildRoot>\release\              # Local packaged release assets
```

`<RepoRoot>` must be a direct child of `<BuildRoot>` because
`Telegram/build/prepare/prepare.py` walks from the script to that parent.
The configured build tree is `<RepoRoot>\out`. Inspect its `CMakeCache.txt`
instead of assuming compiler or Qt versions from another machine.

The currently verified workstation happens to use `D:\TBuild\tdesktop`, but
that path is only a local snapshot. Never create or select it automatically on
another machine.

## Build Configuration

### Native Windows Debug build

After a feature or fix reaches its validation stage, build Debug first:

```powershell
cmake --build <RepoRoot>\out --config Debug --target Telegram --parallel 4
```

The executable is `<RepoRoot>\out\Debug\AyuGram.exe`. Give this build to the
developer for confirmation before integrating and releasing the change.

### Native Windows Release build

Release is allowed only after the developer confirms Debug and the signed
change is integrated into a clean `dev`:

```powershell
cmake --build <RepoRoot>\out --config Release --target Telegram --parallel 4
```

The executable is `<RepoRoot>\out\Release\AyuGram.exe`. Package and publish it
only through the procedure in
`docs/ayugram-local-development.md`.

### WSL build

For a separate WSL/Linux checkout, use its Docker entry point only when the
user requests a Linux build:

```bash
Telegram/build/docker/centos_env/build_debug.sh
```

## Platform-Specific Requirements

### Windows
- A fresh environment follows `docs/building-win-x64.md`: Visual Studio 2022,
  MSVC v143, Windows SDK 10.0.26100.0, x64, and the Qt version selected by
  `Telegram/build/qt_version.py`.
- An existing `out` tree may use a separately validated compiler snapshot. The
  current `D:\TBuild\tdesktop\out` uses Visual Studio 18 2026 and Qt 6.11.1.
  Re-check `out/CMakeCache.txt` after every configure instead of copying those
  values into portable scripts.
- Must run from appropriate Native Tools Command Prompt:
  - "x64 Native Tools Command Prompt" for `win64`
  - "x86 Native Tools Command Prompt" for `win`
  - "ARM64 Native Tools Command Prompt" for `winarm`
- Native Windows x64 dependencies are under
  `<BuildRoot>\Libraries\win64`.

### macOS
- Requires Xcode
- Dependencies: `../Libraries/local/Qt-*`
- Set `QT` environment variable: `export QT=6.8`

### Linux
- Build dependencies in `../Libraries`
- Set `QT` environment variable if needed

## Key Files

- **`Telegram/build/version`** - Version information
- **`out/`** - Build output directory

## Troubleshooting

### "Libraries not found"
Ensure `<RepoRoot>` is directly under `<BuildRoot>` and that
`<BuildRoot>\Libraries\win64` exists. If the machine has not been prepared,
follow `docs/building-win-x64.md`; do not copy dependency folders from a
different toolchain snapshot.

### Build fails with "wrong command prompt"
On Windows, use the correct Visual Studio Native Tools Command Prompt matching your target (x64/x86/ARM64).

### Build fails with PDB or EXE access errors

**Critical: do not retry the build until the locked process is closed.**

If the build fails with any of these errors:
- `fatal error C1041: cannot open program database`
- `cannot open output file 'AyuGram.exe'`
- `LNK1104: cannot open file`
- Any "access denied" or "file in use" error.

Stop the build. These errors usually mean that `AyuGram.exe`, a debugger, or
another build process still owns the output or PDB file.

1. Record the exact locked path and error.
2. Do not delete the output or retry the same build.
3. Ask the user to close `AyuGram.exe` and any attached debugger.
4. Wait for confirmation, verify that the process is gone, then retry once.

## Build practices

- Use Debug for developer validation of each completed feature or fix.
- Build Release only after the developer confirms Debug and the signed change
  is integrated into a clean `dev`.
- Never publish an unconfirmed Debug result or an untagged Release package.
- Treat `.github/workflows/windows-release.yml` as a manual fallback. The normal
  publication path is the signed local package procedure documented in
  `docs/ayugram-local-development.md`.

## Text File Format

- On Windows, keep project text files with CRLF line endings.
- Do not save source, header, build/config, style, or localization files as UTF-8 with BOM. Use UTF-8 without BOM.
- When rewriting project text files for normalization, preserve file content otherwise and do not introduce a BOM.

## Secrets and personal data

- Never commit or paste actual `TDESKTOP_API_ID`, `TDESKTOP_API_HASH`, GitHub
  tokens, signing passphrases, private keys, passwords, or secret-manager
  output. Read required build credentials from process environment variables
  and keep generated CMake caches outside Git.
- Do not put a person's real name, private email, phone number, home-directory
  username, hostname, public IP address, or other machine-specific identifier
  in repository documentation. Use placeholders such as `<BuildRoot>`,
  `<RepoRoot>`, `<fork-url>`, and `<tag>`.
- The public repository owner and remote names documented for this project are
  intentional project identifiers. Do not infer or record additional personal
  identity from them.
- Never run commands that print credential values, such as `gh auth token` or
  an unrestricted environment dump, into agent logs.
- Before every commit, inspect the exact staged paths and run a secret scanner
  with full redaction over the staged diff. Use the pinned Gitleaks version and
  verified installation procedure in `docs/building-win-x64.md`. A clean
  scanner result does not replace manual review for personal identifiers.

## Commits

- Subject: one concise, plain-language line summarizing the change, ~50-60 characters, matching the style of recent `git log` subjects. This is usually the entire message.
- Add a short plain-language body only when the subject can't carry it (what was done, not the technical how) — a line or two at most.
- Never add a `Co-Authored-By:` line or any tool/assistant attribution trailer.
- Never add `Autotask:`/attempt or other workflow markers — commits read like normal history.
- Sign every project and customized-submodule commit with `git commit -S`, then
  verify it with `git verify-commit HEAD`.
- Use signed annotated release tags that point at the verified `dev` commit.
- Do not push commits, tags, or release assets until the user explicitly asks.

## Release documentation

Every release tag ships with documentation updates, committed to `dev`
**before** the tag is created so the tag contains them:

- Add a one-line summary of the tag to the fork-release summary section of
  `README.md` (English) and `README.zh-TW.md` (Traditional Chinese).
- Add a detailed entry for the tag to `CHANGELOG.md`, bilingual in
  Traditional Chinese and English, newest tag first.
- Write the GitHub Release notes describing what the **current tag** changed,
  always bilingual in Traditional Chinese and English. Do not restate
  the full history in later releases; link to `CHANGELOG.md` for older tags.
- Keep the three layers consistent: README carries summaries, CHANGELOG.md
  carries details, and the GitHub Release carries the current tag's work plus
  build information (commit, versions, SHA-256).

## Local Storage Serialization

Both app-level (`Core::Settings`) and session-level (`Main::SessionSettings`) use sequential binary serialization via `QDataStream`. Key rules:

- New fields must ALWAYS be appended at the **end** of the stream, never inserted in the middle
- Reading new fields must be guarded with `!stream.atEnd()` and provide a meaningful default/fallback
- Inserting in the middle breaks reading of data saved by older versions (the new read code consumes bytes that belong to subsequent fields)
- For simple flags and values, prefer using the generic KV prefs facility (`writePref<Type>` / `readPref<Type>`) instead of adding to the binary stream -- this avoids serialization ordering issues entirely

---

# Development Guidelines

## Coding Style

**Do NOT write comments in code:**

This is important! Do not write single-line comments that describe what the next line does - they are bloat. Comments are allowed ONLY to describe complex algorithms in detail, when the explanation requires at least 4-5 lines. Self-documenting code with clear variable and function names is preferred.

```cpp
// BAD - don't do this:
// Get the user's name
auto name = user->name();
// Check if premium
if (user->isPremium()) {

// GOOD - no comments needed, code is self-explanatory:
auto name = user->name();
if (user->isPremium()) {

// ACCEPTABLE - complex algorithm explanation (4+ lines):
// The algorithm works by first collecting all visible messages
// in the viewport, then calculating their intersection with
// the clip rectangle. Messages are grouped by date headers,
// and we need to account for sticky headers that may overlap
// with the first message in each group.
```

**Style and formatting rules** are in `REVIEW.md` — see that file for empty-line-before-closing-brace, operator placement in multi-line expressions, if-with-initializer, and other mechanical style rules.

**Use `auto` for type deduction:**

Prefer `auto` (or `const auto`, `const auto &`) instead of explicit types:

```cpp
// Prefer this:
auto currentTitle = tr::lng_settings_title(tr::now);
auto nameProducer = GetNameProducer();

// Instead of this:
QString currentTitle = tr::lng_settings_title(tr::now);
rpl::producer<QString> nameProducer = GetNameProducer();
```

**Use trailing return types only when the normal form is too long:**

Prefer the normal return type form when the opening line fits comfortably, roughly around 77 characters or less:

```cpp
// GOOD:
[[nodiscard]] TextWithEntities FlattenSummaryBlocks(
	const std::vector<Block> &blocks);
```

Do not use one-line trailing return types, or put the trailing return type after `)` on the same line. If it fits on one line with trailing syntax, the normal form would be shorter and easier to read:

```cpp
// BAD:
auto ComputeTitle() -> QString;

// BAD:
[[nodiscard]] auto FlattenSummaryBlocks(
	const std::vector<Block> &blocks) -> TextWithEntities;
```

Use `auto` with a trailing return type only when the normal opening line
`{attributes} {return-type} {class-name::}{function-name(}` would be too long, or would force the return type onto its own line. Put the arrow and return type on the next line so the return type remains easy to find:

```cpp
// BAD:
not_null<HistoryView::Controls::ComposeAiButton*>
HistoryView::Controls::SetupCaptionAiButton(SetupCaptionAiButtonArgs &&args);
```

```cpp
// GOOD:
auto HistoryView::Controls::SetupCaptionAiButton(
		SetupCaptionAiButtonArgs &&args)
-> not_null<HistoryView::Controls::ComposeAiButton*>;
```

This applies to both declarations and definitions.

**Use `_q` for QString literals:**

Prefer the project literal `u"..."_q` instead of the verbose `QStringLiteral("...")` macro when creating `QString` values:

```cpp
// Prefer this:
auto text = u"Settings"_q;

// Instead of this:
auto text = QStringLiteral("Settings");
```

**Never use `Q_OS_LINUX` for platform checks in new code:**

Telegram Desktop distinguishes at most three platforms: Windows / macOS / all-other. The "all-other" branch covers Linux, the BSD variants and more — and this is almost always the branch you want. `Q_OS_LINUX` narrows it to Linux alone, silently excluding the non-Linux Unix platforms, which is almost never intended. For the all-other branch use `!defined Q_OS_WIN && !defined Q_OS_MAC` at compile time, or its runtime equivalent `Platform::IsLinux()` — which, despite the name, means exactly `!defined Q_OS_WIN && !defined Q_OS_MAC` ("everything except Windows and macOS"), not Linux specifically:

```cpp
// BAD - excludes FreeBSD and other non-Linux Unix:
#ifdef Q_OS_LINUX
UnixSpecificCode();
#endif // Q_OS_LINUX

// GOOD - the all-other branch, compile time:
#if !defined Q_OS_WIN && !defined Q_OS_MAC
UnixSpecificCode();
#endif // !Q_OS_WIN && !Q_OS_MAC

// GOOD - the all-other branch, runtime (same meaning, NOT Linux-only):
if (Platform::IsLinux()) {
	UnixSpecificCode();
}
```

`Q_OS_LINUX` is only for the rare case where you genuinely want exactly Linux and not the other Unix-like systems — usually you don't. The few existing uses (`Telegram/SourceFiles/core/sandbox.cpp`, `Telegram/SourceFiles/platform/linux/specific_linux.cpp`) are such genuinely Linux-only code paths and stay as-is.

## API Usage

### API Schema Files

API definitions use [TL Language](https://core.telegram.org/mtproto/TL):

1. **`Telegram/SourceFiles/mtproto/scheme/mtproto.tl`** - MTProto protocol (encryption, auth, etc.)
2. **`Telegram/SourceFiles/mtproto/scheme/api.tl`** - Telegram API (messages, users, chats, etc.)

### Making API Requests

Standard pattern using `api()`, generated `MTP...` types, and callbacks:

```cpp
api().request(MTPnamespace_MethodName(
    MTP_flags(flags_value),
    MTP_inputPeer(peer),
    MTP_string(messageText),
    MTP_long(randomId),
    MTP_vector<MTPMessageEntity>()
)).done([=](const MTPResponseType &result) {
    // Handle successful response

    // Multiple constructors - use .match() or check type:
    result.match([&](const MTPDuser &data) {
        // use data.vfirst_name().v
    }, [&](const MTPDuserEmpty &data) {
        // handle empty user
    });

    // Single constructor - use .data() shortcut:
    const auto &data = result.data();
    // use data.vmessages().v

}).fail([=](const MTP::Error &error) {
    // Handle API error
    if (error.type() == u"FLOOD_WAIT_X"_q) {
        // Handle flood wait
    }
}).handleFloodErrors().send();
```

**Key points:**
- Always refer to `api.tl` for method signatures and return types
- Use generated `MTP...` types for parameters (`MTP_int`, `MTP_string`, etc.)
- For multiple constructors, use `.match()` or check `.type()` against `mtpc_` constants then call `.c_constructorName()`:
  ```cpp
  // Using match:
  result.match([&](const MTPDuser &data) { ... }, [&](const MTPDuserEmpty &data) { ... });
  // Or explicit type check:
  if (result.type() == mtpc_user) {
      const auto &data = result.c_user(); // asserts on type mismatch
  }
  ```
- For single constructors, use `.data()` shortcut
- Include `.handleFloodErrors()` before `.send()` in rare cases where you want special case flood error handling
- Silently ignore HTTP 406 errors in UI: the server uses 406 to mean "show nothing to the user". Guard toasts with `MTP::IgnoreError(error)` or use `MTP::ShowErrorFallback(show, error)` (both in `mtproto/mtproto_response.h`) which shows `error.type()` as a toast unless the error should be ignored.

## UI Styling

### Style Files

UI styles are defined in `.style` files using custom syntax:

```style
using "ui/basic.style";
using "ui/widgets/widgets.style";

MyButtonStyle {
    textPadding: margins;
    icon: icon;
    height: pixels;
}

defaultButton: MyButtonStyle {
    textPadding: margins(10px, 15px, 10px, 15px);
    icon: icon{{ "gui/icons/search", iconColor }};
    height: 30px;
}

primaryButton: MyButtonStyle(defaultButton) {
    icon: icon{{ "gui/icons/check", iconColor }};
}
```

**Built-in types:**
- `int` - Integer numbers (e.g., `maxLines: 3;`)
- `bool` - Boolean values (e.g., `useShadow: true;`)
- `pixels` - Pixel values with `px` suffix (e.g., `10px`)
- `color` - Named colors from `ui/colors.palette`
- `icon` - Inline icon definition: `icon{{ "path/stem", color }}`
- `margins` - Four values: `margins(top, right, bottom, left)`
- `size` - Two values: `size(width, height)`
- `point` - Two values: `point(x, y)`
- `align` - Alignment: `align(center)`, `align(left)`
- `font` - Font: `font(14px semibold)`
- `double` - Floating point

**Multi-part icons** (layers drawn bottom-up):
```style
myComplexIcon: icon{
  { "gui/icons/background", iconBgColor },
  { "gui/icons/foreground", iconFgColor }
};
```

**Borders** are typically separate fields, not a single property:
```style
chatInput {
  border: 1px;                       // width
  borderFg: defaultInputFieldBorder; // color
}
```

**Never hardcode sizes in code:**

The app supports different interface scale options. Style `px` values are automatically scaled at runtime, but raw integer constants in code are not. Never use hardcoded numbers for margins, paddings, spacing, sizes, coordinates, or any other dimensional values. Always define them in `.style` files and reference via `st::`.

```cpp
// BAD - breaks at non-100% interface scale:
p.drawText(10, 20, text);
widget->setFixedHeight(48);
auto margin = 8;
auto iconSize = QSize(24, 24);

// GOOD - define in .style file and reference:
p.drawText(st::myWidgetTextLeft, st::myWidgetTextTop, text);
widget->setFixedHeight(st::myWidgetHeight);
auto margin = st::myWidgetMargin;
auto iconSize = st::myWidgetIconSize;
```

**Duration constants**: Animation durations should NOT go in `.style` files, this is a legacy approach. Prefer `constexpr auto kName = crl::time(N)` in an anonymous namespace in the relevant `.cpp` file.

### Usage in Code

```cpp
#include "styles/style_widgets.h"

// Access style members
int height = st::primaryButton.height;
const style::icon &icon = st::primaryButton.icon;
style::margins padding = st::primaryButton.textPadding;

// Use in painting
void MyWidget::paintEvent(QPaintEvent *e) {
    Painter p(this);
    p.fillRect(rect(), st::chatInput.backgroundColor);
}
```

## Localization

### String Definitions

Strings are defined in `Telegram/Resources/langs/lang.strings`:

```
"lng_settings_title" = "Settings";
"lng_confirm_delete_item" = "Are you sure you want to delete {item_name}?";
"lng_files_selected#one" = "{count} file selected";
"lng_files_selected#other" = "{count} files selected";
```

### Usage in Code

**Immediate (current value):**

```cpp
auto currentTitle = tr::lng_settings_title(tr::now);

auto currentConfirmation = tr::lng_confirm_delete_item(
    tr::now,
    lt_item_name, currentItemName);

auto filesText = tr::lng_files_selected(tr::now, lt_count, count);
```

**Reactive (rpl::producer):**

```cpp
auto titleProducer = tr::lng_settings_title();

auto confirmationProducer = tr::lng_confirm_delete_item(
    lt_item_name,
    std::move(itemNameProducer));

auto filesTextProducer = tr::lng_files_selected(
    lt_count,
    countProducer | tr::to_count());
```

**Key points:**
- Pass `tr::now` as first argument for immediate `QString`
- Omit `tr::now` for reactive `rpl::producer<QString>`
- Placeholders use `lt_tag_name, value` pattern
- For `{count}`: immediate uses `int`, reactive uses `rpl::producer<float64>` with `| tr::to_count()`
- Move producers with `std::move` when passing to placeholders
- Rich text projectors — these `tr::` helpers serve double duty: as the **last argument** (projector) they set the return type to `TextWithEntities`, and as **placeholder values** they wrap individual substitutions in formatting. Always prefer them over `Ui::Text::Bold()`, `Ui::Text::RichLangValue`, etc. — see REVIEW.md for the full mapping.
  - `tr::marked` — basic projection, converts `QString` to `TextWithEntities`
  - `tr::rich` — interprets `**bold**`/`__italic__` markup in the string
  - `tr::bold`, `tr::italic`, `tr::underline` — wrap text in that formatting
  - `tr::link` — wrap as a clickable link
  - `tr::url(u"https://..."_q)` — returns a projection that converts text to a link pointing to the given URL; can be passed to `rpl::map` or directly to a `tr::lng_...` call
  ```cpp
  // As last argument (projector):
  auto title = tr::lng_export_progress_title(tr::now, tr::bold);
  auto text = tr::lng_proxy_incorrect_secret(tr::now, tr::rich);
  // As placeholder value wrapper + projector:
  auto desc = tr::lng_some_key(
      tr::now,
      lt_name,
      tr::bold(userName),
      lt_group,
      tr::bold(groupName),
      tr::rich);
  // Nested tr::lng as placeholder:
  auto linked = tr::lng_settings_birthday_contacts(
      lt_link,
      tr::lng_settings_birthday_contacts_link(tr::url(link)),
      tr::marked);
  ```

## RPL (Reactive Programming Library)

### Core Concepts

**Producers** represent streams of values over time:

```cpp
auto intProducer = rpl::single(123);  // Emits single value
auto lifetime = rpl::lifetime();       // Manages subscription lifetime
```

### Starting Pipelines

```cpp
std::move(counter) | rpl::on_next([=](int value) {
    qDebug() << "Received: " << value;
}, lifetime);

// Without lifetime parameter - MUST store returned lifetime:
auto subscriptionLifetime = std::move(counter) | rpl::on_next([=](int value) {
    // process value
});
```

### Transforming Producers

```cpp
auto strings = std::move(ints) | rpl::map([](int value) {
    return QString::number(value * 2);
});

auto evenInts = std::move(ints) | rpl::filter([](int value) {
    return (value % 2 == 0);
});
```

### Combining Producers

**`rpl::combine`** - combines latest values (lambdas receive unpacked arguments):

```cpp
auto combined = rpl::combine(countProducer, textProducer);

std::move(combined) | rpl::on_next([=](int count, const QString &text) {
    qDebug() << "Count=" << count << ", Text=" << text;
}, lifetime);
```

**`rpl::merge`** - merges producers of same type:

```cpp
auto merged = rpl::merge(sourceA, sourceB);

std::move(merged) | rpl::on_next([=](QString &&value) {
    qDebug() << "Merged value: " << value;
}, lifetime);
```

**Other pipeline starters** — besides `rpl::on_next`, there are:
- `rpl::on_error([=](Error &&e) { ... }, lifetime)` — handle errors
- `rpl::on_done([=] { ... }, lifetime)` — handle stream completion
- `rpl::on_next_error_done(nextCb, errorCb, doneCb, lifetime)` — handle all three

The `Error` template parameter defaults to `rpl::no_error`: `rpl::producer<Type, Error = no_error>`.

**Key points:**
- Explicitly `std::move` producers when starting pipelines
- Pass `rpl::lifetime` to `on_...` methods or store returned lifetime
- Use `rpl::duplicate(producer)` to reuse a producer multiple times
- Combined producers automatically unpack tuples in lambdas (works with `rpl::map`, `rpl::filter`, and `rpl::on_next`)
