package simpleblemeshnetwork.exampleapp.network;

import simpleblemeshnetwork.exampleapp.network.NetworkIdentifier;

/**
 * @author Sebastian Blomberg
 */
public interface Connection {
    public boolean isActive();
    public NetworkIdentifier getNetworkIdentifier();
}
