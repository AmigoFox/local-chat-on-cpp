# 💬 Qt Local Chat

Простой локальный чат, реализованный на **C++ и Qt** с использованием **QTcpServer** и **QTcpSocket**.  
Позволяет общаться между пользователями в одной локальной сети (LAN) без внешнего сервера.  

---

## ⚙️ Основные возможности

- 📡 **Локальное соединение** через TCP  
- 💬 **Обмен сообщениями** между клиентами в одной сети  
- 💾 **Сохранение истории** чата в `chat.txt`  
- 👤 **Сохранение ника пользователя** в `nick.txt`  
- 🪟 Удобный **графический интерфейс** (Qt Widgets)  
- 🧱 Полная поддержка сборки через **CMake**  
- 💽 Есть **готовый установщик** (.exe), который можно просто запустить и начать пользоваться  

---

## 🧩 Используемые технологии

- **C++ 17**
- **Qt 6.9+**
- **Qt Widgets / Network**
- **CMake**
- **Qt Installer Framework**

---

## 🗂️ Структура проекта

```text
D:\Proj_cpp\untitled
│
├── .gitignore
├── CMakeLists.txt
├── CMakeLists.txt.user
├── main.cpp
├── mainwindow.cpp
├── mainwindow.h
├── mainwindow.ui
├── untitled_ru_RU.ts
│
├── build/                          # Каталог сборки (CMake)
│
└── installer/                      # Установщик приложения (Qt Installer Framework)
    ├── config.xml
    ├── MyAppInstaller.exe          # Готовый установщик (.exe)
    │
    └── packages/
        └── myapp/
            ├── meta/
            │   ├── installscript.qs
            │   └── package.xml
            │
            └── data/
                ├── untitled.exe            # Готовый исполняемый файл приложения
                ├── chat.txt                # Лог чата
                ├── nick.txt                # Сохранённый ник пользователя
                ├── *.dll                   # Qt-библиотеки
                ├── CMakeCache.txt
                ├── build.ninja
                ├── imageformats/
                ├── platforms/
                ├── translations/
                ├── ...
