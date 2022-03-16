#ifndef SFA_CONFIG_UTIL_HPP
#define SFA_CONFIG_UTIL_HPP

#include <string>
#include <unordered_set>
#include <unordered_map>

#include "sfa/core/Element.hpp"
#include "sfa/sup/EnumHash.hpp"
#include "sfa/sup/TokenIterator.hpp"
#include "sfa/sup/ConfigTokenizer.hpp"

struct ElementTypeInfo final
{
    static const ElementTypeInfo i8;
    static const ElementTypeInfo i16;
    static const ElementTypeInfo i32;
    static const ElementTypeInfo i64;
    static const ElementTypeInfo u8;
    static const ElementTypeInfo u16;
    static const ElementTypeInfo u32;
    static const ElementTypeInfo u64;
    static const ElementTypeInfo f32;
    static const ElementTypeInfo f64;
    static const ElementTypeInfo boolean;

    static const std::unordered_map<std::string, ElementTypeInfo> fromName;

    static const std::unordered_map<ElementType, ElementTypeInfo, EnumHash>
        fromEnum;

    ElementType enumVal;
    std::string name;
    U32 sizeBytes;
    bool arithmetic;
    bool fp;
    bool sign;
};

struct OperatorInfo final
{
    enum Type : U8
    {
        OP_NOT,
        OP_MULT,
        OP_DIV,
        OP_ADD,
        OP_SUB,
        OP_LT,
        OP_LTE,
        OP_GT,
        OP_GTE,
        OP_EQ,
        OP_NEQ,
        OP_AND,
        OP_OR
    };
    
    static const OperatorInfo lnot;
    static const OperatorInfo mult;
    static const OperatorInfo divide;
    static const OperatorInfo add;
    static const OperatorInfo sub;
    static const OperatorInfo lt;
    static const OperatorInfo lte;
    static const OperatorInfo gt;
    static const OperatorInfo gte;
    static const OperatorInfo eq;
    static const OperatorInfo neq;
    static const OperatorInfo land;
    static const OperatorInfo lor;

    static const std::unordered_map<std::string, OperatorInfo> fromStr;

    static const std::unordered_set<std::string> relOps;

    Type enumVal;
    std::string str;
    U32 precedence;
    bool unary;
};

namespace ConfigUtil
{
    extern const std::unordered_set<std::string> reserved;

    void setError(ConfigErrorInfo* const kConfigErr,
                  const Token& kTokErr,
                  const std::string kText,
                  const std::string kSubtext);

    bool checkEof(const TokenIterator& kIt,
                  const Token& kTokLast,
                  const std::string kErrText,
                  ConfigErrorInfo* const kConfigErr);
}

#endif
