#include "TokenGenerator.cpp"
#include "SearchRanking.cpp"

/*
	��� ����� ����� � ���������� ������.�� ����� �������, ���� �� ���������� ��� ����, ����� ��������
	�� ��� ������. � ���� ���� ��������� ��������� ������ .txt . ���� ����� �� ��������� ����, �����

*/

int main()
{
	porter_stemmer_tokenization ps;
	search_ranking search();

	std::string user_input;
	while (true) {
		std::cin >> user_input;
		std::cout << ps.tokenize(user_input) << " ";

	}

	return 0;
}