#include <iostream>
#include <queue>
#include <stack>
#include <pthread.h>
#include <windows.h>

using namespace std;
// Потоки
pthread_t threadFisrt, threadSecond, threadThird;
// Количество деталей
const size_t COUNT = 100;
// Структура детали
struct pin
{
    // Кривая ли деталь
    bool isCurved;
    // Качественная ли заточка
    bool isGoodQuality;

    pin()
    {
        isCurved = rand() % 2;
    }
};
// Контейнер с произведенными деталями
stack<pin> producted;
// Третий рабочий
void* thirdWorker(void* param)
{
    // Задержка на выполнение операции
    Sleep(500);
    pin* takenPin = (pin*)param;
    // Проверка на качество
    if (takenPin->isGoodQuality)
    {
        // Конец производства
        cout << "Рабочий С: проверил качество детали и закончил ее производство." << endl;
        producted.push(*takenPin);
    }

    else
    {
        // Брак
        cout << "Рабочий C: качество детали было низкое, поэтому она забракована." << endl;
    }
    return NULL;
}
// Второй рабочий
void* secondWorker(void* param)
{
    pin* takenPin = (pin*)param;
    // Заточка
    takenPin->isGoodQuality = rand() % 2;
    Sleep(500);
    // Передача детали третьему рабочему
    cout << "Рабочий B: заточил деталь и передал третьему рабочему." << endl;
    pthread_join(threadThird, NULL);
    pthread_create(&threadThird, NULL, thirdWorker, takenPin);
    return NULL;
}
// Первый рабочий 
void* firstWorker(void* param)
{
    queue<pin>* conveer = (queue<pin>*)param;
    while (!conveer->empty())
    {
        pin takenPin = conveer->front();
        conveer->pop();
        // Задержка на выполнение операции
        Sleep(500);
        // Проверка на кривую деталь
        if (!takenPin.isCurved)
        {
            cout << "Рабочий A: передал деталь второму рабочему." << endl;
            // Передает второму рабочему
            pthread_create(&threadSecond, NULL, secondWorker, &takenPin);
            pthread_join(threadSecond, NULL);
            pthread_join(threadThird, NULL);
        }
        else
            cout << "Рабочий A: взятая деталь оказалась кривой." << endl;
    }
    return NULL;
}

// Входная точка программы
int main()
{
    setlocale(LC_ALL, "Russian");
    // Конвеер в виде очереди
    queue<pin> conveer;
    for (int i = 0; i < COUNT; i++)
        conveer.emplace(pin());
    // Создание потока первого рабочего
    pthread_create(&threadFisrt, NULL, firstWorker, &conveer);
    pthread_join(threadFisrt, NULL);
    // Всего произведено
    cout << "Producted: " << producted.size() << endl;
    // Выход из программы
    return 0;
}