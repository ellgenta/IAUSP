#include "TokenGenerator.cpp"
#include "SearchRanking.cpp"

/*
	Ёто точка входа в консольный проект.ќн будет таковым, пока не заработает как надо, потом перейдет
	на веб версию. ” теб€ есть несколько локальных файлов .txt . “ебе нужно их загрузить сюда, потом

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