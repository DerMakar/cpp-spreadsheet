#pragma once

#include "common.h"
#include "formula.h"
#include "sheet.h"

#include <optional>
#include <unordered_set>

class Impl {
public:
    virtual ~Impl() = default;
    virtual std::string GetText() const = 0;
    virtual CellInterface::Value GetValue(const SheetInterface& sheet) const = 0;
    virtual std::vector<Position> GetReferencedCells() const = 0;
};

class EmptyImpl : public Impl {
public:
    std::string GetText() const override;

    CellInterface::Value GetValue(const SheetInterface& sheet) const override;

    std::vector<Position> GetReferencedCells() const override;

};

class TextImpl : public Impl {
public:
    TextImpl(std::string text) : text_(text) {
    }

    std::string GetText() const override;

    CellInterface::Value GetValue(const SheetInterface& sheet) const override;

    std::vector<Position> GetReferencedCells() const override;
    
private:
    std::string text_;
};

class FormulaImpl : public Impl {
public:
    FormulaImpl(std::string text) : formula_(ParseFormula(text)) {
    }
    
    std::string GetText() const override;

    CellInterface::Value GetValue(const SheetInterface& sheet) const override;

    std::vector<Position> GetReferencedCells() const override;

 private:
    std::unique_ptr<FormulaInterface> formula_;
};

class Cell : public CellInterface {
public:
    Cell() = default;

    Cell(Sheet* sheet) : sheet_(sheet) {
    }

    ~Cell() = default;

    void SetSheet(Sheet* sheet);

    void Set(std::string text);

    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    void CircularDependency();

    bool IsReferenced() const;

    void InvalidateCash();    

private:
    Sheet* sheet_ = nullptr;
    std::unique_ptr<Impl> impl_;
    std::vector<Cell*> ref_cells; // vector Cells which are in formula
    std::vector<Cell*> parent_cells; // vector Cells which are referenced by this
    mutable std::optional<double> cashe;
    std::string set_text;

    std::vector<Cell*> MakeRefCellsPtr(const std::vector<Position>& ref_cells_pos);

    void CircularDependency(const  std::vector<Cell*>& references);
    void CircularDependency(std::unordered_set<Cell*>& counter, Cell* start);
    void AddParent(Cell* parent);
    void PopParent(Cell* parent);
 };