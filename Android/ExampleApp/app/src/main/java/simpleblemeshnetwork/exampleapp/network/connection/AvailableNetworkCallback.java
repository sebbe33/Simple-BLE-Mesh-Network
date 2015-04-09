package simpleblemeshnetwork.exampleapp.network.connection;


import java.util.List;

import simpleblemeshnetwork.exampleapp.network.NetworkIdentifier;

/**
 * @author Sebastian Blomberg
 */
public interface AvailableNetworkCallback {
    public void onNetworkFound(NetworkIdentifier result);
    public void onSearchDone(List<? extends NetworkIdentifier> results);
}
