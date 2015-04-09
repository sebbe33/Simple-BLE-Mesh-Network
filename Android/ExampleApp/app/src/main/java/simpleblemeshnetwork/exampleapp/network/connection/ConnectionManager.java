package simpleblemeshnetwork.exampleapp.network.connection;

import simpleblemeshnetwork.exampleapp.network.NetworkIdentifier;

/**
 * @author Sebastian Blomberg
 */
public interface ConnectionManager{
    public static int DEFAULT_SEARCH_PERIOD = 7000;

    /**
     * Searches for available networks in a time period of <code>DEFAULT_SEARCH_PERIOD</code>.
     * Calls the specified callback with a list of available networks.
     * @return available network identifiers
     */
    public void searchForAvailableNetworks(AvailableNetworkCallback callback);

    /**
     * Searches for available networks in a specified time period (in ms).
     * Calls the specified callback with a list of available networks.
     * @return available network identifiers
     */
    public void searchForAvailableNetworks(AvailableNetworkCallback callback, int searchPeriod);

    public void establishConnection(NetworkIdentifier networkId, ConnectionCallback callback);

    public void discontinueConnection(Connection connection);

    public MeshBluetoothService getbtService();
}


