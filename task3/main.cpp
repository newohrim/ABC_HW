#include <iostream>
#include <queue>
#include <stack>
#include <pthread.h>
#include <windows.h>

using namespace std;
// ������
pthread_t threadFisrt, threadSecond, threadThird;
// ���������� �������
const size_t COUNT = 100;
// ��������� ������
struct pin
{
    // ������ �� ������
    bool isCurved;
    // ������������ �� �������
    bool isGoodQuality;

    pin()
    {
        isCurved = rand() % 2;
    }
};
// ��������� � �������������� ��������
stack<pin> producted;
// ������ �������
void* thirdWorker(void* param)
{
    // �������� �� ���������� ��������
    Sleep(500);
    pin* takenPin = (pin*)param;
    // �������� �� ��������
    if (takenPin->isGoodQuality)
    {
        // ����� ������������
        cout << "������� �: �������� �������� ������ � �������� �� ������������." << endl;
        producted.push(*takenPin);
    }

    else
    {
        // ����
        cout << "������� C: �������� ������ ���� ������, ������� ��� �����������." << endl;
    }
    return NULL;
}
// ������ �������
void* secondWorker(void* param)
{
    pin* takenPin = (pin*)param;
    // �������
    takenPin->isGoodQuality = rand() % 2;
    Sleep(500);
    // �������� ������ �������� ��������
    cout << "������� B: ������� ������ � ������� �������� ��������." << endl;
    pthread_join(threadThird, NULL);
    pthread_create(&threadThird, NULL, thirdWorker, takenPin);
    return NULL;
}
// ������ ������� 
void* firstWorker(void* param)
{
    queue<pin>* conveer = (queue<pin>*)param;
    while (!conveer->empty())
    {
        pin takenPin = conveer->front();
        conveer->pop();
        // �������� �� ���������� ��������
        Sleep(500);
        // �������� �� ������ ������
        if (!takenPin.isCurved)
        {
            cout << "������� A: ������� ������ ������� ��������." << endl;
            // �������� ������� ��������
            pthread_create(&threadSecond, NULL, secondWorker, &takenPin);
            pthread_join(threadSecond, NULL);
            pthread_join(threadThird, NULL);
        }
        else
            cout << "������� A: ������ ������ ��������� ������." << endl;
    }
    return NULL;
}

// ������� ����� ���������
int main()
{
    setlocale(LC_ALL, "Russian");
    // ������� � ���� �������
    queue<pin> conveer;
    for (int i = 0; i < COUNT; i++)
        conveer.emplace(pin());
    // �������� ������ ������� ��������
    pthread_create(&threadFisrt, NULL, firstWorker, &conveer);
    pthread_join(threadFisrt, NULL);
    // ����� �����������
    cout << "Producted: " << producted.size() << endl;
    // ����� �� ���������
    return 0;
}