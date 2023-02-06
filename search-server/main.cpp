// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь:

#include <iostream>
#include <string>

using namespace std;

int main() {
	int answer = 0;
	string str;
	for (int count = 1; count <= 1000; ++count) {
		str = to_string(count);
		for (char item : str) {
			if (item == '3') {
				answer += 1;
				break;
			}
		}
	}
	cout << answer;

	return 0;
}

// Закомитьте изменения и отправьте их в свой репозиторий.
