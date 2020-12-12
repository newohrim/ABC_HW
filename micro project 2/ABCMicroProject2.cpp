/* ВАРИАНТ 20 | Сценарий работы программы.
Говоруны бывают двух типов: одни звонят, другие ожидают звонка.
Говорит или звонит говорун - определяется случайно, однако гаранитируется, что всегда
будет хотя бы один говорун, звонящий, и один говорун, ожидающий.
Если говорун ожидает звонка, то его поток сразу завершает работу.
Если говорун, звонящий, то он случайно выбирает другого говоруна, ожидающего звонка, из списка
и встает в очередь (через мьютекс) на звонок. Время звонка определяется случайно (в коммантариях описано как).
После звонка звонящий говорун может как поменять свое состояние на ожидающего, так и остаться прежним.
Когда все говоруны завершат свой процесс (поток), программа завершится.
*/

#include <iostream>
#include <pthread.h>
#include <vector>
#include <thread>
#include <ctime>
using namespace std;

const size_t TALKERS_COUNT = 10;
//const size_t TALKERS_COUNT = 100;
//const size_t TALKERS_COUNT = 500;
//const size_t TALKERS_COUNT = 1000;
//const size_t TALKERS_COUNT = 1500;
const size_t MAX_TALK_TIME = 5;
const size_t MIN_TALK_TIME = 1;
const size_t BALANCE = 2;

// Потоки  
vector<pthread_t> threads(TALKERS_COUNT);
// Глобавльный мьютекс для вывода в консоль
pthread_mutex_t mute;

// Объект говоруна.
struct talker
{
    // Разговаривает ли говорун.
    bool isTalking = false;
    // Ждет ли говорун звонка.
    bool isAvailable = true;
    // Время разговора.
    size_t talkTime;
    // Идентификатор.
    size_t id;
    // Баланс мобильного оператора.
    size_t balance = BALANCE;
    // Локальный мьютекс.
    pthread_mutex_t local_mutex;

    talker(int id) 
    {
        pthread_mutex_init(&local_mutex, NULL);
        talkTime = rand() % (MAX_TALK_TIME - MIN_TALK_TIME) + MIN_TALK_TIME;
        isAvailable = rand() % 2;
        this->id = id;
    }

    talker(int id, bool isAvailable) 
    {
        pthread_mutex_init(&local_mutex, NULL);
        talkTime = rand() % (MAX_TALK_TIME - MIN_TALK_TIME) + MIN_TALK_TIME;
        this->isAvailable = isAvailable;
        this->id = id;
    }

    talker() {}

    // Позвонить другому говоруну.
    void talkTo(talker& another)
    {
        // Состояние позже меняется на true в takePhoneFrom.
        isTalking = false;
        // Блокировка локального мьютекса.
        pthread_mutex_lock(&mute);
        cout << "Talker " << id << ": is calling " << another.id << "." << endl;
        pthread_mutex_unlock(&mute);
        // Ожидающий звонка говорун берет трубку.
        another.takePhoneFrom(*this);
        isTalking = false;
        balance--;
        // Случайная смена состояния.
        isAvailable = rand() % 2;
        if (isAvailable) 
        {
            pthread_mutex_lock(&mute);
            cout << "Talker " << id << ": is now waiting for call..." << endl;
            pthread_mutex_unlock(&mute);
        }
    }

    // Взять трубку
    void takePhoneFrom(talker& from) 
    {
        // Блокировка локального мьютекса.
        pthread_mutex_lock(&local_mutex);
        from.isTalking = true;
        isTalking = true;
        // Время разговора определяется как max(talkTime, from.talkTime).
        int time = talkTime;
        if (from.talkTime > time)
            time = from.talkTime;
        pthread_mutex_lock(&mute);
        cout << "Talker " << id << ": took a call from " << from.id << " for " << time << " seconds." << endl;
        pthread_mutex_unlock(&mute);
        // Разговор длиной в time (засыпает звонящий поток).
        std::this_thread::sleep_for(std::chrono::seconds(time));
        isTalking = false;
        // Разблокировка локального мьютекса.
        pthread_mutex_unlock(&local_mutex);
    }
};

// Все говоруны
vector<talker> talkers(TALKERS_COUNT);
// Барьер для говорунов
pthread_barrier_t barr;

// Поиск первого говоруна, ожидающего звонка, кроме index
size_t firstActiveTalker(size_t index) 
{
    for (int i = 0; i < talkers.size(); i++) 
    {
        if (index != i && talkers[i].isAvailable)
            return i;
    }
    return -1;
}

// Основной процесс деятельности всех говорунов.
void* work(void* param)
{
    // Конкретный говорун.
    talker* currentTalker = (talker*)param;
    // Барьер, пока все потоки говорунов не будут запущены.
    pthread_barrier_wait(&barr);
    // Пока у говоруна есть баланс звонков.
    while (currentTalker->balance > 0)
    {
        // Если говорун звонящий,
        if (!currentTalker->isAvailable)
        {
            // то определяем случайный индекс говоруна, ожидающего звонка.
            size_t index;
            int count = 0;
            do
            {
                // Критический случай, если остался всего один говорун или не получается найти ожидающего говоруна.
                if (count > 100)
                {
                    index = firstActiveTalker(currentTalker->id);
                    if (index == -1 || index == currentTalker->id)
                    {
                        cout << "Talker " << currentTalker->id << ": is the only talker left with balance." << endl;
                        return NULL;
                    }
                    break;
                }
                // Задание сида для рандомизатора (достаточно случайно).
                srand((currentTalker->id + 1) * (count + 1) + currentTalker->balance);
                // Определение индеса.
                index = rand() % talkers.size();
                count++;
            } while (!talkers[index].isAvailable || talkers[index].balance == 0 || index == currentTalker->id);
            // Звонок.
            currentTalker->talkTo(talkers[index]);
        }
        else
            return NULL; // Завершение потока.
    }
    
    pthread_mutex_lock(&mute);
    cout << "Talker " << currentTalker->id << ": is out of balance." << endl;
    pthread_mutex_unlock(&mute);
    // Перевод в режим ожидания звонка.
    currentTalker->isAvailable = true;
    return NULL;
}

// Входная точка программы.
int main()
{
    // Хотя бы 3 говоруна, иначе модель не показательна.
    if (TALKERS_COUNT <= 2)
    {
        cout << "TALKERS_COUNT must be at least 3." << endl;
        return EXIT_FAILURE;
    }
    srand(time(NULL));
    // Инициаллизация барьера.
    pthread_barrier_init(&barr, nullptr, TALKERS_COUNT);
    // Инициаллизация глобального мьютекса для вывода в консоль.
    pthread_mutex_init(&mute, NULL);
    // Инициаллизация первых двух говорунов: первый ожидает звонка, второй звонит.
    talkers[0] = talker(0, true);
    cout << "Talker 0: WAITING" << endl;
    talkers[1] = talker(1, false);
    cout << "Talker 1: CALLING" << endl;
    for (size_t i = 2; i < TALKERS_COUNT; i++)
    {
        // Инициаллизация остальных говорунов (состояние определяется случайно в конструкторе).
        talkers[i] = talker(i);
        if (talkers[i].isAvailable)
            cout << "Talker " << i << ": WAITING" << endl;
        else
            cout << "Talker " << i << ": CALLING" << endl;
    }
    // Создание потоков.
    for (size_t i = 0; i < TALKERS_COUNT; i++)
        pthread_create(&threads[i], NULL, work, &talkers[i]);
    // Ожидание завершения всех потоков.
    for (size_t i = 0; i < TALKERS_COUNT; i++)
        pthread_join(threads[i], nullptr);
    // Корректное завершение программы.
    return EXIT_SUCCESS;
}