# AudioChannels

Monitor de canales de audio para Windows: vive en la bandeja del sistema y muestra en tiempo real qué canales (2.0, 5.1, 7.1...) del dispositivo de salida activo tienen señal.

*Windows tray app that shows in real time which channels (2.0, 5.1, 7.1...) of your active audio output device have signal. See [English](#english) below.*

## Descarga

No hace falta instalador. Bajá el `.exe` de la [última release](../../releases/latest) y ejecutalo — es un solo archivo portable, no toca nada del sistema salvo que vos actives explícitamente el autoarranque desde Configuración.

Requiere Windows 10 (versión 1703 o posterior) o Windows 11, de 64 bits.

## Qué hace

- Detecta automáticamente la cantidad de canales del dispositivo de salida activo (vía WASAPI), y se reconfigura solo si cambiás el dispositivo predeterminado en Windows.
- Ícono de bandeja con un punto por canal: verde si tiene señal, gris si no.
- Clic en el ícono: ventana de detalle con nombre, actividad y nivel en dB por canal.
- Clic derecho: elegir manualmente el dispositivo a monitorear, activar autoarranque con Windows, o cambiar el idioma (español/inglés, automático según Windows por defecto).

## Compilar desde el código fuente

Para desarrolladores que quieran modificar la app o compilarla ellos mismos:

**Requisitos:**
- Visual Studio 2022 (o más nuevo) con el workload **"Desarrollo de escritorio con C++"** (incluye el Windows SDK).

**Pasos:**
1. Cloná el repo.
2. Abrí `AudioChannels.sln`.
3. Seleccioná configuración `Release`, plataforma `x64`.
4. Compilar (`Ctrl+Shift+B`). El ejecutable queda en `build\x64\Release\AudioChannels.exe`.

No hay dependencias externas (NuGet, vcpkg, etc.) — es Win32 + WASAPI nativo.

## Licencia

[MIT](LICENSE).

---

## English

Windows tray app that shows in real time which channels (2.0, 5.1, 7.1...) of the active audio output device have signal — one dot per channel, green when it has signal.

### Download

No installer needed. Grab the `.exe` from the [latest release](../../releases/latest) and run it — it's a single portable file that touches nothing on your system unless you explicitly enable autostart from Settings.

Requires 64-bit Windows 10 (version 1703+) or Windows 11.

### Features

- Automatically detects the channel count of the active output device (via WASAPI) and reconfigures itself when you change the default device in Windows.
- Tray icon with one dot per channel: green when active, gray otherwise.
- Click the icon for a detail window with per-channel name, activity, and dB level.
- Right-click to manually pick which device to monitor, enable autostart with Windows, or switch language (Spanish/English, auto-detected from Windows by default).

### Building from source

**Requirements:**
- Visual Studio 2022 (or newer) with the **"Desktop development with C++"** workload (includes the Windows SDK).

**Steps:**
1. Clone the repo.
2. Open `AudioChannels.sln`.
3. Select `Release` configuration, `x64` platform.
4. Build (`Ctrl+Shift+B`). The executable lands in `build\x64\Release\AudioChannels.exe`.

No external dependencies (NuGet, vcpkg, etc.) — plain native Win32 + WASAPI.

### License

[MIT](LICENSE).
