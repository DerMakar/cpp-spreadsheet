#pragma once


#include "common.h"

#include <functional>
#include <deque>

class Cell;

class Sheet : public SheetInterface {
public:
    Sheet() = default;

    ~Sheet() = default;

    void SetCell(Position pos, std::string text) override;

    void SetEmptyCell(Position pos);

    const CellInterface* GetCell(Position pos) const override;
    
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    
    void PrintTexts(std::ostream& output) const override;

    bool IsValid(Position pos) const;

private:
    mutable std::vector<std::vector<std::unique_ptr<Cell>>> sheet_;
    Size sheet_size_;
};

