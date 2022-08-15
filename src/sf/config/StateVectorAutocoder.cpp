////////////////////////////////////////////////////////////////////////////////
///                             S U R E F I R E
///                             ---------------
/// This file is part of Surefire, a C++ framework for building flight software
/// applications. Surefire is open-source under the Apache License 2.0 - a copy
/// of the license may be obtained at www.apache.org/licenses/LICENSE-2.0.
///
/// Copyright (c) 2022 the Surefire authors. All rights reserved.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "sf/config/Autocode.hpp"
#include "sf/config/StateVectorAutocoder.hpp"
#include "sf/core/Assert.hpp"

Result StateVectorAutocoder::code(std::ostream& kOs,
                                  const String kName,
                                  const Ref<const StateVectorAssembly> kSvAsm)
{
    // Check that parse is non-null.
    if (kSvAsm == nullptr)
    {
        return E_SVA_NULL;
    }

    // Get state vector config from assembly.
    const StateVector::Config& svConfig = kSvAsm->config();
    SF_ASSERT(svConfig.elems != nullptr);
    SF_ASSERT(svConfig.regions != nullptr);

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
    // between adjacent members as required by the state vector. Since this
    // struct is static, all state vector elements will initially be 0.
    a("// State vector backing");
    a("#pragma pack(push, 1)");
    a("static struct");
    a("{");
    a.increaseIndent();

    // Region containing current element.
    const StateVector::RegionConfig* region = svConfig.regions;

    // Whether the autocode is currently in the middle of a region struct
    // definition.
    bool inRegionStruct = false;

    // Vectors which will collect definitions for element and region objects
    // while we autocode the backing storage. These definitions will be inserted
    // into the autocode after the backing storage is defined.
    Vec<String> elemDefs;
    Vec<String> regionDefs;

    // Loop through elements. The backing for each element will be in a region
    // struct nested within the backing struct.
    for (const StateVector::ElementConfig* elem = svConfig.elems;
         elem->name != nullptr;
         ++elem)
    {
        // If not in a region struct, begin a new region struct definition.
        if (!inRegionStruct)
        {
            a("struct");
            a("{");
            a.increaseIndent();
            inRegionStruct = true;
        }

        // Look up element type info.
        const IElement* const elemObj = elem->elem;
        SF_ASSERT(elemObj != nullptr);
        auto typeInfoIt = TypeInfo::fromEnum.find(elemObj->type());
        SF_ASSERT(typeInfoIt != TypeInfo::fromEnum.end());
        const TypeInfo& elemTypeInfo = (*typeInfoIt).second;

        // Define struct member for element.
        a("%% %%;", elemTypeInfo.name, elem->name);

        // Create element object definitions for insertion into autocode later.
        elemDefs.push_back(
            Autocode::format("static Element<%%> elem%%(backing.%%.%%);",
                             elemTypeInfo.name,
                             elem->name,
                             region->name,
                             elem->name));

        // If the end address of the element is equal to the end address of the
        // region, end the region struct definition.
        const U8* const elemEnd =
            (static_cast<const U8*>(elemObj->addr()) + elemObj->size());
        const Region* const regionObj = region->region;
        const U8* const regionEnd =
            (static_cast<const U8*>(regionObj->addr()) + regionObj->size());
        if (elemEnd == regionEnd)
        {
            // Save region object definition for insertion into autocode later.
            regionDefs.push_back(
                Autocode::format("static Region region%%(&backing.%%, sizeof(backing.%%));",
                                 region->name, region->name, region->name));

            // Close region struct definition.
            a.decreaseIndent();
            a("} %%;", region->name);
            ++region;
            inRegionStruct = false;
        }
    }

    a.decreaseIndent();
    a("} backing;");
    a("#pragma pack(pop)");
    a();

    // Define element objects.
    a("// Elements");
    for (const String& elemDef : elemDefs)
    {
        a(elemDef);
    }

    a();

    // Define region objects.
    a("// Regions");
    for (const String& regionDef : regionDefs)
    {
        a(regionDef);
    }

    a();

    // Define element config array.
    a("// Element configs");
    a("static StateVector::ElementConfig elemConfigs[] =");
    a("{");
    a.increaseIndent();

    for (const StateVector::ElementConfig* elem = svConfig.elems;
         elem->name != nullptr;
         ++elem)
    {
        a("{\"%%\", &elem%%},", elem->name, elem->name);
    }

    a("{nullptr, nullptr}"); // Null terminator
    a.decreaseIndent();
    a("};");
    a();

    // Define region config array.
    a("// Region configs");
    a("static StateVector::RegionConfig regionConfigs[] =");
    a("{");
    a.increaseIndent();

    for (const StateVector::RegionConfig* region = svConfig.regions;
         region->name != nullptr;
         ++region)
    {
        a("{\"%%\", &region%%},", region->name, region->name);
    }

    a("{nullptr, nullptr}"); // Null terminator
    a.decreaseIndent();
    a("};");
    a();

    // Define state vector config and return to caller.
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
