#include <iostream>
#include <queue>
#include <omp.h>
using namespace std;

const size_t pinsCount = 100;
//const size_t pinsCount = 1000;
//const size_t pinsCount = 10000;
// Прогресс рабочих (0 - все работают, 1 - A закончил работу, 2 - C и B закончили работу)
int status = 0;

// Класс детали
struct pin 
{
	// Идентификатор детали
	int id = 0;
	// Является ли деталь кривой
	bool isCurved = false;
	// Качественная ли заточка детали
	bool isGoodQuality = false;

	pin(int id) 
	{
		isCurved = rand() % 2;
		this->id = id;
	}
};

// Рабочий A (производитель)
void first_worker(queue<pin>& input, queue<pin>& output, queue<pin>& defective) 
{
	while (!input.empty()) 
	{
		// Берется деталь
		pin takenPin = input.front();
		input.pop();
		if (!takenPin.isCurved)
		{
			// Передается рабочему B
			output.push(takenPin);
			#pragma omp critical
			{
				cout << "Worker A: Проверил деталь " << takenPin.id << " и передал рабочему B" << endl;
			}
		}
		else
		{
			// Бракуется
			defective.push(takenPin);
			#pragma omp critical 
			{
				cout << "Worker A: Деталь " << takenPin.id << " оказалась кривой." << endl;
			}
		}
		
		status = 0;
	}
	cout << "Worker A: done" << endl;
	status = 1;
}

// Рабочий B (потребитель для A и производитель для C)
void second_worker(queue<pin>& input, queue<pin>& output) 
{
	while (true) 
	{
		while (!input.empty()) 
		{
			// Берется деталь
			pin takenPin = input.front();
			input.pop();
			// Заточка и передача рабочему C
			takenPin.isGoodQuality = rand() % 2;
			output.push(takenPin);
			#pragma omp critical
			{
				cout << "Worker B: Заточил деталь " << takenPin.id << " и передал ее рабочему C." << endl;
			}
		}
		if (status == 2)
			break;
	}
	cout << "Worker B: done" << endl;
}

// Рабочий C (потребитель)
void third_worker(queue<pin>& input, queue<pin>& secondWorkerInput, queue<pin>& output) 
{
	while (true) 
	{
		while (!input.empty()) 
		{
			// Берется деталь
			pin takenPin = input.front();
			input.pop();
			if (takenPin.isGoodQuality)
			{
				// Заканчивает производство детали
				output.push(takenPin);
				#pragma omp critical
				{
					cout << "Worker C: Закончил производство детали " << takenPin.id << "." << endl;
				}
				// Условие окончания работы цеха
				if (input.empty() && secondWorkerInput.empty() && status == 1)
					status = 2;
			}
			else
			{
				// Возвращается рабочему B на доработку
				secondWorkerInput.push(takenPin);
				#pragma omp critical
				{
					cout << "Worker C: Остался недоволен работой рабочего B. "
						<< "Возвращает деталь " << takenPin.id << " рабочему B." << endl;
				}
			}
		}
		if (status == 2)
			break;
	}
	cout << "Worker C: done" << endl;
}

// Входная точка программы
int main()
{
	setlocale(LC_ALL, "Russian");
	if (pinsCount == 0) 
	{
		cout << "Всего произведено: 0" << endl;
		cout << "Всего брака: 0" << endl;
		return 0;
	}
	// Обнуление состояния
	status = 0;
	// Вход для рабочего A
	queue<pin> conveer1;
	for (int i = 0; i < pinsCount; i++)
		conveer1.emplace(pin(i + 1));
	// Вход для рабочего B
	queue<pin> conveer2;
	// Вход для рабочего C
	queue<pin> conveer3;
	// Произведенные детали
	queue<pin> producted;
	// Забракованные детали
	queue<pin> defective;

	// Параллелизм по секциям в три потока
	#pragma omp parallel sections num_threads(3)
	{
		#pragma omp section 
		{
			first_worker(conveer1, conveer2, defective);
		}
		#pragma omp section 
		{
			second_worker(conveer2, conveer3);
		}
		#pragma omp section 
		{
			third_worker(conveer3, conveer2, producted);
		}
	}
	// Итог
	cout << "Всего произведено: " << producted.size() << endl;
	cout << "Всего брака: " << defective.size() << endl;
	// Выход из программы.
	return 0;
}