//
// Copyright 2016 Pixar
//
// Licensed under the terms set forth in the LICENSE.txt file available at
// https://openusd.org/license.
//
#include "pxr/usd/usdContrived/multipleApplyAPI_1.h"
#include "pxr/usd/usd/schemaBase.h"

#include "pxr/usd/sdf/primSpec.h"

#include "pxr/usd/usd/pyConversions.h"
#include "pxr/base/tf/pyAnnotatedBoolResult.h"
#include "pxr/base/tf/pyContainerConversions.h"
#include "pxr/base/tf/pyResultConversions.h"
#include "pxr/base/tf/pyUtils.h"
#include "pxr/base/tf/wrapTypeHelpers.h"

#include "pxr/external/boost/python.hpp"

#include <string>

PXR_NAMESPACE_USING_DIRECTIVE

using namespace pxr_boost::python;

namespace {

#define WRAP_CUSTOM                                                     \
    template <class Cls> static void _CustomWrapCode(Cls &_class)

// fwd decl.
WRAP_CUSTOM;

        
static UsdAttribute
_CreateTestAttrOneAttr(UsdContrivedMultipleApplyAPI_1 &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateTestAttrOneAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Int), writeSparsely);
}
        
static UsdAttribute
_CreateTestAttrTwoAttr(UsdContrivedMultipleApplyAPI_1 &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateTestAttrTwoAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Double), writeSparsely);
}

static bool _WrapIsMultipleApplyAPI_1Path(const SdfPath &path) {
    TfToken collectionName;
    return UsdContrivedMultipleApplyAPI_1::IsMultipleApplyAPI_1Path(
        path, &collectionName);
}

static std::string
_Repr(const UsdContrivedMultipleApplyAPI_1 &self)
{
    std::string primRepr = TfPyRepr(self.GetPrim());
    std::string instanceName = TfPyRepr(self.GetName());
    return TfStringPrintf(
        "UsdContrived.MultipleApplyAPI_1(%s, '%s')",
        primRepr.c_str(), instanceName.c_str());
}

struct UsdContrivedMultipleApplyAPI_1_CanApplyResult : 
    public TfPyAnnotatedBoolResult<std::string>
{
    UsdContrivedMultipleApplyAPI_1_CanApplyResult(bool val, std::string const &msg) :
        TfPyAnnotatedBoolResult<std::string>(val, msg) {}
};

static UsdContrivedMultipleApplyAPI_1_CanApplyResult
_WrapCanApply(const UsdPrim& prim, const TfToken& name)
{
    std::string whyNot;
    bool result = UsdContrivedMultipleApplyAPI_1::CanApply(prim, name, &whyNot);
    return UsdContrivedMultipleApplyAPI_1_CanApplyResult(result, whyNot);
}

} // anonymous namespace

void wrapUsdContrivedMultipleApplyAPI_1()
{
    typedef UsdContrivedMultipleApplyAPI_1 This;

    UsdContrivedMultipleApplyAPI_1_CanApplyResult::Wrap<UsdContrivedMultipleApplyAPI_1_CanApplyResult>(
        "_CanApplyResult", "whyNot");

    class_<This, bases<UsdAPISchemaBase> >
        cls("MultipleApplyAPI_1");

    cls
        .def(init<UsdPrim, TfToken>((arg("prim"), arg("name"))))
        .def(init<UsdSchemaBase const&, TfToken>((arg("schemaObj"), arg("name"))))
        .def(TfTypePythonClass())

        .def("Get",
            (UsdContrivedMultipleApplyAPI_1(*)(const UsdStagePtr &stage, 
                                       const SdfPath &path))
               &This::Get,
            (arg("stage"), arg("path")))
        .def("Get",
            (UsdContrivedMultipleApplyAPI_1(*)(const UsdPrim &prim,
                                       const TfToken &name))
               &This::Get,
            (arg("prim"), arg("name")))
        .staticmethod("Get")

        .def("GetAll",
            (std::vector<UsdContrivedMultipleApplyAPI_1>(*)(const UsdPrim &prim))
                &This::GetAll,
            arg("prim"),
            return_value_policy<TfPySequenceToList>())
        .staticmethod("GetAll")

        .def("CanApply", &_WrapCanApply, (arg("prim"), arg("name")))
        .staticmethod("CanApply")

        .def("Apply", &This::Apply, (arg("prim"), arg("name")))
        .staticmethod("Apply")

        .def("GetSchemaAttributeNames",
             (const TfTokenVector &(*)(bool))&This::GetSchemaAttributeNames,
             arg("includeInherited")=true,
             return_value_policy<TfPySequenceToList>())
        .def("GetSchemaAttributeNames",
             (TfTokenVector(*)(bool, const TfToken &))
                &This::GetSchemaAttributeNames,
             arg("includeInherited"),
             arg("instanceName"),
             return_value_policy<TfPySequenceToList>())
        .staticmethod("GetSchemaAttributeNames")

        .def("_GetStaticTfType", (TfType const &(*)()) TfType::Find<This>,
             return_value_policy<return_by_value>())
        .staticmethod("_GetStaticTfType")

        .def(!self)

        
        .def("GetTestAttrOneAttr",
             &This::GetTestAttrOneAttr)
        .def("CreateTestAttrOneAttr",
             &_CreateTestAttrOneAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetTestAttrTwoAttr",
             &This::GetTestAttrTwoAttr)
        .def("CreateTestAttrTwoAttr",
             &_CreateTestAttrTwoAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))

        .def("IsMultipleApplyAPI_1Path", _WrapIsMultipleApplyAPI_1Path)
            .staticmethod("IsMultipleApplyAPI_1Path")
        .def("__repr__", ::_Repr)
    ;

    _CustomWrapCode(cls);
}

// ===================================================================== //
// Feel free to add custom code below this line, it will be preserved by 
// the code generator.  The entry point for your custom code should look
// minimally like the following:
//
// WRAP_CUSTOM {
//     _class
//         .def("MyCustomMethod", ...)
//     ;
// }
//
// Of course any other ancillary or support code may be provided.
// 
// Just remember to wrap code in the appropriate delimiters:
// 'namespace {', '}'.
//
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--

namespace {

WRAP_CUSTOM {
}

}
