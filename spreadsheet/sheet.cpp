#include "sheet.h"
#include "cell.h"
#include "common.h"
#include <algorithm>
#include <iostream>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) throw InvalidPositionException("Недопустимая позиция ячейки."s);
    Cell* cell_existing = static_cast<Cell*>(GetCell(pos));
    if (cell_existing == nullptr) {
        std::unique_ptr<Cell> cell = std::make_unique<Cell>(*this, pos);
        cell->Set(text);
        data_[pos] = std::move(cell);
    } else {
        if (cell_existing->GetText() == text) return;
        cell_existing->Set(text);
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) throw InvalidPositionException("Недопустимая позиция ячейки."s);
    if (data_.count(pos) > 0) {
        return data_.at(pos).get();
    } else {
        return nullptr;
    }
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) throw InvalidPositionException("Недопустимая позиция ячейки."s);
    if (data_.count(pos) > 0) {
        return data_[pos].get();
    } else {
        return nullptr;
    }
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) throw InvalidPositionException("Недопустимая позиция ячейки."s);
    if (data_.count(pos) > 0) {
        data_[pos]->Clear();
        data_.erase(pos);
    }
}

Size Sheet::GetPrintableSize() const {
    if (data_.empty()) return {0,0};
    int max_row = 0;
    int max_col = 0;
    for (auto& it : data_) {
        max_row = std::max(max_row, it.first.row);
        max_col = std::max(max_col, it.first.col);
    }
    return {max_row + 1, max_col + 1};
}


void Sheet::Print(std::ostream& output, TypePrint type_print) const {
    if (data_.empty()) {
           output << ""s;
           return;
       }

       Size printable_area = GetPrintableSize();
       for (int row = 0; row < printable_area.rows; ++row)
       {
           for (int col = 0; col < printable_area.cols; ++col)
           {
               const CellInterface *cell = GetCell(
               { row, col });
               if (cell)
               {
                   if(type_print == TypePrint::VALUE) {
                       PrintValue(cell, output);
                   }
                   else if (type_print == TypePrint::TEXT){
                       output << cell->GetText();
                   }
                   else{
                       throw std::runtime_error("Выбран не верный тип значений для вывода на экран.");
                   }
               }
               else
               {
                   output << "";
               }
               if (col < printable_area.cols - 1)
               {
                   output << '\t';
               }
           }
           output << '\n';
       }
}

void Sheet::PrintValues(std::ostream& output) const {
    Print(output, TypePrint::VALUE);
}

void Sheet::PrintTexts(std::ostream& output) const {
    Print(output, TypePrint::TEXT);
}

void Sheet::PrintValue(const CellInterface* cell, std::ostream& output) const{
    CellInterface::Value val = cell->GetValue();
    if (std::holds_alternative<double>(val)){
        output << std::get<double>(val);
    }
    else if(std::holds_alternative<std::string>(val)){
        output << std::get<std::string>(val);
    }
    else if(std::holds_alternative<FormulaError>(val)){
        output << std::get<FormulaError>(val);
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
