#ifndef SYMTAB_HH_
#define SYMTAB_HH_

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "tree.hh"

struct Declaration
{
    Declaration(const std::string& id)
        : id(id), type(), arity()
    {}

    Declaration(const std::string& id, TypePtr type)
        : id(id), type(type), arity()
    {}

    Declaration(const std::string& id, std::optional<TypePtr> type)
        : id(id), type(type), arity()
    {}

    Declaration(const std::string& id, TypePtr type, size_t arity)
        : id(id), type(type), arity(arity)
    {}

    Declaration(const std::string& id, std::optional<TypePtr> type, size_t arity)
        : id(id), type(type), arity(arity)
    {}
    
    std::string id;
    std::optional<TypePtr> type;
    std::optional<size_t> arity;
    // SourceLocation location;
};

class SymbolTable
{
private:
    using ScopeTable = std::unordered_map<std::string, Declaration>;
public:
    SymbolTable()
        : m_scope_level(0), m_table()
    {
        m_table.push_back(std::make_unique<ScopeTable>());
    }

    SymbolTable(SymbolTable& other)
        : m_scope_level(0), m_table(std::move(other.m_table))
    {
        // Copy ctor
    }

    SymbolTable(SymbolTable&& other)
        : m_scope_level(0), m_table(std::move(other.m_table))
    {
        // Move ctor
    }

    ~SymbolTable() { }

    void enter_scope()
    {
        m_table.push_back(std::make_unique<ScopeTable>());
        ++m_scope_level;
    }

    void exit_scope()
    {
        m_table.pop_back();
        --m_scope_level;
    }

    bool insert(Declaration decl)
    {
        auto [_, inserted] = m_table.back()->insert({decl.id, decl});
        return inserted;
    }

    std::optional<Declaration> lookup(const std::string& name)
    {
        for (int i = (int) m_table.size() - 1; i >= 0; --i) {
            auto it = m_table[i]->find(name);
            if (it != m_table[i]->end())
                return it->second;
        }
        return {};
    }

    size_t scope_level()
    {
        return m_scope_level;
    }

    void trace_table()
    {
        for (int i = m_table.size() - 1; i >= 0; --i) {
            fmt::println("Scope level {}:", i);
            for (auto& [name, decl] : *m_table[i]) {
                fmt::println("  Declared name {}", name);
            }
        }
    }
private: 
    size_t m_scope_level; // 0 for global
    std::vector<std::unique_ptr<ScopeTable>> m_table;
};

struct TreeSymtabVisitor : ITreeVisitor
{
    TreeSymtabVisitor();
    
    virtual void visit(TreeSeqNode *node);
    virtual void visit(TreeParamsNode *node);
    virtual void visit(TreeListNode *node);
    virtual void visit(TreeBindingNode *node);
    virtual void visit(TreeBinopNode *node);
    virtual void visit(TreeApplyNode *node);
    virtual void visit(TreeIdentNode *node);
    virtual void visit(TreeIntegerNode *node);
    virtual void visit(TreeStringNode *node);

    SymbolTable table;
};

#endif // SYMTAB_HH_
