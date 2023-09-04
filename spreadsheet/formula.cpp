#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <string>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#DIV/0!";
}

std::optional<double> IsDigit(std::string_view text) {
    bool IsNegative = false;
    bool IsFloat = false;
    if (text[0] == '-') {
        IsNegative = true;
        text.remove_prefix(1);
    }
    for (char c : text) {
        if (!isdigit(c)) {
            if (c == '.' && !IsFloat) {
                IsFloat = true;
            }
            else {
                return std::nullopt;
            }
        }
    }
    if (IsNegative) {
       return std::stod(std::string(text.substr(1))) * (-1.0);
    }
    else {
       return std::stod(std::string(text.substr()));
    }
}

namespace {
    class Formula : public FormulaInterface {
    public:
    explicit Formula(std::string expression) : ast_ (ParseFormulaAST(expression)){
        }
           
    Value Evaluate(const SheetInterface& sheet) const override {
        std::function<std::variant<double, FormulaError>(Position)> GetValue = [&sheet](const Position pos) -> std::variant<double, FormulaError> {
            if (!pos.IsValid()) {
                return FormulaError(FormulaError::Category::Ref);
            }
            CellInterface::Value result;
            if (sheet.GetCell(pos) == nullptr) return 0.0;
            result = sheet.GetCell(pos)->GetValue();
            if (std::holds_alternative<double>(result)) {
                return std::get<double>(result);
            }
            else if (std::holds_alternative<std::string>(result)) {
                if (std::get<std::string>(result) == "") {
                    return 0.0;
                }
                std::optional<double> d_text = IsDigit(std::get<std::string>(result));
                if (d_text.has_value()) {
                    return d_text.value();
                }
                else {
                    return  FormulaError(FormulaError::Category::Value);
                }
            }
            else {
                return std::get<FormulaError>(result);
            }            
        };
        Value result = 0.0;
        try{
            result = ast_.Execute(GetValue);
        }
        catch (FormulaError& fe) {
            result = fe;
        }
        return result;
    }

    std::string GetExpression() const override {
        std::ostringstream formula;
        ast_.PrintFormula(formula);
        return formula.str();
    }

    std::vector<Position> GetReferencedCells() const override{
        std::vector<Position> result;
        for (Position pos : ast_.GetCells()) {
            if(find(result.begin(), result.end(), pos) == result.end()) result.push_back(pos);
        }
        return result;
    }
   
private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...) {
        throw FormulaException("wrong form");
    }
}