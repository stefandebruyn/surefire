// #ifndef SF_STATE_SCRIPT_COMPILER_HPP
// #define SF_STATE_SCRIPT_COMPILER_HPP

// #include "sf/config/StateMachineCompiler.hpp"
// #include "sf/config/StateScriptParser.hpp"

// namespace StateScriptCompiler
// {
//     struct Input final
//     {
//         Ref<IExprNode<bool>> guard;
//         Ref<IAction*> action;
//     };

//     struct Assertion final
//     {
//         Ref<IExprNode<bool>> guard;
//         Ref<IExprNode<bool>> assertion;
//     };

//     struct Section final
//     {
//         String name;
//         Vec<Input> inputs;
//         Vec<Assertion> assertions;
//     };

//     class Assembly final
//     {
//     public:

//         Assembly(const Vec<const StateScriptCompiler::Section>& kSections,
//                  const Ref<const StateMachineCompiler::Assembly> kSmAsm);

//         bool run(ErrorInfo* const kErr);

//     private:

//         const Vec<StateScriptCompiler::Section> mSections;

//         const Ref<const StateMachineCompiler::Assembly> kSmAsm;
//     };

//     Result compile(const StateScriptParser::Parse& kParse,
//                    const Ref<const StateMachineCompiler::Assembly> kSmAsm,
//                    Ref<const StateScriptCompiler::Assembly>& kAsm,
//                    ErrorInfo* const kErr);
// };

// #endif
