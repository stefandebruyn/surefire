// #include "sfa/sup/StateVectorCompiler.hpp"

// /////////////////////////////////// Private ////////////////////////////////////

// namespace StateVectorCompiler
// {
//     Result allocateElement(const ElementParse& kElem,
//                            StateVector::ElementConfig& kElemInfo,
//                            char*& kBumpPtr);
// }

// Result StateVectorCompiler::allocateElement(
//     const ElementParse& kElem,
//     StateVector::ElementConfig& kElemInfo,
//     char*& kBumpPtr)
// {
//     // Allocate a copy of the element name for the element config representing
//     // this element.
//     char* nameCpy = new char[kElem.tokName.str.size() + 1];
//     std::strcpy(nameCpy, kElem.tokName.str.c_str());
//     kElemInfo.name = nameCpy;

//     if (kElem.tokType.str == "I8")
//     {
//         I8& backing = *reinterpret_cast<I8*>(kBumpPtr);
//         kElemInfo.elem = static_cast<IElement*>(new Element<I8>(backing));
//         kBumpPtr += sizeof(backing);
//     }
//     else if (kElem.tokType.str == "I16")
//     {
//         I16& backing = *reinterpret_cast<I16*>(kBumpPtr);
//         kElemInfo.elem = static_cast<IElement*>(new Element<I16>(backing));
//         kBumpPtr += sizeof(backing);
//     }
//     else if (kElem.tokType.str == "I32")
//     {
//         I32& backing = *reinterpret_cast<I32*>(kBumpPtr);
//         kElemInfo.elem = static_cast<IElement*>(new Element<I32>(backing));
//         kBumpPtr += sizeof(backing);
//     }
//     else if (kElem.tokType.str == "I64")
//     {
//         I64& backing = *reinterpret_cast<I64*>(kBumpPtr);
//         kElemInfo.elem = static_cast<IElement*>(new Element<I64>(backing));
//         kBumpPtr += sizeof(backing);
//     }
//     else if (kElem.tokType.str == "U8")
//     {
//         U8& backing = *reinterpret_cast<U8*>(kBumpPtr);
//         kElemInfo.elem = static_cast<IElement*>(new Element<U8>(backing));
//         kBumpPtr += sizeof(backing);
//     }
//     else if (kElem.tokType.str == "U16")
//     {
//         U16& backing = *reinterpret_cast<U16*>(kBumpPtr);
//         kElemInfo.elem = static_cast<IElement*>(new Element<U16>(backing));
//         kBumpPtr += sizeof(backing);
//     }
//     else if (kElem.tokType.str == "U32")
//     {
//         U32& backing = *reinterpret_cast<U32*>(kBumpPtr);
//         kElemInfo.elem = static_cast<IElement*>(new Element<U32>(backing));
//         kBumpPtr += sizeof(backing);
//     }
//     else if (kElem.tokType.str == "U64")
//     {
//         U64& backing = *reinterpret_cast<U64*>(kBumpPtr);
//         kElemInfo.elem = static_cast<IElement*>(new Element<U64>(backing));
//         kBumpPtr += sizeof(backing);
//     }
//     else if (kElem.tokType.str == "F32")
//     {
//         F32& backing = *reinterpret_cast<F32*>(kBumpPtr);
//         kElemInfo.elem = static_cast<IElement*>(new Element<F32>(backing));
//         kBumpPtr += sizeof(backing);
//     }
//     else if (kElem.tokType.str == "F64")
//     {
//         F64& backing = *reinterpret_cast<F64*>(kBumpPtr);
//         kElemInfo.elem = static_cast<IElement*>(new Element<F64>(backing));
//         kBumpPtr += sizeof(backing);
//     }
//     else if (kElem.tokType.str == "bool")
//     {
//         bool& backing = *reinterpret_cast<bool*>(kBumpPtr);
//         kElemInfo.elem = static_cast<IElement*>(new Element<bool>(backing));
//         kBumpPtr += sizeof(backing);
//     }
//     else
//     {
//         delete[] kElemInfo.name;
//         return E_SVP_ELEM_TYPE;
//     }

//     return SUCCESS;
// }

// /////////////////////////////////// Public /////////////////////////////////////

// StateVectorCompiler::Assembly::Assembly(
//         const StateVector::Config kSvConfig,
//         const char* const kSvBacking,
//         const StateVectorParser::Parse& kParse) :
//     mSvConfig(kSvConfig), mSvBacking(kSvBacking), mParse(kParse)
// {
// }

// StateVectorCompiler::Assembly::~Assembly()
// {
//     // Delete name string and object for each element.
//     for (U32 i = 0; mSvConfig.elems[i].name != nullptr; ++i)
//     {
//         delete[] mSvConfig.elems[i].name;
//         delete mSvConfig.elems[i].elem;
//     }

//     // Delete name string and object for each region.
//     for (U32 i = 0; mSvConfig.regions[i].name != nullptr; ++i)
//     {
//         delete[] mSvConfig.regions[i].name;
//         delete mSvConfig.regions[i].region;
//     }

//     // Delete element config array.
//     delete[] mSvConfig.elems;

//     // Delete region config array.
//     delete[] mSvConfig.regions;

//     // Delete state vector backing storage.
//     delete[] mSvBacking;
// }

// const StateVector::Config& StateVectorCompiler::Assembly::getConfig() const
// {
//     return mSvConfig;
// }

// const StateVectorParser::Parse& StateVectorCompiler::Assembly::getParse() const
// {
//     return mParse;
// }

// Result StateVectorCompiler::compile(const StateVectorParser::Parse& kParse,
//                                     std::shared_ptr<Assembly>& kAsm,
//                                     ConfigErrorInfo* kConfigErr)
// {
//     // Count the number of elements, regions, and bytes in the state vector.
//     U32 elemCnt = 0;
//     const U32 regionCnt = kParse.regions.size();
//     U32 svSizeBytes = 0;
//     for (const RegionParse& region : kParse.regions)
//     {
//         // Check that region contains at least 1 element.
//         if (region.elems.size() == 0)
//         {
//             if (kConfigErr != nullptr)
//             {
//                 kConfigErr->lineNum = region.tokName.lineNum;
//                 kConfigErr->colNum = region.tokName.colNum;
//                 kConfigErr->text = gErrText;
//                 kConfigErr->subtext = "region is empty";
//             }
//             return E_SVP_RGN_EMPTY;
//         }

//         elemCnt += region.elems.size();
//         for (const ElementParse& elem : region.elems)
//         {
//             svSizeBytes += elem.sizeBytes;
//         }
//     }

//     // Allocate array for element configs.
//     StateVector::ElementConfig* elemConfigs =
//         new StateVector::ElementConfig[elemCnt + 1];
//     elemConfigs[elemCnt] = {nullptr, nullptr}; // Null terminator

//     // Allocate array for region configs.
//     StateVector::RegionConfig* regionConfigs =
//         new StateVector::RegionConfig[regionCnt + 1];
//     regionConfigs[regionCnt] = {nullptr, nullptr}; // Null terminator

//     // Allocate backing storage for state vector and zero it out.
//     char* svBacking = new char[svSizeBytes];
//     std::memset(svBacking, 0, svSizeBytes);

//     // Allocate element and region objects and put them into the arrays we just
//     // allocated.
//     char* bumpPtr = svBacking;
//     U32 elemIdx = 0;
//     for (U32 regionIdx = 0; regionIdx < kParse.regions.size(); ++regionIdx)
//     {
//         const RegionParse& regionParse = kParse.regions[regionIdx];

//         // Save a copy of the bump pointer, which right now points to the start
//         // of the region.
//         char* const regionPtr = bumpPtr;

//         // Allocate elements and populate element config array.
//         for (const ElementParse& elemParse : regionParse.elems)
//         {
//             Result res = allocateElement(
//                 elemParse, elemConfigs[elemIdx], bumpPtr);
//             if (res != SUCCESS)
//             {
//                 // Clean up allocations since aborting parse.
//                 delete[] elemConfigs;
//                 delete[] regionConfigs;
//                 delete[] svBacking;
//                 return res;
//             }
//             ++elemIdx;
//         }

//         // Allocate a copy of the region name for the region config object
//         // representing this region.
//         char* regionNameCpy = new char[regionParse.plainName.size() + 1];
//         std::strcpy(regionNameCpy, regionParse.plainName.c_str());
//         regionConfigs[regionIdx].name = regionNameCpy;

//         // Compute the size of the region. Since the `allocateElement` calls
//         // above will have bumped the bump pointer to the end of the region,
//         // we compute the region size as the difference between the bump pointer
//         // and the region pointer we saved at the top of the loop.
//         const U64 regionSizeBytes =
//             (reinterpret_cast<U64>(bumpPtr) - reinterpret_cast<U64>(regionPtr));

//         // Allocate region and put into config array.
//         Region* region = new Region(regionPtr, regionSizeBytes);
//         regionConfigs[regionIdx] = {regionNameCpy, region};
//     }

//     // Create state vector config and wrap it in an assembly, which will handle
//     // deallocation of all the memory we just allocated.
//     StateVector::Config svConfig = {elemConfigs, regionConfigs};
//     kAsm.reset(new Assembly(svConfig, svBacking, kParse));

//     return SUCCESS;
// }
