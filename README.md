# Транспортный справочник
Программа для создания базы автобусных маршрутов и получения информации по ним. Обрабатывает запросы в формате JSON. Имеются два режима работы:
- создание базы данных
- обработка запросов к созданной базе

Запросы к базе могут быть следующих типов:
- информация об остановках и маршрутах
- поиск кратчайшего маршрута между двумя остановками
- вывод карты в формате SVG

## Использование
1) Установить требуемые компоненты
2) Запустить программу с нужным параметром (make_base или process_request, примеры можно найти в Examples.txt и main.cpp)

## Системные требования
1. С++17 (STL)
4. GCC (MinGW-w64) 11.2.0

## Планы по доработке
1. Добавить пользовательский интерфейс
2. Добавить карту города

## Стек технологий
1. Cmake 3.10
2. Protobuf-cpp 3.21.4
