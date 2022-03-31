#include "sf/config/Autocode.hpp"
#include "sf/config/StateVectorAssembly.hpp"
#include "sf/config/StateVectorAutocoder.hpp"
#include "sf/core/Assert.hpp"

Result StateVectorAutocoder::code(std::ostream& kOs,
                                  const String kName,
                                  const Ref<const StateVectorParse> kParse,
                                  ErrorInfo* const kErr)
{
    // Check that parse is non-null.
    if (kParse == nullptr)
    {
        SF_ASSERT(false);
    }

    // Validate parse by attempting to compile a state vector with it.
    Ref<const StateVectorAssembly> svAsm;
    const Result res = StateVectorAssembly::compile(kParse, svAsm, kErr);
    if (res != SUCCESS)
    {
        return res;
    }

    Autocode a(kOs);

    // Add preamble.
    a("///");
    a("/// THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT MANUALLY EDIT.");
    a("///");
    a();

    // Begin define guard.
    a("#ifndef %%_HPP", kName);
    a("#define %%_HPP", kName);
    a();

    // Add includes.
    a("#include \"sf/core/StateVector.hpp\"");
    a();

    // Begin namespace.
    a("namespace %%", kName);
    a("{");
    a();

    // Add function docstring.
    a("///");
    a("/// @brief Initializes a state vector from the autocoded config.");
    a("///");
    a("/// @note The config is static. This function should only be called once.");
    a("///");
    a("/// @param[out] kSv  State vector to initialize.");
    a("///");
    a("/// @retval SUCCESS  Successfully initialized state vector.");
    a("/// @retval [other]  Initialization failed.");
    a("///");

    // Add function signature.
    a("static Result getConfig(StateVector::Config& kSvConfig)");
    a("{");
    a.increaseIndent();

    // Define backing storage struct. Use the `pack` pragma to remove padding
    // between adjacent members as required by the state vector.
    a("#pragma pack(push, 1)");
    a("static struct");
    a("{");
    a.increaseIndent();

    // Define regions as structs nested within the backing storage struct.
    for (const StateVectorParse::RegionParse& region : kParse->regions)
    {
        a("struct");
        a("{");
        a.increaseIndent();
        
        // Define elements in region.
        for (const StateVectorParse::ElementParse& elem : region.elems)
        {
            SF_SAFE_ASSERT(elem.tokType.typeInfo != nullptr);
            a("%% %%;", elem.tokType.typeInfo->name, elem.tokName.str);
        }

        a.decreaseIndent();
        a("} %%;", region.plainName);
    }

    a.decreaseIndent();
    a("} backing;");
    a("#pragma pack(pop)");
    a();

    // Define element objects.
    for (const StateVectorParse::RegionParse& region : kParse->regions)
    {
        for (const StateVectorParse::ElementParse& elem : region.elems)
        {
            SF_SAFE_ASSERT(elem.tokType.typeInfo != nullptr);
            a("static Element<%%> elem%%(backing.%%.%%);",
              elem.tokType.typeInfo->name,
              elem.tokName.str,
              region.plainName,
              elem.tokName.str);
        }
    }

    a();

    // Define region objects.
    for (const StateVectorParse::RegionParse& region : kParse->regions)
    {
        a("static Region region%%(&backing.%%, sizeof(backing.%%));",
          region.plainName, region.plainName, region.plainName);
    }

    a();

    // Define element config array.
    a("static StateVector::ElementConfig elemConfigs[] =");
    a("{");
    a.increaseIndent();

    for (const StateVectorParse::RegionParse& region : kParse->regions)
    {
        for (const StateVectorParse::ElementParse& elem : region.elems)
        {
            a("{\"%%\", &elem%%},", elem.tokName.str, elem.tokName.str);
        }
    }

    a("{nullptr, nullptr}"); // Null terminator
    a.decreaseIndent();
    a("};");
    a();

    // Define region config array.
    a("static StateVector::RegionConfig regionConfigs[] =");
    a("{");
    a.increaseIndent();

    for (const StateVectorParse::RegionParse& region : kParse->regions)
    {
        a("{\"%%\", &region%%},", region.plainName, region.plainName);
    }

    a("{nullptr, nullptr}"); // Null terminator
    a.decreaseIndent();
    a("};");
    a();

    // Return state vector config to caller.
    a("kSvConfig = {elemConfigs, regionConfigs};");
    a();

    // Add return statement.
    a("return SUCCESS;");

    // Close function definition.
    a.decreaseIndent();
    a("}");
    a();

    // End namespace.
    a("} // namespace %%", kName);
    a();

    // End define guard.
    a("#endif");

    return SUCCESS;
}
