format PE console

entry start

include 'win32a.inc'

section '.data' data readable writable
        ;Строка форматирования для вывода результата вычислений
        strResult   db "E = %1.10f", 13, 10, 0

        A           dq  0.0     ;Вещественный индексатор
        result      dq  1.0     ;Результат вычислений (вещественный)
        N           dd  10      ;Количество итераций
        factorial   dq  1.0     ;Текущее значение факториала
        increment   dq  1.0     ;Вещественный константный инкремент

section '.code' code readable executable
;Входная точка программы
start:
        FINIT
        call CalcE

        invoke printf, strResult, dword[result], dword[result + 4]
finish:
        call [getch]
        push 0
        call [ExitProcess]

CalcE:
        xor ecx, ecx            ;Обнуление регистра индексатора
        inc ecx                 ;+1 т.к. счет результата начинается с 1
startLoop:
        cmp ecx, [N]            ;Проверка на завершение цикла
        je endLoop
        FLD [A]                 ;Загрузка вещественного индексатора в ST(0)
        FADD [increment]        ;Инкрементируем ST(0)
        FST [A]                 ;Результат сохраняем в A
        FMUL [factorial]        ;Умножаем индексатор на прошлое значение факториала
        FSTP [factorial]        ;Сохраняем новое значение факториала, очищ. ST(0)

        FLD [increment]         ;Загружаем константный вещественный инкремент в ST(0)
        FDIV [factorial]        ;Делим его на новое значение факториала
        FADD [result]           ;Добавляем туда общий результат суммы
        FSTP [result]           ;Сохраняем новое значение суммы, очищ. ST(0)

        inc ecx                 ;Инкрементируем регистр ecx
        jmp startLoop           ;Возвращаемся на начало цикла
endLoop:
        ret  ;Основной стек никак не задействован, поэтому нет смысла его восстанавливать

section '.idata' import data readable
    library kernel, 'kernel32.dll',\
            msvcrt, 'msvcrt.dll'

    import kernel,\
           ExitProcess, 'ExitProcess'
    import msvcrt,\
           printf, 'printf',\
           sprintf, 'sprintf',\
           scanf, 'scanf',\
           getch, '_getch'