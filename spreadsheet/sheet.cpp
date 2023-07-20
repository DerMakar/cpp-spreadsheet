#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

bool Sheet::IsValid(Position pos) const {
    return sheet_.size() > size_t(pos.row) && sheet_[pos.row].size() > size_t(pos.col);
}


void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
   
    if (pos.row + 1 > sheet_size_.rows) {
        if (sheet_.size() < size_t(pos.row + 1)) sheet_.resize(pos.row + 1);
        if (sheet_[pos.row].size() < size_t(pos.col + 1)) sheet_[pos.row].resize(pos.col + 1);
    }
    if (sheet_[pos.row].size() < size_t(pos.col + 1)) {
        sheet_[pos.row].resize(std::max(pos.col + 1, sheet_size_.cols));
    }

    if (sheet_[pos.row][pos.col] == nullptr) {
        sheet_[pos.row][pos.col] = std::make_unique<Cell>();
        sheet_[pos.row][pos.col]->SetSheet(this);
    }
    sheet_[pos.row][pos.col]->Set(text);
    sheet_[pos.row][pos.col]->InvalidateCash();
    sheet_size_.rows = std::max(sheet_size_.rows, pos.row + 1);
    sheet_size_.cols = std::max(sheet_size_.cols, pos.col + 1);
}

void Sheet::SetEmptyCell(Position pos) {
    if (!pos.IsValid()) throw InvalidPositionException("");
    if (size_t(pos.row) + 1 > sheet_.size()) sheet_.resize(pos.row + 1);
    if (sheet_[pos.row].size() < size_t(pos.col + 1)) {
        sheet_[pos.row].resize(pos.col + 1);
        sheet_[pos.row][pos.col] = std::make_unique<Cell>();
        sheet_[pos.row][pos.col]->Set("");
        sheet_[pos.row][pos.col]->SetSheet(this);       
    }
}

const CellInterface* Sheet::GetCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (IsValid(pos)) {
        if (size_t(pos.row) + 1 > sheet_.size() ||
            sheet_[pos.row].size() < size_t(pos.col + 1)) return nullptr;
        Cell* value = sheet_[pos.row][pos.col].get();
        return value;
    }
    return nullptr;
}

CellInterface* Sheet::GetCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (IsValid(pos)) {
        Cell* value = sheet_[pos.row][pos.col].get();
        return value;
    }
    return nullptr;
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("");
    }
    if (IsValid(pos) && sheet_[pos.row][pos.col] != nullptr) {
        sheet_[pos.row][pos.col].release();
        if (sheet_size_.rows == pos.row + 1) {
            if (sheet_size_.rows == 1) {
                sheet_size_.rows = 0;
            }
            for (int i = sheet_size_.rows - 2; i >= 0; --i) {
                if (find_if(sheet_[i].begin(), sheet_[i].end(), [](const auto& el) { return el.get() != nullptr; }) != sheet_[i].end()) {
                    sheet_size_.rows = i+1;
                    break;
                }
            }
        }
        if (sheet_size_.cols == pos.col + 1) {
            size_t max_col = 0;
            for (int i = 0; i < sheet_size_.rows; ++i) {
                for (size_t m = sheet_[i].size() - 1; int(m) >= 0; --m) {
                    if (sheet_[i][m] != nullptr && m + 1 > max_col) {
                        max_col = m + 1;
                        continue;
                    }
                }
            }
            sheet_size_.cols = max_col;
        }               
    }
    
}

Size Sheet::GetPrintableSize() const {
    return sheet_size_;
}


void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < sheet_size_.rows; ++i) {
        for (int m = 0; m < sheet_size_.cols; ++m) {
           if (size_t(m) < sheet_[i].size() && sheet_[i][m] != nullptr) {
               std::visit(
                   [&](const auto& x) {
                       output << x;
                   },
                   sheet_[i][m]->GetValue());
            }
            if (m != sheet_size_.cols - 1) output << "\t";
        }
        output << "\n";
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    //std::cout << "start print" << std::endl;
    for (int i = 0; i < sheet_size_.rows; ++i) {
        for (int m = 0; m < sheet_size_.cols; ++m) {
            //std::cout << "printing " << i << " " << m << " elem" << std::endl;
            if (size_t(m) < sheet_[i].size() && sheet_[i][m] != nullptr) {
                output << sheet_[i][m]->GetText();
            }
            if(m != sheet_size_.cols - 1) output << "\t";
        }
       output << "\n";
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}