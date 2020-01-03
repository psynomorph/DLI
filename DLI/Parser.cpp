#include "Parser.h"

// Получть следующую лексему - определенное ключевое слово
// Если она не является таковой, то ошибка
KeywordToken* Parser::GetKeyword(const std::string &str)
{
	auto token = GetToken<KeywordToken>();
	if (token->GetKeyword() != str) throw std::exception("Unexpected keyword");
	return token;
}

// Получить следующую лексему
Token* Parser::NextToken()
{
	if (it != end)
	{
		auto token = *it;
		it++;
		return token;
	}
	throw std::exception("Unexpected end of file");
}

// Создать синтаксический анализатор
// tokens - последовательность лексем
Parser::Parser(std::list<Token*>& tokens)
{
	it = tokens.begin();
	end = tokens.end();
}

// Выполнить синтаксический разбор
Expression* Parser::Parse()
{
	return ParseExpression();
}

// Базовый метод рекурсивного спуска
// Читаем произвольное выражение
Expression* Parser::ParseExpression()
{
	// Читаем открывающуюся скобку
	static_cast<void>(GetToken<OpenBracketToken>());
	// Затем ключевое слово
	auto keyword = GetToken<KeywordToken>();
	Expression* expression = nullptr;

	const std::string& str = keyword->GetKeyword();

	// По значению ключевого слова определяем тип выражения
	// и вызываем для его разбора соответствующий метод
	if (str == "block")
	{
		expression = (Expression*) ParseBlockExpression();
	} 
	else if (str == "val") 
	{
		expression = (Expression*) ParseValExpression();
	}
	else if (str == "let") {
		expression = (Expression*) ParseLetExpression();
	}
	else if (str == "var") {
		expression = (Expression*)ParseVarExpression();
	}
	else if (str == "add") {
		expression = (Expression*)ParseAddExpression();
	}
	else if (str == "if") {
		expression = (Expression*)ParseIfxpression();
	}
	else if (str == "function") {
		expression = (Expression*)ParseFunctionExpression();
	}
	else if (str == "call") {
		expression = (Expression*)ParseCallExpression();
	}
	else if (str == "set") {
		expression = (Expression*)ParseSetExpression();
	}

	// Читаем закрывающуюся скобку
	static_cast<void>(GetToken<CloseBracketToken>());
	return expression;
}

// Читаем выражение (val <integer>)
ValExpression* Parser::ParseValExpression()
{
	auto valueToken = GetToken<ValueToken>();
	int value = valueToken->GetValue();
	return new ValExpression(value);
}

// Читаем выражение (block <expression>+)
BlockExpression* Parser::ParseBlockExpression()
{
	BlockExpression * expr = new BlockExpression();

	// Заглядываем на лексему вперёд
	do {
		auto token = NextToken();
		// Пока не найдём закрывающуюся скобку
		auto closeBracket = dynamic_cast<CloseBracketToken*>(token);
		
		it--;
		
		// Если нашли
		if (closeBracket) {
			break; // То заканчиваем чтение выражения
		} else {
			// Если нет, читаем вложенные выражения
			Expression* nestedExpression = ParseExpression();
			expr->AddExpression(nestedExpression);
		}

	} while (true);

	// Блок выражения не может быть пустым
	if (expr->GetExpressions().size() == 0)
	{
		delete expr;
		throw new std::exception("Block epxression should have one nested expression at least");
	}

	return expr;
}

// Читаем выражение (let <id> = <expression> in <expression>)
LetExpression* Parser::ParseLetExpression()
{
	auto id = GetToken<IdentifierToken>();
	static_cast<void>(GetToken<AssignOperatorToken>());
	auto expression = ParseExpression();
	static_cast<void>(GetKeyword("in"));
	auto body = ParseExpression();
	return new LetExpression(id->GetId(), expression, body);
}

// Читаем выражение (var <id>)
VarExpression* Parser::ParseVarExpression()
{
	auto token = GetToken<IdentifierToken>();
	return new VarExpression(token->GetId());
}

// Читаем выражение (add <expression> <expression>)
AddExpression* Parser::ParseAddExpression()
{
	auto left = ParseExpression();
	auto right = ParseExpression();
	return new AddExpression(left, right);
}

// Читаем выражение (if <expression> <expression> then <expression> else <expression>)
IfExpression* Parser::ParseIfxpression()
{
	auto left = ParseExpression();
	auto right = ParseExpression();
	static_cast<void>(GetKeyword("then"));
	auto trueBranch = ParseExpression();
	static_cast<void>(GetKeyword("else"));
	auto elseBranch = ParseExpression();
	return new IfExpression(left, right, trueBranch, elseBranch);
}

// Читаем выражение (function <id> <expression>)
FunctionExpression* Parser::ParseFunctionExpression()
{
	auto id = GetToken<IdentifierToken>();
	auto body = ParseExpression();
	return new FunctionExpression(id->GetId(), body);
}

// Читаем выражение (call <expression> <expression>)
CallExpression* Parser::ParseCallExpression()
{
	auto function = ParseExpression();
	auto argument = ParseExpression();
	return new CallExpression(function, argument);
}

// Читаем выражение (set <id> <expression>)
SetExpression* Parser::ParseSetExpression()
{
	auto id = GetToken<IdentifierToken>();
	auto val = ParseExpression();
	return new SetExpression(id->GetId(), val);
}