
#include "PeerWhitelist.h"
#include <sstream>
using namespace std;
using namespace dev;

PeerWhitelist::PeerWhitelist(std::vector<std::string> _strList, bool _enable) : m_enable(_enable)
{
    for (auto str : _strList)
    {
        m_whitelist.insert(NodeID(str));
    }
}

bool PeerWhitelist::has(NodeID _peer) const
{
    if (!m_enable)
    {
        // If not enable, all peer is in whitelist
        return true;
    }

    auto itr = m_whitelist.find(_peer);
    return itr != m_whitelist.end();
}

bool PeerWhitelist::has(const std::string& _peer) const
{
    return has(NodeID(_peer));
}

std::string PeerWhitelist::dump(bool _isAbridged)
{
    stringstream ret;
    ret << LOG_KV("enable", m_enable) << LOG_KV("size", m_whitelist.size()) << ",list[";
    for (auto nodeID : m_whitelist)
    {
        if (_isAbridged)
        {
            ret << nodeID.abridged();
        }
        else
        {
            ret << nodeID;
        }
        ret << ",";  // It's ok to tail with ",]"
    }
    ret << "]";

    return ret.str();
}

bool PeerWhitelist::isNodeIDOk(NodeID _nodeID)
{
    return NodeID() != _nodeID;
}

bool PeerWhitelist::isNodeIDOk(const string& _nodeID)
{
    try
    {
        NodeID nodeID = NodeID(_nodeID);
        return isNodeIDOk(nodeID);
    }
    catch (...)
    {
        return false;
    }

    return false;
}