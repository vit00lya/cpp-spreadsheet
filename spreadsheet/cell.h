#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <functional>
#include <unordered_set>

class CellImpl;

class Cell : public CellInterface {
public:
    explicit Cell(SheetInterface& sheet, Position position);
    ~Cell();

    void Set(const std::string& text);
    void Clear();
    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    using PositionsSet = std::unordered_set<Position, std::hash<Position>>;

    SheetInterface& sheet_;
    std::unique_ptr<CellImpl> impl_;
    Position position_;
    PositionsSet cells_referring_me_;
    PositionsSet cells_referring_by_me_;

    void RemoveDependencies();
    void AddDependencies();
    void UpdateDependencies(PositionsSet&& cells_included_by_me_tmp);
    bool HasCircularDependencies(Position start_position, PositionsSet& visited, const PositionsSet& new_dependents) const;
    bool HasCircularDependencies(const PositionsSet& new_dependents) const;
    void InvalidateCache();
};

class CellImpl {
public:
    using Value = std::variant<std::string, double, FormulaError>;
    virtual std::string GetText() const = 0;
    virtual Value GetValue(const SheetInterface& sheet) const = 0;
    virtual bool HasCache() const;
    virtual void InvalidateCache();
    virtual std::vector<Position> GetReferencedCells() const;
    virtual ~CellImpl() = default;
};

class EmptyImpl : public CellImpl {
public:
    std::string GetText() const override { return ""s; }
    Value GetValue(const SheetInterface&) const override { return ""s; }
};

class TextImpl : public CellImpl {
public:
    explicit TextImpl(std::string expression);
    std::string GetText() const override;
    Value GetValue(const SheetInterface&) const override;
private:
    std::string value_;
};

class FormulaImpl : public CellImpl {
public:
    explicit FormulaImpl(const std::string& expression);
    std::string GetText() const override;
    Value GetValue(const SheetInterface& sheet) const override;
    virtual bool HasCache() const override;
    void InvalidateCache() override;
    std::vector<Position> GetReferencedCells() const override;
private:
    std::unique_ptr<FormulaInterface> formula_;
    mutable std::optional<Value> cache_;
};
