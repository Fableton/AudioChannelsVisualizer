# Code Signing Policy

Free code signing provided by [SignPath.io](https://signpath.io), certificate by [SignPath Foundation](https://signpath.org).

## Team

| Role | User |
|---|---|
| Authors (commit access, can modify the repository without additional review) | [@Fableton](https://github.com/Fableton) |
| Reviewers (review changes from non-committers) | [@Fableton](https://github.com/Fableton) |
| Approvers (authorize releases for code signing) | [@Fableton](https://github.com/Fableton) |

AudioChannelsVisualizer is currently maintained by a single developer, who holds all three roles above. Team members use multi-factor authentication for both GitHub and SignPath access.

## Privacy

AudioChannelsVisualizer does not collect, transmit, or store any user data. It reads local audio device information via the Windows WASAPI API purely to display it on-screen; nothing leaves the machine it runs on. The only persistent state it writes is local: the user's own preferences (selected device, language, autostart) under `HKEY_CURRENT_USER\Software\Fableton\AudioChannels`, and, only if the user opts in, an autostart entry under `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run`.

## Distribution

Releases are built and published automatically from this repository's public GitHub Actions workflows (`.github/workflows/release.yml`) and distributed as a single portable `.exe` via [GitHub Releases](https://github.com/Fableton/AudioChannelsVisualizer/releases). No installer, no bundled third-party software.
