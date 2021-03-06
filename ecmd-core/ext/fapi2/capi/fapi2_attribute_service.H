/* IBM_PROLOG_BEGIN_TAG                                                   */
/* 
 * Copyright 2017 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* IBM_PROLOG_END_TAG                                                     */
///
/// @file fapi2_attribute_service.H
///
/// @brief Defines the FAPI_ATTR_GET and FAPI_ATTR_SET macros that a user
/// calls to get/set attributes and a check function that the macros use to
/// verify correct usage
///

#ifndef FAPI2ATTRIBUTESERVICE_H_
#define FAPI2ATTRIBUTESERVICE_H_
#include <stdint.h>
#include <attribute_ids.H>
#include <return_code.H>
#include <target.H>
#include <target_types.H>
#include <plat_attribute_service.H>

/// @brief Macros called by user to get/set attributes for FAPI2 targets
///
/// Code must have a reference to a FAPI2 Target and an attribute ID (from
/// XML file):
///   fapi2::ReturnCode l_rc;
///   fapi2::Target<target type>& l_target = ????;
///          Ex: Target<TARGET_TYPE_PROC_CHIP>& l_target = ????;
///
/// To get a copy of an integer attribute and set the attribute
///   uint64_t l_val = 0;
///   l_rc = FAPI_ATTR_GET(<ID>, l_target, l_val);
///   l_rc = FAPI_ATTR_SET(<ID>, l_target, l_val);
///
/// To get a copy of an integer array attribute and set the attribute
///   uint32_t l_pVal[4] = {0};
///   l_rc = FAPI_ATTR_GET(<ID>, l_target, l_pVal);
///   l_rc = FAPI_ATTR_SET(<ID>, l_target, l_pVal);
///
/// A priveleged attribute is one that a HWP should not generally access,
/// examples include ATTR_NAME and ATTR_EC, where usage can lead to a non
/// data-driven design. A privileged attribute can be accessed with
/// FAPI_ATTR_GET_PRIVILEGED and FAPI_ATTR_SET_PRIVILEGED
///
/// The non-PRIVILEGED macros first call a template function (compiler will
/// optimize out) that will cause a compile failure if the attribute is
/// privileged, they then call a PRIVILEGED macro to get/set the attribute
///
/// The PRIVILEGED macros call a template function (compiler will optimize out)
/// that will cause a compile failure if the ID is not valid or VAL is not the
/// correct type.
//

#define FAPI_ATTR_GET(ID, TARGET, VAL) \
    (fapi2::failIfPrivileged<ID##_Privileged>(), \
     fapi2::Target<ID##_TargetType>(TARGET), \
     fapi2::checkIdType<ID##_Type>(ID, VAL), \
     ID##_GETMACRO(ID, TARGET, VAL))

#define FAPI_ATTR_SET(ID, TARGET, VAL) \
    (fapi2::failIfPrivileged<ID##_Privileged>(), \
     fapi2::Target<ID##_TargetType>(TARGET), \
     fapi2::checkIdType<ID##_Type>(ID, VAL), \
     ID##_SETMACRO(ID, TARGET, VAL))

#define FAPI_ATTR_GET_PRIVILEGED(ID, TARGET, VAL) \
    (fapi2::checkIdType<ID##_Type>(ID, VAL), \
     ID##_GETMACRO(ID, TARGET, VAL))

#define FAPI_ATTR_SET_PRIVILEGED(ID, TARGET, VAL) \
    (fapi2::checkIdType<ID##_Type>(ID, VAL), \
     ID##_SETMACRO(ID, TARGET, VAL))

namespace fapi2
{

///
/// @brief Get raw access to a FAPI2 Attribute.  This is used to dump FAPI2
///        Attrs
///
/// This function gets a copy of an attribute. In the case of an array attribute,
/// The value in the specified index is retrieved. This should be used by the
/// InitFile HWP only, that HWP processes a binary InitFile and therefore needs
/// to read a variable ID of a variable data type. Standard HWPs should use the
/// FAPI2_ATTR_GET macro which automatically checks the type for correct usage.
///
/// If there are ever attributes with more than 4 dimensions then this function
/// will need to be updated.
///
/// @tparam K template parameter, passed in target.
/// @param[in]  i_id          AttributeID
/// @param[in]  i_target      Reference to fapi2::Target (can be NULL for system)
/// @param[out] o_val         Reference to uint64_t where attribute value is set
/// @param[in]  i_arrayIndex1 If array attribute then index1
/// @param[in]  i_arrayIndex2 If at least 2D array attribute then index2
/// @param[in]  i_arrayIndex3 If at least 3D array attribute then index3
/// @param[in]  i_arrayIndex4 If at least 4D array attribute then index4
///
/// @return ReturnCode. Zero if success
///
ReturnCode rawAccessAttr(const AttributeId i_id,
                         const Target<TARGET_TYPE_ALL>& i_target,
                         uint8_t* o_val);

/**
 * @brief Check the ID and TYPE
 *
 * This is called by FAPI code to check at compile time that a FAPI attribute
 * access is using the correct data type and a valid AttributeId
 */
template<typename T> inline void checkIdType(AttributeId, T&) {}

/**
 * @brief Fail if attribute privileged
 *
 * This is called by FAPI code to check at compile time that a standard FAPI
 * attribute access (FAPI_ATTR_GET) is not accessing a privileged attribute
 */
class ErrorAccessingPrivilegedAttribute;
template<const bool PRIVILEGED> void failIfPrivileged()
{
    ErrorAccessingPrivilegedAttribute();
}
template <> inline void failIfPrivileged<false>() {}

}

#endif // FAPI2ATTRIBUTESERVICE_H_
