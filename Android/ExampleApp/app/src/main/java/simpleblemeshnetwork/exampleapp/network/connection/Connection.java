package simpleblemeshnetwork.exampleapp.network.connection;

import simpleblemeshnetwork.exampleapp.network.NetworkIdentifier;

/**
 * @author Sebastian Blomberg
 */
public interface Connection {
    public boolean isActive();
    public NetworkIdentifier getNetworkIdentifier();
}
