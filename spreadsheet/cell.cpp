#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>


// Реализуйте следующие методы

std::string EmptyImpl::GetText() const {
        return "";
    }

    CellInterface::Value EmptyImpl::GetValue(const SheetInterface& sheet) const {
        return "";
    }

    std::vector<Position> EmptyImpl::GetReferencedCells() const {
        return {};
    }



    std::string TextImpl::GetText() const {
        return text_;
    }

    CellInterface::Value TextImpl::GetValue(const SheetInterface& sheet) const {
        if (text_[0] == ESCAPE_SIGN) {
            return text_.substr(1);
        }
        return text_;
    }

    std::vector<Position> TextImpl::GetReferencedCells() const {
        return {};
    }


    std::string FormulaImpl::GetText() const {
        return FORMULA_SIGN + formula_->GetExpression();
    }

    std::vector<Position> FormulaImpl::GetReferencedCells() const {
        return formula_->GetReferencedCells();
    }

    CellInterface::Value FormulaImpl::GetValue(const SheetInterface& sheet) const {
        FormulaInterface::Value result = formula_->Evaluate(sheet);
        if (std::holds_alternative<double>(result)) {
            return std::get<double>(result);
        }
        else {
            return std::get<FormulaError>(result);
        }
    }


    void Cell::Set(std::string text) {
        if (!set_text.empty() && text == set_text) return;
        if (text.empty()) {
            impl_ = std::unique_ptr<EmptyImpl>(nullptr);           
        }
        else if (text[0] == FORMULA_SIGN && text.size() > 1) {
           try {
                FormulaImpl new_formula(text.substr(1));
                const auto& references = new_formula.GetReferencedCells();
                std::vector<Cell*> nf_ref_cells = MakeRefCellsPtr(references);
                try {
                    CircularDependency(nf_ref_cells);
                }
                catch (CircularDependencyException&) {
                    throw;
                }
                impl_ = std::make_unique<FormulaImpl>(std::move(new_formula));
                ref_cells = nf_ref_cells;
                for (Cell* cell : ref_cells) {
                    if (cell == nullptr) continue;
                    cell->AddParent(cell);
                }
            }
           catch (CircularDependencyException&) {
               throw;
           }
           catch(...){
                throw FormulaException("");
            }
            
        }
        else {
            impl_ = std::make_unique<TextImpl>(text);  
            ref_cells.clear();
        }
        set_text = text;
    }

    void Cell::Clear() {
        impl_.release();
        for (Cell* cell : ref_cells) {
            cell->PopParent(this);            
        }
        ref_cells.clear();
        parent_cells.clear();
        cashe.reset();
    }

    Cell::Value Cell::GetValue() const  {
        if (impl_ == nullptr) return 0.0;
        return impl_->GetValue(*sheet_);
    }
    std::string Cell::GetText() const {
        if (impl_ == nullptr) {
            return "";
        }
        return impl_->GetText();
    }

    void Cell::InvalidateCash() {
        if (cashe.has_value()) {
            cashe.reset();
            for (Cell* cell : parent_cells) {
                cell->InvalidateCash();
            }
        }
    }

    std::vector<Cell*> Cell::MakeRefCellsPtr(const std::vector<Position>& ref_cells_pos) {
        std::vector<Cell*> result;
        result.reserve(ref_cells_pos.size());
        for (Position pos : ref_cells_pos) {
            sheet_->SetEmptyCell(pos);
            result.push_back(dynamic_cast<Cell*>(sheet_->GetCell(pos)));
        }
        return result;

    }

    void Cell::CircularDependency(const  std::vector<Cell*>& references) {
        std::unordered_set<Cell*> counter;
        counter.insert(this);
        for (Cell* cell : references) {
            if (cell == nullptr) continue;
            if (!counter.count(cell)) {
                counter.insert(cell);
                cell->CircularDependency(counter, this);
            }
            if (cell == this) throw CircularDependencyException("IsCircle");
        }
    }

    void Cell::CircularDependency(std::unordered_set<Cell*>& counter, Cell* start){
        for (Cell* cell : ref_cells) {
            if (!counter.count(cell)) {
                counter.insert(cell);
                cell->CircularDependency(counter, start);
            }
            if (cell == start) throw CircularDependencyException("IsCircle");
        }
    }
    
    void Cell::AddParent(Cell* parent) {
        parent_cells.push_back(parent);
    }

    void Cell::PopParent(Cell* parent) {
        parent_cells.erase(find(parent_cells.begin(), parent_cells.end(), parent));
    }

    void Cell::CircularDependency() {
        CircularDependency(ref_cells);
    }

    void Cell::SetSheet(Sheet* sheet) {
        sheet_ = sheet;
    }

    bool Cell::IsReferenced() const {
        return !ref_cells.empty();
    }
    
    std::vector<Position> Cell::GetReferencedCells() const {
        if (impl_ == nullptr) return {};
        return impl_->GetReferencedCells();
    }