# Requerimientos: Monitor de Canales de Audio (Windows)

## 1. Objetivo
Aplicación de escritorio para Windows que permita visualizar en tiempo real qué canales de audio (2.0, 5.1, 7.1, etc.) están activos según el dispositivo de salida configurado en el sistema, indicando cuáles tienen señal de audio y cuáles no.

## 2. Alcance funcional

### 2.1 Detección de canales
- La app debe detectar automáticamente la cantidad de canales configurados en el dispositivo de salida de Windows (ej. 2 para estéreo, 6 para 5.1, 8 para 7.1).
- La detección se hace leyendo el formato del dispositivo de salida activo (vía WASAPI), no por configuración manual del usuario.
- Si el usuario cambia el dispositivo de salida predeterminado en Windows (ej. de bocinas 5.1 a audífonos estéreo) mientras la app está corriendo, la app debe detectar el cambio automáticamente y reconfigurarse sola (recalcular canales, actualizar ícono y ventana de detalle).

### 2.2 Ícono en la bandeja del sistema (system tray)
- Muestra N puntos, donde N = número de canales detectados del dispositivo activo.
- Los puntos se organizan/identifican por posición física del canal (ej. layout típico de 5.1: frontal izq/der, centro, LFE, trasero izq/der).
- Cada punto se enciende en **verde** cuando ese canal tiene señal de audio, sin importar el nivel (cualquier señal mínima cuenta como "activo"). No hay umbral de dB ni antirrebote/delay: es un indicador binario de actividad, no de volumen.
- Cuando un canal no tiene señal, su punto permanece apagado/inactivo (color neutro).

### 2.3 Ventana de detalle (al hacer clic en el ícono)
- Se despliega una ventana pequeña anclada cerca del ícono (comportamiento estándar de bandeja de Windows).
- Muestra una barra de nivel por cada canal detectado.
- Cada barra incluye:
  - Nombre completo del canal (ej. Front Left, Front Right, Center, LFE, Rear Left, Rear Right, Side Left, Side Right, según corresponda al layout detectado).
  - Indicador de actividad (activo/inactivo).
  - Nivel real de audio (dB) en tiempo real.

### 2.4 Selección de dispositivo
- Por defecto, la app monitorea el dispositivo de salida **predeterminado** del sistema.
- El usuario debe poder seleccionar manualmente otro dispositivo de salida disponible desde una pantalla/menú de configuración.

### 2.5 Configuración
- Checkbox para habilitar/deshabilitar **inicio automático con Windows** (autoarranque). Desactivado por defecto.
- Selector de dispositivo de salida a monitorear (ver 2.4).

## 3. Requerimientos no funcionales
- Debe correr en segundo plano como ícono de bandeja del sistema (no como ventana siempre visible).
- Actualización en tiempo real con baja latencia perceptible (varias actualizaciones por segundo tanto para el ícono como para las barras de la ventana de detalle).
- Consumo de recursos bajo, apropiado para correr todo el tiempo en background.
- Debe generarse como aplicación de escritorio nativa de Windows (ejecutable .exe).

## 4. Fuera de alcance (no solicitado)
- No se requiere grabar ni guardar histórico de niveles de audio.
- No se requiere control de volumen ni configuración de canales desde la app (solo monitoreo/lectura).
- No se requiere soporte multiplataforma (solo Windows).

## 5. Notas técnicas de referencia
- Captura de audio recomendada vía **WASAPI en modo loopback**, que permite leer el audio de salida ya mezclado en sus canales nativos sin necesidad de hardware adicional.
- Opciones de implementación evaluadas:
  - **Python + PyAudioWPatch**: fork de PyAudio con soporte nativo de loopback WASAPI multicanal; prototipado rápido; empaquetable a .exe con PyInstaller.
  - **C++ con WASAPI directo** (`IAudioClient` en modo loopback): mayor control y rendimiento, más código de bajo nivel.
  - **Rust con crates `wasapi` o `cpal`**: buen balance rendimiento/ergonomía, compila a ejecutable ligero sin runtime pesado.
- Se descarta continuar con el intento previo en .NET por solicitud explícita del usuario (aunque NAudio también soporta loopback WASAPI multicanal, se prefiere evaluar otras opciones).
