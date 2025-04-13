#include "common.h"

#include <cctype>
#include <sstream>
#include <algorithm>
#include <charconv>

const int LETTERS = 26;
const int MAX_POSITION_LENGTH = 17;
const int MAX_POS_LETTER_COUNT = 3;
const short CODE_THE_SYMBOL_BEGIN = 65;

const Position Position::NONE = {-1, -1};

bool Position::operator==(const Position rhs) const {
    return row == rhs.row && col == rhs.col;
}

bool Position::operator<(const Position rhs) const {
    return std::tie(row, col) < std::tie(rhs.row, rhs.col);
}

bool Position::IsValid() const {
    return row >= 0 && col >= 0 && row < MAX_ROWS && col < MAX_COLS;
}

std::string Position::ToString() const {

    if (!IsValid()) {
        return "";
    }

    std::string result;
    int row_res = row;
    ++row_res;
    result = ConvertColToString(col) + std::to_string(row_res);
    return result;

}
std::string Position::ConvertColToString(int val) const {
    std::string result;
    while (val >= 0) {
      char ch = CODE_THE_SYMBOL_BEGIN + (val % LETTERS);
      result = ch + result;
      val = val / LETTERS - 1;
   }
   return result;
}

Position Position::FromString(std::string_view str) {
    Position pos = Position::NONE;
    std::string num;

    int str_lenght = 0;

    for (char ch : str){
       if (std::isdigit(static_cast<unsigned char>(ch))){
           num.push_back(ch);
       }
       else{
           if (num.size() != 0)  return Position::NONE; // Проверка если после букв цифры попадаются тогда дальше обрабатывать нельзя.
           if (ch >= 'A' && ch <= 'Z'){
               if (pos.col == -1) pos.col = 0;
               pos.col *= LETTERS;
               pos.col += ch - CODE_THE_SYMBOL_BEGIN + 1;
           }else{
               return Position::NONE;
           }
           ++str_lenght;
          }
       }
    --pos.col;
    if(MAX_POSITION_LENGTH <= str_lenght){
        return Position::NONE;
    }

    if(num.size() != 0 && MAX_POSITION_LENGTH >= num.size()){
        int res_num;
        auto result = std::from_chars(num.data(), num.data() + num.size(), res_num);
        if (result.ec == std::errc::invalid_argument ||
                result.ec == std::errc::result_out_of_range)
            {
                return Position::NONE;
            }

        if (res_num > 0){
            res_num--;
            pos.row = res_num;
        }
    }

    return pos;
}

bool Size::operator==(Size rhs) const {
    return cols == rhs.cols && rows == rhs.rows;
}

FormulaError::FormulaError(Category category)
: category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    return string_view_of_errors.at(category_);
}

const std::unordered_map<FormulaError::Category, std::string_view> FormulaError::string_view_of_errors = {
        {FormulaError::Category::Arithmetic, "#ARITHM!"sv},
        {FormulaError::Category::Value, "#VALUE!"sv},
        {FormulaError::Category::Ref, "#REF!"sv}
};
