### Программа, модифицирующая входные файлы бинарной операцией с заданным значением
Порядок работы программы:  
После запуска настраивается пользователем:  
1. Маска входных файлов, например .txt, testFile.bin
2. Настройка необходимости удалять входные файлы или нет
3. Путь для сохранения результирующих файлов
4. Действия при повторении имени выходного файла: перезапись или
модификация, например, счетчик к имени файла
5. Работа по таймеру или разовый запуск
6. Периодичность опроса наличия входного файла (таймер)
7. Значение 8 байт для бинарной операции модификации файла
   
Функциональность: модифицирует входные файлы, например операция XOR с 8-байтной переменной, введенной с формы (Пункт 1,g)
Защита от «дурака»: если входной файл не закрыт - не трогать его.
