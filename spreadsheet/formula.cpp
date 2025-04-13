#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <sstream>
#include <charconv>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {

    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression);
        Value Evaluate(const SheetInterface &sheet) const override;
        std::string GetExpression() const override;
        std::vector<Position> GetReferencedCells() const override;

    private:
        FormulaAST ast_;
        std::vector<Position> referenced_cells_;
        double GetCellValueAsDouble(const SheetInterface& sheet, Position pos) const;
    };

    Formula::Formula(std::string expression)
    try : ast_(ParseFormulaAST(expression)),
          referenced_cells_(ast_.GetCells().begin(), ast_.GetCells().end())
    {
        auto end_iterator = std::unique(referenced_cells_.begin(), referenced_cells_.end());
        referenced_cells_.resize(end_iterator - referenced_cells_.begin());
    } catch (std::exception& error) {
        throw FormulaException("Некорректная формула: "s.append(error.what()));
    }

    FormulaInterface::Value Formula::Evaluate(const SheetInterface& sheet) const {

        double result;
        try {
            result = ast_.Execute([this, &sheet](Position pos) -> double {
                return GetCellValueAsDouble(sheet, pos);
            });
        } catch (FormulaError &err) {
            return err;
        }
        return result;
    }

    double Formula::GetCellValueAsDouble(const SheetInterface& sheet, Position pos) const {
        const CellInterface* cell = sheet.GetCell(pos);
        if (cell == nullptr) return 0.0;

        CellInterface::Value value = cell->GetValue();
        if (std::holds_alternative<double>(value)) {
            return std::get<double>(value);
        }

        if (std::holds_alternative<std::string>(value)) {
            std::string string_value = std::get<std::string>(value);
            if (string_value.empty()) return 0.0;

            for (char ch : string_value){ // Строка должна состоять только из чисел, почему-то from_chars преобразует строку 3C в 3
                if (!std::isdigit(static_cast<unsigned char>(ch))){
                    throw FormulaError(FormulaError::Category::Value);
                }
            }
            int res_num;
            auto result = std::from_chars(string_value.data(),
                                          string_value.data() + string_value.size(),
                                          res_num);

            if (result.ec == std::errc::invalid_argument ||
                    result.ec == std::errc::result_out_of_range)
                {
                throw FormulaError(FormulaError::Category::Value);
                }

            return res_num;
        }

        throw FormulaError(std::get<FormulaError>(value));
    }

    std::string Formula::GetExpression() const {
        std::stringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

    std::vector<Position> Formula::GetReferencedCells() const {
        return referenced_cells_;
    }
}

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
        return std::make_unique<Formula>(std::move(expression));
}
