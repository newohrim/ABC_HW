format PE console

entry start

include 'win32a.inc'

section '.data' data readable writable
        ;������ �������������� ��� ������ ���������� ����������
        strResult   db "E = %1.10f", 13, 10, 0

        A           dq  0.0     ;������������ ����������
        result      dq  1.0     ;��������� ���������� (������������)
        N           dd  10      ;���������� ��������
        factorial   dq  1.0     ;������� �������� ����������
        increment   dq  1.0     ;������������ ����������� ���������

section '.code' code readable executable
;������� ����� ���������
start:
        FINIT
        call CalcE

        invoke printf, strResult, dword[result], dword[result + 4]
finish:
        call [getch]
        push 0
        call [ExitProcess]

CalcE:
        xor ecx, ecx            ;��������� �������� �����������
        inc ecx                 ;+1 �.�. ���� ���������� ���������� � 1
startLoop:
        cmp ecx, [N]            ;�������� �� ���������� �����
        je endLoop
        FLD [A]                 ;�������� ������������� ����������� � ST(0)
        FADD [increment]        ;�������������� ST(0)
        FST [A]                 ;��������� ��������� � A
        FMUL [factorial]        ;�������� ���������� �� ������� �������� ����������
        FSTP [factorial]        ;��������� ����� �������� ����������, ����. ST(0)

        FLD [increment]         ;��������� ����������� ������������ ��������� � ST(0)
        FDIV [factorial]        ;����� ��� �� ����� �������� ����������
        FADD [result]           ;��������� ���� ����� ��������� �����
        FSTP [result]           ;��������� ����� �������� �����, ����. ST(0)

        inc ecx                 ;�������������� ������� ecx
        jmp startLoop           ;������������ �� ������ �����
endLoop:
        ret  ;�������� ���� ����� �� ������������, ������� ��� ������ ��� ���������������

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