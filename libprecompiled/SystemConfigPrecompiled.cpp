
#include "SystemConfigPrecompiled.h"

#include "libprecompiled/EntriesPrecompiled.h"
#include "libprecompiled/TableFactoryPrecompiled.h"
#include <libethcore/ABI.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

using namespace dev;
using namespace dev::blockverifier;
using namespace dev::storage;
using namespace dev::precompiled;

const char* const SYSCONFIG_METHOD_SET_STR = "setValueByKey(string,string)";

SystemConfigPrecompiled::SystemConfigPrecompiled()
{
    name2Selector[SYSCONFIG_METHOD_SET_STR] = getFuncSelector(SYSCONFIG_METHOD_SET_STR);
}

bytes SystemConfigPrecompiled::call(
    ExecutiveContext::Ptr context, bytesConstRef param, Address const& origin, Address const&)
{
    PRECOMPILED_LOG(TRACE) << LOG_BADGE("SystemConfigPrecompiled") << LOG_DESC("call")
                           << LOG_KV("param", toHex(param));

    // parse function name
    uint32_t func = getParamFunc(param);
    bytesConstRef data = getParamData(param);

    dev::eth::ContractABI abi;
    bytes out;
    int count = 0;
    int result = 0;
    if (func == name2Selector[SYSCONFIG_METHOD_SET_STR])
    {
        // setValueByKey(string,string)
        std::string configKey, configValue;
        abi.abiOut(data, configKey, configValue);
        // Uniform lowercase configKey
        boost::to_lower(configKey);
        PRECOMPILED_LOG(DEBUG) << LOG_BADGE("SystemConfigPrecompiled")
                               << LOG_DESC("setValueByKey func") << LOG_KV("configKey", configKey)
                               << LOG_KV("configValue", configValue);

        if (!checkValueValid(configKey, configValue))
        {
            PRECOMPILED_LOG(DEBUG)
                << LOG_BADGE("SystemConfigPrecompiled") << LOG_DESC("set invalid value")
                << LOG_KV("configKey", configKey) << LOG_KV("configValue", configValue);
            getErrorCodeOut(out, CODE_INVALID_CONFIGURATION_VALUES);
            return out;
        }

        storage::Table::Ptr table = openTable(context, SYS_CONFIG);

        auto condition = table->newCondition();
        auto entries = table->select(configKey, condition);
        auto entry = table->newEntry();
        entry->setField(SYSTEM_CONFIG_KEY, configKey);
        entry->setField(SYSTEM_CONFIG_VALUE, configValue);
        entry->setField(SYSTEM_CONFIG_ENABLENUM,
            boost::lexical_cast<std::string>(context->blockInfo().number + 1));

        if (entries->size() == 0u)
        {
            count = table->insert(configKey, entry, std::make_shared<AccessOptions>(origin));
            if (count == storage::CODE_NO_AUTHORIZED)
            {
                PRECOMPILED_LOG(DEBUG)
                    << LOG_BADGE("SystemConfigPrecompiled") << LOG_DESC("permission denied");
                result = storage::CODE_NO_AUTHORIZED;
            }
            else
            {
                PRECOMPILED_LOG(DEBUG) << LOG_BADGE("SystemConfigPrecompiled")
                                       << LOG_DESC("setValueByKey successfully");
                result = count;
            }
        }
        else
        {
            count =
                table->update(configKey, entry, condition, std::make_shared<AccessOptions>(origin));
            if (count == storage::CODE_NO_AUTHORIZED)
            {
                PRECOMPILED_LOG(DEBUG)
                    << LOG_BADGE("SystemConfigPrecompiled") << LOG_DESC("permission denied");
                result = storage::CODE_NO_AUTHORIZED;
            }
            else
            {
                PRECOMPILED_LOG(DEBUG) << LOG_BADGE("SystemConfigPrecompiled")
                                       << LOG_DESC("update value by key successfully");
                result = count;
            }
        }
    }
    else
    {
        PRECOMPILED_LOG(ERROR) << LOG_BADGE("SystemConfigPrecompiled")
                               << LOG_DESC("call undefined function") << LOG_KV("func", func);
    }
    getErrorCodeOut(out, result);
    return out;
}

bool SystemConfigPrecompiled::checkValueValid(std::string const& key, std::string const& value)
{
    if (SYSTEM_KEY_TX_COUNT_LIMIT == key)
    {
        try
        {
            return (boost::lexical_cast<int64_t>(value) >= TX_COUNT_LIMIT_MIN);
        }
        catch (boost::bad_lexical_cast& e)
        {
            PRECOMPILED_LOG(ERROR) << LOG_BADGE(e.what());
            return false;
        }
    }
    else if (SYSTEM_KEY_TX_GAS_LIMIT == key)
    {
        try
        {
            return (boost::lexical_cast<int64_t>(value) >= TX_GAS_LIMIT_MIN);
        }
        catch (boost::bad_lexical_cast& e)
        {
            PRECOMPILED_LOG(ERROR) << LOG_BADGE(e.what());
            return false;
        }
    }
    else if (SYSTEM_KEY_RPBFT_EPOCH_SEALER_NUM == key)
    {
        try
        {
            return (boost::lexical_cast<int64_t>(value) >= RPBFT_EPOCH_SEALER_NUM_MIN);
        }
        catch (boost::bad_lexical_cast& e)
        {
            PRECOMPILED_LOG(ERROR)
                << LOG_DESC("checkValueValid failed") << LOG_KV("errInfo", e.what());
            return false;
        }
    }
    else if (SYSTEM_KEY_RPBFT_EPOCH_BLOCK_NUM == key)
    {
        try
        {
            return (boost::lexical_cast<int64_t>(value) >= RPBFT_EPOCH_BLOCK_NUM_MIN);
        }
        catch (boost::bad_lexical_cast& e)
        {
            PRECOMPILED_LOG(ERROR)
                << LOG_DESC("checkValueValid failed") << LOG_KV("errInfo", e.what());
            return false;
        }
    }
    // only can insert tx_count_limit and tx_gas_limit, rpbft_epoch_sealer_num,
    // rpbft_epoch_block_num as system config
    return false;
}
