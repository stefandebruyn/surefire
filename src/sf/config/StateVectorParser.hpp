#ifndef SF_STATE_VECTOR_PARSER_HPP
#define SF_STATE_VECTOR_PARSER_HPP

#include <istream>

#include "sf/config/ErrorInfo.hpp"
#include "sf/config/StlTypes.hpp"
#include "sf/config/TokenIterator.hpp"
#include "sf/core/StateVector.hpp"

class StateVectorParse final
{
public:

    struct ElementParse final
    {
        Token tokType;
        Token tokName;
    };

    struct RegionParse final
    {
        Token tokName;
        String plainName;
        Vec<StateVectorParse::ElementParse> elems;
    };

    Vec<StateVectorParse::RegionParse> regions;

private:

    friend class StateVectorParser;

    StateVectorParse(Vec<StateVectorParse::RegionParse>& kRegions);
};

class StateVectorParser final
{
public:

    static const Vec<String> ALL_REGIONS;

    static Result parse(const Vec<Token>& kToks,
                        Ref<const StateVectorParse>& kParse,
                        ErrorInfo* const kErr,
                        const Vec<String> kRegions = ALL_REGIONS);

    StateVectorParser() = delete;

private:

    static Result parseRegion(TokenIterator& kIt,
                              StateVectorParse::RegionParse& kRegion,
                              ErrorInfo* const kErr);
};

#endif
