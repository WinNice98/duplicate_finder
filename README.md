# Duplicate Finder

GUI-приложение на Qt для поиска и удаления дубликатов файлов по хэшу. Программа сканирует выбранную папку, считает хэши (MD5/SHA‑1/SHA‑256), группирует совпадения и позволяет удалить отмеченные дубликаты.

## Возможности

- Поиск дубликатов по MD5, SHA‑1 или SHA‑256.
- Отображение групп дубликатов в виде дерева.
- Быстрое удаление отмеченных файлов.
- Работа через SQLite без внешних зависимостей.

## Сборка

### Требования

- Qt 5.15+ или Qt 6 (Widgets, Sql, Concurrent)
- CMake 3.16+
- Компилятор C++17

### Linux/macOS

```bash
cmake -S . -B build
cmake --build build
./build/duplicate_finder
```

### Windows (MSVC)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## Установка и упаковка

### Windows (windeployqt)

1. Соберите проект в Release.
2. Запустите `windeployqt` для подготовки переносимой папки:

```powershell
windeployqt build/Release/duplicate_finder.exe
```

После этого папку можно упаковать в ZIP/Installer.

### Linux (AppImage через linuxdeployqt)

1. Соберите проект в Release.
2. Установите в AppDir:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build --prefix AppDir
```

3. Соберите AppImage:

```bash
linuxdeployqt AppDir/usr/bin/duplicate_finder -appimage
```

### CPack (архивы)

CMake уже настроен для `CPack`. Вы можете собрать архивы так:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cpack --config build/CPackConfig.cmake
```

## Идеи для оптимизации

1. **Пакетные вставки в SQLite + единичное обновление UI.**
   Сейчас интерфейс обновляется на каждый файл; лучше собирать результаты и обновлять дерево 1 раз в конце.
2. **Двухэтапная фильтрация.**
   Сначала группировать по размеру, а хэшировать только файлы с одинаковым размером.
3. **Параллельное хэширование.**
   Использовать `QtConcurrent`/`QThreadPool` для загрузки нескольких файлов одновременно.
4. **Потоковая обработка списка файлов.**
   Не хранить полный список путей в памяти — отправлять файлы в очередь по мере обхода.
5. **Индексы в SQLite.**
   Индекс по `hash` и `size` ускорит выборку групп дубликатов.

## Лицензия

Укажите выбранную лицензию, например MIT или GPL.
