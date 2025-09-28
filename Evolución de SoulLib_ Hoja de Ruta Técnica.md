# **Informe Técnico: Hoja de Ruta Evolutiva para SoulLib (Actualización 2025)**

PARA: Liderazgo Técnico, Proyecto SoulLib  
DE: Arquitecto Senior de Software, Especialista en Motores de Juego  
FECHA: 27 de septiembre de 2025  
ASUNTO: Estado arquitectónico, hitos completados y siguientes pasos para SoulLib

---

## **1. Resumen Ejecutivo**

Durante el último ciclo se completaron los hitos más críticos de la fase “Mediano Plazo” del plan estratégico original: IO asíncrona, refactor de red dual (UDP/TCP), estandarización documental con Doxygen, evolución del sistema de tiempo a un scheduler con DAGs y corrutinas, y la herramienta de visualización de memoria. Además, la base de código migró a un toolchain C++20, habilitando el uso nativo de corrutinas y `std::span`, y se extendieron los contenedores asociativos (`SoulSet`, `SoulMap`, `SoulFlatMap`, `SoulMultiset`, `SoulMultimap`) y no ordenados (`SoulUnorderedSet`, `SoulUnorderedMap`) junto con el contenedor secuencial (`SoulVector`) con constructores e inserciones basados en ranges. Como parte de la transición gradual hacia C++23 se habilitó la opción experimental `SOULLIB_ENABLE_CPP23` y se validó la suite completa de tests (93/93) bajo este estándar. Este documento actualiza la hoja de ruta para reflejar los entregables, detalla la arquitectura vigente y recalibra los pendientes de largo plazo, incorporando el plan de transición gradual hacia C++23.

---

## **1.1. Tabla de Progreso Actualizada**

| Fase | Subsistema | Tarea | Prioridad | Dependencias | Impacto | Estado |
| :---- | :---- | :---- | :---- | :---- | :---- | :---- |
| **Corto Plazo** | Memory | Refactorizar MemoryManager para eliminar el patrón singleton. | Crítica | \- | Estabilidad, Rendimiento | ✅ Completado |
|  | Memory | Implementar PoolAllocator y ArenaAllocator. | Crítica | Refactor MemoryManager | Rendimiento | ✅ Completado |
|  | Memory | Optimizar tags de memoria con hashes constexpr. | Alta | \- | Rendimiento | ✅ Completado |
|  | Build/Test | Modernizar CMake para exportar targets (find\_package). | Alta | \- | Ecosistema, API | ✅ Completado |
|  | Build/Test | Integrar Google Benchmark para Memory y Containers. | Alta | \- | Estabilidad | ✅ Completado |
|  | Containers | Implementar SoulFlatMap y SoulVector con SBO. | Media | ArenaAllocator | Rendimiento, API | ✅ Completado |
| **Mediano Plazo** | FileSystem | Diseñar e implementar I/O asíncrona (IAsyncFileIO). | Crítica | ArenaAllocator | Rendimiento, API | ✅ Completado |
|  | Networking | Re-arquitectura completa: soporte TCP/UDP, paquetes. | Crítica | ISerializer | Funcionalidad, API | ✅ Completado |
|  | Networking | Implementar manejo de endianness y fiabilidad UDP básica. | Alta | Re-arq. Networking | Estabilidad | ✅ Completado |
|  | Docs | Establecer y aplicar estándar de documentación Doxygen. | Media | \- | Ecosistema | ✅ Completado |
| **Largo Plazo** | Time | Evolucionar TimerManager a TaskScheduler con DAGs. | Crítica | I/O Asíncrona | Rendimiento, API | ✅ Completado (FrameScheduler) |
|  | Time | Integrar corrutinas de C++20 en el TaskScheduler. | Alta | TaskScheduler | API | ✅ Completado |
|  | C++20/23 | Migrar gradualmente el código base a Módulos. | Media | Soporte de toolchain | Rendimiento (build) | ⏳ Prototipo MSVC compilando |
|  | C++20/23 | Aplicar Conceptos y Rangos en APIs de contenedores. | Media | \- | API, Estabilidad | ✅ Completado (SoulVector, SoulFlatMap, SoulSet, SoulMap, SoulMultiset, SoulMultimap, SoulUnorderedSet, SoulUnorderedMap) |
|  | C++23 | Planificar migración completa de toolchain y estándar (MSVC/Clang/GCC). | Media | Auditoría de dependencias, CI | Ecosistema, Feature-set | ⏳ Evaluación (build experimental validado) |
|  | Ecosistema | Desarrollar herramienta de visualización de memoria. | Baja | Tags de memoria optimizados | Ecosistema | ✅ Completado |

---

## **2. Hitos Entregados**

### **2.1. Plataforma Asíncrona (Async/TaskScheduler + FrameScheduler)**

* Nuevo módulo `Async/Task.h` implementa un scheduler basado en corrutinas con soporte para DAGs de dependencias, reanudación en pool de hilos y compatibilidad con `co_await`.
* `FrameScheduler` (en `time/FrameScheduler.h`) extiende el scheduler para orquestar trabajos por frame y tareas diferidas (`schedule_after`). Reemplaza a `TimerManager` como solución recomendada, conservando compatibilidad hacia atrás.
* Pruebas unitarias en `test/time/FrameSchedulerTests.cpp` validan orden topológico y retrasos garantizados.

### **2.2. FileSystem Asíncrono**

* `FileSystem/IO/IAsyncFileIO.h` define la interfaz asíncrona. `ThreadPoolAsyncFileIO` (implementación por defecto) ejecuta lecturas/escrituras en el `TaskScheduler`.
* `AsyncFileManager` encapsula IO + estrategia de encriptación y expone `Task<ReadFileResult>` / `Task<WriteFileResult>`.
* Test de regresión en `test/FileSystem/IO/AsyncFileManagerTests.cpp` cubre round-trip binario y texto.

### **2.3. Networking Dual y Fiabilidad Básica**

* Nuevo módulo `Networking/` con `NetworkManager`, `UdpTransport`, `TcpTransport`, `Packet` y utilidades de endianess.
* Los paquetes incluyen cabecera con secuencia, ACK y máscara de fiabilidad. UDP envía `header+payload` contiguo; TCP usa flujo length-prefixed.
* Test `NetworkManagerTests.cpp` valida el envío/recepción UDP en loopback con seguimiento de puertos origen/destino.

### **2.4. Tooling y Documentación**

* `docs/Doxyfile` y la opción `SOULLIB_BUILD_DOCS` estandarizan la generación de documentación automática.
* La guía principal (`docs/Architecture.md`) se reescribió para reflejar el nuevo flujo asíncrono y la dualidad de red.
* Se creó el CLI `SoulLibMemoryViz` (CMake opcional `SOULLIB_BUILD_TOOLS`) que ahora soporta snapshots JSON o Graphviz DOT (`MemoryVisualizer::WriteJson` / `WriteGraphviz`) y expone el selector `--format`.

### **2.5. Migración a C++20 y preparación para C++23**

* El toolchain se fija en C++20 (`CMAKE_CXX_STANDARD 20`).
* `Async/Task` y `FrameScheduler` usan corrutinas nativas; el FileSystem emplea `std::span` y `std::byte`.
* Se inició la evaluación de toolchains MSVC v17.10, Clang 18 y GCC 14 para garantizar soporte homogéneo en C++23.
* Pendientes inmediatos: módulos C++20 y adopción incremental de tipos de C++23 (ver §4 y §5).

### **2.6. Fiabilidad UDP opcional**

* `NetworkManager::enable_udp_reliability` habilita retransmisión por canal sobre UDP conservando los encabezados existentes (secuencias + máscara de ACK).
* `configure_udp_retransmission` ajusta timeout y número máximo de reintentos; los ACK se envían como paquetes livianos marcados con flags.
* Nuevos tests (`UdpReliableRetransmission`) validan la recuperación ante pérdida inicial de datagramas usando un transporte con caídas artificiales.

### **2.7. Prototipo de Módulos C++20**

* Se añadió la opción `SOULLIB_EXPERIMENTAL_MODULES` en CMake para compilar un `STATIC` auxiliar (`SoulLibMemoryModules`) cuando el toolchain es MSVC.
* El módulo `export module Soul.Memory.Statistics;` encapsula utilidades de `MemoryStatistics` y expone un consumidor de prueba (`MemoryStatisticsImport.cpp`) para medir compatibilidad.
* La compilación dedicada (`cmake --build build-modules --target SoulLibMemoryModules`) finaliza correctamente con MSVC 19.29, estableciendo la base para segmentar subsistemas completos en el siguiente ciclo.

---

## **3. Arquitectura Actualizada por Subsistema**

### **3.1. Async + Time**

* `TaskScheduler` administra un pool configurable; `run_async` permite encapsular trabajos bloqueantes en corrutinas.
* `FrameScheduler` agrega semántica de frame y retrasos, devolviendo `TaskHandle` con `TaskToken` para dependencias explícitas.
* `TimerManager` continúa disponible pero recomendado sólo para compatibilidad.

### **3.2. FileSystem**

* `AsyncFileManager` + `ThreadPoolAsyncFileIO` constituyen el camino estándar. Operaciones devuelven `std::error_code` junto con la ruta canónica.
* La capa de encriptación (`IEncryptionStrategy`) permanece pluggable; `NoEncryption` sigue siendo la opción por defecto.

### **3.3. Networking**

* `NetworkManager` decide transporte según `DeliveryGuarantee` (`Reliable` → TCP, `Unreliable` → UDP).
* Los encabezados incluyen `sequence`, `acknowledgment` y `acknowledgmentMask`, habilitando extensión futura de fiabilidad.
* `Platform.h` abstrae diferencias Winsock/POSIX (cierre de sockets, startup de Winsock).

### **3.4. Memory & Tooling**

* `MemoryManager::snapshot()` produce `MemoryStatistics` con totales y agregados por tag.
* `MemoryVisualizer::WriteJson` (y el CLI) generan reportes para dashboards o pipelines CI.

### **3.5. Containers**

* `SoulVector` (header-only) mantiene SBO configurable y reacondicionamiento al buffer pequeño tras `shrink_to_fit`.
* `SoulFlatMap` usa vector ordenado + búsqueda binaria; todos los contenedores usan `TaggedMemoryAllocator`.
* `SoulVector` incorpora constructores y `append_range` basados en ranges, utilizando `std::ranges` y `std::convertible_to` para facilitar operaciones genéricas.
* `SoulFlatMap`, `SoulSet`, `SoulMap`, `SoulMultiset`, `SoulMultimap`, `SoulUnorderedSet` y `SoulUnorderedMap` aceptan rangos para construcción e inserción masiva, integrando `std::ranges::input_range` y reservando capacidad cuando se conoce el tamaño del rango.
* Próximo objetivo: evaluar la exposición de ranges en adaptadores y algoritmos auxiliares (`ContainerManager`, utilidades de filtrado) manteniendo la interfaz estable de SoulLib.

### **3.6. Build/Test**

* `CMakeLists.txt` actualiza a versión 0.2.0, expone opciones de tests, benchmarks, tools y docs.
* Pruebas nuevas integradas a `SoulLibTests`; `test/CMakeLists.txt` ahora incluye FileSystem/Async automáticamente.
* Benchmarks permanecen sin cambios (se recomienda extender a contenedores renovados en la próxima iteración).
* Nuevo flag `SOULLIB_ENABLE_CPP23` permite habilitar compilación experimental con C++23 manteniendo C++20 como valor por defecto.

---

## **4. Próximos Pasos (Largo Plazo)**

1. **C++20/23 profundizado**
    * Investigar viabilidad de módulos C++20 por subsistema (`memory`, `containers`, `async`), evaluando impacto en tiempos de compilación.
   * Consolidar el prototipo `SoulLibMemoryModules` como plantilla para futuras particiones, validado ya en MSVC 19.29 mediante `SOULLIB_EXPERIMENTAL_MODULES`.
    * Introducir Concepts/Ranges en APIs (ej. `SoulFlatMap` podría aceptar rangos `std::ranges::range`).
   * Estado actual: El ecosistema de contenedores (`SoulVector`, `SoulFlatMap`, `SoulSet`, `SoulMap`, `SoulMultiset`, `SoulMultimap`, `SoulUnorderedSet`, `SoulUnorderedMap`) ya opera con constructores e inserciones basados en ranges.
    * Preparar actualización del `CMAKE_CXX_STANDARD` a 23 tras validar toolchains soportados en CI y sincronizar dependencias de SoulBox.
2. **Tuning de Fiabilidad UDP**
   * ✅ Completado — retransmisión por canal integrada, ACK livianos y pruebas con pérdida simulada.
3. **Integración CI/CD**
   * Automatizar ejecución de `SoulLibMemoryViz` y benchmarks comparativos en pipeline.
4. **Documentación API detallada**
   * Aprovechar la nueva configuración Doxygen para instrumentar headers críticos (Async, Networking, FileSystem) con comentarios `@brief`, `@param`, etc.
   * Estado: en curso — `Async/Task.h`, `time/FrameScheduler.h`, `FileSystem/Core/AsyncFileManager.h` y `Networking/NetworkManager.h` ya incluyen bloques documentados.
5. **Herramientas Avanzadas**
   * Extender MemoryVisualizer hacia Graphviz / UI interactiva.
   * Explorar visualización de DAGs programados en `FrameScheduler`.

---

## **5. Plan de Migración a C++23**

1. **Auditoría de Dependencias y Toolchain**
   * Validar soporte de C++23 en MSVC (17.10), Clang (18) y GCC (14) y documentar flags equivalentes (`/std:c++latest`, `-std=c++23`).
   * Confirmar que bibliotecas externas (solo estándar) carecen de incompatibilidades conocidas.
2. **Actualización Escalonada de Build System**
   * Añadir opción CMake `SOULLIB_ENABLE_CPP23` que active compilación con C++23 en entornos controlados. ✅ Implementado y configurado en el build local.
   * Instrumentar CI para ejecutar matrices mixtas (C++20 estable / C++23 experimental) antes del cambio definitivo.
   * Resultado preliminar: la configuración Debug en MSVC 19.29 compiló correctamente y ejecutó 95/95 pruebas unitarias con la opción activada; adicionalmente, el objetivo experimental `SoulLibMemoryModules` se genera con éxito utilizando módulos C++20.
3. **Adopción de Características Graduales**
   * Priorizar `std::expected` para reemplazar `Result` locales en subsistemas asincrónicos y de IO.
   * Evaluar `std::mdspan` para estructuras de datos en benchmarks de containers y memoria.
   * Analizar beneficios de `std::print` en herramientas (manteniendo formato de bajo nivel en runtime).
4. **Sincronización con SoulBox**
   * Coordinar con el equipo de SoulBox para asegurar compatibilidad binaria y actualización simultánea del estándar.
   * Definir ventana de congelamiento de API para mitigar riesgos durante la transición.
5. **Cierre y Retroalimentación**
   * Medir tiempos de compilación y desempeño antes/después del cambio.
   * Documentar lecciones aprendidas y ajustar guías de estilo para reflejar nuevas primitivas del estándar.

---

## **6. Conclusiones**

SoulLib alcanzó el “Salto Asíncrono” previsto: IO no bloqueante, scheduler con corrutinas, red dual, toolchain C++20 y tooling de memoria. Los pendientes de largo plazo se centran en refinamientos (conceptos/rangos, módulos, fiabilidad avanzada) y en la madurez del ecosistema (CI, documentación generada automáticamente). La base actual es estable para integrarse en SoulBox y habilita futuras migraciones a C++20/23 sin riesgos significativos.  

> **Recomendación inmediata**: priorizar la formalización de documentación Doxygen por subsistema y la investigación de módulos C++20, manteniendo en paralelo pruebas de estrés de networking.
