#include "TokenGenerator.cpp"

class SearchRequest
{
private:
	porter_stemmer_tokenization ps;
public:
	void write_request(std::string& request)
	{
		const std::string& tokenized_request = ps.tokenize(request);

		// ��������� � ������ ������ � ������� �������� ����������, ��� �������� ����� �� ������� �����������


	}
};