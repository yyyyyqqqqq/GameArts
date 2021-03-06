 
/** @file RingSigPrecompiled.cpp
 *  @author shawnhe
 *  @date 20190821
 */

#include "RingSigPrecompiled.h"
#include <group_sig/algorithm/RingSig.h>
#include <libdevcore/Common.h>
#include <libethcore/ABI.h>
#include <string>

using namespace dev;
using namespace dev::blockverifier;
using namespace dev::precompiled;

const char* const RingSig_METHOD_SET_STR = "ringSigVerify(string,string,string)";

RingSigPrecompiled::RingSigPrecompiled()
{
    name2Selector[RingSig_METHOD_SET_STR] = getFuncSelector(RingSig_METHOD_SET_STR);
}

bytes RingSigPrecompiled::call(
    ExecutiveContext::Ptr, bytesConstRef param, Address const&, Address const&)
{
    PRECOMPILED_LOG(TRACE) << LOG_BADGE("RingSigPrecompiled") << LOG_DESC("call")
                           << LOG_KV("param", toHex(param));

    // parse function name
    uint32_t func = getParamFunc(param);
    bytesConstRef data = getParamData(param);

    dev::eth::ContractABI abi;
    bytes out;

    if (func == name2Selector[RingSig_METHOD_SET_STR])
    {
        // ringSigVerify(string,string,string)
        std::string signature, message, paramInfo;
        abi.abiOut(data, signature, message, paramInfo);
        bool result = false;

        try
        {
            result = RingSigApi::LinkableRingSig::ring_verify(signature, message, paramInfo);
        }
        catch (std::string& errorMsg)
        {
            PRECOMPILED_LOG(ERROR) << LOG_BADGE("RingSigPrecompiled") << LOG_DESC(errorMsg)
                                   << LOG_KV("signature", signature) << LOG_KV("message", message)
                                   << LOG_KV("paramInfo", paramInfo);
            getErrorCodeOut(out, VERIFY_RING_SIG_FAILED);
            return out;
        }
        out = abi.abiIn("", result);
    }
    else
    {
        PRECOMPILED_LOG(ERROR) << LOG_BADGE("RingSigPrecompiled")
                               << LOG_DESC("call undefined function") << LOG_KV("func", func);
        getErrorCodeOut(out, CODE_UNKNOW_FUNCTION_CALL);
    }
    return out;
}
