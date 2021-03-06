

#pragma once

#include "Common.h"
#include "SecureInitializer.h"
#include <libp2p/P2PInterface.h>
#include <libp2p/Service.h>

using namespace dev::p2p;
namespace dev
{
namespace initializer
{
class P2PInitializer : public std::enable_shared_from_this<P2PInitializer>
{
public:
    typedef std::shared_ptr<P2PInitializer> Ptr;

    ~P2PInitializer() { stop(); }

    void stop()
    {
        if (m_p2pService)
        {
            m_p2pService->stop();
        }
    }

    void initConfig(boost::property_tree::ptree const& _pt);

    std::shared_ptr<Service> p2pService() { return m_p2pService; }

    void setSSLContext(std::shared_ptr<bas::context> _SSLContext) { m_SSLContext = _SSLContext; }

    void setKeyPair(KeyPair const& _keyPair) { m_keyPair = _keyPair; }

    void resetWhitelist(const std::string& _configFile);

private:
    PeerWhitelist::Ptr parseWhitelistFromPropertyTree(boost::property_tree::ptree const& _pt);

private:
    std::shared_ptr<Service> m_p2pService;
    std::shared_ptr<bas::context> m_SSLContext;
    KeyPair m_keyPair;
};

}  // namespace initializer

}  // namespace dev
